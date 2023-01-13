#include <Reserve.h>

ReserveUnit::ReserveUnit(Config * config, vector<bitset<64>> * refIndexInfo) {
    this->base = config->parameters["Pipeline"];
    this->RefCount = bitset<32>(refIndexInfo->at(0).to_ulong());
    this->CountReg = refIndexInfo->at(this->base + 1);
    this->OccLastValReg = refIndexInfo->at(this->base + 5);

    this->CRS = new ReservationStation<CRSEntry>(config->children["ComputeReservationStation"]);
    
    this->LRSIdxQ = new Queue<bitset<6>>(config->children["LRSIdxQ"]);
    this->LRSIdxQ->push(bitset<6>(0));
    this->LRSIdxQ->push(bitset<6>(1));
    this->LRS = new ReservationStation<LRSEntry>(config->children["LoadReservationStation"]);
    this->LRS->setScheduledState(0);
    this->LRS->setScheduledState(1);

    this->halted = false;
    this->cycle_count = 0;
    this->pendingToBeReserved = pair<bool, DispatchEntry>(false, *(new DispatchEntry));
}

void ReserveUnit::connect(DispatchUnit * du) {
    this->coreDU = du;
}

void ReserveUnit::step() {
    if (!this->halted) {
        pair<bool, DispatchEntry> currentDispatch;
        if (this->pendingToBeReserved.first)
            currentDispatch = this->pendingToBeReserved;
        else  
            currentDispatch = this->coreDU->popnext(this->base);

        if (currentDispatch.first) {
            CRSEntry * newCRSEntry = new CRSEntry;
            newCRSEntry->LowOccReady = true;
            newCRSEntry->HighOccReady = true;
            newCRSEntry->Count = this->CountReg;

            if (currentDispatch.second.Low == bitset<32>(0))
                newCRSEntry->LowOcc = bitset<64>(0);
            else if (currentDispatch.second.Low == this->RefCount)
                newCRSEntry->LowOcc = this->OccLastValReg;
            else {
                newCRSEntry->LowOccReady = false;
                newCRSEntry->LowOcc = bitset<64>(0);
            }

            if (currentDispatch.second.High == bitset<32>(0))
                newCRSEntry->HighOcc = bitset<64>(0);
            else if (currentDispatch.second.High == this->RefCount)
                newCRSEntry->HighOcc = this->OccLastValReg;
            else {
                newCRSEntry->HighOccReady = false;
                newCRSEntry->HighOcc = bitset<64>(0);
            }

            newCRSEntry->SRSWBIndex = currentDispatch.second.SRSWBIndex;
            int nextCRSIdx = this->CRS->nextFreeEntry();
            
            if (nextCRSIdx != -1) {
                vector<LRSEntry> newLoadRequests;
                if (!newCRSEntry->LowOccReady) {
                    LRSEntry newLoadRequest;
                    newLoadRequest.LowOrHigh = false;
                    newLoadRequest.OccMemoryAddress = currentDispatch.second.Low;
                    newLoadRequest.SRSWBIndex = currentDispatch.second.SRSWBIndex;
                    newLoadRequest.ResStatIndex = nextCRSIdx;
                    newLoadRequests.push_back(newLoadRequest);
                }
                if (!newCRSEntry->HighOccReady) {
                    LRSEntry newLoadRequest;
                    newLoadRequest.LowOrHigh = true;
                    newLoadRequest.OccMemoryAddress = currentDispatch.second.High;
                    newLoadRequest.SRSWBIndex = currentDispatch.second.SRSWBIndex;
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
                    this->pendingToBeReserved = pair<bool, DispatchEntry>(false, currentDispatch.second);
                    
                    cout << "Reserve Unit - Cycle Count:" << this->cycle_count << endl;
                    cout << "LRSIdxQueue:" << endl;
                    this->LRSIdxQ->print();
                    cout << "LoadRS:" << endl;
                    this->LRS->show();
                    cout << "ComputeRS:" << endl;
                    this->CRS->show();
                }
                else
                    this->pendingToBeReserved = pair<bool, DispatchEntry>(true, currentDispatch.second);
            }
            else
                this->pendingToBeReserved = pair<bool, DispatchEntry>(true, currentDispatch.second);
        }
        this->cycle_count++;
    }
}