#include<Fetch.h>

FetchUnit::FetchUnit(Config * config, bitset<32> refCount) {
    this->NextSeedPointer = bitset<32>(0);
    this->RefCount = refCount;

    this->FillIdxQueue = new Queue<bitset<6>>("Fetch_FillIdxQ", config->children["FillIdxQ"]);
    this->FillIdxQueue->push(bitset<6>(0));
    this->FillIdxQueue->push(bitset<6>(1));
    this->FillIdxQueue->push(bitset<6>(2));
    this->FillIdxQueue->push(bitset<6>(3));

    this->SRS = new ReservationStation<SRSEntry>(config->children["SeedReservationStation"]);
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
                    SRSEntry newSRSEntry;
                    newSRSEntry.SeedAddress = bitset<32>(this->SeedPointer.to_ulong() + i);
                    newSRSEntry.Seed = nextReadData;
                    newSRSEntry.LowPointer = bitset<32>(0);
                    newSRSEntry.HighPointer = this->RefCount;
                    newSRSEntry.BasePointer = bitset<6>(0);
                    newSRSEntry.Ready = true;
                    newSRSEntry.Empty = false;
                    this->SRS->fill(bitset<6>(nextIdx), newSRSEntry);
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
            int nextFreeEntry = this->SRS->nextFreeEntry();
            cout << "Fetch: " << nextFreeEntry << endl;
            if (nextFreeEntry != -1) {
                this->FillIdxQueue->push(nextFreeEntry);
                this->SRS->setScheduledForFetch(nextFreeEntry);
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