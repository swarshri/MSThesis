#include<Core.h>

Core::Core(string id, string ioDir, Config * config) {
    this->id = id;

    ifstream mem;
    string line;
    
#ifdef _WIN32
    string filename = "\\CoreReg.mem";
#else
    string filename = "/CoreReg.mem";
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
    else cout<<"Unable to open input file for Core " << this->id << endl;

    bitset<32> referenceCountVal = coremem[0];
    this->FU = new FetchUnit(config->children["FetchUnit"], referenceCountVal);

    this->DU = new DispatchUnit(config->children["DispatchUnit"]);
    this->DU->connect(this->FU);

    this->RUA = new ReserveUnit(config->children["ReserveUnitPipelineA"], &coremem);
    this->RUA->connect(this->DU);
    this->CUA = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUA->connect(this->RUA, this->FU);
    this->LUA = new LoadUnit(config->children["LoadUnit"]);
    this->LUA->connectRU(this->RUA);

    this->RUC = new ReserveUnit(config->children["ReserveUnitPipelineC"], &coremem);
    this->RUC->connect(this->DU);
    this->CUC = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUC->connect(this->RUC, this->FU);
    this->LUC = new LoadUnit(config->children["LoadUnit"]);
    this->LUC->connectRU(this->RUC);

    this->RUG = new ReserveUnit(config->children["ReserveUnitPipelineG"], &coremem);
    this->RUG->connect(this->DU);
    this->CUG = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUG->connect(this->RUG, this->FU);
    this->LUG = new LoadUnit(config->children["LoadUnit"]);
    this->LUG->connectRU(this->RUG);

    this->RUT = new ReserveUnit(config->children["ReserveUnitPipelineT"], &coremem);
    this->RUT->connect(this->DU);
    this->CUT = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUT->connect(this->RUT, this->FU);
    this->LUT = new LoadUnit(config->children["LoadUnit"]);
    this->LUT->connectRU(this->RUT);

    this->SU = new StoreUnit(config->children["StoreUnit"]);
    this->SU->connectDU(this->DU);

    this->halted = false;
}

void Core::connect(DRAM<bitset<32>, bitset<64>> * sdmem, map<char, DRAM<bitset<32>, bitset<32>>*> ocmem, DRAM<bitset<32>, bitset<64>> * simem) {
    this->FU->connect(sdmem);

    this->LUA->connectDRAM(ocmem['A']);
    this->LUC->connectDRAM(ocmem['C']);
    this->LUG->connectDRAM(ocmem['G']);
    this->LUT->connectDRAM(ocmem['T']);

    this->SU->connectDRAM(simem);
}

void Core::step() {
    if (!this->halted) {
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

    if (this->FU->halted && this->DU->halted)
        this->halted = true;
}