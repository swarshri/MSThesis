#include <iostream>

#include <Config.h>
#include <Queue.h>
#include <Dispatch.h>

class StoreUnit {
    public:
        StoreUnit(Config *);

        void step();
        void connectDU(DispatchUnit *);
        void connectDRAM(DRAM<bitset<32>, bitset<64>> *);

    private:
        int cycle_count;
        bool halted;

        DispatchUnit * coreDU;
        DRAM<bitset<32>, bitset<64>> * SIMEM;
};