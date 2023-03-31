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
    this->SRS = new SeedReservationStation("SeedRS", config->children["SeedReservationStation"]);

    cout << "FetchUnit: Created FillIdxQueue and SRS." << endl;

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
        this->SRS->show(cout);

        output.close();
    }
    else
        cout << "Unable to open file for FetchStage Output." << this->op_file_path << endl;
}

bool FetchStage::isHalted() {
    return this->halted;
}

void FetchStage::connectDRAM(DRAMW<32, 64> * sdmem) {
    this->SDMEM = sdmem;
}

void FetchStage::step() {
    cout << "----------------------- Fetch Stage step function --------------------------" << endl;
    if (!this->halted) {
        cout << "Fetch stage not halted: Cycle count: " << this->cycle_count << endl;
        pair<bool, vector<PMAEntry<64>>> nwbe = this->SDMEM->getNextWriteBack();
        // TODO - this only returns 1 entry right now. We need to use the burst mode to use the bandwidth better.
        if (nwbe.first) {
            cout << "nwbe count: " << nwbe.second.size() << endl;
            for (auto pmae : nwbe.second) {
                // Unpack values and put them in SRS in the right places per their RequestID.
                if (pmae.Data.size() != 1) 
                    cout << "More than 1 word returned for request access. Taking only the first data as burst mode is disabled." << endl;

                auto data = pmae.Data[0];
                cout << "FS: Unpacked data from SDMEM: " << data << " at address: " << bitset<32>(pmae.AccessAddress) << endl;
                if (data.count() == 64) {
                    this->halted = true;
                    cout << "Found Halt Seed at seed pointer: " << pmae.AccessAddress << endl;
                    this->print();
                    break;
                }
                else {
                    SRSEntry newSRSEntry;
                    newSRSEntry.SeedAddress = bitset<32>(pmae.AccessAddress);
                    newSRSEntry.Seed = data;
                    newSRSEntry.LowPointer = bitset<32>(0);
                    newSRSEntry.HighPointer = this->RefCount;
                    newSRSEntry.BasePointer = bitset<6>(0);
                    newSRSEntry.StoreFlag = false;
                    newSRSEntry.Ready = true;
                    newSRSEntry.Empty = false;
                    this->SRS->fill(bitset<6>(pmae.RequestID), newSRSEntry);
                }
            }
            cout << "FS: Updated SRS." << endl;
            this->print();
        }

        int nextFreeEntry = this->SRS->nextFreeEntry();
        if (nextFreeEntry != -1 && this->SDMEM->willAcceptRequest(this->SeedPointer, false)) {
            cout << "FS: Sending read request for seed at: " << this->SeedPointer << "." << endl;
            this->SDMEM->readRequest(this->SeedPointer, nextFreeEntry);
            this->SRS->setScheduledState(nextFreeEntry);
            this->SeedPointer = bitset<32>(this->SeedPointer.to_ulong() + 1); // seedpointer + 1 because we are not using the DRAMs in burst mode.
            cout << "FS: Updated seed pointer: " << this->SeedPointer << endl;
        }
        else {
            if (nextFreeEntry == -1)
                cout << "FS: Stalled because SRS is filled. Read request not sent to SDMEM for SP: " << this->SeedPointer << endl;
            else
                cout << "FS: Stalled because SDMEM cannot accept request for address: " << this->SeedPointer << endl;
        }
        this->cycle_count++;
    }
    
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
    // Logic for halting fetch stage.
    if (this->cycle_count > 3000)
        this->halted = true;
}

// void FetchStage::step_old() {
//     cout << "----------------------- Fetch Stage step function --------------------------" << endl;
//     if (this->pendingWB) {
//         for (auto wb:this->pendingWriteBacks) {
//             int idx = wb.first;
//             long int lowResult = wb.second.first.to_ulong();
//             long int highResult = wb.second.second.to_ulong();
//             this->SRS->updateLowPointer(idx, lowResult);
//             this->SRS->updateHighPointer(idx, highResult);
//             cout << "FS: Writing Back into FS SRS at Index: " << idx << endl;
//             if (lowResult >= highResult) {
//                 this->setStoreFlag(idx);
//                 cout << "FS: Setting Store Flag in FS SRS at Index: " << idx << endl;
//             }
//             this->setReadyState(idx);
//             cout << "FS: Setting Ready State in FS SRS at Index: " << idx << endl;
//         }
//         this->pendingWriteBacks.clear();
//         this->pendingWB = false;
//         this->print();
//     }
//     if (this->pendingEmpty) {
//         for (int idx: this->pendingEmptyIdcs) {
//             this->setEmptyState(idx);
//             cout << "FS: Setting Empty State in FS SRS at index: " << idx << endl;
//         }
//         this->pendingEmptyIdcs.clear();
//         this->pendingEmpty = false;
//         this->print();
//     }
//     if (!this->halted) {
//         this->cycle_count++;
//         pair<bool, vector<PMAEntry<64>>> nwbe = this->SDMEM->getNextWriteBack();
//         if (nwbe.first) {
//             if (nwbe.second.size() != 1) {
//                 // Throw error - because for this memory, we are only expecting one BL bytes of data out of it every cycle.
//             }
//             else {
//                 // Unpack values and put them in SRS in the right places in FillIdxQueue.
//                 auto pmae = nwbe.second[0];
//                 int i = 0;
//                 for (auto data = pmae.Data.begin(); data != pmae.Data.end(); data++) {
//                     if (data->count() == 64) {
//                         this->halted = true;
//                         while(!this->FillIdxQueue->isEmpty()) {
//                             int idx = this->FillIdxQueue->pop().to_ulong();
//                             this->SRS->setEmptyState(idx);
//                         }
//                         cout << "Found Halt Seed at i: " << i << endl;
//                         this->print();
//                         return;
//                     }
//                     else {
//                         int nextIdx = this->FillIdxQueue->pop().to_ulong();
//                         SRSEntry newSRSEntry;
//                         newSRSEntry.SeedAddress = bitset<32>(this->SeedPointer.to_ulong() + i); // Don't understand how to do this.
//                         newSRSEntry.Seed = *data;
//                         newSRSEntry.LowPointer = bitset<32>(0);
//                         newSRSEntry.HighPointer = this->RefCount;
//                         newSRSEntry.BasePointer = bitset<6>(0);
//                         newSRSEntry.StoreFlag = false;
//                         newSRSEntry.Ready = true;
//                         newSRSEntry.Empty = false;
//                         this->SRS->fill(bitset<6>(nextIdx), newSRSEntry);
//                     }
//                     i++;
//                 }
//             }
//             cout << "FS: Updated SRS." << endl;
//             cout << "FS: Updated Seed Pointer." << endl;
//             this->print();
//         }
//         if (this->SDMEM->willAcceptRequest(this->SeedPointer, false)) {
//             int srsVacancy = this->FillIdxQueue->getCount();
//             if (srsVacancy >= this->SDMEM->getChannelWidth()) {
//                 this->SDMEM->readRequest(this->SeedPointer);
//                 cout << "FS: Sent Read Request from address: " << this->SeedPointer << endl;
//                 this->SeedPointer = bitset<32>(this->SeedPointer.to_ulong() + this->SDMEM->getChannelWidth());
//             }
//         }
        
//         if (!this->FillIdxQueue->isFull()) {
//             int nextFreeEntry = this->SRS->nextFreeEntry();
//             if (nextFreeEntry != -1) {
//                 this->FillIdxQueue->push(nextFreeEntry);
//                 this->SRS->setScheduledState(nextFreeEntry);
//                 cout << "FS: Updated Fill Index Queue." << endl;
//                 this->print();
//             }
//         }
//         if (this->cycle_count > 50)
//             this->halted = true;
//     }
//     else
//         cout << "FS: Halted" << endl;
// }