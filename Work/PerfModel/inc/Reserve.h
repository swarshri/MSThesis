#include <iostream>
#include <map>

#include <Config.h>
#include <Queue.h>
#include <Dispatch.h>
#include <ReservationStation.h>
#include <Cache.h>

#ifndef RES_H
#define RES_H

struct LRSEntry:RSEntry {
    bitset<32> OccMemoryAddress;
    bool LowOrHigh; // false is low, true is high - DUH!
    bitset<6> ResStatIndex;
    bitset<6> BasePointer;
    
    friend std::ostream& operator <<(std::ostream& os, LRSEntry const& e) {
        return os << e.LowOrHigh << "\t"
                  << e.OccMemoryAddress << "\t"
                  << e.ResStatIndex << "\t"
                  << e.BasePointer << "\t\t"
                  << static_cast<const RSEntry&>(e);
    }
};

struct CRSEntry:RSEntry {
    bitset<32> LowOcc;
    bool LowOccReady;
    bitset<32> HighOcc;
    bool HighOccReady;
    bitset<6> SRSWBIndex;

    friend std::ostream& operator <<(std::ostream& os, CRSEntry const& e) {
        return os << e.LowOcc << "\t"
                  << e.LowOccReady << "\t"
                  << e.HighOcc << "\t"
                  << e.HighOccReady << "\t"
                  << e.SRSWBIndex << "\t\t"
                  << static_cast<const RSEntry&>(e);
    }
};

class ComputeReservationStation: public ReservationStation<CRSEntry> {
    public:
        ComputeReservationStation(string, Config *);
        void fillLowOccVal(int, bitset<32>);
        void fillHighOccVal(int, bitset<32>);
};

class ReserveStage {
    public:
        ReserveStage(Config*, char, string);

        void connect(DispatchStage *);
        void step();

        bool isHalted();

        pair<int, CRSEntry> getNextComputeEntry();
        void setCRSEToEmptyState(int);
        void scheduleToSetCRSEToEmptyState(int);
        void fillInCRS(int, bool, bitset<32>);
        void scheduleToFillInCRS(int, bool, bitset<32>);
        pair<int, LRSEntry> getNextLoadEntry();
        void setLRSEToEmptyState(int);
        void scheduleToSetLRSEToEmptyState(int);
        void scheduleWriteIntoCache(IncomingCacheStruct);

        void print();

    private:
        int cycle_count;
        bool halted;
        char base;
        int base_num;

        DispatchStage * coreDU;
        pair<bool, DispatchQueueEntry> pendingToBeReserved;

        Queue<bitset<6>> * LRSIdxQ;
        ReservationStation<LRSEntry> * LRS; // LoadReservationStation - acts very similar to Queue. Using RS instead because, we want to enqueue atmost two entries in each cycle.
        ComputeReservationStation * CRS; // ComputeReservationStation
        Cache * LocalCache;
        bool hasCache;

        string op_file_path;

        vector<int> pendingEmptyCRSIdcs;
        bool pendingCRSEmpty;

        vector<tuple<int, bool, bitset<32>>> pendingCRSEntries;
        bool pendingCRSE;

        vector<int> pendingEmptyLRSIdcs;
        bool pendingLRSEmpty;

        IncomingCacheStruct pendingCacheInput;
        bool pendingCacheWrite;
};

#endif