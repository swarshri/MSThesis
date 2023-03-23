#include<stdio.h>
#include<string>
#include<vector>
#include<map>
#include<bitset>

#include<Config.h>
#include<DRAMWrapper.h>
#include<Fetch.h>
#include<Dispatch.h>
#include<Reserve.h>
#include<Compute.h>
#include<Load.h>
#include<Store.h>

class Core {
    public:
        bool halted = false;
        Core(string, string, SysConfig *);

        void connect(DRAMW<32, 64> *, map<char, DRAMW<32, 32>*>, DRAMW<32, 64> *);
        void step();
        
    private:
        string id;

        DRAMW<32, 32> * OCMEM;

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