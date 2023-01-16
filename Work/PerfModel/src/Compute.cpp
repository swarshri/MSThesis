#include<Compute.h>

ComputeUnit::ComputeUnit(Config * config) {
    this->halted = false;
    this->cycle_count = 0;
}

void ComputeUnit::connect(ReserveUnit * ru, FetchUnit * fu) {
    this->coreRU = ru;
    this->coreFU = fu;
}

void ComputeUnit::step() {
    if (!this->halted) {
        pair<int, CRSEntry> nce = this->coreRU->getNextComputeEntry();
        if (nce.first != -1) {
            bitset<32> lowResult = bitset<32>(nce.second.Count.to_ulong() + nce.second.LowOcc.to_ulong());
            bitset<32> highResult = bitset<32>(nce.second.Count.to_ulong() + nce.second.HighOcc.to_ulong());
            this->coreFU->writeBack(nce.second.SRSWBIndex.to_ulong(), lowResult, highResult);
            if (lowResult.to_ulong() >= highResult.to_ulong())
                this->coreFU->setStoreFlag(nce.second.SRSWBIndex.to_ulong());
            this->coreFU->setReadyState(nce.second.SRSWBIndex.to_ulong());
            cout << "Write Back into FU:" << this->cycle_count << endl;
            this->coreFU->print();
            this->coreRU->setCRSEToEmptyState(nce.first);
            cout << "Update RU:" << this->cycle_count << endl;
            this->coreRU->print();
        }
        this->cycle_count++;
    }
}