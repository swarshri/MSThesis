import os
import argparse

class FMI(object):
    base_chars = ['$', 'A', 'C', 'G', 'T']
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
        bwt_matrix = [r_str]
        while(r_str[-1] != '$'):
            r_str = r_str[-1] + r_str[:-1]
            bwt_matrix.append(r_str)
            
        bwt_matrix = sorted(bwt_matrix)
        self._suffix_array = [self._length-1-x.find('$') for x in bwt_matrix]
        self._bwt = ''.join([x[-1] for x in bwt_matrix])
        print("BWT:", (self._bwt))
        
    def _render_count(self, r_str):
        alpha_count = [r_str.count(x) for x in self.base_chars]
        self._count = {x: sum(alpha_count[:self.base_chars.index(x)]) for x in self.base_chars[1:]}
        #print("Count:", self._count)
        
    def _get_occurrences(self):
        for i in range(self._length+1):
            for char in self.base_chars[1:]:
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
        
        for ch in self.base_chars[1:]:
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

def mem_format(val):
    val_bin = bin(val).replace('0b', '')
    return '0'*(64-len(val_bin)) + val_bin + '\n'

if __name__ == "__main__":
     
    #parse arguments for input file location
    parser = argparse.ArgumentParser(description='FastAToMem')
    parser.add_argument('--fasta', default="", type=str, help='Path to the input fasta file.')
    parser.add_argument('--mem', default="", type=str, help='Directory path for the output mem files.')
    args = parser.parse_args()

    fasta_file = os.path.abspath(args.fasta)
    print("Fasta Location:", fasta_file)
    mem_dir = os.path.abspath(args.mem)
    print("Mem Directory Location:", mem_dir)

    imem_file = "\\IMEM.txt"
    omem_files = {'A' : "\\OMEM_A.txt",
                  'C' : "\\OMEM_C.txt",
                  'G' : "\\OMEM_G.txt",
                  'T' : "\\OMEM_T.txt"}

    reference_genome = ""
    with open(fasta_file, "r") as fasta:
        reference_name = fasta.readline().split('>')[1].strip()
        reference_genome = ''.join(fasta.readlines()).replace('\n', '')

    print(reference_genome)

    fmi_reference = FMI(reference_name, reference_genome)

    with open(mem_dir + imem_file, "w") as imem:
        write_lines = mem_format(fmi_reference.length)
        for sa_idx in fmi_reference.suffix_array:
            write_lines += mem_format(sa_idx)
        
        imem.writelines(write_lines)

    for base, file in omem_files.items():
        with open(mem_dir + file, "w") as mem:
            write_lines = mem_format(fmi_reference.count[base])
            for occ_val in fmi_reference.occ[base]:
                write_lines += mem_format(occ_val)
            
            mem.writelines(write_lines)