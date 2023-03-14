#include <Load.h>

LoadStage::LoadStage(SysConfig * config, char base, string iodir) {
    this->base = base;
    this->halted = false;
    this->cycle_count = 0;

    this->LRSEntryInProgress.clear();
}

void LoadStage::connectRU(ReserveStage * ru) {
    this->coreRU = ru;
}

void LoadStage::connectDRAM(DRAMW<bitset<32>, bitset<32>> * occmem) {
    this->OCCMEM = occmem;
}

void LoadStage::step() {
    cout << "----------------------- Load Stage " << this->base << " step function --------------------------" << endl;
    if (!this->halted) {
        // Get the next load request from the Load Reservation Station.
        pair<int, LRSEntry> nle = this->coreRU->getNextLoadEntry();
        if (nle.first != -1) {
            bool success = this->OCCMEM->readRequest(nle.second, nle.first);
            if (success)
                this->coreRU->scheduleToSetLRSEToScheduledState(nle.first);
        }

        // Get all finished reads from OCCMEM and write into CRS
        pair<bool, vector<PMAEntry<bitset<32>>>> nwb = this->OCCMEM->getNextWriteBack();
        if (nwb.first) {
            for (auto wbentry = nwb.second.begin(); wbentry != nwb.second.end(); wbentry++) {
                for (auto inprogentry = this->LRSEntryInProgress.begin(); inprogentry != this->LRSEntryInProgress.end(); inprogentry++) {
                    if (inprogentry.first == wbentry.RequestID) {
                        this->coreRU->scheduleToFillInCRS(inprogentry.second.ResStatIndex.to_ulong(), inprogentry.second.LowOrHigh, wbentry.Data[0]);
                        cout << "LS: Scheduled to fill in Compute RS at index: " << this->LRSEntryInProgress.second.ResStatIndex.to_ulong() << endl;
                        cout << "LS: Scheduled data: " << this->LRSEntryInProgress.second.LowOrHigh << " " << this->OCCMEM->lastReadData[0] << endl;
                        this->coreRU->scheduleToSetLRSEToEmptyState(inprogentry.first);
                        cout << "LS: Scheduled to set empty state in Load RS at index: " << this->LRSEntryInProgress.first << endl;

                        // Also try to keep a copy of this in the Local Cache.
                        IncomingCacheStruct newCacheEntry;
                        newCacheEntry.address = inprogentry.second.OccMemoryAddress;
                        newCacheEntry.basePointer = inprogentry.second.BasePointer;
                        newCacheEntry.data = wbentry.Data;
                        this->coreRU->scheduleWriteIntoCache(newCacheEntry);
                        cout << "LS: Scheduled to store a copy in the local cache: " << newCacheEntry << endl;
                    }
                }
            }
        }
    }
}

// void LoadStage::step() {
//     cout << "----------------------- Load Stage " << this->base << " step function --------------------------" << endl;
//     if (!this->halted) {
//         if (this->OCCMEM->readDone) {
//             this->coreRU->scheduleToFillInCRS(this->LRSEntryInProgress.second.ResStatIndex.to_ulong(), this->LRSEntryInProgress.second.LowOrHigh, this->OCCMEM->lastReadData[0]);
//             cout << "LS: Scheduled to fill in Compute RS at index: " << this->LRSEntryInProgress.second.ResStatIndex.to_ulong() << endl;
//             cout << "LS: Scheduled data: " << this->LRSEntryInProgress.second.LowOrHigh << " " << this->OCCMEM->lastReadData[0] << endl;
//             this->coreRU->scheduleToSetLRSEToEmptyState(this->LRSEntryInProgress.first);
//             cout << "LS: Scheduled to set empty state in Load RS at index: " << this->LRSEntryInProgress.first << endl;
//             this->OCCMEM->readDone = false;

//             // Also try to keep a copy of this in the Local Cache.
//             IncomingCacheStruct newCacheEntry;
//             newCacheEntry.address = this->LRSEntryInProgress.second.OccMemoryAddress;
//             newCacheEntry.basePointer = this->LRSEntryInProgress.second.BasePointer;
//             vector<bitset<32>> dataPack;
//             dataPack.push_back(this->OCCMEM->lastReadData[0]);
//             newCacheEntry.data = dataPack;
//             this->coreRU->scheduleWriteIntoCache(newCacheEntry);
//             cout << "LS: Scheduled to store a copy in the local cache: " << newCacheEntry << endl;
//         }
//         else if (this->OCCMEM->isFree()) {
//             pair<int, LRSEntry> nle = this->coreRU->getNextLoadEntry();
//             if (nle.first != -1) {
//                 this->OCCMEM->readRequest(nle.second.OccMemoryAddress);
//                 this->LRSEntryInProgress = nle;
//                 cout << "LS: Sent Read Request from address: " << nle.second.OccMemoryAddress << endl;
//             }
//             else if (this->coreRU->isHalted())
//                 this->halted = true;
//         }
//     }
// }

bool LoadStage::isHalted() {
    return this->halted;
}