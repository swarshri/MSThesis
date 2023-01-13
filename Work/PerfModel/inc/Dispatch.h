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

struct DispatchEntry {
    bitset<2> base;
    bitset<32> Low;
    bitset<32> High;
    bitset<6> SRSWBIndex;
    bool ready = false;
};

class DispatchUnit {
    public:

        DispatchUnit(Config*);

        bool halted = false;

        map<int, Queue<DispatchEntry>*> DispatchQueues;
        
        Queue<DispatchEntry> * Dispatch_AQ;
        Queue<DispatchEntry> * Dispatch_CQ;
        Queue<DispatchEntry> * Dispatch_GQ;
        Queue<DispatchEntry> * Dispatch_TQ;

        void step();
        void connect(FetchUnit *);
        pair<bool, DispatchEntry> popnext(int);

    private:
        int cycle_count = 0;
        int dispatchScheme;
        FetchUnit * coreFU;

        void dispatchSequential(int);
};

#endif