#include <iostream>

#include <Config.h>
#include <Queue.h>
#include <Dispatch.h>

class StoreStage {
    public:
        StoreStage(SysConfig *);

        void step();
        bool isHalted();
        void connectDU(DispatchStage *);
        void connectDRAM(DRAMW<32, 64> *);

    private:
        int cycle_count;
        bool halted;

        DispatchStage * coreDU;
        DRAMW<32, 64> * SIMEM;
};