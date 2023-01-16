#include <iostream>

#include <Config.h>
#include <Reserve.h>
#include <Fetch.h>

#ifndef COMP_H
#define COMP_H

class ComputeUnit {
    public:
        ComputeUnit(Config*);

        void connect(ReserveUnit *, FetchUnit *);
        void step();

    private:
        int cycle_count;
        bool halted;

        ReserveUnit * coreRU;
        FetchUnit * coreFU;
};

#endif