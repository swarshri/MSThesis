#include<stdio.h>
#include<bitset>
#include<vector>

#include<Config.h>
#include<DRAM.h>
#include<Queue.h>

#ifndef FETCH_H
#define FETCH_H
struct SRSEntry {
    bitset<32> SeedAddress;
    bitset<64> Seed;
    bitset<32> LowPointer;
    bitset<32> HighPointer;
    bitset<6> BasePointer;
    bool Ready = false;
    bool Empty = true;
};

class SeedReservationStation {
    public:
        SeedReservationStation(Config *, bitset<32>);

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

class FetchUnit {
    public:
        FetchUnit(Config *, bitset<32>);

        void step();
        void connect(DRAM *);

        pair<int, SRSEntry> getNextReadyEntry();
        void setInProgress(int);
        void setEmptyState(int);
        void setReadyState(int);
        bool emptySRS();
        void print();

        bool halted;

    private:
        bitset<32> SeedPointer;
        bitset<32> NextSeedPointer;
        Queue<bitset<6>> * FillIdxQueue;
        DRAM * SDMEM;
        SeedReservationStation * SRS;

        int bufferSize;
        int bufferPtr;

        int cycle_count;
};
#endif