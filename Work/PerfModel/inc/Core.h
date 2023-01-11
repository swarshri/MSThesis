#include<stdio.h>
#include<string>
#include<vector>
#include<map>
#include<bitset>

#include<Config.h>
#include<DRAM.h>
#include<Fetch.h>
#include<Dispatch.h>

class Core {
    public:
        bool halted = false;
        Core(string, string, Config *);

        void connect(DRAM *, DRAM *);
        void step();
        
    private:
        string id;
        
        bitset<32> RefCountReg;
        map<char, bitset<64>> CountReg;
        map<char, bitset<64>> OccFirstReg;
        map<char, bitset<64>> OccLastReg;

        DRAM * OCMEM;

        FetchUnit * FU;
        DispatchUnit * DU;
};