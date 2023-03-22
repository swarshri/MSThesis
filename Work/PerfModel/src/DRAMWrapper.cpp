#include <DRAMWrapper.h>

#ifndef DRAMW_DEF
#define DRAMW_DEF

template <typename AddressType, typename DataType>
DRAMW<AddressType, DataType>::DRAMW(string name, string ioDir, SysConfig * config, bool readonly) {
    this->id = name;
    this->readonly = readonly;
    this->dataIODir = ioDir;

    this->addressibility = config->parameters["Addressibility"];
    this->channelwidth = config->parameters["ChannelWidth"];
    this->latencymin = config->parameters["LatencyMin"];
    this->latencymax = config->parameters["LatencyMax"];
    this->memsize = pow(2, config->parameters["AddressLength"]);
    this->dramsim3configfile = config->str_parameters["DramSim3ConfigFile"];

    this->MEM.resize(this->memsize);
    int i = 0;
    while(i < this->memsize)
        this->MEM[i++] = DataType(0);

    this->lastReadData.resize(this->channelwidth);
    this->nextWriteData.resize(this->channelwidth);

    this->nextReadAddress = AddressType(0);
    this->readWaitCycles = 0;
    this->readPending = false;

    this->nextWriteAddress = AddressType(0);
    this->writeWaitCycles = 0;
    this->writePending = false;

    string output_dir = "";
    this->MemSystem = new MemorySystem(this->dramsim3configfile, output_dir,
                                       bind(&DRAMW::ReadCompleteHandler, this, std::placeholders::_1),
                                       bind(&DRAMW::WriteCompleteHandler, this, std::placeholders::_1));
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::printStats() { 
    this->MemSystem->PrintStats();
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::input() {
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
            this->MEM[i] = DataType(line);
            i++;
        }
        mem.close();
    }
    else cout<<"Unable to open input file for " << this->id << ": " << ipFilePath << endl;
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::output() {
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
		for (DataType data: this->MEM)
			mem << data <<endl;
	}
	else cout<<"Unable to open input file for " << this->id << ": " << opFilePath << endl;
	mem.close();
}

template <typename AddressType, typename DataType>
bool DRAMW<AddressType, DataType>::readRequest(AddressType address, uint32_t requestid) {
    uint64_t address64 = address.to_ullong();
    if (this->MemSystem->WillAcceptTransaction(address64, false)) {
        bool success = this->MemSystem->AddTransaction(address64, false);
        if (success) {
            // Only keep track of successfully requested transaction.
            // Otherwise just send false and the core will take care of what to do next.
            PMAEntry<DataType> * newma = new PMAEntry<DataType>;
            newma->AccessAddress = address64;
            newma->RequestCoreClock = this->clk;
            newma->DoneCoreClock = -1;
            newma->Data.clear();
            newma->Data.push_back(DataType(0));
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

template <typename AddressType, typename DataType>
bool DRAMW<AddressType, DataType>::writeRequest(AddressType address, vector<DataType> data) {
    uint64_t address64 = address.to_ullong();
    if (this->MemSystem->WillAcceptTransaction(address64, true)) {
        bool success = this->MemSystem->AddTransaction(address64, true);
        if (success) {
            // Only keep track of successfully requested transaction.
            // Otherwise just send false and the core will take care of what to do next.
            PMAEntry<DataType> * newma = new PMAEntry<DataType>;
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

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::ReadCompleteHandler(uint64_t address) {
    for (auto read = this->pendingReads.begin(); read != this->pendingReads.end(); read++) {
        if (address == read->AccessAddress && read->DoneCoreClock == -1 && read->DestID != -1) {
            // This is the read request entry in the list of pending reads that has returned.
            read->DoneCoreClock = this->clk;
            read->Data = this->MEM[address]; // this is what should be returned to core (Read back by core in this case)
            cout << "Finished read scheduled in clock cycle: " << read->RequestCoreClock << " at address: " << read->AccessAddress;
            cout << " in clock cycle: " << read->DoneCoreClock << endl;
            break;
        }
    }
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::WriteCompleteHandler(uint64_t address) {
    for (auto write = this->pendingWrites.begin(); write != this->pendingWrites.end(); write++) {
        if (address == write->AccessAddress && write->DoneCoreClock != -1) {
            write->DoneCoreClock = this->clk;
            cout << "Finished write scheduled in clock cycle: " << write->RequestCoreClock << " at address: " << write->AccessAddress;
            cout << " in clock cycle: " << write->DoneCoreClock << endl;
            break;
        }
    }
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::step() {
    // Remove the entry from the read pending list if (its done clock is not -1 and is more than the start clock) and 
    // their RequestID is set to -1 - indicating the writeback is done. - not necessary done in readHandler


    // Remove entries from the write pending list if (their done clock is not -1 and is more than the start clock).

    // TODO Question - can I do both in the same clock cycle or only one at a time???
}

template <typename AddressType, typename DataType>
pair<bool, vector<PMAEntry<DataType>>> DRAMW<AddressType, DataType>::getNextWriteBack() {
    vector<PMAEntry<DataType>> returnVec;
    for (auto entry : this->pendingReads) {
        if (entry->DoneCoreClock != -1 && entry->RequestID != -1) {
            returnVec.push_back(*entry);
            //this->pendingReads.erase(entry);
            remove(this->pendingReads.begin(), this->pendingReads.end(), entry);
        }
    }
    if (returnVec.size() > 0)
        return pair<bool, vector<PMAEntry<DataType>>>(true, returnVec);
    else
        return pair<bool, vector<PMAEntry<DataType>>>(false, returnVec);
}

template <typename AddressType, typename DataType>
bool DRAMW<AddressType, DataType>::isFree() {
    return false;
}

template <typename AddressType, typename DataType>
int DRAMW<AddressType, DataType>::getChannelWidth() {
    return 4;
}

#endif