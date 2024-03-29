#include <Dispatch.h>

DispatchStage::DispatchStage(SysConfig* config, string iodir) {
    this->dispatchScheme = config->parameters["DispatchScheme"];
    vector<string> disqs = {"DispatchAQ", "DispatchCQ", "DispatchGQ", "DispatchTQ"};
    for (string cfg_name: disqs) {
        char base = cfg_name[8];
        SysConfig * cfg = config->children[cfg_name];
        this->DispatchQueues[base] = new Queue<DispatchQueueEntry>(cfg);
        this->numCyclesWithNewDispatch[base] = 0;
        this->numCyclesWithNoNewDispatch[base] = 0;
        this->numCyclesWithDispatchStalled[base] = 0;
    }
    this->StoreQueue = new Queue<StoreQueueEntry>(config->children["StoreQ"]);
    
    // Output file path    
#ifdef _WIN32
    this->op_file_path = iodir + "\\OP\\DispatchStage.out";
#else
    this->op_file_path = iodir + "OP/DispatchStage.out";
#endif
    // cout << "Output File: " << this->op_file_path << endl;

    ofstream output;
    output.open(this->op_file_path, ios_base::trunc);
    if (output.is_open()) {
        output.clear();
        output.close();
        // cout << "DispatchStage Output file opened." << endl;
    }
    // else
        // cout << "Unable to open file for DispatchStage Output." << this->op_file_path << endl;
}

void DispatchStage::print() {
    // open output file
    ofstream output;
    string line;
    
    output.open(this->op_file_path, ios_base::app);

    if (output.is_open()) {
        for (auto dispatchQueue: this->DispatchQueues)
            dispatchQueue.second->show(cout);
        this->StoreQueue->show(cout);

        output.close();
    }
    // else
        // cout << "Unable to open file for FetchStage Output." << this->op_file_path << endl;
}

bool DispatchStage::isHalted() {
    return this->halted;
}

void DispatchStage::connect(FetchStage * fu) {
    this->coreFU = fu;
}

void DispatchStage::dispatchLRIO(int count) {
    // count is not enabled - only single dispatch is implemented.
    // Implements one-entry-at-a-time scheme - REALLY?! find a better name for your own sake!
    pair<int, SRSEntry> nre = this->coreFU->getNextReadyEntry();
    // cout << "Dispatch Scheme: Single Sequential " << this->cycle_count << endl;
    char base = 'n';
    if (nre.first != -1) {
        // cout << "Seed Address: " << nre.second.SeedAddress << endl;
        // cout << "Seed: " << nre.second.Seed << endl;
        // cout << "BP: " << nre.second.BasePointer << endl;
        unsigned int idx = nre.second.Seed.size() - 1 - nre.second.BasePointer;
        base = nre.second.Seed[idx];
        // cout << "Idx: " << idx << " Base: " << base << endl;
        if (base == 'E' || nre.second.StoreFlag) {
            if (!this->StoreQueue->isFull()) {
                StoreQueueEntry newStoreQueueEntry;
                newStoreQueueEntry.StoreAddress = nre.second.SeedAddress;
                newStoreQueueEntry.StoreValLow = nre.second.LowPointer;
                newStoreQueueEntry.StoreValHigh = nre.second.HighPointer;
                this->StoreQueue->push(newStoreQueueEntry);
                // if (nre.second.SeedAddress == 0) {
                //     cout << "SQ found Seed Address: " << nre.second.SeedAddress << endl;
                //     cout << "Store Val Low: " << nre.second.LowPointer << endl;
                //     cout << "Store Val High: " << nre.second.HighPointer << endl;
                //     cout << "DS: Queued into Store Queue." << endl;
                //     this->StoreQueue->show(cout);
                //     this->print();
                // }
                // Reset SRS Entry status to Empty.
                this->coreFU->scheduleToSetEmptyState(nre.first);
                // cout << "Setting SRS entry at " << nre.first << " to empty state." << endl;
                // this->coreFU->print();
                // cout << "DS: Scheduled to set Empty state in FU SRS at index: " << nre.first << endl;
            }
        }
        //if base queue has space, schedule it and update the SRS entry state.
        else {
            if (!this->DispatchQueues[base]->isFull()) { // This will run into error when base is N or any character other than A, C, G, or T.
                DispatchQueueEntry dispatchNewEntry;
                dispatchNewEntry.base = base;
                dispatchNewEntry.LowPointer = nre.second.LowPointer;
                dispatchNewEntry.HighPointer = nre.second.HighPointer;
                dispatchNewEntry.SRSWBIndex = nre.first;
                dispatchNewEntry.BasePointer = nre.second.BasePointer;
                this->DispatchQueues[base]->push(dispatchNewEntry);
                this->coreFU->setInProgress(nre.first);
                // if (nre.second.SeedAddress == 0) {
                //     cout << "Seed Address 0 being dispatched. Base: " << base << endl;
                //     cout << "Low Pointer: " << nre.second.LowPointer << endl;
                //     cout << "High Pointer: " << nre.second.HighPointer << endl;
                //     cout << "DS: Queued into Dispatch <" << base << "> Queue." << endl;
                //     this->DispatchQueues[base]->show(cout);
                //     this->print();
                // }
                this->numCyclesWithNewDispatch[base]++;
            }
            else {
                this->numCyclesWithDispatchStalled[base]++;
                // Right now, nothing - wait until the queue has vacancy. This could mean that the entire pipeline will
                //      be stalled until the corresponding memory buffer has a vacancy.
                // other possible schemes - Next cycle should not take the last cycle into consideration.
            }
        }
    }

    for (char b: "ACGT") {
        if (b != base)
            this->numCyclesWithNoNewDispatch[b]++;
    }

    // FOLLOWING LINES OF CODE are to be REMOVED - TBI
    // for (int i = 0; i < 4; i++) {
    //     if (!this->DispatchQueues[i]->isEmpty()) {
    //         DispatchEntry de = this->DispatchQueues[i]->pop();
    //         this->coreFU->setReadyState(de.SRSWBIndex.to_ulong());
    //     }
    // }
    // ABOVE LINES OF CODE are to be REMOVED - DEBUG Code to halt in the absence of next stages in pipeline.
}

void DispatchStage::dispatchBWLRIO() {
    // iterate through the dispatch queues.
    for (auto dispq: this->DispatchQueues) {
        char base = dispq.first;
        if (!dispq.second->isFull()) {
            pair<int, SRSEntry> nre = this->coreFU->getNextReadyEntry(base);
            if (nre.first != -1) {
                // if (nre.second.SeedAddress == 4) {
                //     cout << "Seed Address 4 being dispatched. Base: " << base << endl;
                //     cout << "Low Pointer: " << nre.second.LowPointer << endl;
                //     cout << "High Pointer: " << nre.second.HighPointer << endl;
                // }
                DispatchQueueEntry dispatchNewEntry;
                dispatchNewEntry.base = base;
                dispatchNewEntry.LowPointer = nre.second.LowPointer;
                dispatchNewEntry.HighPointer = nre.second.HighPointer;
                dispatchNewEntry.SRSWBIndex = nre.first;
                dispatchNewEntry.BasePointer = nre.second.BasePointer;
                this->DispatchQueues[base]->push(dispatchNewEntry);

                this->coreFU->setInProgress(nre.first);
                // cout << "DS: Queued into Dispatch <" << base << "> Queue." << endl;
                // this->DispatchQueues[base]->show(cout);
                // this->print();
                this->numCyclesWithNewDispatch[base]++;
            }
            else
                this->numCyclesWithNoNewDispatch[base]++;
        }
        else
            this->numCyclesWithDispatchStalled[base]++;
    }
    // Dispatch for Store.
    if (!this->StoreQueue->isFull()) {
        pair<int, SRSEntry> nre = this->coreFU->getNextStoreEntry();
        if (nre.first != -1) {
            StoreQueueEntry newStoreQueueEntry;
            newStoreQueueEntry.StoreAddress = nre.second.SeedAddress;
            newStoreQueueEntry.StoreValLow = nre.second.LowPointer;
            newStoreQueueEntry.StoreValHigh = nre.second.HighPointer;
            this->StoreQueue->push(newStoreQueueEntry);
            // if (nre.second.SeedAddress == 4) {
            //     cout << "SQ found Seed Address: " << nre.second.SeedAddress << endl;
            //     cout << "Store Val Low: " << nre.second.LowPointer << endl;
            //     cout << "Store Val High: " << nre.second.HighPointer << endl;
            //     cout << "DS: Queued into Store Queue." << endl;
            //     this->StoreQueue->show(cout);
            //     this->print();
            // }
            // cout << "DS: Queued into Store Queue." << endl;
            // this->StoreQueue->show(cout);
            // this->print();
            // Reset SRS Entry status to Empty.
            this->coreFU->scheduleToSetEmptyState(nre.first);
            // cout << "Setting SRS entry at " << nre.first << " to empty state." << endl;
            // this->coreFU->print();
            // cout << "DS: Scheduled to set Empty state in FU SRS at index: " << nre.first << endl;
        }
    }
}

void DispatchStage::step() {
    // cout << "----------------------- Dispatch Stage step function --------------------------" << endl;
    if (!this->halted) {
        switch (this->dispatchScheme) {
            case LRIO:
                // cout << "DS: In case BLRIO." << endl;
                this->dispatchLRIO(1);
                break;
            
            case BWLRIO:
                // cout << "DS: In case BWLRIO." << endl;
                this->dispatchBWLRIO();
                break;

            default:
                break;
        }
        // cout << "this->coreFU->emptySRS(): " << this->coreFU->emptySRS() << endl;
        if (this->coreFU->isHalted() && this->coreFU->emptySRS())
            this->halted = true;

        this->cycle_count++;
    }
    else
        cout << "DS: Halted" << endl;
}

pair<bool, DispatchQueueEntry> DispatchStage::popNextDispatch(char base) {
    // cout << "Pop next dispatch for base: " << base << " is Queue empty? " << this->DispatchQueues[base]->isEmpty() << endl;
    if (!this->DispatchQueues[base]->isEmpty())
        return pair<bool, DispatchQueueEntry>(true, this->DispatchQueues[base]->pop());
    return pair<bool, DispatchQueueEntry>(false, *(new DispatchQueueEntry));
}

pair<bool, StoreQueueEntry> DispatchStage::popNextStore() {
    // cout << "Pop next dispatch for store: is Queue empty? " << this->StoreQueue->isEmpty() << endl;
    if (!this->StoreQueue->isEmpty())
        return pair<bool, StoreQueueEntry>(true, this->StoreQueue->pop());
    return pair<bool, StoreQueueEntry>(false, *(new StoreQueueEntry));
}

pair<bool, DispatchQueueEntry> DispatchStage::getNextDispatch(char base) {
    // cout << "Pop next dispatch for base: " << base << " is Queue empty? " << this->DispatchQueues[base]->isEmpty() << endl;
    if (!this->DispatchQueues[base]->isEmpty())
        return pair<bool, DispatchQueueEntry>(true, this->DispatchQueues[base]->next());
    return pair<bool, DispatchQueueEntry>(false, *(new DispatchQueueEntry));
}

pair<bool, StoreQueueEntry> DispatchStage::getNextStore() {
    // cout << "Pop next dispatch for store: is Queue empty? " << this->StoreQueue->isEmpty() << endl;
    if (!this->StoreQueue->isEmpty())
        return pair<bool, StoreQueueEntry>(true, this->StoreQueue->next());
    return pair<bool, StoreQueueEntry>(false, *(new StoreQueueEntry));
}

uint64_t DispatchStage::getNumCyclesWithNewDispatch(char base) {
    return this->numCyclesWithNewDispatch[base];
}

uint64_t DispatchStage::getNumCyclesWithNoNewDispatch(char base) {
    return this->numCyclesWithNoNewDispatch[base];
}