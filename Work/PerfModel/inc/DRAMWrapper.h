#include <vector>
#include <string>

#include "../ext/DRAMsim3/src/memory_system.h"
#include <Config.h>

using namespace dramsim3;
using namespace std;

#ifndef DRAMW_H
#define DRAMW_H

template<typename AddressType, typename DataType>
class DRAMW {
    public:
        string id;
        vector<DataType> lastReadData;
        bool readDone = false;
        bool writeDone = false;

        DRAMW(string, SysConfig *, bool);
        void input(string);
        void output(string);

        void readRequest(AddressType);
        void writeRequest(AddressType, vector<DataType>);
        void ReadCompleteHandler(uint64_t);
        void WriteCompleteHandler(uint64_t);

        void step();

        bool isFree();

        int getChannelWidth();
        void printStats();

    private:
        vector<DataType> MEM;

        int addressibility; // Maybe use this to verify that the chosen DRAM config matches this requirement.
        int channelwidth; // Same as above.
        long int memsize; // Same as above.

        bool readonly;

        AddressType nextReadAddress;
        int readWaitCycles;
        bool readPending;

        AddressType nextWriteAddress;
        vector<DataType> nextWriteData;
        int writeWaitCycles;
        bool writePending;
        
        MemorySystem * MemSystem;
        uint64_t clk_;

        static vector<tuple<uint64_t, uint64_t, bool>> pendingReads;
        static vector<tuple<uint64_t, uint64_t, bool>> pendingWrites;
};

#include <../src/DRAMWrapper.cpp>
#endif