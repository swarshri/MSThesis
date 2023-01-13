#include<stdio.h>
#include<bitset>
#include<vector>

#include<Config.h>
#include<DRAM.h>
#include<Queue.h>
#include<ReservationStation.h>

#ifndef FETCH_H
#define FETCH_H

struct SRSEntry:RSEntry {
    bitset<32> SeedAddress;
    bitset<64> Seed;
    bitset<32> LowPointer;
    bitset<32> HighPointer;
    bitset<6> BasePointer;

    friend std::ostream& operator <<(std::ostream& os, SRSEntry const& e)
    {
        return os << e.SeedAddress << "\t"
                  << e.Seed << "\t"
                  << e.LowPointer << "\t"
                  << e.HighPointer << "\t"
                  << e.BasePointer;
    }
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
        bitset<32> RefCount;
        Queue<bitset<6>> * FillIdxQueue;
        DRAM * SDMEM;
        ReservationStation<SRSEntry> * SRS; // SeedReservationStation

        int bufferSize;
        int bufferPtr;

        int cycle_count;
};
#endif