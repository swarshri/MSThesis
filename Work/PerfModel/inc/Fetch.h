#include<stdio.h>
#include<bitset>
#include<vector>

#include<Config.h>
#include<DRAM.h>

struct SRSEntry {
    bitset<32> SeedAddress;
    bitset<64> Seed;
    bitset<32> LowPointer;
    bitset<32> HighPointer;
    bitset<6> BasePointer;
    bool Ready = false;
    bool Valid = false;
};

class SeedReservationStation {
    public:
        SeedReservationStation(Config *, bitset<32>);

        int numVacant();
        void populate(vector<bitset<64>>, bitset<32>);

        void show();

    private:
        vector<SRSEntry> Entries;
        int numEntries;
        bitset<32> refCount;
        vector<unsigned int> nextFreeEntries;
};

class FetchUnit {
    public:
        FetchUnit(Config *, bitset<32>);

        void step();
        void connect(DRAM *);

    private:
        bitset<32> SeedPointer;
        bitset<32> NextSeedPointer;
        vector<bitset<64>> Buffer;
        DRAM * SDMEM;
        SeedReservationStation * SRS;

        int bufferSize;
        int bufferPtr;
};