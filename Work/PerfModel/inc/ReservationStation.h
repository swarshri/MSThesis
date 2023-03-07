#include <iostream>
#include <bitset>

#include <Config.h>

#ifndef RS_H
#define RS_H

struct RSEntry {
    bool Ready;
    bool Empty;

    friend std::ostream& operator <<(std::ostream& os, RSEntry const& e) {
        return os << e.Ready << "\t"
                  << e.Empty;
    }
};

template <typename EntryType>
class ReservationStation {
    public:
        ReservationStation(string, SysConfig *);

        int numVacant();
        void fill(bitset<6>, EntryType);
        int nextFreeEntry();
        pair<int, EntryType> nextReadyEntry();
        void setEmptyState(int);
        void setScheduledState(int);
        void setReadyState(int);
        void setWaitingState(int);
        bool isEmpty();

        void show(ostream&);

    protected:
        vector<EntryType> Entries;
        int numEntries;

        string id;
};

#include <../src/ReservationStation.cpp>
#endif