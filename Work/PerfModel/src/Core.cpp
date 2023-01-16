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

    vector<bitset<64>> coremem;
    if (mem.is_open()) {
        while (getline(mem, line)) {
            line = line.substr(0, line.find(' '));
            coremem.push_back(bitset<64>(line));
        }
        mem.close();

        this->RefCountReg       = bitset<32>(coremem[0].to_ulong());
        this->CountReg['A']     = coremem[1];
        this->CountReg['C']     = coremem[2];
        this->CountReg['G']     = coremem[3];
        this->CountReg['T']     = coremem[4];
        this->OccFirstReg['A']  = bitset<64>(0);
        this->OccFirstReg['C']  = bitset<64>(0);
        this->OccFirstReg['G']  = bitset<64>(0);
        this->OccFirstReg['T']  = bitset<64>(0);
        this->OccLastReg['A']   = coremem[5];
        this->OccLastReg['C']   = coremem[6];
        this->OccLastReg['G']   = coremem[7];
        this->OccLastReg['T']   = coremem[8];
    }
    else cout<<"Unable to open input file for Core " << this->id << endl;

    this->FU = new FetchUnit(config->children["FetchUnit"], this->RefCountReg);

    this->DU = new DispatchUnit(config->children["DispatchUnit"]);
    this->DU->connect(this->FU);

    this->RUA = new ReserveUnit(config->children["ReserveUnitPipelineA"], &coremem);
    this->RUA->connect(this->DU);
    this->CUA = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUA->connect(this->RUA, this->FU);

    this->RUC = new ReserveUnit(config->children["ReserveUnitPipelineC"], &coremem);
    this->RUC->connect(this->DU);
    this->CUC = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUC->connect(this->RUC, this->FU);

    this->RUG = new ReserveUnit(config->children["ReserveUnitPipelineG"], &coremem);
    this->RUG->connect(this->DU);
    this->CUG = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUG->connect(this->RUG, this->FU);

    this->RUT = new ReserveUnit(config->children["ReserveUnitPipelineT"], &coremem);
    this->RUT->connect(this->DU);
    this->CUT = new ComputeUnit(config->children["ComputeUnit"]);
    this->CUT->connect(this->RUT, this->FU);

    this->halted = false;
}

void Core::connect(DRAM<bitset<32>, bitset<64>> * sdmem, DRAM<bitset<32>, bitset<64>> * ocmem) {
    this->FU->connect(sdmem);

    this->OCMEM = ocmem;
}

void Core::step() {
    if (!this->halted) {
        this->CUA->step();
        this->CUC->step();
        this->CUG->step();
        this->CUT->step();

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