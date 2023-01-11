#include <iostream>
#include <bitset>

#include <Config.h>

template <typename EntryType, typename InputType>
class ReservationStation {
    public:
        ReservationStation(Config *, bitset<32>);

        int numVacant();
        void populate(vector<bitset<64>>, bitset<32>);
        void fill(bitset<6>, bitset<64>, bitset<32>);
        pair<bool, bitset<6>> nextFreeEntry();
        pair<int, SRSEntry> nextReadyEntry();
        void setEmptyState(int);
        void setScheduledForFetch(int);
        void setReadyForDispatch(int);
        void setDispatched(int);
        void updateBasePointer(int);
        bool isEmpty();

        void show();

    private:
        vector<SRSEntry> Entries;
        int numEntries;
        bitset<32> refCount;
};