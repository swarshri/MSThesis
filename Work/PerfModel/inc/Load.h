#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <DRAMWrapper.h>
#include <Cache.h>

class LoadStage {
    public:
        LoadStage(SysConfig *, char, string);

        void connectRU(ReserveStage *);
        void connectDRAM(DRAMW<32, 32> *);
        void step();

        bool isHalted();
    
    private:
        char base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        DRAMW<32, 32> * OCCMEM;

        vector<pair<int, LRSEntry>> LRSEntryInProgress;
};