#include<stdint.h>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>

using namespace std;

class Config {
    public:
        string IODir;
        uint32_t MemSize = 2000;
        uint32_t QMemSize = 2000;
        uint32_t OMemSize = 2000;
        uint32_t MemLatency = 0; // In CPU cycles
        uint8_t QueryRegLength = 255;
        uint16_t PFSuffixDepth = 6;

        Config(string ioDir) {
            this->IODir = ioDir;
            // ToDo - read from file
        }
};

template <class bitsetType, class addressType> 
class Memory {
    public:
        string id;
        string ioDir;
        
        Memory() {}

        Memory(string name, Config & config) {
            this->id = name;
            this->ioDir = config.IODir;
        }

        bitsetType read(addressType address) {
            return MEM.at(address.to_ulong());
        }

    protected:
        bitsetType load(bool returnFirstLine = false) {
            ifstream imem;
            string line;
            
            string filename = "\\" + this->id;
            filename = filename + ".txt";

            bitsetType retData = bitsetType(0);
            
            imem.open(this->ioDir + filename);
            if (imem.is_open()) {
                int i=0;
                while (getline(imem, line)) {
                    //cout << bitsetType(line) << endl;
                    if(i==0 && returnFirstLine) {
                        retData = bitsetType(line);
                        //cout << "retData\t" << retData << endl;
                        returnFirstLine = false;
                    }
                    else {
                        this->MEM.at(i) = bitsetType(line);
                        //cout << i << "\t" << this->MEM.at(i) << endl;
                        i++;
                    }
                }
            }
            else cout<<"Unable to open MEM input file for "<< this->id << endl;
            imem.close();
            return retData;
        }

        vector<bitsetType> MEM;
};

class QueryMemory : public Memory<bitset<3>, bitset<16>> {
    public:
        QueryMemory(): Memory() {}

        QueryMemory(Config & config): Memory<bitset<3>, bitset<16>>("QMEM", config) {
            this->MEM.resize(config.QMemSize);
            this->load();
        }

        vector<vector<bitset<3>>> read(bitset<16> address) {
            uint16_t rdAddress = address.to_ulong();
            vector<vector<bitset<3>>> retData;
            retData.resize(20);
            for (int i = 0; i < 85; i++) {
                //cout << "data at memory: " << i << " : " << this->MEM.at(rdAddress+i) << endl;
                retData.at(i) = this->MEM.at(rdAddress+i);
                //cout << "data at retData: " << i << " : " << retData.at(i) << endl;
            }
            return retData;
        }
};

class IdxMemory : public Memory<bitset<64>, bitset<64>> {
    public:
        IdxMemory(string name, Config & config): Memory<bitset<64>, bitset<64>>(name, config) {
            this->MEM.resize(config.OMemSize);
            this->load();
        }

        void load() {
            this->Count = Memory::load(true);
        }

        bitset<64> readCount() {
            return this->Count;
        }

        bitset<64> read(bitset<64> address) {
            return this->MEM.at(address.to_ulong());
        }
    
    private:
        bitset<64> Count;
};

struct SPIOPEntry {
    bitset<64> start_address_LP;
    bitset<64> prev_address_LP;
    bitset<16> stride_LP;
    bitset<64> start_address_HP;
    bitset<64> prev_address_HP;
    bitset<16> stride_HP;
    bitset<8> pattern_count; // not really useful - linear or otherwise
    bool validity;
};

struct SPIOPUpdateData {
    bitset<64> current_occidx;
    bitset<64> current_address_LP;
    bitset<64> current_address_HP;
};

struct SPIOPGetData {
    bitset<64> current_occidx;
    bitset<8> pattern_count_LP;
    bitset<8> pattern_count_HP;
};

template <class idxType>
class SPIOPrefetcher {
    public:
        string id;
        uint16_t num_entries;

        SPIOPrefetcher(string name, Config & config) {
            this->id = name;
            this->num_entries = pow(2, config.PFSuffixDepth);
            this->Entries.resize(num_entries);
        }

        void update(idxType idx, SPIOPEntry entrydata) {
            SPIOPEntry entryVal;
            entryVal.
            this->Entries.at(idx.to_ulong()) = 
        }

        SPIOPEntry * get(uint16_t idx) {
            return this->Entries.at(idx);
        }

    private:
        vector<SPIOPEntry *> Entries;
};

class FMIBS {
    public:
        bool halted;

        FMIBS(Config & config) {
            this->queryRegisterLength = config.QueryRegLength;
            this->halted = true;
            this->cycle_count = 0;
        }

        void connect(QueryMemory * qMem, vector<IdxMemory *> occMem, IdxMemory * idxMem) {
            this->QMem = qMem;
            this->OMem = occMem;
            this->IMem = idxMem;
        }

        void initialize() {
            this->BWTLength = this->IMem->readCount();
            this->LowRegister = bitset<64>(0);
            this->HighRegister = this->BWTLength;
            this->QueryPtr = 0;
            this->CharPtr = 0;
            this->CountRegisters.push_back(this->OMem.at(0)->readCount());
            this->CountRegisters.push_back(this->OMem.at(1)->readCount());
            this->CountRegisters.push_back(this->OMem.at(2)->readCount());
            this->CountRegisters.push_back(this->OMem.at(3)->readCount());
            this->QueryQueue = this->QMem->read(0);
            this->halted = false;
            this->cycle_count = 0;
        }

        void step() {
            cout << endl << string(80, '-') << endl;
            this->cycle_count++;

            cout << "Cycle count: " << this->cycle_count << endl;

            bitset<3> baseChar = this->QueryQueue.at(this->CharPtr);
            cout << "BC: " << baseChar << endl;
            
            cout << "LR: " << this->LowRegister << endl;
            cout << "HR: " << this->HighRegister << endl;

            if (baseChar == bitset<3>(0b111) || this->CharPtr == this->queryRegisterLength) {
                this->QueryPtr = 0;
                this->CharPtr = 0;
                this->halted = true;
                return;
            }
            
            cout << "Low: " << this->LowRegister.to_ulong() << endl;
            cout << "High: " << this->HighRegister.to_ulong() << endl;

            uint64_t count = this->CountRegisters[baseChar.to_ulong()].to_ulong();
            uint64_t occlow = this->OMem.at(baseChar.to_ulong())->read(this->LowRegister).to_ulong();
            uint64_t occhigh = this->OMem.at(baseChar.to_ulong())->read(this->HighRegister).to_ulong();

            cout << "Count: " << count << endl;
            cout << "OccLow: " << occlow << endl;
            cout << "OccHigh: " << occhigh << endl;

            uint64_t newLow = count + occlow;
            uint64_t newHigh = count + occhigh;
            
            cout << "NewLow: " << newLow << endl;
            cout << "NewHigh: " << newHigh << endl;

            this->LowRegister = bitset<64>(newLow);
            this->HighRegister = bitset<64>(newHigh);

            this->CharPtr++;
        }

    private:
        vector<bitset<3>> QueryQueue;
        uint8_t QueryPtr;
        uint8_t CharPtr;
        bitset<64> LowRegister, HighRegister;
        bitset<64> BWTLength;
        uint8_t queryRegisterLength;
        vector<bitset<64>> CountRegisters;
        vector<IdxMemory*> OMem;
        QueryMemory *QMem;
        IdxMemory *IMem;
        uint64_t cycle_count = 0;
};

int main(int argc, char* argv[]) {
    string ioDir = "";
    if (argc == 1) {
        cout << "Enter path containing the input files: ";
        cin >> ioDir;
    }
    else if (argc > 2) {
        cout << "Invalid number of arguments. Machine stopped." << endl;
        return -1;
    }
    else {
        ioDir = argv[1];
        cout << "IO Directory: " << ioDir << endl;
    }

    Config config = Config(ioDir);

    QueryMemory * QMem = new QueryMemory(config);
    vector<IdxMemory*> OMem;
    OMem.push_back(new IdxMemory("OMEM_A", config));
    OMem.push_back(new IdxMemory("OMEM_C", config));
    OMem.push_back(new IdxMemory("OMEM_G", config));
    OMem.push_back(new IdxMemory("OMEM_T", config));
    IdxMemory * IMem = new IdxMemory("IMEM", config);

    FMIBS Core = FMIBS(config);
    Core.connect(QMem, OMem, IMem);
    Core.initialize();

    while(!Core.halted)
        Core.step();

    return 0;
}