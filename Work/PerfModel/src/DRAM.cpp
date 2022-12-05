#include<DRAM.h>

using namespace std;

template <class dataType, class addressType>
DRAM<dataType, addressType>::DRAM(string name, Config *config, bool readonly) {
    this->id = name;
    this->readonly = readonly;

    this->addressibility = config->parameters["Addressibility"];
    this->channelwidth = config->parameters["ChannelWidth"];
    this->latencymin = config->parameters["LatencyMin"];
    this->latencymax = config->parameters["LatencyMax"];
    this->memsize = pow(2, config->parameters["AddressLength"]);
    this->Data.resize(this->memsize);
    memset(&this->Data[0], dataType(0), sizeof(this->Data[0]) * this->memsize);
}

template <class dataType, class addressType>
void DRAM<dataType, addressType>::load(string ioDir) {
    ifstream mem;
    string line;
    
    string filename = "\\" + this->id;
    filename = filename + ".mem";
    
    mem.open(ioDir + filename);

    if (mem.is_open()) {
        int i=0;
        while (getline(mem, line)) {
            this->Data.at(i) = dataType(line);
            i++;
        }
        mem.close();
    }
    else cout<<"Unable to open input file for " << this->id << endl;
}

template<class dataType, class addressType>
void DRAM<dataType, addressType>::Access(bool write, addressType address, dataType * data) {
    srand(time(NULL));
    int randomLatency = (rand % this->latencymax) + this->latencymin;
    if (write) {
        if (this->readonly)
            cout << "WARNING: Attempt to write into a read-only memory: " << this->id << endl;
        else {
            this->writeWaitCycles = randomLatency;
            this->nextWriteAddress = address;
            this->nextWriteData = data;
            this->writePending = true;
        }
    }
    else {
        this->readWaitCycles = randomLatency;
        this->nextReadAddress = address;
        this->readPending = true;
    }
}

template<class dataType, class addressType>
bool DRAM<dataType, addressType>::isFree() {
    return !(this->readPending || this->writePending);
}

template<class dataType, class addressType>
bool DRAM<dataType, addressType>::step() {
    if (this->readPending) {
        this->readWaitCycles--;
        if (this->readWaitCycles == 0) {
            this->ReadData = this->MEM[this->nextReadAddress];
            return true;
        }
    }
    else if (this->writePending) {
        this->writeWaitCycles--;
        if (this->writeWaitCycles == 0) {
            this->MEM[this->nextWriteAddress] = this->nextWriteData;
            return true;
        }
    }
    return false;
}