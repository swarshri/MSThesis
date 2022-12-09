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

    this->lastReadData.resize(this->channelwidth);
    this->nextWriteData.resize(this->channelwidth);

    this->nextReadAddress = bitset<32>(0);
    this->readWaitCycles = 0;
    this->readPending = false;

    this->nextWriteAddress = bitset<32>(0);
    this->writeWaitCycles = 0;
    this->writePending = false;
}

void DRAM::load(string ioDir) {
    ifstream mem;
    string line;

#ifdef _WIN32
    string filename = "\\" + this->id;
#else
    string filename = this->id;
#endif
    filename = filename + ".mem";

    string filepath = ioDir + filename;
    mem.open(filepath);

    if (mem.is_open()) {
        cout << "File opened: " << filepath << endl;
        int i=0;
        while (getline(mem, line)) {
            this->MEM[i] = bitset<64>(line);
            i++;
        }
        mem.close();

        while(i < this->memsize) {
            this->MEM[i] = bitset<64>(0);
            i++;
        }
    }
    else cout<<"Unable to open input file for " << this->id << endl;
}

void DRAM::readAccess(bitset<32> address) {
    cout << "in readAccess: " << address << endl;
    srand(time(NULL));
    int randomLatency = (rand() % (this->latencymax - this->latencymin)) + this->latencymin;
    cout << "randomLatency: " << randomLatency << endl;
    this->readWaitCycles = randomLatency;
    this->nextReadAddress = address;
    this->readPending = true;
    this->readDone = false;
}

void DRAM::writeAccess(bitset<32> address, vector<bitset<64>> data) {
    cout << "in writeAccess: " << address << endl;
    srand(time(NULL));
    int randomLatency = (rand() % (this->latencymax - this->latencymin)) + this->latencymin;
    cout << "randomLatency: " << randomLatency << endl;
    if (this->readonly)
        cout << "WARNING: Attempt to write into a read-only memory: " << this->id << endl;
    else {
        this->writeWaitCycles = randomLatency;
        this->nextWriteAddress = address;
        this->nextWriteData = data;
        this->writePending = true;
        this->writeDone = false;
    }
}

bool DRAM::isFree() {
    bool ret = true;
    if (this->readPending || this->writePending)
        ret = false;
    cout << "ret in isFree(): " << ret << "\t";
    return ret;
}

int DRAM::getChannelWidth() {
    return this->channelwidth;
}

void DRAM::step() {
    if (this->readPending) {
        this->readWaitCycles--;
        cout << "read pending: " << this->readWaitCycles << endl;
        if (this->readWaitCycles == 0) {
            for (int i = 0; i < this->channelwidth; i++)
                this->lastReadData[i] = this->MEM[this->nextReadAddress.to_ulong() + i];
            cout << "reading done" << endl;
            this->readPending = false;
            this->readWaitCycles = 0;
            this->readDone = true;
        }
    }
    else if (this->writePending) {
        this->writeWaitCycles--;
        cout << "write pending: " << this->writeWaitCycles << endl;
        if (this->writeWaitCycles == 0) {
            for (int i = 0; i < this->channelwidth; i++)
                this->MEM[this->nextWriteAddress.to_ulong() + i] = this->nextWriteData[i];
            cout << "writing done" << endl;
            this->writePending = false;
            this->writeWaitCycles = 0;
            this->writeDone = true;
        }
    }
}