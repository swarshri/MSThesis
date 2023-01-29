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
            cout << "LS: Scheduled to fill in Compute RS at index: " << this->LRSEntryInProgress.second.ResStatIndex.to_ulong() << endl;
            cout << "LS: Scheduled data: " << this->LRSEntryInProgress.second.LowOrHigh << " " << this->OCCMEM->lastReadData[0] << endl;
            this->coreRU->scheduleToSetLRSEToEmptyState(this->LRSEntryInProgress.first);
            cout << "LS: Scheduled to set empty state in Load RS at index: " << this->LRSEntryInProgress.first << endl;
            this->OCCMEM->readDone = false;
        }
        else if (this->OCCMEM->isFree()) {
            pair<int, LRSEntry> nle = this->coreRU->getNextLoadEntry();
            if (nle.first != -1) {
                this->OCCMEM->readAccess(nle.second.OccMemoryAddress);
                this->LRSEntryInProgress = nle;
                cout << "LS: Sent Read Request from address: " << nle.second.OccMemoryAddress << endl;
            }
            else if (this->coreRU->isHalted())
                this->halted = true;
        }
    }
}

bool LoadStage::isHalted() {
    return this->halted;
}