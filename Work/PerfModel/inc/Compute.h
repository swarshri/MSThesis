#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <Fetch.h>

#ifndef COMP_H
#define COMP_H

class ComputeStage {
    public:
        ComputeStage(SysConfig *, string, string, bitset<32>);

        void connect(ReserveStage *, FetchStage *);
        bool isHalted();
        void step();

    private:
        string base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        FetchStage * coreFU;
        bitset<32> CountReg;
};

#endif