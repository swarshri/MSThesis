#include <Reserve.h>

ComputeReservationStation::ComputeReservationStation(Config * config)
: ReservationStation(config) {
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

ReserveUnit::ReserveUnit(Config * config, vector<bitset<32>> * refIndexInfo) {
    this->base = config->parameters["Pipeline"];
    this->RefCount = bitset<32>(refIndexInfo->at(0).to_ulong());
    this->CountReg = refIndexInfo->at(this->base + 1);
    this->OccLastValReg = refIndexInfo->at(this->base + 5);

    this->CRS = new ComputeReservationStation(config->children["ComputeReservationStation"]);
    
    this->LRSIdxQ = new Queue<bitset<6>>(config->children["LRSIdxQ"]);
    this->LRSIdxQ->push(bitset<6>(0));
    this->LRSIdxQ->push(bitset<6>(1));
    this->LRS = new ReservationStation<LRSEntry>(config->children["LoadReservationStation"]);
    this->LRS->setScheduledState(0);
    this->LRS->setScheduledState(1);

    this->halted = false;
    this->cycle_count = 0;
    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(false, *(new DispatchQueueEntry));
}

void ReserveUnit::connect(DispatchUnit * du) {
    this->coreDU = du;
}

void ReserveUnit::step() {
    if (!this->halted) {
        pair<bool, DispatchQueueEntry> currentDispatch;
        if (this->pendingToBeReserved.first)
            currentDispatch = this->pendingToBeReserved;
        else  
            currentDispatch = this->coreDU->popNextDispatch(this->base);

        if (currentDispatch.first) {
            CRSEntry * newCRSEntry = new CRSEntry;
            newCRSEntry->LowOccReady = true;
            newCRSEntry->HighOccReady = true;
            newCRSEntry->Count = this->CountReg;

            if (currentDispatch.second.LowPointer == bitset<32>(0))
                newCRSEntry->LowOcc = bitset<32>(0);
            else if (currentDispatch.second.LowPointer == this->RefCount)
                newCRSEntry->LowOcc = this->OccLastValReg;
            else {
                newCRSEntry->LowOccReady = false;
                newCRSEntry->LowOcc = bitset<32>(0);
            }

            if (currentDispatch.second.HighPointer == bitset<32>(0))
                newCRSEntry->HighOcc = bitset<32>(0);
            else if (currentDispatch.second.HighPointer == this->RefCount)
                newCRSEntry->HighOcc = this->OccLastValReg;
            else {
                newCRSEntry->HighOccReady = false;
                newCRSEntry->HighOcc = bitset<32>(0);
            }

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
                        this->LRS->fill(this->LRSIdxQ->pop(), *nlr);
                        int nfe = this->LRS->nextFreeEntry();
                        if (nfe != -1)
                            this->LRSIdxQ->push(bitset<6>(nfe));
                    }
                    this->CRS->fill(bitset<6>(nextCRSIdx), *newCRSEntry);
                    if (newLoadRequests.size() > 0)
                        this->CRS->setScheduledState(nextCRSIdx);
                    else
                        this->CRS->setReadyState(nextCRSIdx);
                    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(false, currentDispatch.second);
                    
                    cout << "Reserve Unit - Cycle Count:" << this->cycle_count << endl;
                    cout << "LRSIdxQueue:" << endl;
                    this->LRSIdxQ->print();
                    cout << "LoadRS:" << endl;
                    this->LRS->show();
                    cout << "ComputeRS:" << endl;
                    this->CRS->show();
                }
                else
                    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(true, currentDispatch.second);
            }
            else
                this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(true, currentDispatch.second);
        }
        this->cycle_count++;
    }
}

bool ReserveUnit::isHalted() {
    return this->halted;
}

pair<int, CRSEntry> ReserveUnit::getNextComputeEntry() {
    return this->CRS->nextReadyEntry();
}

void ReserveUnit::setCRSEToEmptyState(int idx) {
    this->CRS->setEmptyState(idx);
}

void ReserveUnit::fillInCRS(int idx, bool high, bitset<32> dataVal) {
    if (high)
        this->CRS->fillHighOccVal(idx, dataVal);
    else
        this->CRS->fillLowOccVal(idx, dataVal);
}

pair<int, LRSEntry> ReserveUnit::getNextLoadEntry() {
    return this->LRS->nextReadyEntry();
}

void ReserveUnit::setLRSEToEmptyState(int idx) {
    this->LRS->setEmptyState(idx);
}

void ReserveUnit::print() {
    this->CRS->show();
}