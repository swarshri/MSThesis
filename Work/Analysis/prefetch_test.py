# -*- coding: utf-8 -*-
"""
Created on Tue Oct 18 23:07:21 2022

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
        self._occ = []
        
        self._bwttransform(r_str)
        self._render_count(r_str)
        self._get_occurrences()
        
        self._occ_access_locations_row_wise = []
        self._occ_access_locations_col_wise = []
        
    def _bwttransform(self, r_str):
        bwt_matrix = [r_str]
        print("Suffix array:", r_str)
        while(r_str[-1] != '$'):
            r_str = r_str[-1] + r_str[:-1]
            bwt_matrix.append(r_str)
            print("Suffix array:", r_str)
            
        bwt_matrix = sorted(bwt_matrix)
        print("BWT matrix:", bwt_matrix)
        self._suffix_array = [self._length-1-x.find('$') for x in bwt_matrix]
        self._bwt = ''.join([x[-1] for x in bwt_matrix])
        
    def _render_count(self, r_str):
        alpha_count = [r_str.count(x) for x in self.base_chars]
        self._count = {x: sum(alpha_count[:self.base_chars.index(x)]) for x in self.base_chars[1:]}
        
    def _get_occurrences(self):
        for i in range(self._length+1):
            dict_t = {}
            for char in self.base_chars[1:]:
                dict_t[char] = self._bwt[:i].count(char)
            self._occ.append(dict_t)
    
    def count(self, char):
        return self._count[char]
    
    def occ(self, char, idx):
        occ_access_location_row_wise = (self.base_chars.index(char)-1) * (self._length+1) + idx
        self._occ_access_locations_row_wise.append(occ_access_location_row_wise)
        #print("new access row:", occ_access_location_row_wise, "   length:", len(self._occ_access_locations_col_wise))
        occ_access_location_col_wise = (len(self.base_chars)-1)*idx + self.base_chars.index(char)-1
        self._occ_access_locations_col_wise.append(occ_access_location_col_wise)
        #print("new access col:", occ_access_location_col_wise, "   length:", len(self._occ_access_locations_col_wise))
        return self._occ[idx][char]
    
    def occ_by_idx(self, idx):
        return self._occ[idx]
    
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

class ReadAlignment(object):
    
    def __init__(self, ref_fmi):
        self._fmi = ref_fmi
        self._pos = {}
        self._fetched_occ_vals = []
        self._fetched_occ_low_vals = []
        self._fetched_occ_high_vals = []
        
    def prefetch(self):
        it_count = 0
        low = 0; high = self._fmi.length
        next_idcs = [low, high]
        next_lows = [low]
        next_highs = [high]
        
        while(True):
            self._fetched_occ_vals.append([self._fmi.occ_by_idx(idx) for idx in next_idcs])
            next_idcs = []
            for idx_occ in self._fetched_occ_vals[-1]:
                next_idcs.extend([self._fmi.count(char) + val for char, val in idx_occ.items()])
            next_idcs = set(next_idcs)
            print("next idcs:", next_idcs)
            '''
            self._fetched_occ_low_vals.append([self._fmi.occ_by_idx(low) for low in next_lows])
            next_lows = []
            for idx_occ in self._fetched_occ_low_vals[-1]:
                next_lows.extend([self._fmi.count(char) + low for char, low in idx_occ.items()])
            next_lows = set(next_lows)
            print("next lows:", next_lows)
            
            self._fetched_occ_high_vals.append([self._fmi.occ_by_idx(high) for low in next_highs])
            next_highs = []
            for idx_occ in self._fetched_occ_high_vals[-1]:
                next_highs.extend([self._fmi.count(char) + high for char, high in idx_occ.items()])
            next_highs = set(next_highs)
            print("next highs:", next_highs)'''
            
    def run_fmibackwardsearch(self, query):
        it_count = 0
        low = 0; high = self._fmi.length
        lows = [low]
        highs = [high]
        m = query.length
        for i in range(m-1, -1, -1):
            char = query.char_from_idx(i)
            low = self._fmi.count(char) + self._fmi.occ(char, low)
            high = self._fmi.count(char) + self._fmi.occ(char, high)
            lows.append(low)
            highs.append(high)
            #print("iteration:", it_count, "low:", low, "high:", high)
            #print("")
            
            #if (low >= high):
                #return
            
            it_count += 1
        
        print("lows:", lows)
        print("highs:", highs)
        
        pos_list = []
        for i in range(low, high):
            pos_list.append(self._fmi.sa_idx(i))
        
        self._pos[query] = sorted(pos_list)
        
if __name__ == "__main__":
    
    fasts_file = r"C:\Swarnashri\Masters\TandonCourses\Thesis\Benchmarks\genomicsbench\InputData\fmi\sample\SRR7733443_1520_sample.fasts"
    ref_genome_seq = ""
    with open(fasts_file, "r") as fast:
        ref_genome_seq = ''.join(fast.readlines()).replace('\n', '')
    
    #ref_genome_seq = "AGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTT"
    #ref_genome_seq = "AGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTACGGTTAGGGTTAGGGTTAGGGTTAGGGTT"
    #ref_genome_seq = "AGGGTTACCGTTAGGGTTAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTAAGGGTTACCGTTAGGGTTA"
    #ref_genome_seq = "CACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCACACTAAGCTTAAGCA"
    
    fmi2 = FMI('RS02', ref_genome_seq)
    ra2 = ReadAlignment(fmi2)
    #query2 = Query("TCACCCCAA")
    #q_str = "AGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTTAGGGTT"
    q_str = "AGGGTTAGGGTT"
    query2 = Query(q_str)
    
    print("Reference Length:", len(ref_genome_seq))
    print("Query length:", query2.length)
    
    ra2.run_fmibackwardsearch(query2)
    ra2.prefetch()