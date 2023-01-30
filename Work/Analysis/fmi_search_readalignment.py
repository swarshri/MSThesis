# -*- coding: utf-8 -*-
"""
Created on Fri Sep 30 11:29:37 2022

@author: cswar
"""

import matplotlib.pyplot as plt

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
        #print("Suffix array:", r_str)
        while(r_str[-1] != '$'):
            r_str = r_str[-1] + r_str[:-1]
            bwt_matrix.append(r_str)
            #print("Suffix array:", r_str)
            
        bwt_matrix = sorted(bwt_matrix)
        #print("BWT matrix:", bwt_matrix)
        self._suffix_array = [self._length-1-x.find('$') for x in bwt_matrix]
        self._bwt = ''.join([x[-1] for x in bwt_matrix])
        sa_to_print = [bin(val).replace('0b', '') for val in self._suffix_array]
        sa_to_print = ['0'*(64-len(val)) + val for val in sa_to_print]
        #print("Suffix Index Array:", sa_to_print)
        
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

class ReadAlignment(object):
    
    def __init__(self, ref_fmi):
        self._fmi = ref_fmi
        self._pos = {}
        
    def run_fmibackwardsearch(self, query):
        it_count = 0
        print("self._fmi.length:", self._fmi.length)
        low = 0; high = self._fmi.length
        m = query.length
        for i in range(m-1, -1, -1):
            char = query.char_from_idx(i)
            print("iteration:", it_count, "char:", char, "low:", low, "high:", high)
            low = self._fmi.count(char) + self._fmi.occ(char, low)
            high = self._fmi.count(char) + self._fmi.occ(char, high)
            print("iteration:", it_count, "char:", char, "low:", low, "high:", high)
            print("")
            
            if (low >= high):
                print("Low greater than or equal to high:", low, high)
                return
            
            it_count += 1
        
        pos_list = []
        for i in range(low, high):
            pos_list.append(self._fmi.sa_idx(i))
        
        print("Index locations:", pos_list)
        self._pos[query] = sorted(pos_list)

if __name__ == "__main__":
    
    fasts_file = r"/home/sc8781/Thesis/git/Work/PerfModel/data/sample/trial_Ref1400Base/GRCh38_1400.fasta"
    ref_genome_seq = ""
    with open(fasts_file, "r") as fast:
        ref_genome_seq = ''.join([line for line in fast.readlines() if '>' not in line]).replace('\n', '')

    print ("RGS:", ref_genome_seq)
    
    #ref_genome_seq = "AGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTT"
    #ref_genome_seq = "AGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTT"
    #ref_genome_seq = "AGGGTTACCGTTAGGGTTAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTA"
    #ref_genome_seq = "CACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCA"
    #ref_genome_seq = "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"
    #ref_genome_seq = "ATATATGACTGCGACTAACCCTACGACTAACCTCCTCAACACCCTTGACTAATCCCTTTCCCAACTACAGACTAAAACCGGACTGCACCGATATATATATAGACTGCACCGGACTGCACCGTATATATAGACTGCACCGGACTGCACCGTATATATATATATATCGCGCGGACTGCACCGGACTGCACCGCGGCGCGCGACTGCACCGGACTGCACCGGCGCGCGGCGACTGCACCGGACTGCACCGGCGCGCGACTGACCGACTGACCGCGCGGCGACTGACCGACTGACCGCGCGCGCGCGGCGCGCGCGCGCGGCGCGCGCGCGCGGACTGACCGACTGACCGCGCG"
    #ref_genome_seq = "GACTAACCCACGTACGTACGTACGTACGTACGTACGTACGTACGTTACGACTAACCTCCTCAACGTACGTACGTACGTACGTACGTACGTACGTACGTACACCCTTGACTAATCCCTTTCCCAACTACAGACTAAATATATGACTGCACCGGACTGCACCGATATATATATAGACTGCACCGGACTGCACCGTATATGACTAACCCTACGACTAACCTCCTCAACACCCTTGACTAATCCCTTTCCCAACTACAGACTAAATATATGACTGCACCGGACTGCACCGATATATATATAGACTGCACCGGACTGCACCGTATATGACTAACCCTACGACTAACCTCCTCAACACCCTTGACTAATCCCTTTCCCAACTACAGACTAAATATATGACTGCACCGGACTGCACCGATATATATATAGACTGCACCGGACTGCACCGTATAT"
    #ref_genome_seq = "GACGACTAACCTTTCAGACTAACCTTCAGAGACTAACCTTTCAGAGGCTACTAACCTTCAGATAACCTTTCAGACGACTAACCTTTCAGACTAACCTTCAGAGACTAACCTTTCAGAGGCTACTAACCTTCAGATAACCTTCAGAAAAAAAAAATTTTTTTTTTTTTTTTTCCCCCCCCCCCGACTAACCTTTCAGACTAACCTTCAGACCCCCCTTTTTACCCCCGACTAACCTTTCAGACTAACCTTCAGAGACTAACCTTTCAGACTAACCTTCAGA"
    #ref_genome_seq = "TCTTATCAAATCTTCGGGACAGATCTTCAGTTCTCATGACCACAAAAGAGGATACTAAAGCTCAGACAGGAGAAGAGACGTGGCCAGCCTGTGTCCCCAGGGCCTATGGTCTTACCACTAGGTTACAGTGTTTCCAGATATCACATGTTGTGAGATTTTTGCTTTAAAATGAACCAAAAAAAAACCAAAGGTGAAAAAGGCATAAGCTATTAAAAAGTGGGAGAAACACTAAGAGAACCTTAAGCATGTAACTAAAAATATTATGGAAATGTTATTGAATACATTAGCAAATTTAGTGCTAGGTTTTCATTGAGGAGTAGGTTATATTACTCATGATGAAGAAAAATGTTCATTTTAAGTATATTAACATAAATACCATCAATATTGTTTATCATGTTTAAATGTTCACTTAAAGCAATTCAGTTAAAATTCTGCATATCATACAATTTTATAGTTTGCTAGTAGGTTACAAGTAAATAGTCACCCAAATAAAAACATCATGTTTTCCACTGGTTGTTGCTCTTTTTTAGGTGAGTATTTGATATATACCAACAGAGAGAGGATAATAACAAATCGCTAATTTCTTTCATCACTATATAAAGGTGGCTTCAGGATAGAATAGTATCAGTGTAATGATGAATTTGAAATCTAACATCAATTCAGTGATGCATCAAGATAAAAGTAGAGACAACAGGGGCACCTTGGTGAGTACTGAACATTTTATTTATTTATTTATTTTGAGATGGAGTTTTGCTCTTTTTGCCCAGGCTACAGTGCAATGGTGCCAACCTCGCCTCACTGCAACCTCTGCCTCCTGGGTTCAAGCGATTCTCCTGCCTTGGCCTCCCGAATAGCTGGGATTACAGACATGCGCCACCACACCCGTCTAATTTTGTATTTTTAGTAGAGACGGGGTTTCTCCATGTTGGTCAGGCTGGTCTCGAACTCCCGACCTAGATATCTGCCTGCCTTGGCCTCCCAAAGTGCTGGGATTACAGGTGTGAGCCACCACGCCCAGATGAATTCCAAATTTAACAAAGCAGACTAAGA"
    #ref_genome_seq = "ACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCAAACCCTAACCCTAACCCCAACCCTACCCCTAACCCCAACCCAAACCCAACCCCTAAGCAAAAACCTCACCCTAACCCCCACCCAAGCCCCCCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCCAACCCTAACCCTAACCCTAACCCTAACCCTAACCCAGACCCGAATCGCACACAGAGGCCTTCCACAAACAGCGACCGTAGCGTTAGGGTTAGGGTTAGGGTTAGGGTTGGGGTAGGGGTAGGGGTAGGGGTTAGGGTGAAGGGTTGGGGATAGGGCTGGGGTTTAGGGACAGGGGTAGGGCTAGGGTGAGGGCTTGGGCTAGGGTTAGGGTTAGGGTTAGGGGTGGGGTTAGGTTAGGGTTAGGGTTAGGGTTAGGGTTGGGGTAGGGGTAGGGGTAGGGGTTAGGGTGAAGGGTTGGGGATAGGGCTGGGGTTTAGGGACAGGGGTAGGGCTAGGGTGAGGGCTTGGGCTAGGGTTAGGGTTAGGGTTAGGGGTGGGGTTAGGGNTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGCTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTGAGGGGTAGGCCTAGGGCGAGGGTTAGGGTCAGGGCTTGGTATAGCGTGAGGGGNTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGCTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTGAGGGGTAGGCCTAGGGCGAGGGTTAGGGTCAGGGCTTGGTATAGCGTGAGGTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCAAACCCTAACCCTAACCCCAACCCTACCCCTAACCCCAACCCAAACCCAACCCCTAAGCAAAAACCTCACCCTAACCCCCACCCAAGCCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCAAACCCTAACCCTAACCCCAACCCTACCCCTAACCCCAACCCAAACCCAACCCCTAAGCAAAAACCTCACCCTAACCCCCACCCAAGCCCCCTAAGCAAAAACCTCACCCTAACCCCCACCCGGGCTAGGGTGAGGGCTTGGGCTAGGGTTAGGGTTAGGGTTAGGGGTGGGGTTAGGTTAGGGTTAGGGTTAGGGTTAGGGTTGGGGTAGGGGTAGGGGTAGGGGTTAGGGTGAAGGGTTGGGGATAGGGCTGGGGTTTAG"
    # ref_genome_seq = ""
    fmi2 = FMI('RS02', ref_genome_seq)
    
    ra2 = ReadAlignment(fmi2)
    #query2 = Query("TCACCCCAA")
    #q_str = "AGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTT"
    #q_str = "TTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACAC"
    #q_str = "ACGTACGTACGTACGTACGTACGTACGTACGTACGT"
    #q_str = "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"
    #q_str = "GACTAACCCTACGACTAACCTCCTCAACACCCTTGACTAATCCCTTTCCCAACTACAGACTAAA"
    #q_str = "GACTAACCTTTCAGACTAACCTTCAGAGACTAACCTTTCAGACTAACCTTCAGA"
    #q_str = "GACTAACCTTTCAGACTAACCTTCAGAGACTAACCTTTCAGAGGCTACTAACCTTCAGA"
    #q_str = "TTGAGGAGTAGGTTATATTACTCATGATGAAGAAAAATGTTCATTTTAAGTATATTAACATAAATACCATCAATATTGTTTATCATGTTTAAATGTTCACTTAAAGCAATTCAGTTAAAATTCTGCATATCATACAATT"
    #q_str = "CCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAA"
    q_str = "TGAGGGCTTGGGCTAGGGTT"
    query = Query(q_str)
    ra2.run_fmibackwardsearch(query)
    
    print("Reference Length:", len(ref_genome_seq))
    print("Query Length:", query.length)
    x = []
    for i in range(int(len(fmi2.occ_access_locations_row_wise)/2)):
        x.extend([i, i])
    plt.scatter(x, fmi2.occ_access_locations_row_wise)
    plt.xlabel("Iteration count")
    plt.ylabel("Memory address being accessed")
    plt.show()
    
    plt.scatter(x, fmi2.occ_access_locations_col_wise)
    plt.xlabel("Iteration count")
    plt.ylabel("Memory address being accessed")
    plt.show()
    
    y = []
    for i in range(int(len(fmi2.occ_char_wise_access_location('A'))/2)):
        y.extend([i, i])
    
    suffix_depth = 5
    for base, acc_locations in fmi2.occ_char_wise_access_locations.items():
        plt.scatter(y, acc_locations)
        for i, xval in enumerate(y):
            if(acc_locations[i] != 0):
                if(len(q_str[-(xval+1):]) < suffix_depth):
                    pt_label = q_str[-(xval+1):]
                else:
                    pt_label = q_str[-(xval+1):-(xval+1)+suffix_depth+1]
                plt.annotate(pt_label, (xval, acc_locations[i]))
        plt.xlabel("Iteration count")
        plt.ylabel("Index in Occ['" + base + "']")
        plt.show()
    
    print("occ char wise access locations:")
    for ch in fmi2.base_chars[1:]:
        fmi_list = [val for val in fmi2.occ_char_wise_access_location(ch) if val != 0]
        print(ch, fmi_list)