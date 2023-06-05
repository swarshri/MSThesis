#include <iostream>
#include <bitset>
#include <map>

#include <Config.h>
#include <Queue.h>
#include <Fetch.h>

#ifndef DISP_H
#define DISP_H

enum DispatchScheme {
    LRIO = 0,
    BWLRIO = 1
};

struct DispatchQueueEntry {
    char base = '0'; // Seems unnecessary without the prefetcher. Will need more bits for prefetcher though.
    uint64_t LowPointer;
    uint64_t HighPointer;
    int SRSWBIndex;
    int BasePointer;

    friend std::ostream& operator <<(std::ostream& os, const DispatchQueueEntry& dqe) {
        os << dqe.base << "\t" << dqe.LowPointer << "\t" << dqe.HighPointer << "\t" << dqe.SRSWBIndex << "\t" << dqe.BasePointer;
        return os;
    }
};

struct StoreQueueEntry {
    uint64_t StoreAddress;
    uint64_t StoreValLow;
    uint64_t StoreValHigh;

    friend std::ostream& operator <<(std::ostream& os, const StoreQueueEntry& sqe) {
        os << sqe.StoreAddress << "\t" << sqe.StoreValLow << "\t" << sqe.StoreValHigh;
        return os;
    }
};

class DispatchStage {
    public:
        // Constructor
        DispatchStage(SysConfig *, string);

        // Common for all Pipeline stages - called from core
        void print();
        bool isHalted();
        void connect(FetchStage *);
        void step();

        // API methods for getting from internal queues.
        pair<bool, DispatchQueueEntry> popNextDispatch(char);
        pair<bool, StoreQueueEntry> popNextStore();
        pair<bool, DispatchQueueEntry> getNextDispatch(char);
        pair<bool, StoreQueueEntry> getNextStore();

        uint64_t getNumCyclesWithNewDispatch(char);
        uint64_t getNumCyclesWithNoNewDispatch(char);

    private:
        // Dispatch scheme from the config file.
        int dispatchScheme;
        void dispatchLRIO(int); //dispatch Lowest Ready Index Out.
        void dispatchBWLRIO(); //dispatch Base-Wise Lowest Ready Index Out.
        
        // Performance measurement related
        int cycle_count = 0;
        bool halted = false;
        
        // External component - other stages in the core.
        FetchStage * coreFU;

        // PRINT - Registers/Sequential logic that changes at clock trigger.
        map<char, Queue<DispatchQueueEntry>*> DispatchQueues;
        map<char, uint64_t> numCyclesWithNewDispatch;
        map<char, uint64_t> numCyclesWithNoNewDispatch;
        map<char, uint64_t> numCyclesWithDispatchStalled;
        Queue<StoreQueueEntry> * StoreQueue;

        // Dispatch Output file
        string op_file_path;
};

#endif