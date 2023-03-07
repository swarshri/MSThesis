#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <DRAM.h>
#include <Cache.h>

class LoadStage {
    public:
        LoadStage(SysConfig *, char, string);

        void connectRU(ReserveStage *);
        void connectDRAM(DRAM<bitset<32>, bitset<32>> *);
        void step();

        bool isHalted();
    
    private:
        char base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        DRAM<bitset<32>, bitset<32>> * OCCMEM;

        pair<int, LRSEntry> LRSEntryInProgress;
};