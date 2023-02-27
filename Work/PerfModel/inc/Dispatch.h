#include <iostream>
#include <bitset>
#include <map>

#include <Config.h>
#include <Queue.h>
#include <Fetch.h>

#ifndef DISP_H
#define DISP_H

enum DispatchScheme {
    SEQ1PE = 0,
    PAR4PE = 1
};

struct DispatchQueueEntry {
    bitset<2> base; // Seems unnecessary without the prefetcher. WIll need more bits for prefetcher though.
    bitset<32> LowPointer;
    bitset<32> HighPointer;
    bitset<6> SRSWBIndex;
    bitset<6> BasePointer;

    friend std::ostream& operator <<(std::ostream& os, const DispatchQueueEntry& dqe) {
        os << dqe.base << "\t" << dqe.LowPointer << "\t" << dqe.HighPointer << "\t" << dqe.SRSWBIndex << "\t" << dqe.BasePointer;
        return os;
    }
};

struct StoreQueueEntry {
    bitset<32> StoreAddress;
    bitset<64> StoreVal;

    friend std::ostream& operator <<(std::ostream& os, const StoreQueueEntry& sqe) {
        os << sqe.StoreAddress << "\t" << sqe.StoreVal;
        return os;
    }
};

class DispatchStage {
    public:
        // Constructor
        DispatchStage(Config*, string);

        // Common for all Pipeline stages - called from core
        void print();
        bool isHalted();
        void connect(FetchStage *);
        void step();

        // API methods for getting from internal queues.
        pair<bool, DispatchQueueEntry> popNextDispatch(int);
        pair<bool, StoreQueueEntry> popNextStore();

    private:
        // Dispatch scheme from the config file.
        int dispatchScheme;
        void dispatchSequential(int);
        
        // Performance measurement related
        int cycle_count = 0;
        bool halted = false;
        
        // External component - other stages in the core.
        FetchStage * coreFU;

        // PRINT - Registers/Sequential logic that changes at clock trigger.
        map<int, Queue<DispatchQueueEntry>*> DispatchQueues;
        Queue<StoreQueueEntry> * StoreQueue;

        // Dispatch Output file
        string op_file_path;
};

#endif