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

void LoadStage::connectDRAM(DRAMW<32, 32> * occmem) {
    this->OCCMEM = occmem;
}

void LoadStage::step() {
    cout << "----------------------- Load Stage " << this->base << " step function --------------------------" << endl;
    if (!this->halted) {
        // Get the next load request from the Load Reservation Station.
        pair<int, LRSEntry> nle = this->coreRU->getNextLoadEntry();
        if (nle.first != -1) {
            bool success = this->OCCMEM->willAcceptRequest(nle.second.OccMemoryAddress, false);
            if (success) {
                this->OCCMEM->readRequest(nle.second.OccMemoryAddress, nle.first);
                this->coreRU->scheduleToSetLRSEToScheduledState(nle.first);
                cout << "Successfully sent read request for LRS entry index: " << nle.first << endl;
                cout << "Read request sent for OccMEM address: " << nle.second.OccMemoryAddress << endl;
                this->LRSEntryInProgress.push_back(nle);
            }
            else
                cout << "Couldn't send read request to OccMEM address: " << nle.second.OccMemoryAddress << endl; 
        }
        else {
            cout << "No ready entry in LRS to request read." << endl;
        }

        // Get all finished reads from OCCMEM and write into CRS
        pair<bool, vector<PMAEntry<32>>> nwb = this->OCCMEM->getNextWriteBack();
        cout << "next writeback acquired. nwb.first:" << nwb.first << endl;
        if (nwb.first) {
            for (auto wbentry : nwb.second) {
                for (auto inprogentry : this->LRSEntryInProgress) {
                    if (inprogentry.first == wbentry.RequestID) {
                        this->coreRU->scheduleToFillInCRS(inprogentry.second.ResStatIndex.to_ulong(), inprogentry.second.LowOrHigh, wbentry.Data[0]);
                        cout << "LS: Scheduled to fill in Compute RS at index: " << inprogentry.second.ResStatIndex.to_ulong() << endl;
                        cout << "LS: Scheduled data: " << inprogentry.second.LowOrHigh << " " << this->OCCMEM->lastReadData[0] << endl;
                        this->coreRU->scheduleToSetLRSEToEmptyState(inprogentry.first);
                        cout << "LS: Scheduled to set empty state in Load RS at index: " << inprogentry.first << endl;

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
        this->cycle_count++;
    }
    // Logic for halting load stage.
    if (this->cycle_count > 3000)
        this->halted = true;
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