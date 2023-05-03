#include<iostream>
#include<bitset>
#include<vector>

#include<Config.h>
#include<DRAMWrapper.h>
#include<Queue.h>
#include<Memory.h>
#include<ReservationStation.h>

#ifndef FETCH_H
#define FETCH_H

struct SRSEntry:RSEntry {
    uint64_t SeedAddress;
    string Seed;
    uint64_t LowPointer;
    uint64_t HighPointer;
    int BasePointer;
    bool StoreFlag;

    friend std::ostream& operator <<(std::ostream& os, SRSEntry const& e) {
        return os << e.SeedAddress << "\t"
                  << e.Seed << "\t"
                  << e.LowPointer << "\t"
                  << e.HighPointer << "\t"
                  << e.StoreFlag << "\t"
                  << e.BasePointer << "\t\t"
                  << static_cast<const RSEntry&>(e);
    }
};

class SeedReservationStation: public ReservationStation<SRSEntry> {
    public:
        SeedReservationStation(string, SysConfig *);
        void setStoreFlag(int);
        void updateBasePointer(int);
        void updateLowPointer(int, uint64_t);
        void updateHighPointer(int, uint64_t);
};

class FetchStage {
    public:
        // Constructor
        FetchStage(SysConfig *, string, uint64_t);

        // Common for all Pipeline stages - called from core
        void print();
        bool isHalted();
        void connect(); // connect with other components within core
        void connectDRAM(SeedMemory *); // connect with off-chip components
        void step(); // clock trigger
        void step_old(); // clock trigger

        // API methods for getting and setting from internal registers.
        pair<int, SRSEntry> getNextReadyEntry();
        void writeBack(int, uint64_t, uint64_t);
        void scheduleToSetEmptyState(int);
        void setInProgress(int);
        void setEmptyState(int);
        void setReadyState(int);
        void setStoreFlag(int);
        bool emptySRS();
        
    private:
        // Constant for a reference genome - input from CoreReg.mem file.
        uint64_t RefCount;

        // PRINT - Registers/Sequential logic that changes at clock trigger.
        uint64_t SeedPointer;
        SeedReservationStation * SRS; // SeedReservationStation - intermediate between Fetch stage and the 
        // Queue<bitset<6>> * FillIdxQueue; // Helper structure to fill multiple SRS entries per cycle.

        // External component - This one is off-chip DRAM that stores the seed queries.
        SeedMemory * SDMEM; // Initial state input from SdMEM.mem file.

        // Performance measurement related
        int cycle_count;
        bool halted;

        // Fetch Output file
        string op_file_path;

        vector<pair<int, pair<uint64_t, uint64_t>>> pendingWriteBacks;
        bool pendingWB;

        vector<int> pendingEmptyIdcs;
        bool pendingEmpty;
};

#endif