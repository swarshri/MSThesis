#include <iostream>
#include <vector>
#include <string>
#include <bitset>

#include "../ext/DRAMsim3/src/memory_system.h"
#include <Config.h>

using namespace dramsim3;
using namespace std;

#ifndef DRAMW_H
#define DRAMW_H

template <int dlen>
struct PMAEntry { //:RSEntry { // PMA stands for Pending Memory Access.
    // this is not really a ReservationStation entry.
    // But it is convenient to model it this way.
    // just to reuse the Ready and Empty bits from the RSEntry.
    uint64_t AccessAddress;
    int64_t RequestCoreClock;
    int64_t DoneCoreClock;
    vector<bitset<dlen>> Data;
    int32_t RequestID; // This is unused for Write Requests. // For read requests use this to determine if the writeback has finished too.
    bool BurstMode;

    friend std::ostream& operator <<(std::ostream& os, PMAEntry const& e) {
        return os << e.AccessAddress << "\t"
                  << e.RequestCoreClock << "\t"
                  << e.DoneCoreClock << "\t"
                  << e.RequestID << "\t"
                  << e.BurstMode; // "\t\t"
                  //<< static_cast<const RSEntry&>(e);
    }
};

template<int alen, int dlen>
class DRAMW {
    public:
        vector<bitset<dlen>> lastReadData;
        bool readDone = false;
        bool writeDone = false;

        DRAMW(string, string, SysConfig *, SysConfig *, bool);
        void input();
        void output();

        bool willAcceptRequest(bitset<alen>, bool);
        bool readRequest(bitset<alen>, uint32_t = 0, bool = false);
        bool writeRequest(bitset<alen>, vector<bitset<dlen>>, bool = false);
        void ReadCompleteHandler(uint64_t);
        void WriteCompleteHandler(uint64_t);

        void step();

        bool isFree(bool);

        int getChannelWidth();

        pair<bool, vector<PMAEntry<dlen>>> getNextWriteBack();

        void printStats();

    private:
        vector<bitset<dlen>> MEM;

        int addressibility; // Maybe use this to verify that the chosen DRAM config matches this requirement.
        int channelwidth; // Same as above.
        long int memsize; // Same as above.
        string dramsim3configfile;

        string id;
        bool readonly;
        string dataIODir;

        bitset<alen> nextReadAddress;
        int readWaitCycles;
        bool readPending;

        bitset<alen> nextWriteAddress;
        vector<bitset<dlen>> nextWriteData;
        int writeWaitCycles;
        bool writePending;
        
        MemorySystem * MemSystem;
        uint16_t memSysClockTriggerConst, clkTriggerCount;
        int64_t clk;

        vector<PMAEntry<dlen>*> pendingReads;
        vector<PMAEntry<dlen>*> pendingWrites;
        // Doesn't mean this requires a hardware Reservation Station structure.
        // Modeling it this way for convenience and reuse the behaviour.
        //ReservationStation<PMAEntry> pendingReads;
        //ReservationStation<PMAEntry> pendingWrites;

        // Right now I am going strike this out because the reservation station
        // is a structure with pre-configured number of elements.
        // That is not how we want the pending memory accesses to be.
        // And using that here will require more changes to the ReservationStation class.
};

#include <../src/DRAMWrapper.cpp>
#endif