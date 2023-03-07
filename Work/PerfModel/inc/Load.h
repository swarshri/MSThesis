#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <DRAMWrapper.h>
#include <Cache.h>

class LoadStage {
    public:
        LoadStage(SysConfig *, char, string);

        void connectRU(ReserveStage *);
        void connectDRAM(DRAMW<bitset<32>, bitset<32>> *);
        void step();

        bool isHalted();
    
    private:
        char base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        DRAMW<bitset<32>, bitset<32>> * OCCMEM;

        pair<int, LRSEntry> LRSEntryInProgress;
};