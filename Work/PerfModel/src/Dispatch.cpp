#include <Dispatch.h>

DispatchUnit::DispatchUnit(Config* config) {
    this->dispatchScheme = config->parameters["DispatchScheme"];
    map<int, string> baseQName = {{0, "DispatchAQ"}, {1, "DispatchCQ"}, {2, "DispatchGQ"}, {3, "DispatchTQ"}};
    for (int i = 0; i < 4; i++) {
        Config * cfg = config->children[baseQName[i]];
        this->DispatchQueues[i] = new Queue<DispatchQueueEntry>(cfg);
    }
    this->StoreQueue = new Queue<StoreQueueEntry>(config->children["StoreQ"]);
}

void DispatchUnit::connect(FetchUnit * fu) {
    this->coreFU = fu;
}

bool DispatchUnit::isHalted() {
    return this->halted;
}

pair<bool, DispatchQueueEntry> DispatchUnit::popNextDispatch(int base) {
    if (!this->DispatchQueues[base]->isEmpty())
        return pair<bool, DispatchQueueEntry>(true, this->DispatchQueues[base]->pop());
    return pair<bool, DispatchQueueEntry>(false, *(new DispatchQueueEntry));
}

pair<bool, StoreQueueEntry> DispatchUnit::popNextStore() {
    if (!this->StoreQueue->isEmpty())
        return pair<bool, StoreQueueEntry>(true, this->StoreQueue->pop());
    return pair<bool, StoreQueueEntry>(false, *(new StoreQueueEntry));
}

void DispatchUnit::dispatchSequential(int count) {
    // count is not enabled - only single dispatch is implemented.
    // Implements one-entry-at-a-time scheme - REALLY?! find a better name for your own sake!
    pair<int, SRSEntry> nre = this->coreFU->getNextReadyEntry();
    cout << "Dispatcher Unit: " << this->cycle_count << "\t" << nre.first << endl;
    if (nre.first != -1) {
        cout << "Seed: " << nre.second.Seed << endl;
        cout << "BP: " << nre.second.BasePointer << endl;
        bitset<3> base = bitset<3>((nre.second.Seed.to_ulong() >> nre.second.BasePointer.to_ulong()) & 7);
        cout << "Base: " << base << endl;
        if (base == bitset<3>(7)) {
            StoreQueueEntry newStoreQueueEntry;
            newStoreQueueEntry.StoreAddress = nre.second.SeedAddress;
            newStoreQueueEntry.StoreVal = bitset<64>((nre.second.LowPointer.to_ulong() << 32) + nre.second.HighPointer.to_ulong());
            this->StoreQueue->push(newStoreQueueEntry);
            // Reset SRS Entry status to Empty.
            this->coreFU->setEmptyState(nre.first);
            cout << "Store Queue:" << endl;
            this->StoreQueue->print();
        }
        //if base queue has space, schedule it and update the SRS entry state.
        else if (!this->DispatchQueues[base.to_ulong()]->isFull()) {
            DispatchQueueEntry dispatchNewEntry;
            dispatchNewEntry.base = bitset<2>(base.to_ulong() & 3);
            dispatchNewEntry.LowPointer = nre.second.LowPointer;
            dispatchNewEntry.HighPointer = nre.second.HighPointer;
            dispatchNewEntry.SRSWBIndex = bitset<6>(nre.first);
            dispatchNewEntry.ready = true;
            this->DispatchQueues[base.to_ulong()]->push(dispatchNewEntry);

            this->coreFU->setInProgress(nre.first);
            cout << "Dispatch Queue:" << endl;
            this->DispatchQueues[base.to_ulong()]->print();
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

void DispatchUnit::step() {
    if (!this->halted) {
        switch (this->dispatchScheme) {
            case SEQ1PE: {
                this->dispatchSequential(1);
                break;
            }
            
            case PAR4PE:
                break;

            default:
                break;
        }
        if (this->coreFU->halted && this->coreFU->emptySRS())
            this->halted = true;

        this->cycle_count++;
    }
}