#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <Fetch.h>

#ifndef COMP_H
#define COMP_H

class ComputeStage {
    public:
        ComputeStage(Config*, char, string, bitset<32>);

        void connect(ReserveStage *, FetchStage *);
        bool isHalted();
        void step();

    private:
        char base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        FetchStage * coreFU;
        bitset<32> CountReg;
};

#endif