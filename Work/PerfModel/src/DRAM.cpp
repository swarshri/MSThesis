#include<DRAM.h>

using namespace std;

#ifndef DRAM_DEF
#define DRAM_DEF

template <typename AddressType, typename DataType>
DRAM<AddressType, DataType>::DRAM(string name, Config *config, bool readonly) {
    this->id = name;
    this->readonly = readonly;

    this->addressibility = config->parameters["Addressibility"];
    this->channelwidth = config->parameters["ChannelWidth"];
    this->latencymin = config->parameters["LatencyMin"];
    this->latencymax = config->parameters["LatencyMax"];
    this->memsize = pow(2, config->parameters["AddressLength"]);
    this->MEM.resize(this->memsize);

    this->lastReadData.resize(this->channelwidth);
    this->nextWriteData.resize(this->channelwidth);

    this->nextReadAddress = AddressType(0);
    this->readWaitCycles = 0;
    this->readPending = false;

    this->nextWriteAddress = AddressType(0);
    this->writeWaitCycles = 0;
    this->writePending = false;
}

template <typename AddressType, typename DataType>
void DRAM<AddressType, DataType>::input(string ioDir) {
    ifstream mem;
    string line;

#ifdef _WIN32
    string filename = "\\mem\\" + this->id;
#else
    string filename = "mem/" + this->id;
#endif
    filename = filename + ".mem";

    string ipFilePath = ioDir + filename;
    mem.open(ipFilePath);

    if (mem.is_open()) {
        cout << "File opened: " << ipFilePath << endl;
        int i=0;
        while (getline(mem, line)) {
            this->MEM[i] = DataType(line);
            i++;
        }
        mem.close();

        while(i < this->memsize) {
            this->MEM[i] = DataType(0);
            i++;
        }
    }
    else cout<<"Unable to open input file for " << this->id << ": " << ipFilePath << endl;
}

template <typename AddressType, typename DataType>
void DRAM<AddressType, DataType>::output(string ioDir) {
    ofstream mem;

#ifdef _WIN32
    string filename = "\\mem\\" + this->id;
#else
    string filename = "mem/" + this->id;
#endif
    filename = filename + ".mem";

    string opFilePath = ioDir + filename;
	mem.open(opFilePath, std::ios_base::trunc);
	if (mem.is_open()) {
        cout << "File opened: " << opFilePath << endl;
		for (DataType data: this->MEM)
			mem << data <<endl;
	}
	else cout<<"Unable to open input file for " << this->id << ": " << opFilePath << endl;
	mem.close();
}

template <typename AddressType, typename DataType>
void DRAM<AddressType, DataType>::readAccess(AddressType address) {
    srand(time(NULL));
    int randomLatency = (rand() % (this->latencymax - this->latencymin)) + this->latencymin;
    this->readWaitCycles = 5; // randomLatency;
    this->nextReadAddress = address;
    this->readPending = true;
    this->readDone = false;
    cout << "DRAM: " << this->id << " Received read request from address: " << address;
    cout << " Assigned Latency of: " << randomLatency << " core cycles." << endl;
}

template <typename AddressType, typename DataType>
void DRAM<AddressType, DataType>::writeAccess(AddressType address, vector<DataType> data) {
    srand(time(NULL));
    int randomLatency = (rand() % (this->latencymax - this->latencymin)) + this->latencymin;
    if (this->readonly)
        cout << "WARNING: Attempt to write into a read-only memory: " << this->id << endl;
    else {
        this->writeWaitCycles = 5; // randomLatency;
        this->nextWriteAddress = address;
        this->nextWriteData = data;
        this->writePending = true;
        this->writeDone = false;
    }
    cout << "DRAM: " << this->id << " Received write request from address: " << address << endl;
    cout << " Write request data: " << endl;
    for (auto wd: data)
        cout << wd << endl;
    cout << " Assigned Latency of : " << randomLatency << " core cycles." << endl;
}

template <typename AddressType, typename DataType>
bool DRAM<AddressType, DataType>::isFree() {
    return !(this->readPending || this->writePending);
}

template <typename AddressType, typename DataType>
int DRAM<AddressType, DataType>::getChannelWidth() {
    return this->channelwidth;
}

template <typename AddressType, typename DataType>
void DRAM<AddressType, DataType>::step() {
    cout << "----------------------- DRAM " << this->id << " step function --------------------------" << endl;
    if (this->readPending) {
        this->readWaitCycles--;
        cout << "DRAM: " << this->id << " Read Pending Cycles: " << this->readWaitCycles << endl;
        if (this->readWaitCycles == 0) {
            for (int i = 0; i < this->channelwidth; i++)
                this->lastReadData[i] = this->MEM[this->nextReadAddress.to_ulong() + i];
            cout << "DRAM: " << this->id << " Reading Done." << endl;
            this->readPending = false;
            this->readWaitCycles = 0;
            this->readDone = true;
        }
    }
    else if (this->writePending) {
        this->writeWaitCycles--;
        cout << "DRAM: " << this->id << " Write Pending Cycles: " << this->writeWaitCycles << endl;
        if (this->writeWaitCycles == 0) {
            for (int i = 0; i < this->channelwidth; i++)
                this->MEM[this->nextWriteAddress.to_ulong() + i] = this->nextWriteData[i];
            cout << "DRAM: " << this->id << " Writing Done." << endl;
            this->writePending = false;
            this->writeWaitCycles = 0;
            this->writeDone = true;
        }
    }
}

#endif