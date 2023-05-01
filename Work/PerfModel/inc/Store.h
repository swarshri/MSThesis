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
        void connectDRAM(SiMemory *);

    private:
        int cycle_count;
        bool halted;

        DispatchStage * coreDU;
        SiMemory * SIMEM;
};