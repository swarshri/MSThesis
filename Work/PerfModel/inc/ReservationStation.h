#include <iostream>
#include <bitset>

#include <Config.h>

#ifndef RS_H
#define RS_H

struct RSEntry {
    bool Ready;
    bool Empty;
};

template <typename EntryType>
class ReservationStation {
    public:
        ReservationStation(Config *);

        int numVacant();
        void fill(bitset<6>, EntryType);
        int nextFreeEntry();
        pair<int, EntryType> nextReadyEntry();
        void setEmptyState(int);
        void setScheduledState(int);
        void setReadyState(int);
        void setWaitingState(int);
        bool isEmpty();

        void show();

    protected:
        vector<EntryType> Entries;
        int numEntries;
};

#include <../src/ReservationStation.cpp>
#endif