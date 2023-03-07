#include<Fetch.h>

SeedReservationStation::SeedReservationStation(string name, SysConfig * config)
: ReservationStation<SRSEntry>(name, config) {
    for (auto entry = this->Entries.begin(); entry != this->Entries.end(); entry++)
        (*entry).StoreFlag = false;
}

void SeedReservationStation::setStoreFlag(int idx) {
    this->Entries[idx].StoreFlag = true;
}

void SeedReservationStation::updateBasePointer(int idx) {
    this->Entries[idx].BasePointer = bitset<6>(this->Entries[idx].BasePointer.to_ulong() + 3);
}

void SeedReservationStation::updateLowPointer(int idx, bitset<32> val) {
    this->Entries[idx].LowPointer = val;
}

void SeedReservationStation::updateHighPointer(int idx, bitset<32> val) {
    this->Entries[idx].HighPointer = val;
}

/*--------------
FETCH STAGE
---------------*/
FetchStage::FetchStage(SysConfig * config, string iodir, bitset<32> refCount) {
    // Microarchitecture configuration

    // Reference genome inputs
    this->RefCount = refCount;

    // Set initial state of the registers.
    this->SeedPointer = bitset<32>(0);

    this->FillIdxQueue = new Queue<bitset<6>>(config->children["FillIdxQ"]);
    this->FillIdxQueue->push(bitset<6>(0));
    this->FillIdxQueue->push(bitset<6>(1));
    this->FillIdxQueue->push(bitset<6>(2));
    this->FillIdxQueue->push(bitset<6>(3));

    this->SRS = new SeedReservationStation("SeedRS", config->children["SeedReservationStation"]);
    this->SRS->setScheduledState(0);
    this->SRS->setScheduledState(1);
    this->SRS->setScheduledState(2);
    this->SRS->setScheduledState(3);

    // Clear performance metrics
    this->cycle_count = 0;
    this->halted = false;

    pendingWB = false;
    pendingEmpty = false;

    // Output file path    
#ifdef _WIN32
    this->op_file_path = iodir + "\\OP\\FetchStage.out";
#else
    this->op_file_path = iodir + "OP/FetchStage.out";
#endif
    cout << "Output File: " << this->op_file_path << endl;

    ofstream output;
    output.open(this->op_file_path, ios_base::trunc);
    if (output.is_open()) {
        output.clear();
        output.close();
        cout << "FetchStage Output file opened." << endl;
    }
    else
        cout << "Unable to open file for FetchStage Output." << this->op_file_path << endl;
}

// API function definitions
pair<int, SRSEntry> FetchStage::getNextReadyEntry() {
    return this->SRS->nextReadyEntry();
}

void FetchStage::writeBack(int idx, bitset<32> lowVal, bitset<32> highVal) {
    pair<int, pair<bitset<32>, bitset<32>>> newWB = pair<int, pair<bitset<32>, bitset<32>>>(idx, pair<bitset<32>, bitset<32>>(lowVal, highVal));
    this->pendingWriteBacks.push_back(newWB);
    this->pendingWB = true;
}

void FetchStage::scheduleToSetEmptyState(int idx) {
    this->pendingEmptyIdcs.push_back(idx);
    this->pendingEmpty = true;
}

void FetchStage::setInProgress(int idx) {
    this->SRS->setWaitingState(idx);
    this->SRS->updateBasePointer(idx);
}

void FetchStage::setEmptyState(int idx) {
    this->SRS->setEmptyState(idx);
}

void FetchStage::setReadyState(int idx) {
    this->SRS->setReadyState(idx);
}

void FetchStage::setStoreFlag(int idx) {
    this->SRS->setStoreFlag(idx);
}

bool FetchStage::emptySRS() {
    return this->SRS->isEmpty();
}

// Stage functions
void FetchStage::print() {
    // open output file
    ofstream output;
    string line;
    
    output.open(this->op_file_path, ios_base::app);

    if (output.is_open()) {
        cout << "Seed Pointer: " << this->SeedPointer << endl;
        this->FillIdxQueue->show(cout);
        this->SRS->show(cout);

        output.close();
    }
    else
        cout << "Unable to open file for FetchStage Output." << this->op_file_path << endl;
}

bool FetchStage::isHalted() {
    return this->halted;
}

void FetchStage::connectDRAM(DRAM<bitset<32>, bitset<64>> * sdmem) {
    this->SDMEM =  sdmem;
}

void FetchStage::step() {
    cout << "----------------------- Fetch Stage step function --------------------------" << endl;
    if (this->pendingWB) {
        for (auto wb:this->pendingWriteBacks) {
            int idx = wb.first;
            long int lowResult = wb.second.first.to_ulong();
            long int highResult = wb.second.second.to_ulong();
            this->SRS->updateLowPointer(idx, lowResult);
            this->SRS->updateHighPointer(idx, highResult);
            cout << "FS: Writing Back into FS SRS at Index: " << idx << endl;
            if (lowResult >= highResult) {
                this->setStoreFlag(idx);
                cout << "FS: Setting Store Flag in FS SRS at Index: " << idx << endl;
            }
            this->setReadyState(idx);
            cout << "FS: Setting Ready State in FS SRS at Index: " << idx << endl;
        }
        this->pendingWriteBacks.clear();
        this->pendingWB = false;
        this->print();
    }
    if (this->pendingEmpty) {
        for (int idx: this->pendingEmptyIdcs) {
            this->setEmptyState(idx);
            cout << "FS: Setting Empty State in FS SRS at index: " << idx << endl;
        }
        this->pendingEmptyIdcs.clear();
        this->pendingEmpty = false;
        this->print();
    }
    if (!this->halted) {
        this->cycle_count++;
        if (this->SDMEM->readDone) {
            for (int i = 0; i < this->SDMEM->getChannelWidth(); i++) {
                bitset<64> nextReadData = this->SDMEM->lastReadData[i];
                if (nextReadData.count() == 64) {
                    this->halted = true;
                    while(!this->FillIdxQueue->isEmpty()) {
                        int idx = this->FillIdxQueue->pop().to_ulong();
                        this->SRS->setEmptyState(idx);
                    }
                    cout << "Found Halt Seed at i: " << i << endl;
                    this->print();
                    return;
                }
                else {
                    int nextIdx = this->FillIdxQueue->pop().to_ulong();
                    SRSEntry newSRSEntry;
                    newSRSEntry.SeedAddress = bitset<32>(this->SeedPointer.to_ulong() + i);
                    newSRSEntry.Seed = nextReadData;
                    newSRSEntry.LowPointer = bitset<32>(0);
                    newSRSEntry.HighPointer = this->RefCount;
                    newSRSEntry.BasePointer = bitset<6>(0);
                    newSRSEntry.StoreFlag = false;
                    newSRSEntry.Ready = true;
                    newSRSEntry.Empty = false;
                    this->SRS->fill(bitset<6>(nextIdx), newSRSEntry);
                }
            }
            this->SDMEM->readDone = false;
            this->SeedPointer = bitset<32>(this->SeedPointer.to_ulong() + this->SDMEM->getChannelWidth());
            cout << "FS: Updated SRS." << endl;
            cout << "FS: Updated Seed Pointer." << endl;
            this->print();
        }

        if (this->SDMEM->isFree()) {
            int srsVacancy = this->FillIdxQueue->getCount();
            if (srsVacancy >= this->SDMEM->getChannelWidth()) {
                this->SDMEM->readAccess(this->SeedPointer);
                cout << "FS: Sent Read Request from address: " << this->SeedPointer << endl;
            }
        }
        
        if (!this->FillIdxQueue->isFull()) {
            int nextFreeEntry = this->SRS->nextFreeEntry();
            if (nextFreeEntry != -1) {
                this->FillIdxQueue->push(nextFreeEntry);
                this->SRS->setScheduledState(nextFreeEntry);
                cout << "FS: Updated Fill Index Queue." << endl;
                this->print();
            }
        }
    }
    else
        cout << "FS: Halted" << endl;
}