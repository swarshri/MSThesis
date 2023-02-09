#include <Reserve.h>

ComputeReservationStation::ComputeReservationStation(string name, Config * config)
: ReservationStation(name, config) {
    for (auto entry = this->Entries.begin(); entry != this->Entries.end(); entry++) {
        (*entry).LowOccReady = false;
        (*entry).HighOccReady = false;
    }
}

void ComputeReservationStation::fillLowOccVal(int idx, bitset<32> data) {
    this->Entries[idx].LowOcc = data;
    this->Entries[idx].LowOccReady = true;
    if (this->Entries[idx].HighOccReady)
        this->setReadyState(idx);
}

void ComputeReservationStation::fillHighOccVal(int idx, bitset<32> data) {
    this->Entries[idx].HighOcc = data;
    this->Entries[idx].HighOccReady = true;
    if (this->Entries[idx].LowOccReady)
        this->setReadyState(idx);
}

ReserveStage::ReserveStage(Config * config, char base, string iodir) {
    this->base = base;
    this->base_num = config->BaseMap[base];
    // this->RefCount = bitset<32>(refIndexInfo->at(0).to_ulong());
    // this->CountReg = refIndexInfo->at(this->base_num + 1);
    // this->OccLastValReg = refIndexInfo->at(this->base_num + 5);

    this->CRS = new ComputeReservationStation("ComputeRS", config->children["ComputeReservationStation"]);
    
    this->LRSIdxQ = new Queue<bitset<6>>(config->children["LRSIdxQ"]);
    this->LRSIdxQ->push(bitset<6>(0));
    this->LRSIdxQ->push(bitset<6>(1));
    this->LRS = new ReservationStation<LRSEntry>("LoadRS", config->children["LoadReservationStation"]);
    this->LRS->setScheduledState(0);
    this->LRS->setScheduledState(1);

    this->halted = false;
    this->cycle_count = 0;
    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(false, *(new DispatchQueueEntry));
    
    this->pendingCRSEmpty = false;
    this->pendingCRSE = false;
    this->pendingLRSEmpty = false;

    // Output file path    
#ifdef _WIN32
    this->op_file_path = iodir + "\\OP\\ReserveStage.out";
#else
    this->op_file_path = iodir + "OP/ReserveStage.out";
#endif
    cout << "Output File: " << this->op_file_path << endl;

    ofstream output;
    output.open(this->op_file_path, ios_base::trunc);
    if (output.is_open()) {
        output.clear();
        output.close();
        cout << "ReserveStage Output file opened." << endl;
    }
    else
        cout << "Unable to open file for ReserveStage Output." << this->op_file_path << endl;
}

void ReserveStage::connect(DispatchStage * du) {
    this->coreDU = du;
}

void ReserveStage::step() {
    cout << "----------------------- Reserve " << this->base << " Stage step function --------------------------" << endl;
    if (this->pendingCRSEmpty) {
        for (int idx: this->pendingEmptyCRSIdcs) {
            this->setCRSEToEmptyState(idx);
            cout << "RS: Setting CRS to Empty state at index: " << idx << endl;
        }
        this->pendingEmptyCRSIdcs.clear();
        this->pendingCRSEmpty = false;
    }
    if (this->pendingCRSE) {
        for (auto entry: this->pendingCRSEntries) {
            int idx = get<0>(entry);
            bool lorh = get<1>(entry);
            bitset<32> value = get<2>(entry);
            this->fillInCRS(idx, lorh, value);
            cout << "RS: Filling in CRS data at index: " << idx << " with data: " << lorh << " " << value << endl;
        }
        this->pendingCRSEntries.clear();
        this->pendingCRSE = false;
    }
    if (this->pendingLRSEmpty) {
        for (int idx: this->pendingEmptyLRSIdcs) {
            this->setLRSEToEmptyState(idx);
            cout << "RS: Setting LRS to Empty state at index: " << idx << endl;
        }
        this->pendingEmptyLRSIdcs.clear();
        this->pendingLRSEmpty = false;
        this->print();
    }
    if (!this->halted) {
        pair<bool, DispatchQueueEntry> currentDispatch;
        if (this->pendingToBeReserved.first) {
            currentDispatch = this->pendingToBeReserved;
            cout << "RS: Pending to be reserved due to resource constraints." << endl;
        }
        else {
            currentDispatch = this->coreDU->popNextDispatch(this->base_num);
            cout << "RS: Getting new dispatch. currentDispatch.first: " << currentDispatch.first << endl;
        }

        if (currentDispatch.first) {
            CRSEntry * newCRSEntry = new CRSEntry;
            newCRSEntry->LowOccReady = true;
            newCRSEntry->HighOccReady = true;
            // newCRSEntry->Count = this->CountReg;

            // if (currentDispatch.second.LowPointer == bitset<32>(0))
            //     newCRSEntry->LowOcc = bitset<32>(0);
            // else if (currentDispatch.second.LowPointer == this->RefCount)
            //     newCRSEntry->LowOcc = this->OccLastValReg;
            // else {
                newCRSEntry->LowOccReady = false;
                newCRSEntry->LowOcc = bitset<32>(0);
            // }

            // if (currentDispatch.second.HighPointer == bitset<32>(0))
            //     newCRSEntry->HighOcc = bitset<32>(0);
            // else if (currentDispatch.second.HighPointer == this->RefCount)
            //     newCRSEntry->HighOcc = this->OccLastValReg;
            // else {
                newCRSEntry->HighOccReady = false;
                newCRSEntry->HighOcc = bitset<32>(0);
            // }

            newCRSEntry->SRSWBIndex = currentDispatch.second.SRSWBIndex;
            int nextCRSIdx = this->CRS->nextFreeEntry();
            
            if (nextCRSIdx != -1) {
                vector<LRSEntry> newLoadRequests;
                if (!newCRSEntry->LowOccReady) {
                    LRSEntry newLoadRequest;
                    newLoadRequest.LowOrHigh = false;
                    newLoadRequest.OccMemoryAddress = currentDispatch.second.LowPointer;
                    newLoadRequest.ResStatIndex = nextCRSIdx;
                    newLoadRequests.push_back(newLoadRequest);
                }
                if (!newCRSEntry->HighOccReady) {
                    LRSEntry newLoadRequest;
                    newLoadRequest.LowOrHigh = true;
                    newLoadRequest.OccMemoryAddress = currentDispatch.second.HighPointer;
                    newLoadRequest.ResStatIndex = nextCRSIdx;
                    newLoadRequests.push_back(newLoadRequest);
                }
                if (this->LRSIdxQ->getCount() >= newLoadRequests.size()) {
                    for (auto nlr = newLoadRequests.begin(); nlr != newLoadRequests.end(); nlr++) {
                        bitset<6> idx = this->LRSIdxQ->pop();
                        this->LRS->fill(idx, *nlr);
                        this->LRS->setReadyState(idx.to_ulong());
                        int nfe = this->LRS->nextFreeEntry();
                        if (nfe != -1) {
                            this->LRSIdxQ->push(bitset<6>(nfe));
                            this->LRS->setScheduledState(nfe);
                        }
                    }
                    cout << "RS: Updated Load Reservation Station with " << newLoadRequests.size() << " new Load Requests." << endl;
                    cout << "RS: Possibly updated LRS Index Queue as well." << endl;

                    this->CRS->fill(bitset<6>(nextCRSIdx), *newCRSEntry);
                    cout << "RS: Added into Compute Reservation Station at index: " << nextCRSIdx << endl;
                    if (newLoadRequests.size() > 0)
                        this->CRS->setScheduledState(nextCRSIdx);
                    else
                        this->CRS->setReadyState(nextCRSIdx);

                    // Reset PendingToBeReserved entry.
                    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(false, currentDispatch.second);                    
                    this->print();
                }
                else { // Not enough available resource in Load Reservation Station.
                    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(true, currentDispatch.second);
                    cout << "RS: Not enough available resource in Load Reservation Station. Stalling by adding to pending."  << endl;
                    cout << "RS: Pending Dispatch: " << this->pendingToBeReserved.second << endl;
                }
            }
            else { // No available resource in Compute Reservation Station.
                this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(true, currentDispatch.second);
                cout << "RS: Not enough available resource in Compute Reservation Station. Stalling by adding to pending."  << endl;
                cout << "RS: Pending Dispatch: " << this->pendingToBeReserved.second << endl;
            }
        }
        else if (this->coreDU->isHalted() && !this->pendingToBeReserved.first)
            this->halted = true;

        this->cycle_count++;
    }
    else
        cout << "RS: Halted" << endl;
}

void ReserveStage::print() {
    // open output file
    ofstream output;
    string line;
    
    output.open(this->op_file_path, ios_base::app);

    if (output.is_open()) {
        this->LRSIdxQ->show(cout);
        this->LRS->show(cout);
        this->CRS->show(cout);

        output.close();
    }
    else
        cout << "Unable to open file for FetchStage Output." << this->op_file_path << endl;
}

bool ReserveStage::isHalted() {
    return this->halted;
}

pair<int, CRSEntry> ReserveStage::getNextComputeEntry() {
    return this->CRS->nextReadyEntry();
}

void ReserveStage::setCRSEToEmptyState(int idx) {
    this->CRS->setEmptyState(idx);
}

void ReserveStage::scheduleToSetCRSEToEmptyState(int idx) {
    this->pendingEmptyCRSIdcs.push_back(idx);
    this->pendingCRSEmpty = true;
}

void ReserveStage::fillInCRS(int idx, bool high, bitset<32> dataVal) {
    if (high)
        this->CRS->fillHighOccVal(idx, dataVal);
    else
        this->CRS->fillLowOccVal(idx, dataVal);
}

void ReserveStage::scheduleToFillInCRS(int idx, bool high, bitset<32> dataVal) {
    this->pendingCRSEntries.push_back(tuple<int, bool, bitset<32>>(idx, high, dataVal));
    this->pendingCRSE = true;
}

pair<int, LRSEntry> ReserveStage::getNextLoadEntry() {
    return this->LRS->nextReadyEntry();
}

void ReserveStage::setLRSEToEmptyState(int idx) {
    this->LRS->setEmptyState(idx);
}

void ReserveStage::scheduleToSetLRSEToEmptyState(int idx) {
    this->pendingEmptyLRSIdcs.push_back(idx);
    this->pendingLRSEmpty = true;
}