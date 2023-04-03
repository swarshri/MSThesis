#include <Store.h>

StoreStage::StoreStage(SysConfig * config) {
    this->halted = false;
    this->cycle_count = 0;
}

void StoreStage::connectDU(DispatchStage * du)  {
    this->coreDU = du;
}

void StoreStage::connectDRAM(DRAMW<32, 64> * simem) {
    this->SIMEM = simem;
}

bool StoreStage::isHalted() {
    return this->halted;
}

void StoreStage::step() {
    cout << "----------------------- Store Stage step function --------------------------" << endl;
    if (!this->halted) {
        // TODO: use the new dramsim3
        pair<bool, StoreQueueEntry> nse = this->coreDU->getNextStore();
        cout << "SS: nse.first: " << nse.first << endl;
        if (nse.first && this->SIMEM->willAcceptRequest(nse.second.StoreAddress, true)) {
            if (nse.first) {
                vector<bitset<64>> storeVals;
                storeVals.push_back(nse.second.StoreVal);
                bool writeReqSuccess = this->SIMEM->writeRequest(nse.second.StoreAddress, storeVals);
                if (writeReqSuccess) {
                    cout << "SS: Successfully sent Write request to address: " << nse.second.StoreAddress << endl;
                    this->coreDU->popNextStore();
                    cout << "SS: Popped the Store request off the Store Queue. Store value: " << nse.second.StoreVal << endl;
                }
                else {
                    cout << "SS: Couldn't send the Write request to address: " << nse.second.StoreAddress << endl;
                    cout << "SS: Will be tried again in the next cycle." << endl;
                }                
            }
            else if (this->coreDU->isHalted()) {
                this->halted = true;
                cout << "SS: Set halted to true." << endl;
            }
        }
        this->cycle_count++;
    }
    else
        cout << "SS: Halted" << endl;

    if (this->cycle_count >= 3000)
        this->halted = true;
}