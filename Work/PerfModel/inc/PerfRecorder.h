#include <iostream>

using namespace std;

#ifndef PERF_REC_H
#define PERF_REC_H

struct UnitPerfMetrics {
    bool active; // not halted.
};

struct FetchUnitPM:UnitPerfMetrics {

};

struct DispatchUnitPM:UnitPerfMetrics {

};

struct ReserveUnitPM:UnitPerfMetrics {

};

struct LoadUnitPM:UnitPerfMetrics {

};

struct ComputeUnitPM:UnitPerfMetrics {

};

struct PerfMetrics {
    int numCycles;
    FetchUnitPM fetchpm;
    DispatchUnitPM dispatchpm;
    ReserveUnitPM reservepm;
    LoadUnitPM loadpm;
    ComputeUnitPM computepm;
};

class PerformanceRecorder {
    public:
        PerformanceRecorder(string);

        void write();

    private:
        string opFilePath;

        PerfMetrics Metrics;
};

#endif