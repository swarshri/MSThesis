#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <map>

#include "../../External/BWA/bwa.h"
#include "../../External/BWA/bwt.h"
// #include "../../External/BWA/bwtindex.c"

using namespace std;

class Reference {
    public:
        Reference(char*, bool);
        void construct_fmdi();
        vector<vector<uint64_t>> find_seed(string);
        void print_fmdi();

    private:
        map<int, char> int_base = {{0, 'A'}, {1, 'C'}, {2, 'G'}, {3, 'T'}};
        char * faPath;
        char * bwtPath;
        string ref = "";
        string bwt = "";
        bwt_t * bwaBwt;
        int seqLen; // bwa's bwt->seq_len + 1.
        int occLen; // seqLen + 1 = bwa's bwt->seq_len + 2.
        map<char, bwtint_t> Count; // length is 4 - 1 int64 for each character.
        map<char, vector<bwtint_t>> Occ; // length is 4 - vector of length occLen for each character.
        vector<bwtint_t> SA; // length is seqLen.
};

struct SeedResult {
    string seed;
    vector<uint64_t> si_values;
    vector<uint64_t> sa_values;
};

class Reads {
    public:
        Reads(string);
        void make_seeds(int);
        void print();
        string get_seed(int);
        uint64_t get_seedsCount();
        void save_result(string, vector<uint64_t>, vector<uint64_t>);
        void print_seedresults();

    private:
        string fqPath;
        vector<string> reads;
        vector<string> seeds;
        vector<SeedResult> seedresults;
        int maxSeedLen;
};