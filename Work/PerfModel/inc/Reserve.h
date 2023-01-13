#include <iostream>

#include <Config.h>
#include <Queue.h>
#include <Dispatch.h>
#include <ReservationStation.h>

struct LRSEntry:RSEntry {
    bitset<32> OccMemoryAddress;
    bool LowOrHigh; // false is low, true is high - DUH!
    bitset<6> SRSWBIndex;
    bitset<6> ResStatIndex;
    
    friend std::ostream& operator <<(std::ostream& os, LRSEntry const& e)
    {
        return os << e.LowOrHigh << "\t"
                  << e.OccMemoryAddress << "\t"
                  << e.SRSWBIndex << "\t"
                  << e.ResStatIndex;
    }
};

struct CRSEntry:RSEntry {
    bitset<64> Count;
    bitset<64> LowOcc;
    bool LowOccReady;
    bitset<64> HighOcc;
    bool HighOccReady;
    bitset<6> SRSWBIndex;

    friend std::ostream& operator <<(std::ostream& os, CRSEntry const& e)
    {
        return os << e.Count << "\t"
                  << e.LowOcc << "\t"
                  << e.LowOccReady << "\t"
                  << e.HighOcc << "\t"
                  << e.HighOccReady << "\t"
                  << e.SRSWBIndex;
    }
};

class ReserveUnit {
    public:
        ReserveUnit(Config*, vector<bitset<64>> *);

        void connect(DispatchUnit *);
        void step();

    private:
        int cycle_count;
        bool halted;
        int base;
        bitset<32> RefCount;
        bitset<64> CountReg;
        bitset<64> OccLastValReg;

        DispatchUnit * coreDU;
        pair<bool, DispatchEntry> pendingToBeReserved;

        Queue<bitset<6>> * LRSIdxQ;
        ReservationStation<LRSEntry> * LRS;
        ReservationStation<CRSEntry> * CRS; // ComputeReservationStation
};