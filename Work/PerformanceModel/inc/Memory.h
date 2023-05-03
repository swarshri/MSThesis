#include <iostream>
#include <vector>
#include <string>
#include <bitset>

#include "../../External/DRAMsim3/src/memory_system.h"
#include "../../Common/inc/DataInput.h"
#include <Config.h>
#include <DRAMWrapper.h>

using namespace std;
using namespace dramsim3;

#ifndef MEM_H
#define MEM_H

class SeedMemory: public DRAMW<string> {
    public:
        SeedMemory(string, string, SysConfig *, SysConfig *);
        void input(Reads *);
        void ReadCompleteHandler(uint64_t);
        void WriteCompleteHandler(uint64_t);

    private:
        Reads * READS;
};

class OccMemory: public DRAMW<> {
    public:
        OccMemory(string, string, SysConfig *, SysConfig *);
        void input(Reference *);
        void ReadCompleteHandler(uint64_t);
        void WriteCompleteHandler(uint64_t);

    private:
        string base;
        Reference * REF;
};

class SiMemory: public DRAMW<vector<uint64_t>> {
    public:
        SiMemory(string, string, SysConfig *, SysConfig *);
        void output();
};

#endif