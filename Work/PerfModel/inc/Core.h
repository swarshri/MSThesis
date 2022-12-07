#include<stdio.h>
#include<string>
#include<vector>
#include<map>
#include<bitset>

#include<Config.h>
#include<DRAM.h>
#include<Fetch.h>

class Core {
    public:
        Core(string, string, Config *);

        void connect(DRAM *, DRAM *);

    private:
        string id;
        
        bitset<64> RefCountReg;
        map<char, bitset<64>> CountReg;
        map<char, bitset<64>> OccFirstReg;
        map<char, bitset<64>> OccLastReg;

        DRAM * SDMEM;
        DRAM * OCMEM;

        FetchUnit * FU;
};