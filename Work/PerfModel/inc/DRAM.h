#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<cmath>

#include<Config.h>

using namespace std;

template <class dataType, class addressType>
class DRAM {
    public:
        string id;
        dataType ReadData;

        DRAM(string, Config *, bool) {};
        void load(string) {};

        void Access(bool, addressType, dataType *);
        bool step();

        bool isFree();

    private:
        vector<dataType> MEM;

        uint16_t addressibility;
        uint16_t channelwidth;
        uint16_t latencymin;
        uint16_t latencymax;

        uint64_t memsize;
        bool readonly;

        addressType nextReadAddress;
        uint16_t readWaitCycles;
        bool readPending;

        addressType nextWriteAddress;
        dataType nextWriteData;
        uint16_t writeWaitCycles;
        bool writePending;
};