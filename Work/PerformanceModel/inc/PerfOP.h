#include <iostream>
#include <map>
#include <queue>
#include <sys/stat.h>
#include <fstream>

#include <Config.h>
#include <Core.h>

using namespace std;

#ifndef PERF_OP_H
#define PERF_OP_H

struct IOInfo {
    string reffilename;
    string conffilename;
    uint64_t reflength;
    uint64_t seedscount;
};

class PerformanceOutput {
    public:
        PerformanceOutput(string, IOInfo*, SysConfig*, Core*);
        void output();

    private:
        string opFilePath;
        IOInfo * ioinfo;
        map<string, queue<string>> data;
        uint64_t id;
        Core * coreptr;
};

#endif