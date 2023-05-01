#include<Memory.h>

#ifndef MEM_DEF
#define MEM_DEF

SeedMemory::SeedMemory(string name, string ioDir, SysConfig * config, SysConfig * coreConfig)
    :DRAMW(name, ioDir, config, coreConfig, true) {

    this->MemSystem = new MemorySystem(this->dramsim3configfile, "",
                                       bind(&SeedMemory::ReadCompleteHandler, this, std::placeholders::_1),
                                       bind(&SeedMemory::WriteCompleteHandler, this, std::placeholders::_1));    
}

void SeedMemory::ReadCompleteHandler(uint64_t address) {
    cout << this->id << " - In seed memory ReadCompleteHandler." << endl;
    
    for (int i = 0; i < this->pendingReads.size(); i++) {
        auto read = this->pendingReads[i];
        if (address == read->AccessAddress && read->DoneCoreClock == -1 && read->RequestID != -1) {
            // This is the read request entry in the list of pending reads that has returned.
            read->DoneCoreClock = this->clk;
            read->Data.clear(); // this is what should be returned to core (Read back by core in this case)
            int bl = 1;
            if (read->BurstMode)
                bl = this->MemSystem->GetBurstLength();
            cout << this->id << " - Burst Length: " << bl << endl;
            for (int i = 0; i < bl; i++) {                
                string seed = this->READS->get_seed(address + i);
                cout << this->id << " - Seed at address: " << address + i << ": " << seed << endl;
                if (seed != "EOS")
                    seed = "E" + seed;
                string filler(21 - seed.size(), '0'); 
                read->Data.push_back(filler + seed); // this is what should be returned to core (Read back by core in this case)
            }
            // TODO - This needs to be changed to return one word of data every DRAM cycle till we reach burst length.
            cout << this->id << " - Finished read scheduled in clock cycle: " << read->RequestCoreClock << " at address: " << read->AccessAddress;
            cout << " in clock cycle: " << read->DoneCoreClock << endl;
            cout << " pending reads count: " << this->pendingReads.size() << endl;
            break;
        }
    }
}

void SeedMemory::WriteCompleteHandler(uint64_t address) {
    cout << this->id << " - In seed memory WriteCompleteHandler." << endl;
    cout << this->id << "Invalid operation." << endl;
}

void SeedMemory::input(Reads * reads) {
    cout << this->id << " - In seed memory input function." << endl;
    this->READS = reads;
    this->READS->make_seeds(20);
}

OccMemory::OccMemory(string base, string ioDir, SysConfig * config, SysConfig * coreConfig)
    :DRAMW("Occ"+base+"MEM", ioDir, config, coreConfig, true) {

    this->base = base;
    cout << this->id << " - OM: this->base value: " << this->base << endl;
    this->MemSystem = new MemorySystem(this->dramsim3configfile, "",
                                       bind(&OccMemory::ReadCompleteHandler, this, std::placeholders::_1),
                                       bind(&OccMemory::WriteCompleteHandler, this, std::placeholders::_1));    
}

void OccMemory::ReadCompleteHandler(uint64_t address) {
    cout << this->id << " - In occ memory ReadCompleteHandler." << endl;    

    for (int i = 0; i < this->pendingReads.size(); i++) {
        auto read = this->pendingReads[i];
        if (address == read->AccessAddress && read->DoneCoreClock == -1 && read->RequestID != -1) {
            // This is the read request entry in the list of pending reads that has returned.
            read->DoneCoreClock = this->clk;
            read->Data.clear(); // this is what should be returned to core (Read back by core in this case)
            int bl = 1;
            if (read->BurstMode)
                bl = this->MemSystem->GetBurstLength();
            cout << this->id << " - Burst Length: " << bl << endl;
            for (int i = 0; i < bl; i++) {
                uint64_t occ = this->REF->getOcc(this->base[0], address);
                cout << this->id << " - Returned Occ Value at address: " << address << ": " << occ << endl;
                read->Data.push_back(occ); // this is what should be returned to core (Read back by core in this case)
            }
            // TODO - This needs to be changed to return one word of data every DRAM cycle till we reach burst length.
            cout << this->id << " - Finished read scheduled in clock cycle: " << read->RequestCoreClock << " at address: " << read->AccessAddress;
            cout << " in clock cycle: " << read->DoneCoreClock << endl;
            cout << " pending reads count: " << this->pendingReads.size() << endl;
            break;
        }
    }
}

void OccMemory::WriteCompleteHandler(uint64_t address) {
    cout << this->id << " - In occ memory WriteCompleteHandler." << endl;
    cout << this->id << "Invalid operation." << endl;
}

void OccMemory::input(Reference * ref) {
    cout << this->id << " - In occ memory input function." << endl;
    this->REF = ref;
}

SiMemory::SiMemory(string id, string ioDir, SysConfig * config, SysConfig * coreConfig)
    :DRAMW(id, ioDir, config, coreConfig, false) {
    this->MEM.resize(this->memsize);
    int i = 0;
    while(i < this->memsize)
        this->MEM[i++] = {0, 0};
}

void SiMemory::output() {
    ofstream mem;

#ifdef _WIN32
    string filename = "\\" + this->id + ".mem";
#else
    string filename = "/" + this->id + ".mem";
#endif

    string opFilePath = this->dataIODir + filename;
	mem.open(opFilePath, std::ios_base::out);
	if (mem.is_open()) {
        cout << this->id << " - File opened: " << opFilePath << endl;
		for (vector<uint64_t> data: this->MEM) {
            string line = to_string(data[0]) + "\t" + to_string(data[1]);
			mem << line << endl;
        }
	}
	else cout << this->id << " - Unable to open output file: " << opFilePath << endl;
	mem.close();
}

#endif