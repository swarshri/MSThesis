#include<Fetch.h>

SeedReservationStation::SeedReservationStation(Config * config, bitset<32> refCount) {
    this->numEntries = config->parameters["EntryCount"];
    this->Entries.resize(this->numEntries);
    this->refCount = refCount;
}

void SeedReservationStation::fill(bitset<6> idx, bitset<64> seed, bitset<32> seedaddress) {
    int indx = idx.to_ulong();
    this->Entries[indx].SeedAddress = seedaddress;
    this->Entries[indx].Seed = seed;
    this->Entries[indx].LowPointer = bitset<32>(0);
    this->Entries[indx].HighPointer = this->refCount;
    this->Entries[indx].BasePointer = bitset<6>(0);
    this->Entries[indx].Ready = true;
    this->Entries[indx].Empty = false;
}

pair<bool, bitset<6>> SeedReservationStation::nextFreeEntry() {
    for (int i = 0; i < this->numEntries; i++) {
        if (this->Entries[i].Empty && !this->Entries[i].Ready) {
            this->Entries[i].Ready = true;
            return pair<bool, bitset<6>>(true, bitset<6>(i));
        }
    }
    return pair<bool, bitset<6>>(false, bitset<6>(0));
}

pair<int, SRSEntry> SeedReservationStation::nextReadyEntry() {
    for (int i = 0; i < this->numEntries; i++) {
        // cout << "NextReadyEntry: " << i << endl;
        // cout << "Entry.Empty: " << this->Entries[i].Empty;
        // cout << "Entry.Ready: " << this->Entries[i].Ready;
        if (!this->Entries[i].Empty && this->Entries[i].Ready) {
            return pair<int, SRSEntry>(i, this->Entries[i]);
        }
    }
    return pair<int, SRSEntry>(-1, *(new SRSEntry));
}

void SeedReservationStation::setEmptyState(int idx) {
    this->Entries[idx].Empty = true;
    this->Entries[idx].Ready = false;
}

void SeedReservationStation::setScheduledForFetch(int idx) {
    this->Entries[idx].Empty = true;
    this->Entries[idx].Ready = true;
}

void SeedReservationStation::setReadyForDispatch(int idx) {
    this->Entries[idx].Empty = false;
    this->Entries[idx].Ready = true;
}

void SeedReservationStation::setDispatched(int idx) {
    this->Entries[idx].Empty = false;
    this->Entries[idx].Ready = false;
}

void SeedReservationStation::updateBasePointer(int idx) {
    this->Entries[idx].BasePointer = bitset<6>(this->Entries[idx].BasePointer.to_ulong() + 3);
}

bool SeedReservationStation::isEmpty() {
    for (auto entry = this->Entries.begin(); entry != this->Entries.end(); entry++) {
        if (!(*entry).Empty || (*entry).Ready)
            return false;
    }
    return true;
}

void SeedReservationStation::show() {
    cout << "=================================================================================================================" << endl;
    cout << "Idx\t Seed Address\t\t\t\t Seed\t\t\t\t\t\t\t\t\t Low Pointer\t\t\t\t High Pointer\t\t\t\t Base Pointer\t Ready\t Valid" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------------------------------------";
    cout << "------------------------------------------------------------------------------------------------" << endl;
    int i = 0;
    for (auto entry: this->Entries) {
        cout << i << '\t' << entry.SeedAddress << '\t' << entry.Seed << '\t' << entry.LowPointer << '\t' << entry.HighPointer << '\t' <<  entry.BasePointer << "\t\t" << entry.Ready << '\t' << entry.Empty << endl;
        i++;
    }
}

FetchUnit::FetchUnit(Config * config, bitset<32> refCount) {
    this->NextSeedPointer = bitset<32>(0);

    this->FillIdxQueue = new Queue<bitset<6>>("Fetch_FillIdxQ", config->children["FillIdxQ"]);
    this->FillIdxQueue->push(bitset<6>(0));
    this->FillIdxQueue->push(bitset<6>(1));
    this->FillIdxQueue->push(bitset<6>(2));
    this->FillIdxQueue->push(bitset<6>(3));

    this->SRS = new SeedReservationStation(config->children["SeedReservationStation"], refCount);
    this->SRS->setScheduledForFetch(0);
    this->SRS->setScheduledForFetch(1);
    this->SRS->setScheduledForFetch(2);
    this->SRS->setScheduledForFetch(3);

    this->cycle_count = 0;
    this->halted = false;
}

void FetchUnit::connect(DRAM * sdmem) {
    this->SDMEM =  sdmem;
}

void FetchUnit::step() {
    if (!this->halted) {
        if (this->SDMEM->readDone) {
            for (int i = 0; i < this->SDMEM->getChannelWidth(); i++) {
                bitset<64> nextReadData = this->SDMEM->lastReadData[i];
                if (nextReadData.count() == 64) {
                    this->halted = true;
                    while(!this->FillIdxQueue->isEmpty()) {
                        int idx = this->FillIdxQueue->pop().to_ulong();
                        this->SRS->setEmptyState(idx);
                        cout << "Fetch Halted: " << endl;
                        this->SRS->show();
                    }
                    break;
                }
                else {
                    int nextIdx = this->FillIdxQueue->pop().to_ulong();
                    this->SRS->fill(bitset<6>(nextIdx), nextReadData, bitset<32>(this->SeedPointer.to_ulong() + i));
                }
            }
            this->SDMEM->readDone = false;
            this->SRS->show();
        }

        if (this->SDMEM->isFree()) {
            int srsVacancy = this->FillIdxQueue->getCount();
            if (srsVacancy >= this->SDMEM->getChannelWidth()) {
                this->SeedPointer = this->NextSeedPointer;
                this->SDMEM->readAccess(this->SeedPointer);
                this->NextSeedPointer = bitset<32>(this->SeedPointer.to_ulong() + this->SDMEM->getChannelWidth());
            }
        }
        else if (!this->FillIdxQueue->isFull()) {
            pair<bool, bitset<6>> nfe = this->SRS->nextFreeEntry();
            if (nfe.first) {
                this->FillIdxQueue->push(nfe.second);
                this->SRS->setScheduledForFetch(nfe.second.to_ulong());
            }
            this->FillIdxQueue->print();
        }
        
        this->cycle_count++;
    }
}

pair<int, SRSEntry> FetchUnit::getNextReadyEntry() {
    return this->SRS->nextReadyEntry();
}

void FetchUnit::setInProgress(int idx) {
    this->SRS->setDispatched(idx);
    this->SRS->updateBasePointer(idx);
}

void FetchUnit::setEmptyState(int idx) {
    this->SRS->setEmptyState(idx);
}

void FetchUnit::setReadyState(int idx) {
    this->SRS->setReadyForDispatch(idx);
}

bool FetchUnit::emptySRS() {
    return this->SRS->isEmpty();
}

void FetchUnit::print() {
    this->SRS->show();
}