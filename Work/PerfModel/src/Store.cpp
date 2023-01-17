#include <Store.h>

StoreUnit::StoreUnit(Config * config) {

}

void StoreUnit::connectDU(DispatchUnit * du)  {
    this->coreDU = du;
}

void StoreUnit::connectDRAM(DRAM<bitset<32>, bitset<64>> * simem) {
    this->SIMEM = simem;
}

void StoreUnit::step() {
    if (!this->halted) {
        if (this->SIMEM->isFree()) {
            pair<bool, StoreQueueEntry> nse = this->coreDU->popNextStore();
            if (nse.first) {
                vector<bitset<64>> storeVals;
                storeVals.push_back(nse.second.StoreVal);
                this->SIMEM->writeAccess(nse.second.StoreAddress, storeVals);
            }
            else if (this->coreDU->isHalted())
                this->halted = true;
        }
    }
}