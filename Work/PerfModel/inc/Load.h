#include <iostream>
#include <pair>

#include <Config.h>
#include <Queue.h>
#include <Dispatch.h>

struct LoadQueueEntry {
    bitset<32> OccMemoryAddress;
    bool LowOrHigh; // false is low, true is high - DUH!
    bitset<6> SRSWBIndex;
    bitset<6> ResStatIndex;
};

struct ComputeQueueEntry {
    bitset<2> base;
    bitset<64> Count;
    bitset<64> LowOcc;
    bitset<64> HighOcc;
    bitset<6> SRSWBIndex;
};

struct ReservationStationEntry {
    bitset<64> Count;
    bitset<64> LowOcc;
    bool LowOccReady;
    bitset<64> HighOcc;
    bool HighOccReady;
    bitset<6> SRSWBIndex;
};

class ReservationStation {
    public:
        ReservationStation(Config *);

        pair<bool, ReservationStationBuffer> 

    private:
        int cycle_count;
        
        vector<ReservationStationEntry> Buffer;
};

class MemoryBuffer {
    public:
        MemoryBuffer(Config *);

    private:
        int cycle_count;

        Queue<MemoryBufferEntry> * MBQueue;
};

class ComputeBuffer {
    public:
        ComputeBuffer();

    private:
        int cycle_count;
};

class LoadUnit {
    public:
        LoadUnit(Config *, vector<bitset<64>> *);

        void connect(DispatchUnit *);
        void step();
    
    private:
        int cycle_count;
        bool halted;
        int base;
        int reference_count;
        bitset<32> RefCount;
        bitset<64> CountReg;
        bitset<64> OccFirstValReg;
        bitset<64> OccLastValReg;

        DispatchUnit * coreDU;

        Queue<MemoryBufferEntry> * MemoryQueue;
        Queue<ComputeBufferEntry> * ComputeQueue;
        vector<ReservationStationEntry> * ReservationBuffer;
};