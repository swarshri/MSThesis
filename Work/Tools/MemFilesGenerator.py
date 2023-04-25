import os
import shutil
import argparse
import time
import random

class Query(object):
    count = -1
    
    def __init__(self, q_str):
        
        self._str = q_str
        self._len = len(q_str)
        Query.count += 1
        self._id = self.count
        
    @property
    def uid(self):
        return self._id
    
    @property
    def length(self):
        return self._len     
    
    @property
    def qstr(self):
        return self._str
    
    def char_from_idx(self, idx):
        return self._str[idx]

class ReadAlignment(object):    
    def __init__(self, ref_fmi):
        self._fmi = ref_fmi
        self._pos = {}
        
    def run_fmibackwardsearch(self, query):
        it_count = 0
        # print("self._fmi.length:", self._fmi.length)
        low = 0; high = self._fmi.length
        m = query.length
        # print ("Query:", query.qstr)
        for i in range(m-1, -1, -1):
            char = query.char_from_idx(i)
            # print("iteration:", it_count, "char:", char, "self._fmi.count(char):",  self._fmi.count(char), "self._fmi.occ(char, low):", self._fmi.occ(char, low), "self._fmi.occ(char, high):", self._fmi.occ(char, high))
            low = self._fmi.count(char) + self._fmi.occ(char, low)
            high = self._fmi.count(char) + self._fmi.occ(char, high)
            # print("iteration:", it_count, "char:", char, "low:", low, "high:", high)
            # print("")
            
            if (low >= high):
                # print("Low greater than or equal to high:", low, high)
                low_bin = bin(low).replace('0b', '')
                low_bin = '0'*(32-len(low_bin)) + low_bin
                high_bin = bin(high).replace('0b', '')
                high_bin = '0'*(32-len(high_bin)) + high_bin
                return [query.qstr + " " + str(low) + "\t" + str(high) + "\t\t" + str([]) + "\n",
                        low_bin + high_bin + "\n"]
            
            it_count += 1
        
        pos_list = []
        for i in range(low, high):
            pos_list.append(self._fmi.sa_idx(i))
        
        # print("Index locations:", pos_list)
        self._pos[query] = sorted(pos_list)
        low_bin = bin(low).replace('0b', '')
        low_bin = '0'*(32-len(low_bin)) + low_bin
        high_bin = bin(high).replace('0b', '')
        high_bin = '0'*(32-len(high_bin)) + high_bin
        return [query.qstr + " " + str(low) + "\t" + str(high) + "\t\t" + str(pos_list) + "\n", low_bin + high_bin + "\n"]

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
        print("Reference:", len(r_str))
        bwt_matrix = [r_str]
        while(r_str[-1] != '$'):
            r_str = r_str[-1] + r_str[:-1]
            bwt_matrix.append(r_str)
            
        bwt_matrix = sorted(bwt_matrix)
        # print("BWT Matrix:")
        # for sr in bwt_matrix:
        #     print(sr)
        self._suffix_array = [self._length-1-x.find('$') for x in bwt_matrix]
        self._bwt = ''.join([x[-1] for x in bwt_matrix])
        print("SA:", [self._suffix_array[i] for i in range(0, len(self._suffix_array), int(self._length/10))])
        # print("BWT:", (self._bwt))
        
    def _render_count(self, r_str):
        alpha_count = [r_str.count(x) for x in base_chars]
        self._count = {x: sum(alpha_count[:base_chars.index(x)]) for x in base_chars[1:]}
        print("Count:", self._count)
        
    def _get_occurrences(self):
        for char in base_chars[1:]:
            self._occ[char] = [self._bwt[:i].count(char) for i in range(self._length+1)]
            print("Occ[", char, "]:", [self._occ[char][i] for i in range(0, len(self._occ[char]), int(self._length/10))])
                
        for char, occ_list in self._occ.items():
            to_print = [bin(val).replace('0b', '') for val in occ_list]
            to_print = ['0'*(64-len(val)) + val for val in to_print]
            #print("OCC for Char", char, ": ", to_print)
    
    def count(self, char):
        return self._count[char]
    
    def occ(self, char, idx = None):
        if idx != None:
            occ_access_location_row_wise = (base_chars.index(char)-1) * (self._length+1) + idx
            self._occ_access_locations_row_wise.append(occ_access_location_row_wise)
            #print("new access row:", occ_access_location_row_wise, "   length:", len(self._occ_access_locations_col_wise))
            occ_access_location_col_wise = (len(base_chars)-1)*idx + base_chars.index(char)-1
            self._occ_access_locations_col_wise.append(occ_access_location_col_wise)
            #print("new access col:", occ_access_location_col_wise, "   length:", len(self._occ_access_locations_col_wise))

            for ch in base_chars[1:]:
                if ch == char:
                    self._occ_char_wise_access_location[ch].append(idx)
                else:
                    self._occ_char_wise_access_location[ch].append(0)

            return self._occ[char][idx]
        else:
            return self._occ[char]
    
    @property
    def length(self):
        return self._length
    
    @property
    def suffix_array(self):
        return self._suffix_array

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
        self._fa_file_path = fastafile
        self._fq_file_path = fastqfile
        self._op_dir_path = os.path.join(dirpath, "mem")

        self._ref_name = ""
        self._ref_genome_seq = ""
        self._ref_fmi = None
        self._fastq_reads = []
        self._seeds = []

    def parse(self):        
        reference_genome = ""
        with open(self._fa_file_path, "r") as fasta:
            reference_name = fasta.readline().split('>')[1].strip()
            reference_genome = ''.join(fasta.readlines()).replace('\n', '').upper()
            reference_genome_comp = self.get_genome_complement(reference_genome)
            reference_genome_pair = reference_genome + reference_genome_comp
            # print("ref:", reference_genome)
            # print("ref_pair:", reference_genome + ' ' + reference_genome_comp) # TAACCCTAACGTTAGGGTTA for TAACCCTAAC
            self._ref_genome_seq = reference_genome_pair
            # print ("referencegenome:", self._ref_genome_seq)

        fa_pair_file_path = self._fa_file_path.replace(".fasta", ".pair")
        with open(fa_pair_file_path, "w") as fastap:
            wrlns = [self._ref_genome_seq[i:i+70] + "\n" for i in range(0, len(self._ref_genome_seq), 70)]
            fastap.writelines(wrlns)

        start_time = time.time()
        if self._ref_genome_seq != "":
            self._ref_fmi = FMI(reference_name, self._ref_genome_seq)

        time_elapsed = time.time() - start_time
        print("Time taken to get FMI for the given reference genome is:", time_elapsed, "seconds.")

        with open(self._fq_file_path, "r") as fastq:
            sample_name = fastq.readline().split('.')[0].replace('@', '')
            self._fastq_reads = [line.upper().strip() for line in fastq.readlines() if line[0] not in ['@', '+', '?', '\'', '5', '#']]
        print("Fastq Reads:", len(self._fastq_reads), len(self._fastq_reads[-1]))
            
        for read in self._fastq_reads:
            self._seeds.extend([read[i:i+20] for i in range(0, len(read), 20) if 'N' not in read[i:i+20]])
        print("Seeds:", len(self._seeds))
        
        # for read in self._fastq_reads:
        #     if 'N' not in read:
        #         randi = [random.randint(0, len(read)) for i in range(200)]
        #         self._seeds.extend([read[i:i+20] for i in randi])
        # print("Seeds:", len(self._seeds))

    def get_genome_complement(self, ref_genome):
        complement = {'A': 'T',
                      'C': 'G',
                      'G': 'C',
                      'T': 'A',
                      'N': 'N'}
        return ''.join([complement[base] for base in ref_genome[::-1]])

    def backwardSearch(self):
        readAligner = ReadAlignment(self._ref_fmi)
        line_op = ""
        line_bop = ""
        for seed in self._seeds:
            query = Query(seed)
            lines = readAligner.run_fmibackwardsearch(query)
            line_op += lines[0]
            line_bop += lines[1]
        
        fop_path = os.path.join(self._op_dir_path, "FuncOP.out")
        with open(fop_path, "w") as fop:
            line_op = "Seed \t\t\t\t Low\tHigh\tIndex Positions\n" + line_op
            fop.writelines(line_op)
        
        fbop_path = os.path.join(self._op_dir_path, "ExpSiMEM.out")
        with open(fbop_path, "w") as fbop:
            fbop.writelines(line_bop)

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
                lines = [mem_format(occval) for occval in ref_fmi.occ(base)]
                lines[-1] = lines[-1].replace('\n', '')
                omem.writelines(lines)

    def generateCoreRegFile(self, ref_fmi):
        corereg_file_path = os.path.join(self._op_dir_path, "CoreReg.mem")
        with open(corereg_file_path, "w") as creg:
            lines = [mem_format(ref_fmi.length)]
            for base in base_chars[1:]:
                lines += mem_format(ref_fmi.count(base))
            for base in base_chars[1:]:
                lines += mem_format(ref_fmi.occ(base)[-1])
            lines[-1] = lines[-1].replace('\n', '')
            creg.writelines(lines)

    def generateSdMemFile(self, fastq_reads):
        sdmem_file = os.path.join(self._op_dir_path, "SdMEM.mem")
        with open(sdmem_file, "w") as sdmem:
            lines = []
            print("-------------------------------------------")
            for seed in self._seeds:
                write_bitstr = "111"
                write_bitstr = write_bitstr + "".join([base_mem_dict[base] for base in seed])
                write_bitstr = '0'*(64-len(write_bitstr)) + write_bitstr + "\n"
                lines.append(write_bitstr)
                # print(seed, ":", write_bitstr)
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

        if self._ref_fmi != None:
            self.generateSaiMemFile(self._ref_fmi)
            self.generateOccMemFiles(self._ref_fmi)
            self.generateCoreRegFile(self._ref_fmi)
        else:
            print("WARNING!!! Found empty Reference Genome from the fasta file:", self._fa_file_path)
            print("No SaiMEM, OccMEM, and CoreReg file generated.")

        if len(self._fastq_reads) != 0:
            self.generateSdMemFile(self._fastq_reads)
        else:
            print("WARNING!!! Found no Read Sequences from the fastq file:", self._fq_file_path)
            print("No SdMEM file generated.")

        # self.generateSiMemFile()

if __name__ == "__main__":
    #parse arguments for input file location
    parser = argparse.ArgumentParser(description='Mem Files generator')
    parser.add_argument('--fasta', default="", type=str, help='Path to the input fasta file.')
    parser.add_argument('--fastq', default="", type=str, help='Path to the input fastq file.')
    parser.add_argument('--opdir', default="", type=str, help='Path to place the resulting .mem files.')
    args = parser.parse_args()

    fasta_file = os.path.abspath(args.fasta)
    fastq_file = os.path.abspath(args.fastq)
    op_dir = os.path.abspath(args.opdir)
    print("="*40)
    print("FASTA Path:", fasta_file)
    print("FASTQ Path:", fastq_file)
    print("OP Directory:", op_dir)

    print("Parsing the Reference Genome (fasta) file:", fasta_file, "and the Read Sequence (fastq) file:", fastq_file)
    ffParser = FastFilesParser(op_dir, fasta_file, fastq_file)
    ffParser.parse()
    ffParser.generateMemFiles()

    start_time = time.time()
    ffParser.backwardSearch()
    time_elapsed = time.time() - start_time
    print("Time taken for backward search on all seeds:", time_elapsed)