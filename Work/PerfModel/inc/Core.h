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
#include<Load.h>
#include<Store.h>

class Core {
    public:
        bool halted = false;
        Core(string, string, Config *);

        void connect(DRAM<bitset<32>, bitset<64>> *, map<char, DRAM<bitset<32>, bitset<32>>*>, DRAM<bitset<32>, bitset<64>> *);
        void step();
        
    private:
        string id;

        DRAM<bitset<32>, bitset<32>> * OCMEM;

        FetchUnit * FU;
        DispatchUnit * DU;

        ReserveUnit * RUA;
        ComputeUnit * CUA;
        LoadUnit * LUA;

        ReserveUnit * RUC;
        ComputeUnit * CUC;
        LoadUnit * LUC;

        ReserveUnit * RUG;
        ComputeUnit * CUG;
        LoadUnit * LUG;
        
        ReserveUnit * RUT;
        ComputeUnit * CUT;
        LoadUnit * LUT;

        StoreUnit * SU;
};