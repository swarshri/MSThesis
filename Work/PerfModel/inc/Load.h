#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <DRAMWrapper.h>
#include <Cache.h>
#include <PerfRecorder.h>

class LoadStage {
    public:
        LoadStage(SysConfig *, string, string, PerformanceRecorder *);

        void connectRU(ReserveStage *);
        void connectDRAM(DRAMW<32, 32> *);
        void step();

        bool isHalted();
    
    private:
        string base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        DRAMW<32, 32> * OCCMEM;
        PerformanceRecorder * perf;

        vector<LQEntry> LQEntryInProgress;
};