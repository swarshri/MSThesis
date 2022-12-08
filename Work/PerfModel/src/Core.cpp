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
};

void Core::connect(DRAM * sdmem, DRAM * ocmem) {
    this->FU->connect(sdmem);
    this->OCMEM = ocmem;
}

void Core::step() {
    this->FU->step();
}