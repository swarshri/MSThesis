#include<Compute.h>

ComputeStage::ComputeStage(SysConfig * config, char base, string iodir, bitset<32> countVal) {
    this->base = base;
    this->halted = false;
    this->cycle_count = 0;

    this->CountReg = countVal;
}

void ComputeStage::connect(ReserveStage * ru, FetchStage * fu) {
    this->coreRU = ru;
    this->coreFU = fu;
}

bool ComputeStage::isHalted() {
    return this->halted;
}

void ComputeStage::step() {
    cout << "----------------------- Compute " << this->base << " Stage step function --------------------------" << endl;
    if (!this->halted) {
        pair<int, CRSEntry> nce = this->coreRU->getNextComputeEntry();
        if (nce.first != -1) {
            bitset<32> lowResult = bitset<32>(this->CountReg.to_ulong() + nce.second.LowOcc.to_ulong());
            bitset<32> highResult = bitset<32>(this->CountReg.to_ulong() + nce.second.HighOcc.to_ulong());
            this->coreFU->writeBack(nce.second.SRSWBIndex.to_ulong(), lowResult, highResult);
            cout << "CS: Write Back scheduled into FS SRS at Index: " << nce.second.SRSWBIndex << " LowResult: " << lowResult << endl;
            cout << "CS: Write Back scheduled into FS SRS at Index: " << nce.second.SRSWBIndex << " HighResult: " << highResult << endl;
            this->coreRU->scheduleToSetCRSEToEmptyState(nce.first);
            cout << "CS: Scheduling to set Empty State in RS CRS at Index: " << nce.first << endl;
        }
        else if (this->coreRU->isHalted())
            this->halted = true;

        this->cycle_count++;
    }
    else
        cout << "CS: Halted" << endl;
}