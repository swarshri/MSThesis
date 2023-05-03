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

    this->RUA = new ReserveStage(config->children["ReserveStage"], "A", ioDir, perf);
    this->RUA->connect(this->DU);
    this->CUA = new ComputeStage(config->children["ComputeStage"], "A", ioDir, counts[0]);
    this->CUA->connect(this->RUA, this->FU);
    this->LUA = new LoadStage(config->children["LoadStage"], "A", ioDir, perf);
    this->LUA->connectRU(this->RUA);

    this->RUC = new ReserveStage(config->children["ReserveStage"], "C", ioDir, perf);
    this->RUC->connect(this->DU);
    this->CUC = new ComputeStage(config->children["ComputeStage"], "C", ioDir, counts[1]);
    this->CUC->connect(this->RUC, this->FU);
    this->LUC = new LoadStage(config->children["LoadStage"], "C", ioDir, perf);
    this->LUC->connectRU(this->RUC);

    this->RUG = new ReserveStage(config->children["ReserveStage"], "G", ioDir, perf);
    this->RUG->connect(this->DU);
    this->CUG = new ComputeStage(config->children["ComputeStage"], "G", ioDir, counts[2]);
    this->CUG->connect(this->RUG, this->FU);
    this->LUG = new LoadStage(config->children["LoadStage"], "G", ioDir, perf);
    this->LUG->connectRU(this->RUG);

    this->RUT = new ReserveStage(config->children["ReserveStage"], "T", ioDir, perf);
    this->RUT->connect(this->DU);
    this->CUT = new ComputeStage(config->children["ComputeStage"], "T", ioDir, counts[3]);
    this->CUT->connect(this->RUT, this->FU);
    this->LUT = new LoadStage(config->children["LoadStage"], "T", ioDir, perf);
    this->LUT->connectRU(this->RUT);

    this->SU = new StoreStage(config->children["StoreStage"]);
    this->SU->connectDU(this->DU);

    this->halted = false;
    this->cyclecnt = 0; 
}

void Core::connect(SeedMemory * sdmem, map<char, OccMemory*> ocmem, SiMemory * simem) {
    this->FU->connectDRAM(sdmem);

    this->LUA->connectDRAM(ocmem['A']);
    this->LUC->connectDRAM(ocmem['C']);
    this->LUG->connectDRAM(ocmem['G']);
    this->LUT->connectDRAM(ocmem['T']);

    this->SU->connectDRAM(simem);
}

bool Core::allStagesHalted() {
    return this->FU->isHalted() && this->DU->isHalted() && this->SU->isHalted() && 
           this->RUA->isHalted() && this->RUC->isHalted() && this->RUG->isHalted() && this->RUT->isHalted() &&
           this->CUA->isHalted() && this->CUC->isHalted() && this->CUG->isHalted() && this->CUT->isHalted() &&
           this->LUA->isHalted() && this->LUC->isHalted() && this->LUG->isHalted() && this->LUT->isHalted();
}

void Core::step() {
    if (!this->halted) {
        this->SU->step();

        this->CUA->step();
        this->CUC->step();
        this->CUG->step();
        this->CUT->step();
        
        this->LUA->step();
        this->LUC->step();
        this->LUG->step();
        this->LUT->step();

        this->RUA->step();
        this->RUC->step();
        this->RUG->step();
        this->RUT->step();

        this->DU->step();

        this->FU->step();
        
        this->cyclecnt++;
    }

    if (this->allStagesHalted()) // || this->cyclecnt == 1000)
        this->halted = true;

    // this->halted = true;
}

uint64_t Core::getCycleCount() {
    return this->cyclecnt;
}

#endif