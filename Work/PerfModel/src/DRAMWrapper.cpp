#include <DRAMWrapper.h>

#ifndef DRAMW_DEF
#define DRAMW_DEF

template<class dtype>
DRAMW<dtype>::DRAMW(string name, string ioDir, SysConfig * config, SysConfig * coreConfig, bool readonly) {
    this->id = name;
    this->readonly = readonly;
    this->dataIODir = ioDir;
    this->clk = 0;

    this->addressibility = config->parameters["Addressibility"];
    this->channelwidth = config->parameters["ChannelWidth"];
    this->memsize = pow(2, config->parameters["AddressLength"]);
    this->dramsim3configfile = config->str_parameters["DramSim3ConfigFile"];
    cout << "DRAMWrapper " << this->id << " - dramsim3configfile: " << this->dramsim3configfile << endl;

    this->lastReadData.resize(this->channelwidth);
    this->nextWriteData.resize(this->channelwidth);

    this->nextReadAddress = 0;
    this->readWaitCycles = 0;
    this->readPending = false;

    this->nextWriteAddress = 0;
    this->writeWaitCycles = 0;
    this->writePending = false;

    string output_dir = "";
    this->MemSystem = new MemorySystem(this->dramsim3configfile, output_dir,
                                       bind(&DRAMW::ReadCompleteHandler, this, std::placeholders::_1),
                                       bind(&DRAMW::WriteCompleteHandler, this, std::placeholders::_1));

    cout << "Done initializing MemSystem." << endl;
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

    cout << "DRAM " << this->id << " - memSysClockTriggerConst: " << this->memSysClockTriggerConst << endl;
}

template<class dtype>
void DRAMW<dtype>::printStats() { 
    this->MemSystem->PrintStats();
}

template<class dtype>
void DRAMW<dtype>::allocate() { 
    this->MEM.resize(this->memsize);
    int i = 0;
    while(i < this->memsize)
        this->MEM[i++] = 0;
}

template<class dtype>
void DRAMW<dtype>::input() {
    if (this->MEM.size() == 0)
        this->allocate();
        
    ifstream mem;
    string line;

#ifdef _WIN32
    string filename = "\\mem\\" + this->id;
#else
    string filename = "/mem/" + this->id;
#endif
    filename = filename + ".mem";

    string ipFilePath = this->dataIODir + filename;
    mem.open(ipFilePath);

    if (mem.is_open()) {
        cout << this->id << " - File opened: " << ipFilePath << endl;
        int i=0;
        while (getline(mem, line)) {
            this->MEM[i] = stoull(line);
            i++;
        }
        mem.close();
    }
    else cout << this->id << " - Unable to open input file: " << ipFilePath << endl;
}

template<class dtype>
void DRAMW<dtype>::output() {
    ofstream mem;

#ifdef _WIN32
    string filename = "\\mem\\" + this->id;
#else
    string filename = "/mem/" + this->id;
#endif
    filename = filename + ".mem";

    string opFilePath = this->dataIODir + filename;
	mem.open(opFilePath, std::ios_base::out);
	if (mem.is_open()) {
        cout << this->id << " - File opened: " << opFilePath << endl;
		for (dtype data: this->MEM)
			mem << data << endl;
	}
	else cout << this->id << " - Unable to open output file: " << opFilePath << endl;
	mem.close();
}

template<class dtype>
bool DRAMW<dtype>::willAcceptRequest(uint64_t address, bool write) {
    return this->MemSystem->WillAcceptTransaction(address, write);
}

template<class dtype>
bool DRAMW<dtype>::readRequest(uint64_t address, int requestid, bool burstmode) {
    if (this->MemSystem->WillAcceptTransaction(address, false)) {
        bool success = this->MemSystem->AddTransaction(address, false);
        if (success) {
            // Only keep track of successfully requested transaction.
            // Otherwise just send false and the core will take care of what to do next.
            PMAEntry<dtype> * newma = new PMAEntry<dtype>;
            newma->AccessAddress = address;
            newma->RequestCoreClock = this->clk;
            newma->DoneCoreClock = -1;
            newma->RequestID = requestid;
            newma->BurstMode = burstmode; // false by default.
            this->pendingReads.push_back(newma);
            // TODO - think if you want to have a single list of pending reads and writes.
            // I don't think that will make any difference at least at present for this design.
            cout << this->id << " - Scheduled Read in cycle: " << this->clk << " from address: " << address << endl;
        }
        return success;
    }
    return false;
}

template<class dtype>
bool DRAMW<dtype>::writeRequest(uint64_t address, vector<dtype> data, bool burstmode) {
    if (this->MemSystem->WillAcceptTransaction(address, true)) {
        bool success = this->MemSystem->AddTransaction(address, true);
        if (success) {
            // Only keep track of successfully requested transaction.
            // Otherwise just send false and the core will take care of what to do next.
            PMAEntry<dtype> * newma = new PMAEntry<dtype>;
            newma->AccessAddress = address;
            newma->RequestCoreClock = this->clk;
            newma->DoneCoreClock = -1;
            newma->Data = data;
            newma->RequestID = -1; // Nothing to write back.
            newma->BurstMode = burstmode; // false by default.
            this->pendingWrites.push_back(newma);
            // TODO - think if you want to have a single list of pending reads and writes.
            // I don't think that will make any difference at least at present for this design.
            cout << this->id << " - Scheduled Write into address: " << address << " in cycle: " << this->clk << endl;
            cout << this->id << " - Pending writes size: " << this->pendingWrites.size() << endl;
        }
        return success;
    }
    return false;
}

template<class dtype>
void DRAMW<dtype>::ReadCompleteHandler(uint64_t address) {
    cout << this->id << " - Called ReadCompleteHandler on address: " << address << endl;
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
            for (int i = 0; i < bl; i++)
                read->Data.push_back(this->MEM[address + i]); // this is what should be returned to core (Read back by core in this case)
            // TODO - This needs to be changed to return one word of data every DRAM cycle till we reach burst length.
            cout << this->id << " - Finished read scheduled in clock cycle: " << read->RequestCoreClock << " at address: " << read->AccessAddress;
            cout << " in clock cycle: " << read->DoneCoreClock << endl;
            cout << " pending reads count: " << this->pendingReads.size() << endl;
            break;
        }
    }
}

template<class dtype>
void DRAMW<dtype>::WriteCompleteHandler(uint64_t address) {
    cout << this->id << " - Calling WriteCompleteHandler on address: " << address << endl;
    cout << this->id << " - Pending writes size: " << this->pendingWrites.size() << endl;
    for (int i = 0; i < this->pendingWrites.size(); i++) {
        auto write = this->pendingWrites[i];
        cout << "Write accessaddress: " << write->AccessAddress << endl;
        cout << "Write DoneCoreClock: " << write->DoneCoreClock << endl;
        if (address == write->AccessAddress && write->DoneCoreClock == -1) {
            int bl = 1;
            if (write->BurstMode)
                bl = this->MemSystem->GetBurstLength();
            cout << this->id << " - Burst Length: " << bl << endl;
            cout << this->id << " - this->MEM.size(): " << this->MEM.size() << endl;
            cout << this->id << " - write->Data.size(): " << write->Data.size() << endl;
            for (int i = 0; i < bl; i++)
                this->MEM[address + i] = write->Data[i];
            write->DoneCoreClock = this->clk;
            cout << this->id << " - Finished write scheduled in clock cycle: " << write->RequestCoreClock << " at address: " << write->AccessAddress;
            cout << " in clock cycle: " << write->DoneCoreClock << endl;
            this->pendingWrites.erase(this->pendingWrites.begin() + i); 
            // This is done here because there is no writeback required for writes. This is done in the nextWriteBack() function for reads.
            cout << " pending writes count: " << this->pendingWrites.size() << endl;
            break;
        }
    }
}

template<class dtype>
void DRAMW<dtype>::step() {
    // Remove the entry from the read pending list if (its done clock is not -1 and is more than the start clock) and 
    // their RequestID is set to -1 - indicating the writeback is done.
    // Remove entries from the write pending list if (their done clock is not -1 and is more than the start clock).
    // - not necessary done in readHandler
    this->clk++;
    // step the memory system relative to the core clock cycle.
    this->clkTriggerCount++;
    if (this->clkTriggerCount == this->memSysClockTriggerConst) {
        this->MemSystem->ClockTick();
        this->clkTriggerCount = 0;
    }
}

template<class dtype>
pair<bool, vector<PMAEntry<dtype>>> DRAMW<dtype>::getNextWriteBack() {
    cout << this->id << " - Getting nextWriteBack this->pendingReads.size():" << this->pendingReads.size() << endl;
    vector<PMAEntry<dtype>> returnVec;
    for (int i = 0; i < this->pendingReads.size(); i++) {
        auto entry = this->pendingReads[i];
        // cout << "entry at: " << i << ": " << *entry << endl;
        if (entry->DoneCoreClock != -1 && entry->RequestID != -1) {
            returnVec.push_back(*entry);
            this->pendingReads.erase(this->pendingReads.begin() + i);
            cout << this->id << " - After removing an entry this->pendingReads.size():" << this->pendingReads.size() << endl;
            break;
        }
    }
    if (returnVec.size() > 0)
        return pair<bool, vector<PMAEntry<dtype>>>(true, returnVec);
    else
        return pair<bool, vector<PMAEntry<dtype>>>(false, returnVec);
}

template<class dtype>
bool DRAMW<dtype>::isFree(bool write) {
    cout << this->id << " - isFree() pendingWrites size: " << this->pendingWrites.size() << endl;
    if (write)
        return this->pendingWrites.size() == 0;
    else
        return this->pendingReads.size() == 0;
}

template<class dtype>
int DRAMW<dtype>::getChannelWidth() {
    return this->MemSystem->GetBurstLength();
}

#endif