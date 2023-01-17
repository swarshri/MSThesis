#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <DRAM.h>

class LoadUnit {
    public:
        LoadUnit(Config *);

        void connectRU(ReserveUnit *);
        void connectDRAM(DRAM<bitset<32>, bitset<32>> *);
        void step();

        bool isHalted();
    
    private:
        int cycle_count;
        bool halted;

        ReserveUnit * coreRU;
        DRAM<bitset<32>, bitset<32>> * OCCMEM;

        pair<int, LRSEntry> LRSEntryInProgress;
};