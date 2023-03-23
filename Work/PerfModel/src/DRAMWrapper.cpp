#include <DRAMWrapper.h>

#ifndef DRAMW_DEF
#define DRAMW_DEF

template <int alen, int dlen>
DRAMW<alen, dlen>::DRAMW(string name, string ioDir, SysConfig * config, SysConfig * coreConfig, bool readonly) {
    this->id = name;
    this->readonly = readonly;
    this->dataIODir = ioDir;

    this->addressibility = config->parameters["Addressibility"];
    this->channelwidth = config->parameters["ChannelWidth"];
    this->memsize = pow(2, config->parameters["AddressLength"]);
    this->dramsim3configfile = config->str_parameters["DramSim3ConfigFile"];
    cout << "DRAMWrapper " << this->id << " - dramsim3configfile: " << this->dramsim3configfile << endl;

    this->MEM.resize(this->memsize);
    int i = 0;
    while(i < this->memsize)
        this->MEM[i++] = bitset<dlen>(0);

    this->lastReadData.resize(this->channelwidth);
    this->nextWriteData.resize(this->channelwidth);

    this->nextReadAddress = bitset<alen>(0);
    this->readWaitCycles = 0;
    this->readPending = false;

    this->nextWriteAddress = bitset<alen>(0);
    this->writeWaitCycles = 0;
    this->writePending = false;

    string output_dir = "";
    this->MemSystem = new MemorySystem(this->dramsim3configfile, output_dir,
                                       bind(&DRAMW::ReadCompleteHandler, this, std::placeholders::_1),
                                       bind(&DRAMW::WriteCompleteHandler, this, std::placeholders::_1));

    double tdram = this->MemSystem->GetTCK(); // assuming that this is in ns
    double tcore = coreConfig->parameters["ClockCycleTime"]; // in ns
    if (tcore > tdram) {
        cout << "Core clock is slower than the DRAM clock. Tcore: " << tcore << " Tdram: " << tdram << endl;
        cout << "DRAM will be ticked for every cycle of core." << endl;
        this->memSysClockTriggerConst = 1;
    }
    else
        this->memSysClockTriggerConst = ceil(tdram / tcore);

    this->clkTriggerCount = 0;

    cout << "DRAM " << this->id << " - memSysClockTriggerCycle: " << this->memSysClockTriggerCycle << endl;
}

template <int alen, int dlen>
void DRAMW<alen, dlen>::printStats() { 
    this->MemSystem->PrintStats();
}

template <int alen, int dlen>
void DRAMW<alen, dlen>::input() {
    ifstream mem;
    string line;

#ifdef _WIN32
    string filename = "\\mem\\" + this->id;
#else
    string filename = "mem/" + this->id;
#endif
    filename = filename + ".mem";

    string ipFilePath = this->dataIODir + filename;
    mem.open(ipFilePath);

    if (mem.is_open()) {
        cout << "File opened: " << ipFilePath << endl;
        int i=0;
        while (getline(mem, line)) {
            this->MEM[i] = bitset<dlen>(line);
            i++;
        }
        mem.close();
    }
    else cout<<"Unable to open input file for " << this->id << ": " << ipFilePath << endl;
}

template <int alen, int dlen>
void DRAMW<alen, dlen>::output() {
    ofstream mem;

#ifdef _WIN32
    string filename = "\\mem\\" + this->id;
#else
    string filename = "mem/" + this->id;
#endif
    filename = filename + ".mem";

    string opFilePath = this->dataIODir + filename;
	mem.open(opFilePath, std::ios_base::out);
	if (mem.is_open()) {
        cout << "File opened: " << opFilePath << endl;
		for (bitset<dlen> data: this->MEM)
			mem << data <<endl;
	}
	else cout<<"Unable to open input file for " << this->id << ": " << opFilePath << endl;
	mem.close();
}

template <int alen, int dlen>
bool DRAMW<alen, dlen>::readRequest(bitset<alen> address, uint32_t requestid) {
    uint64_t address64 = address.to_ullong();
    if (this->MemSystem->WillAcceptTransaction(address64, false)) {
        bool success = this->MemSystem->AddTransaction(address64, false);
        if (success) {
            // Only keep track of successfully requested transaction.
            // Otherwise just send false and the core will take care of what to do next.
            PMAEntry<dlen> * newma = new PMAEntry<dlen>;
            newma->AccessAddress = address64;
            newma->RequestCoreClock = this->clk;
            newma->DoneCoreClock = -1;
            newma->Data.clear();
            newma->Data.push_back(bitset<dlen>(0));
            newma->RequestID = requestid;
            this->pendingReads.push_back(newma);
            // TODO - think if you want to have a single list of pending reads and writes.
            // I don't think that will make any difference at least at present for this design.
            cout << this->id << " - Scheduled Read in cycle: " << this->clk << " from address: " << address64 << endl;
        }
        return success;
    }
    return false;
}

template <int alen, int dlen>
bool DRAMW<alen, dlen>::writeRequest(bitset<alen> address, vector<bitset<dlen>> data) {
    uint64_t address64 = address.to_ullong();
    if (this->MemSystem->WillAcceptTransaction(address64, true)) {
        bool success = this->MemSystem->AddTransaction(address64, true);
        if (success) {
            // Only keep track of successfully requested transaction.
            // Otherwise just send false and the core will take care of what to do next.
            PMAEntry<dlen> * newma = new PMAEntry<dlen>;
            newma->AccessAddress = address64;
            newma->RequestCoreClock = this->clk;
            newma->DoneCoreClock = -1;
            newma->Data = data;
            newma->RequestID = -1; // Nothing to write back.
            this->pendingWrites.push_back(newma);
            // TODO - think if you want to have a single list of pending reads and writes.
            // I don't think that will make any difference at least at present for this design.
            cout << this->id << " - Scheduled Write in cycle: " << this->clk << " from address: " << address64 << endl;
        }
        return success;
    }
    return false;
}

template <int alen, int dlen>
void DRAMW<alen, dlen>::ReadCompleteHandler(uint64_t address) {
    for (auto read : this->pendingReads) {
        if (address == read->AccessAddress && read->DoneCoreClock == -1 && read->RequestID != -1) {
            // This is the read request entry in the list of pending reads that has returned.
            read->DoneCoreClock = this->clk;
            read->Data.clear(); // this is what should be returned to core (Read back by core in this case)
            read->Data.push_back(this->MEM[address]); // this is what should be returned to core (Read back by core in this case)
            cout << "Finished read scheduled in clock cycle: " << read->RequestCoreClock << " at address: " << read->AccessAddress;
            cout << " in clock cycle: " << read->DoneCoreClock << endl;
            break;
        }
    }
}

template <int alen, int dlen>
void DRAMW<alen, dlen>::WriteCompleteHandler(uint64_t address) {
    for (auto write : this->pendingWrites) {
        if (address == write->AccessAddress && write->DoneCoreClock != -1) {
            write->DoneCoreClock = this->clk;
            cout << "Finished write scheduled in clock cycle: " << write->RequestCoreClock << " at address: " << write->AccessAddress;
            cout << " in clock cycle: " << write->DoneCoreClock << endl;
            break;
        }
    }
}

template <int alen, int dlen>
void DRAMW<alen, dlen>::step() {
    // Remove the entry from the read pending list if (its done clock is not -1 and is more than the start clock) and 
    // their RequestID is set to -1 - indicating the writeback is done.
    // Remove entries from the write pending list if (their done clock is not -1 and is more than the start clock).
    // - not necessary done in readHandler

    // step the memory system relative to the core clock cycle.
    this->clkTriggerCount++;
    if (this->clkTriggerCount == this->memSysClockTriggerConst) {
        this->MemSystem->ClockTick();
        this->clkTriggerCount = 0;
    }
}

template <int alen, int dlen>
pair<bool, vector<PMAEntry<dlen>>> DRAMW<alen, dlen>::getNextWriteBack() {
    vector<PMAEntry<dlen>> returnVec;
    for (auto entry : this->pendingReads) {
        if (entry->DoneCoreClock != -1 && entry->RequestID != -1) {
            returnVec.push_back(*entry);
            //this->pendingReads.erase(entry);
            remove(this->pendingReads.begin(), this->pendingReads.end(), entry);
        }
    }
    if (returnVec.size() > 0)
        return pair<bool, vector<PMAEntry<dlen>>>(true, returnVec);
    else
        return pair<bool, vector<PMAEntry<dlen>>>(false, returnVec);
}

template <int alen, int dlen>
bool DRAMW<alen, dlen>::isFree() {
    return false;
}

template <int alen, int dlen>
int DRAMW<alen, dlen>::getChannelWidth() {
    return 4;
}

#endif