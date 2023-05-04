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

#ifndef CORE_H
#define CORE_H

struct PLPerfMetrics {
    float utilization;
    uint64_t numLowOccLookups;
    uint64_t numLowCacheHits;
    uint64_t numLowCacheMisses;
    uint64_t numHighOccLookups;
    uint64_t numHighCacheHits;
    uint64_t numHighCacheMisses;
    uint64_t numOccLookups;
    uint64_t numCacheHits;
    uint64_t numCacheMisses;
    float cacheHitRate;
    float cacheMissRate;
    uint64_t numCyclesWithNewJobs;
    uint64_t numCyclesWithNoNewJobs;

    friend std::ostream& operator <<(std::ostream& os, PLPerfMetrics const& e) {
        string delim = ", ";
        return os << e.numLowCacheHits << delim
                  << e.numLowCacheMisses << delim
                  << e.numLowOccLookups << delim
                  << e.numHighCacheHits << delim
                  << e.numHighCacheMisses << delim
                  << e.numHighOccLookups << delim
                  << e.numCacheHits << delim
                  << e.numCacheMisses << delim
                  << e.numOccLookups << delim
                  << e.cacheHitRate << delim
                  << e.cacheMissRate << delim
                  << e.numCyclesWithNewJobs << delim
                  << e.numCyclesWithNoNewJobs;
    }

    // string titles(char base) {
    //     return
    // }
};

struct PerfMetrics {
    uint64_t numCycles;
    map<char, PLPerfMetrics> PLMetrics;
    uint64_t totalCacheHits;
    uint64_t totalCacheMisses;
    uint64_t totalOccLookups;
    float overallHitRate;
    float overallMissRate;

    friend std::ostream& operator <<(std::ostream& os, PerfMetrics const& e) {
        string delim = ", ";
        return os << e.numCycles << delim
                  << e.overallHitRate << delim
                  << e.overallMissRate << delim
                  << e.totalCacheHits << delim
                  << e.totalCacheMisses << delim
                  << e.totalOccLookups << delim
                  << e.PLMetrics.at('A').utilization << delim
                  << e.PLMetrics.at('C').utilization << delim
                  << e.PLMetrics.at('G').utilization << delim
                  << e.PLMetrics.at('T').utilization << delim
                  << e.PLMetrics.at('A') << delim
                  << e.PLMetrics.at('C') << delim
                  << e.PLMetrics.at('G') << delim
                  << e.PLMetrics.at('T');
    }
};

class Core {
    public:
        bool halted = false;
        Core(string, string, SysConfig *, PerformanceRecorder *, Reference *);

        void connect(SeedMemory *, map<char, OccMemory*>, SiMemory *);
        void step();
        string getPerfMetricTitles();
        PerfMetrics * getPerfMetrics();
        uint64_t getCycleCount();
        
    private:
        string bases = "ACGT";

        string id;

        FetchStage * FU;
        DispatchStage * DU;

        map<char, ReserveStage *> RUs;
        map<char, ComputeStage *> CUs;
        map<char, LoadStage *> LUs;
        
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

        PerfMetrics * perfmetrics;

        bool allRUsHalted();
        bool allCUsHalted();
        bool allLUsHalted();
        bool allStagesHalted();
        void gatherPLMetrics();
};

#endif