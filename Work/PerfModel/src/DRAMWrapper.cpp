#include <DRAMWrapper.h>

#ifndef DRAMW_DEF
#define DRAMW_DEF

template <typename AddressType, typename DataType>
DRAMW<AddressType, DataType>::DRAMW(string iodir, SysConfig * config, bool readonly) {
    string config_file = "";
    string output_dir = "";
    this->MemSystem = new MemorySystem(config_file, output_dir,
                                       bind(&DRAMW::ReadCompleteHandler, this, std::placeholders::_1),
                                       bind(&DRAMW::WriteCompleteHandler, this, std::placeholders::_1));
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::printStats() { 
    this->MemSystem->PrintStats();
}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::input(string) {

}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::output(string) {

}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::readRequest(AddressType) {

}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::writeRequest(AddressType, vector<DataType>) {

}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::ReadCompleteHandler(uint64_t) {

}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::WriteCompleteHandler(uint64_t) {

}

template <typename AddressType, typename DataType>
void DRAMW<AddressType, DataType>::step() {

}

template <typename AddressType, typename DataType>
bool DRAMW<AddressType, DataType>::isFree() {
    return false;
}

template <typename AddressType, typename DataType>
int DRAMW<AddressType, DataType>::getChannelWidth() {
    return 4;
}

#endif