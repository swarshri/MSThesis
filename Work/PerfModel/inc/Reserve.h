#include <iostream>
#include <map>

#include <Config.h>
#include <Queue.h>
#include <Dispatch.h>
#include <ReservationStation.h>
#include <Cache.h>
#include <PerfRecorder.h>

#ifndef RES_H
#define RES_H

struct LQEntry {
    bitset<32> OccMemoryAddress;
    bool LowOrHigh; // false is low, true is high - DUH!
    bitset<6> ResStatIndex;
    bitset<6> BasePointer;

    friend std::ostream& operator <<(std::ostream& os, LQEntry const& e) {
        return os << e.LowOrHigh << "\t"
                  << e.OccMemoryAddress << "\t"
                  << e.ResStatIndex << "\t"
                  << e.BasePointer;
    }
};

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
        ComputeReservationStation(string, SysConfig *);
        void fillLowOccVal(int, bitset<32>);
        void fillHighOccVal(int, bitset<32>);
};

class ReserveStage {
    public:
        ReserveStage(SysConfig *, string, string, PerformanceRecorder *);

        void connect(DispatchStage *);
        void step();

        bool isHalted();

        pair<int, CRSEntry> getNextComputeEntry();
        void setCRSEToEmptyState(int);
        void scheduleToSetCRSEToEmptyState(int);
        void fillInCRS(int, bool, bitset<32>);
        void scheduleToFillInCRS(int, bool, bitset<32>);
        pair<bool, LQEntry> getNextLoadEntry();
        pair<bool, LQEntry> popNextLoadEntry();
        void scheduleWriteIntoCache(IncomingCacheStruct);

        void print();

    private:
        int cycle_count;
        bool halted;
        string base;
        int base_num;
        string name;

        DispatchStage * coreDU;
        pair<bool, DispatchQueueEntry> pendingToBeReserved;

        Queue<LQEntry> * LQ;
        ComputeReservationStation * CRS; // ComputeReservationStation
        Cache * LocalCache;
        bool hasCache;

        string op_file_path;

        pair<bool, vector<int>> pendingEmptyCRSIdcs;

        pair<bool, vector<tuple<int, bool, bitset<32>>>> pendingCRSEntries;

        pair<bool, IncomingCacheStruct> pendingCacheInput;

        PerformanceRecorder * perf;
};

#endif