#include <Load.h>

MemoryBuffer::MemoryBuffer(Config * config) {
    
}

ComputeBuffer::ComputeBuffer() {

}

LoadUnit::LoadUnit(Config * config, vector<bitset<64>> * refIndexInfo) {
    this->base = config->parameters["Pipeline"];
    this->RefCount = bitset<32>(refIndexInfo->at(0).to_ulong());
    this->CountReg = refIndexInfo->at(this->base + 1);
    this->OccFirstValReg = bitset<64>(0);
    this->OccLastValReg = refIndexInfo->at(this->base + 5);
}

void LoadUnit::connect(DispatchUnit * du) {
    this->coreDU = du;
}

void LoadUnit::step() {
    if (!this->halted) {
        pair<bool, DispatchEntry> currentDispatch = this->coreDU->popnext(this->base);
        if (currentDispatch.first) {
            bitset<64> countVal = this->CountReg;
            bitset<64> OccLowVal;
            bitset<64> OccHighVal;
            bool computeReady = true;
            bool olComputeReady = true;
            bool ohComputeReady = true;
            if (currentDispatch.second.Low == bitset<32>(0))
                OccLowVal = bitset<64>(0);
            else if (currentDispatch.second.Low == this->RefCount)
                OccLowVal = this->OccLastValReg;
            else {
                computeReady = false;
                LoadQueueEntry newLoadRequest;
                newLoadRequest.LowOrHigh = false;
                newLoadRequest.OccMemoryAddress = currentDispatch.second.Low;
                newLoadRequest.SRSWBIndex = currentDispatch.second.SRSWBIndex;
            }


        }
    }
}