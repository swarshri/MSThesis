#include <Store.h>

StoreStage::StoreStage(Config * config) {
    this->halted = false;
    this->cycle_count = 0;
}

void StoreStage::connectDU(DispatchStage * du)  {
    this->coreDU = du;
}

void StoreStage::connectDRAM(DRAM<bitset<32>, bitset<64>> * simem) {
    this->SIMEM = simem;
}

bool StoreStage::isHalted() {
    return this->halted;
}

void StoreStage::step() {
    cout << "----------------------- Store Stage step function --------------------------" << endl;
    if (!this->halted) {
        if (this->SIMEM->isFree()) {
            pair<bool, StoreQueueEntry> nse = this->coreDU->popNextStore();
            cout << "SS: nse.first: " << nse.first << endl;
            if (nse.first) {
                vector<bitset<64>> storeVals;
                storeVals.push_back(nse.second.StoreVal);
                this->SIMEM->writeAccess(nse.second.StoreAddress, storeVals);
                cout << "SS: Sent Write Request to address: " << nse.second.StoreAddress << endl;
            }
            else if (this->coreDU->isHalted()) {
                this->halted = true;
                cout << "SS: Set halted to true." << endl;
            }
        }
    }
    else
        cout << "SS: Halted" << endl;
}