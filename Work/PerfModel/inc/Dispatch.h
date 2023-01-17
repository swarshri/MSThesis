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
    bitset<2> base;
    bitset<32> LowPointer;
    bitset<32> HighPointer;
    bitset<6> SRSWBIndex;
    bool ready = false;

    friend std::ostream& operator <<(std::ostream& os, const DispatchQueueEntry& dqe) {
        os << dqe.base << "\t" << dqe.LowPointer << "\t" << dqe.HighPointer << "\t" << dqe.SRSWBIndex << "\t" << dqe.ready << endl;
        return os;
    }
};

struct StoreQueueEntry {
    bitset<32> StoreAddress;
    bitset<64> StoreVal;

    friend std::ostream& operator <<(std::ostream& os, const StoreQueueEntry& sqe) {
        os << sqe.StoreAddress << "\t" << sqe.StoreVal << endl;
        return os;
    }
};

class DispatchUnit {
    public:

        DispatchUnit(Config*);

        bool halted = false;

        void step();
        void connect(FetchUnit *);
        bool isHalted();
        pair<bool, DispatchQueueEntry> popNextDispatch(int);
        pair<bool, StoreQueueEntry> popNextStore();

    private:
        int cycle_count = 0;
        int dispatchScheme;
        FetchUnit * coreFU;

        map<int, Queue<DispatchQueueEntry>*> DispatchQueues;
        Queue<StoreQueueEntry> * StoreQueue;

        void dispatchSequential(int);
};

#endif