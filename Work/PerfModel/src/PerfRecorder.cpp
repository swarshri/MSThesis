#include <PerfRecorder.h>

PerformanceRecorder::PerformanceRecorder(string iodir) {

#ifdef _WIN32
    string filename = "\\OP\\PerfMetrics.csv";
#else
    string filename = "OP/PerfMetrics.csv";
#endif

    // TODO add current date and time to the output file name - to run multiple times.
    this->opFilePath = iodir + filename;
}

void PerformanceRecorder::write() {
    
}