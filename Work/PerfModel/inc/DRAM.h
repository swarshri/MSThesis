#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<cmath>

#include<Config.h>

using namespace std;

#ifndef DRAM_H
#define DRAM_H

template <typename AddressType, typename DataType>
class DRAM {
    public:
        string id;
        vector<DataType> lastReadData;
        bool readDone = false;
        bool writeDone = false;

        DRAM(string, Config *, bool);
        void load(string);

        void readAccess(AddressType);
        void writeAccess(AddressType, vector<DataType>);
        void step();

        bool isFree();

        int getChannelWidth();

    private:
        vector<DataType> MEM;

        int addressibility;
        int channelwidth;
        int latencymin;
        int latencymax;

        long int memsize;
        bool readonly;

        AddressType nextReadAddress;
        int readWaitCycles;
        bool readPending;

        AddressType nextWriteAddress;
        vector<DataType> nextWriteData;
        int writeWaitCycles;
        bool writePending;
};

#include <../src/DRAM.cpp>
#endif