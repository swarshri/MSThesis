#include <ReservationStation.h>

#ifndef RS_DEF
#define RS_DEF

template <typename EntryType>
ReservationStation<EntryType>::ReservationStation(Config * config) {
    this->numEntries = config->parameters["EntryCount"];
    this->Entries.resize(this->numEntries);

    for (int i = 0; i < this->numEntries; i++)
        this->setEmptyState(i);
}

template <typename EntryType>
void ReservationStation<EntryType>::fill(bitset<6> idx, EntryType entry) {
    int indx = idx.to_ulong();
    this->Entries[indx] = entry;
}

template <typename EntryType>
int ReservationStation<EntryType>::nextFreeEntry() {
    for (int i = 0; i < this->numEntries; i++)
        if (this->Entries[i].Empty && !this->Entries[i].Ready)
            return i;
    return -1;
}

template <typename EntryType>
pair<int, EntryType> ReservationStation<EntryType>::nextReadyEntry() {
    for (int i = 0; i < this->numEntries; i++) {
        if (!this->Entries[i].Empty && this->Entries[i].Ready) {
            return pair<int, EntryType>(i, this->Entries[i]);
        }
    }
    return pair<int, EntryType>(-1, *(new EntryType));
}

template <typename EntryType>
void ReservationStation<EntryType>::setEmptyState(int idx) {
    this->Entries[idx].Empty = true;
    this->Entries[idx].Ready = false;
}

template <typename EntryType>
void ReservationStation<EntryType>::setScheduledState(int idx) {
    this->Entries[idx].Empty = true;
    this->Entries[idx].Ready = true;
}

template <typename EntryType>
void ReservationStation<EntryType>::setReadyState(int idx) {
    this->Entries[idx].Empty = false;
    this->Entries[idx].Ready = true;
}

template <typename EntryType>
void ReservationStation<EntryType>::setWaitingState(int idx) {
    this->Entries[idx].Empty = false;
    this->Entries[idx].Ready = false;
}

template <typename EntryType>
void ReservationStation<EntryType>::updateBasePointer(int idx) {
    this->Entries[idx].BasePointer = bitset<6>(this->Entries[idx].BasePointer.to_ulong() + 3);
}

template <typename EntryType>
bool ReservationStation<EntryType>::isEmpty() {
    for (auto entry = this->Entries.begin(); entry != this->Entries.end(); entry++) {
        if (!(*entry).Empty || (*entry).Ready)
            return false;
    }
    return true;
}

template <typename EntryType>
void ReservationStation<EntryType>::show() {
    cout << "=================================================================================================================" << endl;
    // cout << "Idx\t Seed Address\t\t\t\t Seed\t\t\t\t\t\t\t\t\t Low Pointer\t\t\t\t High Pointer\t\t\t\t Base Pointer\t Ready\t Valid" << endl;
    // cout << "---------------------------------------------------------------------------------------------------------------------------------------------";
    // cout << "------------------------------------------------------------------------------------------------" << endl;
    int i = 0;
    for (auto entry: this->Entries) {
        cout << i << '\t' << entry << "\t\t" << entry.Ready << '\t' << entry.Empty << endl;
        i++;
    }
}

#endif