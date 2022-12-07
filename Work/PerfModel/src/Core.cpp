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
        while (getline(mem, line))
            coremem.push_back(bitset<64>(line));
        mem.close();

        this->RefCountReg       = coremem[0];
        this->CountReg['A']     = coremem[1];
        this->CountReg['C']     = coremem[2];
        this->CountReg['G']     = coremem[3];
        this->CountReg['T']     = coremem[4];
        this->OccFirstReg['A']  = coremem[5];
        this->OccFirstReg['C']  = coremem[6];
        this->OccFirstReg['G']  = coremem[7];
        this->OccFirstReg['T']  = coremem[8];
        this->OccLastReg['A']   = coremem[9];
        this->OccLastReg['C']   = coremem[10];
        this->OccLastReg['G']   = coremem[11];
        this->OccLastReg['T']   = coremem[12];
    }
    else cout<<"Unable to open input file for Core " << this->id << endl;

    this->FU = new FetchUnit(config->children["FetchUnit"]);
};

void Core::connect(DRAM * sdmem, DRAM * ocmem) {
    this->SDMEM = sdmem;
    this->OCMEM = ocmem;
}