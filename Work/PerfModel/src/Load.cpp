#include <Load.h>

LoadUnit::LoadUnit(Config * config) {
    this->halted = false;
    this->cycle_count = 0;
}

void LoadUnit::connectRU(ReserveUnit * ru) {
    this->coreRU = ru;
}

void LoadUnit::connectDRAM(DRAM<bitset<32>, bitset<32>> * occmem) {
    this->OCCMEM = occmem;
}

void LoadUnit::step() {
    if (!this->halted) {
        if (this->OCCMEM->readDone) {
            this->coreRU->fillInCRS(this->LRSEntryInProgress.second.ResStatIndex.to_ulong(), this->LRSEntryInProgress.second.LowOrHigh, this->OCCMEM->lastReadData[0]);
            this->coreRU->setLRSEToEmptyState(this->LRSEntryInProgress.first);
        }

        if (this->OCCMEM->isFree()) {
            pair<int, LRSEntry> nle = this->coreRU->getNextLoadEntry();
            if (nle.first != -1) {
                this->OCCMEM->readAccess(nle.second.OccMemoryAddress);
                this->LRSEntryInProgress = nle;
            }
            else if (this->coreRU->isHalted())
                this->halted = true;
        }
    }
}

bool LoadUnit::isHalted() {
    return this->halted;
}