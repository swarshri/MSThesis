#include <Reserve.h>

ComputeReservationStation::ComputeReservationStation(string name, SysConfig * config)
: ReservationStation(name, config) {
    for (auto entry = this->Entries.begin(); entry != this->Entries.end(); entry++) {
        (*entry).LowOccReady = false;
        (*entry).HighOccReady = false;
    }
}

void ComputeReservationStation::fillLowOccVal(int idx, uint64_t data) {
    this->Entries[idx].LowOcc = data;
    this->Entries[idx].LowOccReady = true;
    if (this->Entries[idx].HighOccReady)
        this->setReadyState(idx);
}

void ComputeReservationStation::fillHighOccVal(int idx, uint64_t data) {
    this->Entries[idx].HighOcc = data;
    this->Entries[idx].HighOccReady = true;
    if (this->Entries[idx].LowOccReady)
        this->setReadyState(idx);
}

ReserveStage::ReserveStage(SysConfig * config, string base, string iodir, PerformanceRecorder * perf) {
    this->base = base;
    this->base_num = config->BaseMap[base];

    this->CRS = new ComputeReservationStation("ComputeRS", config->children["ComputeReservationStation"]);
    this->LQ = new Queue<LQEntry>(config->children["LoadQ"]);

    if (config->has_child("Cache")) {
        this->LocalCache = new Cache(this->base, config->children["Cache"]);
        this->hasCache = true;
    }
    else {
        this->LocalCache = nullptr;
        this->hasCache = false;
    }

    this->halted = false;
    this->cycle_count = 0;
    this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(false, *(new DispatchQueueEntry));
    
    this->pendingEmptyCRSIdcs.first = false;
    this->pendingCRSEntries.first = false;
    this->pendingCacheInput.first = false;

    // Output file path    
#ifdef _WIN32
    this->op_file_path = iodir + "\\OP\\ReserveStage.out";
#else
    this->op_file_path = iodir + "OP/ReserveStage.out";
#endif
    // cout << "Output File: " << this->op_file_path << endl;

    ofstream output;
    output.open(this->op_file_path, ios_base::trunc);
    if (output.is_open()) {
        output.clear();
        output.close();
        // cout << "ReserveStage Output file opened." << endl;
    }
    // else
        // cout << "Unable to open file for ReserveStage Output." << this->op_file_path << endl;   
    
    this->perf = perf;
    this->name = "RS_" + this->base;
    // cout << "RS: name: " << this->name << endl;
    vector<string> metrics{this->name + "_LowOccCacheHit", 
                           this->name + "_HighOccCacheHit"};
                        //    this->name + "_NumberOfLoadRequestsQueued",
                        //    this->name + "_AllocatedCRSEntryNumber",
                        //    this->name + "_LowOccLoadRequestQueued",
                        //    this->name + "_HighOccLoadRequestQueued"};
    this->perf->addMetrics(metrics);

    this->numLowOccLookups = 0;
    this->numLowCacheHits = 0;
    this->numLowCacheMisses = 0;
    this->numHighOccLookups = 0;
    this->numHighCacheHits = 0;
    this->numHighCacheMisses = 0;
}

void ReserveStage::connect(DispatchStage * du) {
    this->coreDU = du;
}

void ReserveStage::step() {
    // cout << "----------------------- Reserve " << this->base << " Stage step function --------------------------" << endl;
    if (this->pendingEmptyCRSIdcs.first) {
        for (int idx: this->pendingEmptyCRSIdcs.second) {
            this->setCRSEToEmptyState(idx);
            // cout << "RS: Setting CRS to Empty state at index: " << idx << endl;
        }
        this->pendingEmptyCRSIdcs.second.clear();
        this->pendingEmptyCRSIdcs.first = false;
    }

    if (this->pendingCRSEntries.first) {
        for (auto entry: this->pendingCRSEntries.second) {
            int idx = get<0>(entry);
            bool lorh = get<1>(entry);
            uint64_t value = get<2>(entry);
            this->fillInCRS(idx, lorh, value);
            // cout << "RS: Filling in CRS data at index: " << idx << " with data: " << lorh << " " << value << endl;
        }
        this->print();
        this->pendingCRSEntries.second.clear();
        this->pendingCRSEntries.first = false;
    }

    if (this->hasCache && this->pendingCacheInput.first) {
        // cout << "RS: Pending Cache input: " << this->pendingCacheInput.second << endl;
        bool written = this->LocalCache->write(this->pendingCacheInput.second);
        // if (written)
        //     cout << "RS: Written into Local Cache." << endl;
        // else
        //     cout << "RS: Not stored in Local Cache." << endl;
        this->pendingCacheInput.first = false;
    }

    string perf_lowocccachehit = to_string(false);
    string perf_highocccachehit = to_string(false);

    if (!this->halted) {
        pair<bool, DispatchQueueEntry> currentDispatch;
        if (this->pendingToBeReserved.first) {
            currentDispatch = this->pendingToBeReserved;
            // cout << "RS: Using dispatch from the pending to be reserved due to resource constraints." << endl;
        }
        else {
            currentDispatch = this->coreDU->popNextDispatch(this->base[0]);
            // cout << "RS: Getting new dispatch. currentDispatch.first: " << currentDispatch.first << endl;
        }

        if (currentDispatch.first) {
            if (!this->pendingToBeReserved.first) this->numLowOccLookups += 2; // One for low and one for high.

            CRSEntry * newCRSEntry = new CRSEntry;
            newCRSEntry->LowOccReady = true;
            newCRSEntry->HighOccReady = true;

            pair<bool, uint64_t> cacheHitLowData;
            pair<bool, uint64_t> cacheHitHighData;
            if (this->hasCache) {
                cacheHitLowData = this->LocalCache->read(currentDispatch.second.LowPointer);
                cacheHitHighData = this->LocalCache->read(currentDispatch.second.HighPointer);
            }
            else {
                cacheHitLowData = pair<bool, uint64_t>(false, 0);
                cacheHitHighData = pair<bool, uint64_t>(false, 0);
            }

            if (cacheHitLowData.first) {
                newCRSEntry->LowOcc = cacheHitLowData.second;
                if (!this->pendingToBeReserved.first) this->numLowCacheHits++;
            }
            else {
                newCRSEntry->LowOccReady = false;
                newCRSEntry->LowOcc = 0;
                if (!this->pendingToBeReserved.first) this->numLowCacheMisses++;
            }
            perf_lowocccachehit = to_string(cacheHitLowData.first);

            if (cacheHitHighData.first) {
                newCRSEntry->HighOcc = cacheHitHighData.second;
                if (!this->pendingToBeReserved.first) this->numHighCacheHits++;
            }
            else {
                newCRSEntry->HighOccReady = false;
                newCRSEntry->HighOcc = 0;
                if (!this->pendingToBeReserved.first) this->numHighCacheMisses++;
            }            
            perf_highocccachehit = to_string(cacheHitHighData.first);

            newCRSEntry->SRSWBIndex = currentDispatch.second.SRSWBIndex;
            int nextCRSIdx = this->CRS->nextFreeEntry();
            
            if (nextCRSIdx != -1) {
                vector<LQEntry> newLoadRequests;
                if (!newCRSEntry->LowOccReady) {
                    LQEntry newLoadRequest;
                    newLoadRequest.LowOrHigh = false;
                    newLoadRequest.OccMemoryAddress = currentDispatch.second.LowPointer;
                    newLoadRequest.ResStatIndex = nextCRSIdx;
                    newLoadRequests.push_back(newLoadRequest);
                } 
                if (!newCRSEntry->HighOccReady) {
                    LQEntry newLoadRequest;
                    newLoadRequest.LowOrHigh = true;
                    newLoadRequest.OccMemoryAddress = currentDispatch.second.HighPointer;
                    newLoadRequest.ResStatIndex = nextCRSIdx;
                    newLoadRequests.push_back(newLoadRequest);
                }

                // this->perf->record(this->cycle_count, this->name + "_LowOccLoadRequestQueued", "NoLoadRequest");
                // this->perf->record(this->cycle_count, this->name + "_HighOccLoadRequestQueued", "NoLoadRequest");
                if (this->LQ->getEmptyCount() >= newLoadRequests.size()) {
                    for (auto nlr = newLoadRequests.begin(); nlr != newLoadRequests.end(); nlr++) {
                        this->LQ->push(*nlr);
                        // cout << "RS: Pushed address: " << nlr->OccMemoryAddress << " into Load Queue." << endl;
                        // cout << "RS: Low or High: " << nlr->LowOrHigh << endl;
                        // if (!nlr->LowOrHigh)
                        //     this->perf->record(this->cycle_count, this->name + "_LowOccLoadRequestQueued", to_string(nlr->OccMemoryAddress));
                        // else
                        //     this->perf->record(this->cycle_count, this->name + "_HighOccLoadRequestQueued", to_string(nlr->OccMemoryAddress));
                    }
                    // cout << "RS: Updated Load Reservation Station with " << newLoadRequests.size() << " new Load Requests." << endl;
                    // this->perf->record(this->cycle_count, this->name + "_NumberOfLoadRequestsQueued", to_string(newLoadRequests.size()));

                    this->CRS->fill(nextCRSIdx, *newCRSEntry);
                    // cout << "RS: Added into Compute Reservation Station at index: " << nextCRSIdx << endl;
                    // this->perf->record(this->cycle_count, this->name + "_AllocatedCRSEntryNumber", to_string(nextCRSIdx));
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
                    // cout << "RS: Not enough available resource in Load Reservation Station. Stalling by adding to pending."  << endl;
                    // cout << "RS: Pending Dispatch: " << this->pendingToBeReserved.second << endl;
                }
            }
            else { // No available resource in Compute Reservation Station.
                this->pendingToBeReserved = pair<bool, DispatchQueueEntry>(true, currentDispatch.second);
                // cout << "RS: Not enough available resource in Compute Reservation Station. Stalling by adding to pending."  << endl;
                // cout << "RS: Pending Dispatch: " << this->pendingToBeReserved.second << endl;
            }
        }
        else if (this->coreDU->isHalted() && !this->pendingToBeReserved.first)
            this->halted = true;
    }
    else
        cout << "RS: Halted" << endl;
    
    // cout << "RS: recording low cache hit: " << perf_lowocccachehit << endl;
    this->perf->record(this->cycle_count, this->name + "_LowOccCacheHit", perf_lowocccachehit);
    // cout << "RS: recording high cache hit: " << perf_highocccachehit << endl;
    this->perf->record(this->cycle_count, this->name + "_HighOccCacheHit", perf_highocccachehit);

    this->cycle_count++;
}

void ReserveStage::print() {
    // open output file
    ofstream output;
    string line;
    
    output.open(this->op_file_path, ios_base::app);

    if (output.is_open()) {
        this->LQ->show(cout);
        this->CRS->show(cout);

        output.close();
    }
    // else
        // cout << "Unable to open file for FetchStage Output." << this->op_file_path << endl;
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
    this->pendingEmptyCRSIdcs.second.push_back(idx);
    this->pendingEmptyCRSIdcs.first = true;
}

void ReserveStage::fillInCRS(int idx, bool high, uint64_t dataVal) {
    if (high)
        this->CRS->fillHighOccVal(idx, dataVal);
    else
        this->CRS->fillLowOccVal(idx, dataVal);
}

void ReserveStage::scheduleToFillInCRS(int idx, bool high, uint64_t dataVal) {
    this->pendingCRSEntries.second.push_back(tuple<int, bool, uint64_t>(idx, high, dataVal));
    this->pendingCRSEntries.first = true;
}

pair<bool, LQEntry> ReserveStage::getNextLoadEntry() {
    if (!this->LQ->isEmpty())
        return pair<bool, LQEntry>(true, this->LQ->next());
    return pair<bool, LQEntry>(false, *(new LQEntry));
}

pair<bool, LQEntry> ReserveStage::popNextLoadEntry() {
    if (!this->LQ->isEmpty())
        return pair<bool, LQEntry>(true, this->LQ->pop());
    return pair<bool, LQEntry>(false, *(new LQEntry));
}

void ReserveStage::scheduleWriteIntoCache(IncomingCacheStruct cacheInput) {
    this->pendingCacheInput.second = cacheInput;
    this->pendingCacheInput.first = true;
}

uint64_t ReserveStage::getNumLowOccLookups() {
    return this->numLowOccLookups;
}

uint64_t ReserveStage::getNumLowCacheHits() {
    return this->numLowCacheHits;
}

uint64_t ReserveStage::getNumLowCacheMisses() {
    return this->numLowCacheMisses;
}

uint64_t ReserveStage::getNumHighOccLookups() {
    return this->numHighOccLookups;
}

uint64_t ReserveStage::getNumHighCacheHits() {
    return this->numHighCacheHits;
}

uint64_t ReserveStage::getNumHighCacheMisses() {
    return this->numHighCacheMisses;
}