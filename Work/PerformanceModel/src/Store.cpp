#include <Store.h>

#ifndef STORE_DEF
#define STORE_DEF

StoreStage::StoreStage(SysConfig * config) {
    this->halted = false;
    this->cycle_count = 0;
}

void StoreStage::connectDU(DispatchStage * du)  {
    this->coreDU = du;
}

void StoreStage::connectDRAM(SiMemory * simem) {
    this->SIMEM = simem;
}

bool StoreStage::isHalted() {
    return this->halted;
}

void StoreStage::step() {
    // cout << "----------------------- Store Stage step function --------------------------" << endl;
    if (!this->halted) {
        // TODO: use the new dramsim3
        pair<bool, StoreQueueEntry> nse = this->coreDU->getNextStore();
        // cout << "SS: nse.first: " << nse.first << endl;
        // // cout << "SS: this->coreDU->isHalted(): " << this->coreDU->isHalted() << endl;
        // // cout << "SS: this->SIMEM->isFree(true): " << this->SIMEM->isFree(true) << endl;
        if (nse.first) {
            if (this->SIMEM->willAcceptRequest(nse.second.StoreAddress, true)) {
                vector<vector<uint64_t>> storeVals;
                storeVals.push_back({nse.second.StoreValLow, nse.second.StoreValHigh});
                bool writeReqSuccess = this->SIMEM->writeRequest(nse.second.StoreAddress, storeVals);
                if (writeReqSuccess) {
                    // cout << "SS: Successfully sent Write request to address: " << nse.second.StoreAddress << endl;
                    this->coreDU->popNextStore();
                    // cout << "SS: Popped the Store request off the Store Queue. Store value: " << nse.second.StoreVal << endl;
                }
                else {
                    // cout << "SS: Couldn't send the Write request to address: " << nse.second.StoreAddress << endl;
                    // cout << "SS: Will be tried again in the next cycle." << endl;
                }
            }
        }
        else if (this->coreDU->isHalted() && this->SIMEM->isFree(true)) {
            this->halted = true;
            // cout << "SS: Set halted." << endl;
        }
        this->cycle_count++;
    }
    else
        cout << "SS: Halted" << endl;
}

#endif