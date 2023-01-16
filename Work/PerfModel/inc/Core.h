#include<stdio.h>
#include<string>
#include<vector>
#include<map>
#include<bitset>

#include<Config.h>
#include<DRAM.h>
#include<Fetch.h>
#include<Dispatch.h>
#include<Reserve.h>
#include<Compute.h>

class Core {
    public:
        bool halted = false;
        Core(string, string, Config *);

        void connect(DRAM<bitset<32>, bitset<64>> *, DRAM<bitset<32>, bitset<64>> *);
        void step();
        
    private:
        string id;
        
        bitset<32> RefCountReg;
        map<char, bitset<64>> CountReg;
        map<char, bitset<64>> OccFirstReg;
        map<char, bitset<64>> OccLastReg;

        DRAM<bitset<32>, bitset<64>> * OCMEM;

        FetchUnit * FU;
        DispatchUnit * DU;

        ReserveUnit * RUA;
        ComputeUnit * CUA;

        ReserveUnit * RUC;
        ComputeUnit * CUC;

        ReserveUnit * RUG;
        ComputeUnit * CUG;
        
        ReserveUnit * RUT;
        ComputeUnit * CUT;
};