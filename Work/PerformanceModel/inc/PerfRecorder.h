#include <iostream>
#include <map>
#include <queue>

#include <Config.h>

using namespace std;

#ifndef PERF_REC_H
#define PERF_REC_H

class PerformanceRecorder {
    public:
        PerformanceRecorder(string, string, SysConfig*);

        void addMetrics(vector<string>);
        void record(uint64_t, string, string);
        void step();
        void dump();

    private:
        string opFilePath;
        uint64_t clkcount;
        uint16_t writeCycle;
        uint16_t pendingRecordCount;

        map<string, queue<string>> data;
};

#endif