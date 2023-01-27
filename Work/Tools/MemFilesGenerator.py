import os
import shutil
import argparse

base_chars = ['$', 'A', 'C', 'G', 'T']
base_mem_dict = {'A': '000',
                 'C': '001',
                 'G': '010',
                 'T': '011',
                 'N': '100',
                 'E': '111'}

def mem_format(val):
    val_bin = bin(val).replace('0b', '')
    return '0'*(32-len(val_bin)) + val_bin + '\n'

class FMI(object):
    def __init__(self, uid, ref_str):
        self._uid = uid
        
        r_str =  '$' + ref_str
        self._length = len(r_str)
        self._bwt = ""
        self._suffix_array = []
        self._count = {'A': 1, 'C': 0, 'G': 0, 'T': 0}
        self._occ = {'A': [], 'C': [], 'G': [], 'T': []}
        
        self._bwttransform(r_str)
        self._render_count(r_str)
        self._get_occurrences()
        
        self._occ_access_locations_row_wise = []
        self._occ_access_locations_col_wise = []
        self._occ_char_wise_access_location = {'A': [], 'C': [], 'G': [], 'T': []}
        
    def _bwttransform(self, r_str):
        print("Reference:", r_str)
        bwt_matrix = [r_str]
        while(r_str[-1] != '$'):
            r_str = r_str[-1] + r_str[:-1]
            bwt_matrix.append(r_str)
            
        bwt_matrix = sorted(bwt_matrix)
        self._suffix_array = [self._length-1-x.find('$') for x in bwt_matrix]
        self._bwt = ''.join([x[-1] for x in bwt_matrix])
        print("BWT:", (self._bwt))
        
    def _render_count(self, r_str):
        alpha_count = [r_str.count(x) for x in base_chars]
        self._count = {x: sum(alpha_count[:base_chars.index(x)]) for x in base_chars[1:]}
        #print("Count:", self._count)
        
    def _get_occurrences(self):
        for i in range(self._length+1):
            for char in base_chars[1:]:
                self._occ[char].append(self._bwt[:i].count(char))
                
        for char, occ_list in self._occ.items():
            to_print = [bin(val).replace('0b', '') for val in occ_list]
            to_print = ['0'*(64-len(val)) + val for val in to_print]
            #print("OCC for Char", char, ": ", to_print)
    
    def count(self, char):
        return self._count[char]
    
    def occ(self, char, idx):
        occ_access_location_row_wise = (self.base_chars.index(char)-1) * (self._length+1) + idx
        self._occ_access_locations_row_wise.append(occ_access_location_row_wise)
        #print("new access row:", occ_access_location_row_wise, "   length:", len(self._occ_access_locations_col_wise))
        occ_access_location_col_wise = (len(self.base_chars)-1)*idx + self.base_chars.index(char)-1
        self._occ_access_locations_col_wise.append(occ_access_location_col_wise)
        #print("new access col:", occ_access_location_col_wise, "   length:", len(self._occ_access_locations_col_wise))
        
        for ch in base_chars[1:]:
            if ch == char:
                self._occ_char_wise_access_location[ch].append(idx)
            else:
                self._occ_char_wise_access_location[ch].append(0)
                
        return self._occ[char][idx]
    
    @property
    def length(self):
        return self._length
    
    @property
    def suffix_array(self):
        return self._suffix_array

    @property
    def count(self):
        return self._count

    @property
    def occ(self):
        return self._occ

    def sa_idx(self, pos):
        return self._suffix_array[pos]
    
    @property
    def occ_access_locations_row_wise(self):
        return self._occ_access_locations_row_wise
    
    @property
    def occ_access_locations_col_wise(self):
        return self._occ_access_locations_col_wise
    
    @property
    def occ_char_wise_access_locations(self):
        return self._occ_char_wise_access_location
    
    def occ_char_wise_access_location(self, char):
        return self._occ_char_wise_access_location[char]

class FastFilesParser(object):
    def __init__(self, dirpath, fastafile, fastqfile):
        self._fa_file_name = fastafile
        self._fq_file_name = fastqfile
        self._dir_path = dirpath
        self._fa_file_path = os.path.join(dirpath, fastafile)
        self._fq_file_path = os.path.join(dirpath, fastqfile)
        self._op_dir_path = os.path.join(dirpath, "mem")

    def generateSaiMemFile(self, ref_fmi):
        saimem_file_path = os.path.join(self._op_dir_path, "SaiMEM.mem")
        with open(saimem_file_path, "w") as imem:
            lines = [mem_format(saidx) for saidx in ref_fmi.suffix_array]
            lines[-1] = lines[-1].replace('\n', '')
            imem.writelines(lines)

    def generateOccMemFiles(self, ref_fmi):
        omem_files = {'A' : "OccAMEM.mem",
                      'C' : "OccCMEM.mem",
                      'G' : "OccGMEM.mem",
                      'T' : "OccTMEM.mem"}
        for base, ofile in omem_files.items():
            omem_file_path = os.path.join(self._op_dir_path, ofile)
            with open(omem_file_path, "w") as omem:
                lines = [mem_format(occval) for occval in ref_fmi.occ[base]]
                lines[-1] = lines[-1].replace('\n', '')
                omem.writelines(lines)

    def generateCoreRegFile(self, ref_fmi):
        corereg_file_path = os.path.join(self._op_dir_path, "CoreReg.mem")
        with open(corereg_file_path, "w") as creg:
            lines = [mem_format(ref_fmi.length)]
            for base in base_chars[1:]:
                lines += mem_format(ref_fmi.count[base])
            for base in base_chars[1:]:
                lines += mem_format(ref_fmi.occ[base][-1])
            lines[-1] = lines[-1].replace('\n', '')
            creg.writelines(lines)

    def generateSdMemFile(self, fastq_reads):
        seeds = []
        for read in fastq_reads:
            seeds.extend([read[i:i+20] for i in range(0, len(read), 20)])
        print("Seeds:", len(seeds), seeds)

        sdmem_file = os.path.join(self._op_dir_path, "SdMEM.mem")
        with open(sdmem_file, "w") as sdmem:
            lines = []
            for seed in seeds:
                write_bitstr = "111"
                write_bitstr = write_bitstr + "".join([base_mem_dict[base] for base in seed])
                write_bitstr = '0'*(64-len(write_bitstr)) + write_bitstr + "\n"
                lines.append(write_bitstr)
            lines.append('1'*64 + '\n')
            lines[-1] = lines[-1].replace('\n', '')
            sdmem.writelines(lines)

    def generateSiMemFile(self):
        simem_file = os.path.join(self._op_dir_path, "SiMEM.mem")
        with open(simem_file, "w") as simem:
            lines = ['0'*64 + '\n' for i in range(pow(2, 8))]
            lines[-1] = lines[-1].replace('\n', '')
            simem.writelines(lines)

    def generateMemFiles(self):
        if os.path.exists(self._op_dir_path):
            shutil.rmtree(self._op_dir_path)
        os.mkdir(self._op_dir_path)

        reference_genome = ""
        with open(self._fa_file_path, "r") as fasta:
            reference_name = fasta.readline().split('>')[1].strip()
            reference_genome = ''.join(fasta.readlines()).replace('\n', '').upper()

        if reference_genome != "":
            ref_fmi = FMI(reference_name, reference_genome)
            self.generateSaiMemFile(ref_fmi)
            self.generateOccMemFiles(ref_fmi)
            self.generateCoreRegFile(ref_fmi)
        else:
            print("WARNING!!! Found empty Reference Genome from the fasta file:", self._fa_file_path)
            print("No SaiMEM, OccMEM, and CoreReg file generated.")

        fastq_reads = []
        with open(self._fq_file_path, "r") as fastq:
            sample_name = fastq.readline().split('.')[0].replace('@', '')
            fastq_reads = [line.upper().strip() for line in fastq.readlines() if line[0] not in ['@', '+', '?', '\'', '5']]

        print("Fastq Reads:", len(fastq_reads), len(fastq_reads[-1]), fastq_reads)
        if len(fastq_reads) != 0:
            self.generateSdMemFile(fastq_reads)
        else:
            print("WARNING!!! Found no Read Sequences from the fastq file:", self._fq_file_path)
            print("No SdMEM file generated.")

        self.generateSiMemFile()

if __name__ == "__main__":
    #parse arguments for input file location
    parser = argparse.ArgumentParser(description='Mem Files generator')
    parser.add_argument('--iodir', default="", type=str, help='Path to the folder containing the fastq and fasta input files, and to place the resulting .mem files.')
    args = parser.parse_args()

    iodir = os.path.abspath(args.iodir)
    print("="*40)
    print("IO Directory:", iodir)

    # The iodir should contain exactly one fasta file (Reference Genome file) and exactly one fastq file (Read Sequence file).
    # The fastq file might create an intermediate fasts file that contain seeds to be queried.
    # These seeds are derived from the reads in fastq files.

    fasta_files = [file for file in os.listdir(iodir) if ".fasta" in file]
    if len(fasta_files) == 0:
        print("No Reference Genome (fasta) file found in the given IO directory.")
        exit(-1)
    elif len(fasta_files) > 1:
        print("Multiple Reference Genome (fasta) files (" + str(len(fasta_files)) + ") found in the given IO directory:")
        print(fasta_files)
        exit(-1)

    fastq_files = [file for file in os.listdir(iodir) if ".fastq" in file]
    if len(fastq_files) == 0:
        print("No Read Sequence (fastq) file found in the given IO directory.")
        exit(-1)
    elif len(fastq_files) > 1:
        print("Multiple Read Sequence (fastq) files (" + str(len(fastq_files)) + ") found in the given IO directory:")
        print(fastq_files)
        exit(-1)
 
    fasta_file = fasta_files[0]
    fastq_file = fastq_files[0]
    print("Parsing the Reference Genome (fasta) file:", fasta_file, "and the Read Sequence (fastq) file:", fastq_file)
    ffParser = FastFilesParser(iodir, fasta_file, fastq_file)
    ffParser.generateMemFiles()