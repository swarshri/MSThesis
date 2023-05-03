#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <DRAMWrapper.h>
#include <Cache.h>
#include <PerfRecorder.h>

#ifndef LOAD_H
#define LOAD_H

class LoadStage {
    public:
        LoadStage(SysConfig *, string, string, PerformanceRecorder *);

        void connectRU(ReserveStage *);
        void connectDRAM(OccMemory *);
        void step();

        bool isHalted();
    
    private:
        string base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        OccMemory * OCCMEM;
        PerformanceRecorder * perf;

        vector<LQEntry> LQEntryInProgress;
};

#endif