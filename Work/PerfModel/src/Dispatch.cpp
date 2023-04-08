#include <Dispatch.h>

DispatchStage::DispatchStage(SysConfig* config, string iodir) {
    this->dispatchScheme = config->parameters["DispatchScheme"];
    map<int, string> baseQName = {{0, "DispatchAQ"}, {1, "DispatchCQ"}, {2, "DispatchGQ"}, {3, "DispatchTQ"}};
    for (int i = 0; i < 4; i++) {
        SysConfig * cfg = config->children[baseQName[i]];
        this->DispatchQueues[i] = new Queue<DispatchQueueEntry>(cfg);
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

void DispatchStage::dispatchSequential(int count) {
    // count is not enabled - only single dispatch is implemented.
    // Implements one-entry-at-a-time scheme - REALLY?! find a better name for your own sake!
    pair<int, SRSEntry> nre = this->coreFU->getNextReadyEntry();
    // cout << "Dispatch Scheme: Single Sequential " << this->cycle_count << endl;
    if (nre.first != -1) {
        cout << "Seed Address: " << nre.second.SeedAddress << endl;
        // cout << "Seed: " << nre.second.Seed << endl;
        // cout << "BP: " << nre.second.BasePointer << endl;
        bitset<3> base = bitset<3>((nre.second.Seed.to_ulong() >> nre.second.BasePointer.to_ulong()) & 7);
        // cout << "Base: " << base << endl;
        if (base == bitset<3>(7) || nre.second.StoreFlag) {
            StoreQueueEntry newStoreQueueEntry;
            newStoreQueueEntry.StoreAddress = nre.second.SeedAddress;
            newStoreQueueEntry.StoreVal = bitset<64>((nre.second.LowPointer.to_ulong() << 32) + nre.second.HighPointer.to_ulong());
            this->StoreQueue->push(newStoreQueueEntry);
            cout << "DS: Queued into Store Queue." << endl;
            this->StoreQueue->show(cout);
            // this->print();
            // Reset SRS Entry status to Empty.
            this->coreFU->scheduleToSetEmptyState(nre.first);
            cout << "Setting SRS entry at " << nre.first << " to empty state." << endl;
            // this->coreFU->print();
            // cout << "DS: Scheduled to set Empty state in FU SRS at index: " << nre.first << endl;
        }
        //if base queue has space, schedule it and update the SRS entry state.
        else if (!this->DispatchQueues[base.to_ulong()]->isFull()) {
            DispatchQueueEntry dispatchNewEntry;
            dispatchNewEntry.base = bitset<2>(base.to_ulong() & 3);
            dispatchNewEntry.LowPointer = nre.second.LowPointer;
            dispatchNewEntry.HighPointer = nre.second.HighPointer;
            dispatchNewEntry.SRSWBIndex = bitset<6>(nre.first);
            dispatchNewEntry.BasePointer = nre.second.BasePointer;
            this->DispatchQueues[base.to_ulong()]->push(dispatchNewEntry);

            this->coreFU->setInProgress(nre.first);
            // cout << "DS: Queued into Dispatch <" << base << "> Queue." << endl;
            // this->DispatchQueues[base.to_ulong()]->show(cout);
            //this->print();
        }
        else {
            // Right now, nothing - wait until the queue has vacancy. This could mean that the entire pipeline will
            //      be stalled until the corresponding memory buffer has a vacancy.
            // other possible schemes - Next cycle should not take the last cycle into consideration.
        }
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

void DispatchStage::step() {
    cout << "----------------------- Dispatch Stage step function --------------------------" << endl;
    if (!this->halted) {
        switch (this->dispatchScheme) {
            case SEQ1PE:
                this->dispatchSequential(1);
                break;
            
            case PAR4PE:
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

pair<bool, DispatchQueueEntry> DispatchStage::popNextDispatch(int base) {
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

pair<bool, DispatchQueueEntry> DispatchStage::getNextDispatch(int base) {
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