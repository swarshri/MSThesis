#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<cmath>

#include<Config.h>

using namespace std;

class DRAM {
    public:
        string id;
        bitset<64> ReadData;

        DRAM(string, Config *, bool);
        void load(string);

        void Access(bool, bitset<32>, bitset<64>);
        bool step();

        bool isFree();

    private:
        vector<bitset<64>> MEM;

        int addressibility;
        int channelwidth;
        int latencymin;
        int latencymax;

        long int memsize;
        bool readonly;

        bitset<32> nextReadAddress;
        int readWaitCycles;
        bool readPending;

        bitset<32> nextWriteAddress;
        bitset<64> nextWriteData;
        int writeWaitCycles;
        bool writePending;
};