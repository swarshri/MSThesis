#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <map>

#include "../../Common/inc/DataInput.h"

using namespace std;

struct SeedResult {
    string seed;
    vector<uint64_t> si_values;
    vector<uint64_t> sa_values;
};

class FMDI {
    public:
        FMDI(Reference *);
        void construct_fmdi();
        SeedResult find_seed(string);
        void print_fmdi();

    private:
        vector<char> bases = {'A', 'C', 'G', 'T'};
        map<int, char> int_base = {{0, 'A'}, {1, 'C'}, {2, 'G'}, {3, 'T'}};
        Reference * ref;
        int seqLen; // bwa's bwt->seq_len + 1.
        int occLen; // seqLen + 1 = bwa's bwt->seq_len + 2.
        map<char, uint64_t> Count; // length is 4 - 1 int64 for each character.
        map<char, vector<uint64_t>> Occ; // length is 4 - vector of length occLen for each character.
        vector<uint64_t> SA; // length is seqLen.
};

class ExactMatchEngine {
    public:
        ExactMatchEngine(FMDI *);
        void find_exact_matches(Reads *);
        void print_seedresults();

    private:
        FMDI * iref;
        vector<SeedResult> seedresults;
};