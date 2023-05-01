#include<Compute.h>

ComputeStage::ComputeStage(SysConfig * config, string base, string iodir, uint64_t countVal) {
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
        this->coreRU->print();
        pair<int, CRSEntry> nce = this->coreRU->getNextComputeEntry();
        if (nce.first != -1) {
            // cout << "Count reg value: " << this->CountReg << endl;
            uint64_t lowResult = this->CountReg + nce.second.LowOcc;
            uint64_t highResult = this->CountReg + nce.second.HighOcc;
            this->coreFU->writeBack(nce.second.SRSWBIndex, lowResult, highResult);
            // cout << "CS: Write Back scheduled into FS SRS at Index: " << nce.second.SRSWBIndex << " LowResult: " << lowResult << endl;
            // cout << "CS: Write Back scheduled into FS SRS at Index: " << nce.second.SRSWBIndex << " HighResult: " << highResult << endl;
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