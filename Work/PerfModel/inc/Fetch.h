#include<stdio.h>
#include<bitset>
#include<vector>

#include<Config.h>

struct SRSEntry {
    bitset<8> Index;
    bitset<32> SeedAddress;
    bitset<64> Seed;
    bitset<32> LowPointer;
    bitset<32> HighPointer;
    bitset<5> BasePointer;
    bool Ready;
};

class SeedReservationStation {
    public:
        SeedReservationStation(Config *);

        void step();

    private:
        vector<SRSEntry> Entries;
};

class FetchUnit {
    public:
        FetchUnit(Config *);

        void step();

    private:
        bitset<32> NextSeedPointer;
};