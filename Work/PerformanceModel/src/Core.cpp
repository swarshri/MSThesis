#include<Core.h>

#ifndef CORE_DEF
#define CORE_DEF

Core::Core(string id, string ioDir, SysConfig * config, PerformanceRecorder * perf, Reference * ref) {
    this->id = id;

//     ifstream mem;
//     string line;
    
// #ifdef _WIN32
//     string filename = "\\mem\\CoreReg.mem";
// #else
//     string filename = "mem/CoreReg.mem";
// #endif
    
//     mem.open(ioDir + filename);

//     vector<uint64_t> coremem;
//     if (mem.is_open()) {
//         while (getline(mem, line)) {
//             line = line.substr(0, line.find(' '));
//             coremem.push_back(uint64_t(line));
//         }
//         mem.close();
//     }
//     else cout<<"Unable to open input file for Core " << this->id << ": " << ioDir + filename << endl;

    uint64_t referenceCountVal = ref->get_seqLen();
    vector<uint64_t> counts;
    for (int i = 0; i < 4; i++)
        counts.push_back(ref->getCount(i));

    this->FU = new FetchStage(config->children["FetchStage"], ioDir, referenceCountVal);

    this->DU = new DispatchStage(config->children["DispatchStage"], ioDir);
    this->DU->connect(this->FU);

    for (char base: this->bases) {
        this->RUs[base] = new ReserveStage(config->children["ReserveStage"], string(1, base), ioDir, perf);
        this->RUs[base]->connect(this->DU);
        this->CUs[base] = new ComputeStage(config->children["ComputeStage"], string(1, base), ioDir, ref->getCount(base));
        this->CUs[base]->connect(this->RUs[base], this->FU);
        this->LUs[base] = new LoadStage(config->children["LoadStage"], string(1, base), ioDir, perf);
        this->LUs[base]->connectRU(this->RUs[base]);
    }

    this->SU = new StoreStage(config->children["StoreStage"]);
    this->SU->connectDU(this->DU);

    this->halted = false;

    this->perfmetrics = new PerfMetrics;
    this->perfmetrics->numCycles = 0; 
}

void Core::connect(SeedMemory * sdmem, map<char, OccMemory*> ocmem, SiMemory * simem) {
    this->FU->connectDRAM(sdmem);

    for (char base: this->bases)
        this->LUs[base]->connectDRAM(ocmem[base]);

    this->SU->connectDRAM(simem);
}

bool Core::allRUsHalted() {
    bool retval = true;
    for (char base: this->bases)
        retval = retval && this->RUs[base]->isHalted();
    return retval;
}

bool Core::allCUsHalted() {
    bool retval = true;
    for (char base: this->bases)
        retval = retval && this->CUs[base]->isHalted();
    return retval;
}

bool Core::allLUsHalted() {
    bool retval = true;
    for (char base: this->bases)
        retval = retval && this->LUs[base]->isHalted();
    return retval;
}

bool Core::allStagesHalted() {
    return this->FU->isHalted() && this->DU->isHalted() && this->SU->isHalted() && 
           this->allRUsHalted() && this->allCUsHalted() && this->allLUsHalted();
}

void Core::step() {
    if (!this->halted) {
        this->SU->step();

        for (char base: this->bases) {
            this->CUs[base]->step();
            this->LUs[base]->step();
            this->RUs[base]->step();
        }

        this->DU->step();

        this->FU->step();
        
        this->perfmetrics->numCycles++;
    }

    if (this->allStagesHalted()) { // || this->cyclecnt == 1000)
        // cout << "Core: All stages halted." << endl;
        this->halted = true;
        this->gatherPLMetrics();
    }
}

string Core::getPerfMetricTitles() {
    string retstr = "# Cycles, Overall Hit Rate, Overall Miss Rate, Total Cache Hits, Total Cache Misses, Total Occ Lookups, ";
    for (char base: this->bases) {
        string delim = ", ";
        retstr = retstr +
                 base + " Utilization" + delim;
    }
    for (char base: this->bases) {
        string delim = ", ";
        retstr = retstr +
                 base + " Num Low Cache Hits" + delim +
                 base + " Num Low Cache Misses" + delim +
                 base + " Num Low Occ Lookups" + delim +
                 base + " Num High Cache Hits" + delim +
                 base + " Num High Cache Misses" + delim +
                 base + " Num High Occ Lookups" + delim +
                 base + " Num Cache Hits" + delim +
                 base + " Num Cache Misses" + delim +
                 base + " Num Occ Lookups" + delim +
                 base + " Cache Hit Rate" + delim +
                 base + " Cache Miss Rate" + delim +
                 base + " Num Cycles With New Jobs" + delim +
                 base + " Num Cycles With No New Jobs" + delim;
    }
    return retstr;
}

PerfMetrics * Core::getPerfMetrics() {
    return this->perfmetrics;
}

void Core::gatherPLMetrics() {
    this->perfmetrics->totalCacheHits = 0;
    this->perfmetrics->totalCacheMisses = 0;
    this->perfmetrics->totalOccLookups = 0;

    cout << "Core: Gathering PL Metrics." << endl;
    for (char base: this->bases) {
        PLPerfMetrics plmetric;
        plmetric.numLowOccLookups = this->RUs[base]->getNumLowOccLookups();
        plmetric.numLowCacheHits = this->RUs[base]->getNumLowCacheHits();
        plmetric.numLowCacheMisses = this->RUs[base]->getNumLowCacheMisses();
        plmetric.numHighOccLookups = this->RUs[base]->getNumHighOccLookups();
        plmetric.numHighCacheHits = this->RUs[base]->getNumHighCacheHits();
        plmetric.numHighCacheMisses = this->RUs[base]->getNumHighCacheMisses();
        plmetric.numOccLookups = plmetric.numLowOccLookups + plmetric.numHighOccLookups;
        plmetric.numCacheHits = plmetric.numLowCacheHits + plmetric.numHighCacheHits;
        plmetric.numCacheMisses = plmetric.numLowCacheMisses + plmetric.numHighCacheMisses;

        cout << "plmetric.numOccLookups: " << plmetric.numOccLookups << endl;
        if (plmetric.numOccLookups != 0) {
            plmetric.cacheHitRate = (float)plmetric.numCacheHits / (float)plmetric.numOccLookups;
            plmetric.cacheMissRate = (float)plmetric.numCacheMisses / (float)plmetric.numOccLookups;
        }
        else {
            plmetric.cacheHitRate = 0;
            plmetric.cacheMissRate = 0;
        }

        plmetric.numCyclesWithNewJobs = this->DU->getNumCyclesWithNewDispatch(base);
        plmetric.numCyclesWithNoNewJobs = this->DU->getNumCyclesWithNoNewDispatch(base);
        plmetric.utilization = (float)plmetric.numCyclesWithNewJobs / (float)this->perfmetrics->numCycles;
        
        this->perfmetrics->PLMetrics[base] = plmetric;
        this->perfmetrics->totalCacheHits += plmetric.numCacheHits;
        this->perfmetrics->totalCacheMisses += plmetric.numCacheMisses;
        this->perfmetrics->totalOccLookups += plmetric.numOccLookups;
    }

    if (this->perfmetrics->totalOccLookups != 0) {
        this->perfmetrics->overallHitRate = (float)this->perfmetrics->totalCacheHits / (float)this->perfmetrics->totalOccLookups;
        this->perfmetrics->overallMissRate = (float)this->perfmetrics->totalCacheMisses / (float)this->perfmetrics->totalOccLookups;
    }
    else {
        this->perfmetrics->overallHitRate = 0;
        this->perfmetrics->overallMissRate = 0;
    }
}

uint64_t Core::getCycleCount() {
    return this->perfmetrics->numCycles;
}

#endif