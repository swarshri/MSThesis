#include<Fetch.h>

SeedReservationStation::SeedReservationStation(Config * config, bitset<32> refCount) {
    this->numEntries = config->parameters["EntryCount"];
    this->Entries.resize(this->numEntries);
    this->refCount = refCount;
}

int SeedReservationStation::numVacant() {
    int vacantCount = 0;
    for (int i = 0; i < this->numEntries; i++) {
        if (!this->Entries[i].Valid) {
            vacantCount++;
            this->nextFreeEntries.push_back(i);
        }
    }
    return vacantCount;
}

void SeedReservationStation::populate(vector<bitset<64>> seeds, bitset<32> seedaddress) {
    int i = 0;
    for (auto itr = seeds.begin(); itr != seeds.end(); itr++) {
        int idx = this->nextFreeEntries[i];
        this->Entries[idx].SeedAddress = bitset<32>(seedaddress.to_ulong() + i);
        this->Entries[idx].Seed = seeds[i];
        this->Entries[idx].LowPointer = bitset<32>(0);
        this->Entries[idx].HighPointer = this->refCount;
        this->Entries[idx].BasePointer = bitset<6>(0);
        this->Entries[idx].Ready = true;
        this->Entries[idx].Valid = true;
        i++;
    }
}

FetchUnit::FetchUnit(Config * config, bitset<32> refCount) {
    this->NextSeedPointer = bitset<32>(0);

    this->bufferSize = config->parameters["BufferSize"];
    this->Buffer.resize(this->bufferSize);
    this->bufferPtr = 0;

    this->SRS = new SeedReservationStation(config->children["SeedReservationStation"], refCount);
}

void FetchUnit::connect(DRAM * sdmem) {
    this->SDMEM =  sdmem;
}

void FetchUnit::step() {
    if (this->SDMEM->readDone) {
        this->SRS->populate(this->SDMEM->lastReadData, this->SeedPointer);
        this->SDMEM->readDone = false;
    }

    int srsVacancy = this->SRS->numVacant();
    if (this-SDMEM->isFree() && srsVacancy >= this->SDMEM->getChannelWidth()) {
        this->SeedPointer = this->NextSeedPointer;
        this->SDMEM->readAccess(this->SeedPointer);
        this->NextSeedPointer = bitset<32>(this->SeedPointer.to_ulong() + this->SDMEM->getChannelWidth());
    }
}