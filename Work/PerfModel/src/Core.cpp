#include<Core.h>

Core::Core(string id, string ioDir, SysConfig * config) {
    this->id = id;

    ifstream mem;
    string line;
    
#ifdef _WIN32
    string filename = "\\mem\\CoreReg.mem";
#else
    string filename = "mem/CoreReg.mem";
#endif
    
    mem.open(ioDir + filename);

    vector<bitset<32>> coremem;
    if (mem.is_open()) {
        while (getline(mem, line)) {
            line = line.substr(0, line.find(' '));
            coremem.push_back(bitset<32>(line));
        }
        mem.close();
    }
    else cout<<"Unable to open input file for Core " << this->id << ": " << ioDir + filename << endl;

    bitset<32> referenceCountVal = coremem[0];
    this->FU = new FetchStage(config->children["FetchStage"], ioDir, referenceCountVal);

    this->DU = new DispatchStage(config->children["DispatchStage"], ioDir);
    this->DU->connect(this->FU);

    this->RUA = new ReserveStage(config->children["ReserveStage"], 'A', ioDir);
    this->RUA->connect(this->DU);
    this->CUA = new ComputeStage(config->children["ComputeStage"], 'A', ioDir, coremem[1]);
    this->CUA->connect(this->RUA, this->FU);
    this->LUA = new LoadStage(config->children["LoadStage"], 'A', ioDir);
    this->LUA->connectRU(this->RUA);

    this->RUC = new ReserveStage(config->children["ReserveStage"], 'C', ioDir);
    this->RUC->connect(this->DU);
    this->CUC = new ComputeStage(config->children["ComputeStage"], 'C', ioDir, coremem[2]);
    this->CUC->connect(this->RUC, this->FU);
    this->LUC = new LoadStage(config->children["LoadStage"], 'C', ioDir);
    this->LUC->connectRU(this->RUC);

    this->RUG = new ReserveStage(config->children["ReserveStage"], 'G', ioDir);
    this->RUG->connect(this->DU);
    this->CUG = new ComputeStage(config->children["ComputeStage"], 'G', ioDir, coremem[3]);
    this->CUG->connect(this->RUG, this->FU);
    this->LUG = new LoadStage(config->children["LoadStage"], 'G', ioDir);
    this->LUG->connectRU(this->RUG);

    this->RUT = new ReserveStage(config->children["ReserveStage"], 'T', ioDir);
    this->RUT->connect(this->DU);
    this->CUT = new ComputeStage(config->children["ComputeStage"], 'T', ioDir, coremem[4]);
    this->CUT->connect(this->RUT, this->FU);
    this->LUT = new LoadStage(config->children["LoadStage"], 'T', ioDir);
    this->LUT->connectRU(this->RUT);

    this->SU = new StoreStage(config->children["StoreStage"]);
    this->SU->connectDU(this->DU);

    this->halted = false;
}

void Core::connect(DRAMW<32, 64> * sdmem, map<char, DRAMW<32, 32>*> ocmem, DRAMW<32, 64> * simem) {
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
    }

    if (this->allStagesHalted())
        this->halted = true;

    this->halted = true;
}