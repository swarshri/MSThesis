#include <iostream>

using namespace std;

#ifndef PERF_REC_H
#define PERF_REC_H

struct PerfMetrics {

};

class PerformanceRecorder {
    public:
        PerformanceRecorder(string);

        void write();

    private:
        string opFilePath;
};

#endif