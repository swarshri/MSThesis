#include<stdio.h>
#include<string>
#include<vector>
#include<map>
#include<bitset>

#include<Config.h>
#include<Fetch.h>
#include<Dispatch.h>
#include<Reserve.h>
#include<Compute.h>
#include<Memory.h>
#include<Load.h>
#include<Store.h>
#include<PerfRecorder.h>
#include"../../Common/inc/DataInput.h"

class Core {
    public:
        bool halted = false;
        Core(string, string, SysConfig *, PerformanceRecorder *, Reference *);

        void connect(SeedMemory *, map<char, OccMemory*>, SiMemory *);
        void step();
        
    private:
        string id;

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

        int cyclecnt;
};