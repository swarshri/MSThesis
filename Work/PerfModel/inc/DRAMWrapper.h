#include <vector>
#include <string>

#include "../ext/DRAMsim3/src/memory_system.h"
#include <Config.h>

using namespace dramsim3;
using namespace std;

#ifndef DRAMW_H
#define DRAMW_H

template <typename DataType>
struct PMAEntry { //:RSEntry { // PMA stands for Pending Memory Access.
    // this is not really a ReservationStation entry.
    // But it is convenient to model it this way.
    // just to reuse the Ready and Empty bits from the RSEntry.
    uint64_t AccessAddress;
    int64_t RequestCoreClock;
    int64_t DoneCoreClock;
    vector<DataType> Data;
    int32_t RequestID; // This is unused for Write Requests. // For read requests use this to determine if the writeback has finished too.

    friend std::ostream& operator <<(std::ostream& os, PMAEntry const& e) {
        return os << e.AccessAddress << "\t"
                  << e.RequestCoreClock << "\t"
                  << e.DoneCoreClock << "\t"
                  << e.Data << "\t"
                  << e.RequestID << endl; // "\t\t"
                  //<< static_cast<const RSEntry&>(e);
    }
};

template<typename AddressType, typename DataType>
class DRAMW {
    public:
        vector<DataType> lastReadData;
        bool readDone = false;
        bool writeDone = false;

        DRAMW(string, string, SysConfig *, bool);
        void input();
        void output();

        bool readRequest(AddressType, uint32_t = 0);
        bool writeRequest(AddressType, vector<DataType>);
        void ReadCompleteHandler(uint64_t);
        void WriteCompleteHandler(uint64_t);

        void step();

        bool isFree();

        int getChannelWidth();

        pair<bool, vector<PMAEntry<DataType>>> getNextWriteBack();

        void printStats();

    private:
        vector<DataType> MEM;

        int addressibility; // Maybe use this to verify that the chosen DRAM config matches this requirement.
        int channelwidth; // Same as above.
        long int memsize; // Same as above.
        string dramsim3configfile;

        string id;
        bool readonly;
        string dataIODir;

        AddressType nextReadAddress;
        int readWaitCycles;
        bool readPending;

        AddressType nextWriteAddress;
        vector<DataType> nextWriteData;
        int writeWaitCycles;
        bool writePending;
        
        MemorySystem * MemSystem;
        int64_t clk;

        vector<PMAEntry<DataType>*> pendingReads;
        vector<PMAEntry<DataType>*> pendingWrites;
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