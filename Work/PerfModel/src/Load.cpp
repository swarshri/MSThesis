#include <Load.h>

LoadStage::LoadStage(SysConfig * config, char base, string iodir) {
    this->base = base;
    this->halted = false;
    this->cycle_count = 0;

    this->LQEntryInProgress.clear();
}

void LoadStage::connectRU(ReserveStage * ru) {
    this->coreRU = ru;
}

void LoadStage::connectDRAM(DRAMW<32, 32> * occmem) {
    this->OCCMEM = occmem;
}

void LoadStage::step() {
    cout << "----------------------- Load Stage " << this->base << " step function --------------------------" << endl;
    if (!this->halted) {
        // Get all finished reads from OCCMEM and write into CRS
        pair<bool, vector<PMAEntry<32>>> nwb = this->OCCMEM->getNextWriteBack();
        // cout << "next writeback acquired. nwb.first:" << nwb.first << endl;
        if (nwb.first) {
            // cout << "nwb.second.size(): " << nwb.second.size() << endl;
            for (auto wbentry : nwb.second) {
                // cout << "wbentry: " << wbentry << endl;
                // cout << "this->LRSEntryInProgress.size(): " << this->LQEntryInProgress.size() << endl;
                for (int i = 0; i < this->LQEntryInProgress.size(); i++) {
                    auto inprogentry = this->LQEntryInProgress[i];
                    // cout << "inprogentry: " << inprogentry << endl;
                    if (inprogentry.OccMemoryAddress == wbentry.AccessAddress && inprogentry.ResStatIndex == wbentry.RequestID) {
                        this->coreRU->scheduleToFillInCRS(inprogentry.ResStatIndex.to_ulong(), inprogentry.LowOrHigh, wbentry.Data[0]);
                        // cout << "LS: Scheduled to fill in Compute RS at index: " << inprogentry.ResStatIndex.to_ulong() << endl;
                        // cout << "LS: Scheduled at Low/High: " << inprogentry.LowOrHigh << " with data: " << wbentry.Data[0] << endl;

                        // Also try to keep a copy of this in the Local Cache.
                        IncomingCacheStruct newCacheEntry;
                        newCacheEntry.address = inprogentry.OccMemoryAddress;
                        newCacheEntry.basePointer = inprogentry.BasePointer;
                        newCacheEntry.data = wbentry.Data;
                        this->coreRU->scheduleWriteIntoCache(newCacheEntry);
                        // cout << "LS: Scheduled to store a copy in the local cache: " << newCacheEntry << endl;

                        this->LQEntryInProgress.erase(this->LQEntryInProgress.begin() + i);
                        // cout << "after process: this->LQEntryInProgress.size(): " << this->LQEntryInProgress.size() << endl;
                        break;
                    }
                }
            }
        }
        // Get the next load request from the Load Reservation Station.
        pair<bool, LQEntry> nle = this->coreRU->getNextLoadEntry();
        if (nle.first) {
            bool success = this->OCCMEM->willAcceptRequest(nle.second.OccMemoryAddress, false);
            if (success) {
                this->OCCMEM->readRequest(nle.second.OccMemoryAddress, nle.second.ResStatIndex.to_ulong());
                this->coreRU->popNextLoadEntry();
                // cout << "Successfully sent read request for LQ entry index: " << nle.first << endl;
                // cout << "Read request sent for OccMEM address: " << nle.second.OccMemoryAddress << endl;
                this->LQEntryInProgress.push_back(nle.second);
            }
            // else
                // cout << "Couldn't send read request to OccMEM address: " << nle.second.OccMemoryAddress << endl; 
        }
        else {
            // cout << "No ready entry in LQ to request read." << endl;
            if (this->coreRU->isHalted() && this->LQEntryInProgress.size() == 0)
                this->halted = true;
        }
        this->cycle_count++;
    }
    else
        cout << "LS: Halted" << endl;
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