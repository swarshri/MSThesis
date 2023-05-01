#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <Fetch.h>

#ifndef COMP_H
#define COMP_H

class ComputeStage {
    public:
        ComputeStage(SysConfig *, string, string, uint64_t);

        void connect(ReserveStage *, FetchStage *);
        bool isHalted();
        void step();

    private:
        string base;
        int cycle_count;
        bool halted;

        ReserveStage * coreRU;
        FetchStage * coreFU;
        uint64_t CountReg;
};

#endif