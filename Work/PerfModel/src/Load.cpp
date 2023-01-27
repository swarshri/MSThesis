#include <Load.h>

LoadStage::LoadStage(Config * config, char base, string iodir) {
    this->base = base;
    this->halted = false;
    this->cycle_count = 0;
}

void LoadStage::connectRU(ReserveStage * ru) {
    this->coreRU = ru;
}

void LoadStage::connectDRAM(DRAM<bitset<32>, bitset<32>> * occmem) {
    this->OCCMEM = occmem;
}

void LoadStage::step() {
    cout << "----------------------- Load Stage " << this->base << " step function --------------------------" << endl;
    if (!this->halted) {
        if (this->OCCMEM->readDone) {
            this->coreRU->scheduleToFillInCRS(this->LRSEntryInProgress.second.ResStatIndex.to_ulong(), this->LRSEntryInProgress.second.LowOrHigh, this->OCCMEM->lastReadData[0]);
            this->coreRU->scheduleToSetLRSEToEmptyState(this->LRSEntryInProgress.first);
        }
        if (this->OCCMEM->isFree()) {
            pair<int, LRSEntry> nle = this->coreRU->getNextLoadEntry();
            if (nle.first != -1) {
                this->OCCMEM->readAccess(nle.second.OccMemoryAddress);
                this->LRSEntryInProgress = nle;
                cout << "Sent Read Request from address: " << nle.second.OccMemoryAddress << endl;
            }
            else if (this->coreRU->isHalted())
                this->halted = true;
        }
    }
}

bool LoadStage::isHalted() {
    return this->halted;
}