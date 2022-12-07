#include<Fetch.h>

SeedReservationStation::SeedReservationStation(Config * config) {
    this->Entries.resize(config->parameters["EntryCount"]);
}

void SeedReservationStation::step() {

}

FetchUnit::FetchUnit(Config * config) {
    this->NextSeedPointer = bitset<32>(0);
}