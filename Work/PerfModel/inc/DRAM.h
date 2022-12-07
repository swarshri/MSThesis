#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<cmath>

#include<Config.h>

using namespace std;

#ifndef _DRAM_h
#define _DRAM_h

class DRAM {
    public:
        string id;
        vector<bitset<64>> lastReadData;
        bool readDone = false;
        bool writeDone = false;

        DRAM(string, Config *, bool);
        void load(string);

        void readAccess(bitset<32>);
        void writeAccess(bitset<32>, vector<bitset<64>>);
        void step();

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
        vector<bitset<64>> nextWriteData;
        int writeWaitCycles;
        bool writePending;
};

#endif