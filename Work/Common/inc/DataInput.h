#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <bitset>
#include <vector>
#include <map>
#include <chrono>

#include "../../External/BWA/bwa.h"
#include "../../External/BWA/bwt.h"

using namespace std;
using namespace chrono;

#ifndef DIP_H
#define DIP_H

class Reference {
    public:
        Reference(string);
        Reference(string, string);
        Reference(vector<string>, bool);
        void restore_bwt_sa();
        bwt_t * getBWT();
        uint64_t get_seqLen();
        uint64_t get_occLen();
        uint64_t getCount(char);
        uint64_t getCount(int);
        uint64_t getOcc(char, uint64_t);
        uint64_t getOcc(int, uint64_t);
        uint64_t getSA(uint64_t);
        void construct_fmdi();
        void print_fmdi();
        vector<vector<uint64_t>> find_seed(string);

    private:
        map<int, char> base_int = {{'A', 0}, {'C', 1}, {'G', 2}, {'T', 3}};
        char * faPath;
        char * bwtPath;
        char * saPath;
        string ref = "";
        string bwt = "";
        bwt_t * bwaBwt;
        uint64_t seqLen; // bwa's bwt->seq_len + 1.
        uint64_t occLen; // seqLen + 1 = bwa's bwt->seq_len + 2.
};

class Reads {
    public:
        Reads(string);
        void make_seeds(int);
        void print();
        string get_seed(uint64_t);
        bitset<64> get_seed_bitset(uint64_t);
        uint64_t get_seedsCount();
        uint64_t get_readsCount();

    private:
        string fqPath;
        vector<string> reads;
        vector<string> seeds;
        int maxSeedLen;
};

#endif