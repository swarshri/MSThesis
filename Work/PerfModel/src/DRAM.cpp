#include<DRAM.h>

using namespace std;

DRAM::DRAM(string name, Config *config, bool readonly) {
    this->id = name;
    this->readonly = readonly;

    this->addressibility = config->parameters["Addressibility"];
    this->channelwidth = config->parameters["ChannelWidth"];
    this->latencymin = config->parameters["LatencyMin"];
    this->latencymax = config->parameters["LatencyMax"];
    this->memsize = pow(2, config->parameters["AddressLength"]);
    this->MEM.resize(this->memsize);
}

void DRAM::load(string ioDir) {
    ifstream mem;
    string line;
    
    string filename = "\\" + this->id;
    filename = filename + ".mem";
    
    mem.open(ioDir + filename);

    if (mem.is_open()) {
        int i=0;
        while (getline(mem, line)) {
            this->MEM.at(i) = bitset<64>(line);
            i++;
        }
        mem.close();

        while(i < this->memsize) {
            this->MEM.at(i) = bitset<64>(0);
            i++;
        }
    }
    else cout<<"Unable to open input file for " << this->id << endl;
}

void DRAM::Access(bool write, bitset<32> address, bitset<64> data) {
    srand(time(NULL));
    int randomLatency = (rand() % this->latencymax) + this->latencymin;
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

bool DRAM::isFree() {
    return !(this->readPending || this->writePending);
}

bool DRAM::step() {
    if (this->readPending) {
        this->readWaitCycles--;
        if (this->readWaitCycles == 0) {
            this->ReadData = this->MEM[this->nextReadAddress.to_ulong()];
            return true;
        }
    }
    else if (this->writePending) {
        this->writeWaitCycles--;
        if (this->writeWaitCycles == 0) {
            this->MEM[this->nextWriteAddress.to_ulong()] = this->nextWriteData;
            return true;
        }
    }
    return false;
}