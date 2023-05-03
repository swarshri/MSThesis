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
    uint64_t OccMemoryAddress;
    bool LowOrHigh; // false is low, true is high - DUH!
    int ResStatIndex;
    int BasePointer;

    friend std::ostream& operator <<(std::ostream& os, LQEntry const& e) {
        return os << e.LowOrHigh << "\t"
                  << e.OccMemoryAddress << "\t"
                  << e.ResStatIndex << "\t"
                  << e.BasePointer;
    }
};

struct LRSEntry:RSEntry {
    uint64_t OccMemoryAddress;
    bool LowOrHigh; // false is low, true is high - DUH!
    int ResStatIndex;
    int BasePointer;
    
    friend std::ostream& operator <<(std::ostream& os, LRSEntry const& e) {
        return os << e.LowOrHigh << "\t"
                  << e.OccMemoryAddress << "\t"
                  << e.ResStatIndex << "\t"
                  << e.BasePointer << "\t\t"
                  << static_cast<const RSEntry&>(e);
    }
};

struct CRSEntry:RSEntry {
    uint64_t LowOcc;
    bool LowOccReady;
    uint64_t HighOcc;
    bool HighOccReady;
    int SRSWBIndex;

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
        void fillLowOccVal(int, uint64_t);
        void fillHighOccVal(int, uint64_t);
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
        void fillInCRS(int, bool, uint64_t);
        void scheduleToFillInCRS(int, bool, uint64_t);
        pair<bool, LQEntry> getNextLoadEntry();
        pair<bool, LQEntry> popNextLoadEntry();
        void scheduleWriteIntoCache(IncomingCacheStruct);

        uint64_t getNumLowOccLookups();
        uint64_t getNumLowCacheHits();
        uint64_t getNumLowCacheMisses();
        uint64_t getNumHighOccLookups();
        uint64_t getNumHighCacheHits();
        uint64_t getNumHighCacheMisses();

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

        pair<bool, vector<tuple<int, bool, uint64_t>>> pendingCRSEntries;

        pair<bool, IncomingCacheStruct> pendingCacheInput;

        PerformanceRecorder * perf;

        uint64_t numLowOccLookups;
        uint64_t numLowCacheHits;
        uint64_t numLowCacheMisses;
        uint64_t numHighOccLookups;
        uint64_t numHighCacheHits;
        uint64_t numHighCacheMisses;
};

#endif