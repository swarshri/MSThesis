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

        FetchStage * FU;
        DispatchStage * DU;

        ReserveStage * RUA;
        ComputeStage * CUA;
        LoadStage * LUA;

        ReserveStage * RUC;
        ComputeStage * CUC;
        LoadStage * LUC;

        ReserveStage * RUG;
        ComputeStage * CUG;
        LoadStage * LUG;
        
        ReserveStage * RUT;
        ComputeStage * CUT;
        LoadStage * LUT;

        StoreStage * SU;

        bool allStagesHalted();
};