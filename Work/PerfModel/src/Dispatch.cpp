#include <Dispatch.h>

std::ostream& operator <<(std::ostream& os, const DispatchEntry& de) {
    os << de.base << "\t" << de.Low << "\t" << de.High << "\t" << de.SRSWBIndex << "\t" << de.ready << endl;
    return os;
}

DispatchUnit::DispatchUnit(Config* config) {
    this->dispatchScheme = config->parameters["DispatchScheme"];
    map<int, string> baseQName = {{0, "AQ"}, {1, "CQ"}, {2, "GQ"}, {3, "TQ"}};
    for (int i = 0; i < 4; i++) {
        Config * cfg = config->children[baseQName[i]];
        this->DispatchQueues[i] = new Queue<DispatchEntry>(cfg);
    }
}

void DispatchUnit::connect(FetchUnit * fu) {
    this->coreFU = fu;
}

pair<bool, DispatchEntry> DispatchUnit::popnext(int base) {
    if (!this->DispatchQueues[base]->isEmpty())
        return pair<bool, DispatchEntry>(true, this->DispatchQueues[base]->pop());
    return pair<bool, DispatchEntry>(false, *(new DispatchEntry));
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
        cout << "base: " << base << endl;
        if (base == bitset<3>(7)) {
            // Send to store queues - TBI
            // Reset SRS Entry status to Empty.
            this->coreFU->setEmptyState(nre.first);
        }
        //if base queue has space, schedule it and update the SRS entry state.
        else if (!this->DispatchQueues[base.to_ulong()]->isFull()) {
            DispatchEntry dispatchNewEntry;
            dispatchNewEntry.base = bitset<2>(base.to_ulong() & 3);
            dispatchNewEntry.Low = nre.second.LowPointer;
            dispatchNewEntry.High = nre.second.HighPointer;
            dispatchNewEntry.SRSWBIndex = bitset<6>(nre.first);
            dispatchNewEntry.ready = true;
            this->DispatchQueues[base.to_ulong()]->push(dispatchNewEntry);

            this->coreFU->setInProgress(nre.first);
            cout << "Dispatch queue:" << endl;
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