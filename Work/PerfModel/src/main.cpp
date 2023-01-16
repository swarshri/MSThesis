#include<stdio.h>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>

#include<Core.h>

using namespace std;

int main(int argc, char * argv[]) {
    string confDir = "";
    string ioDir = "";
    if (argc != 3) {
        cout << "Invalid number of arguments." << endl;
        cout << "Expected path containing the config file and the path containing the input files in order." << endl;
        cout << "Machine stopped." << endl;
        return -1;
    }
    else {
        confDir = argv[1];
        ioDir = argv[2];
        cout << "Config Directory: " << confDir << endl;
        cout << "IO Directory: " << ioDir << endl;
    }

    Config * config = ConfigParser().parse(confDir);

    DRAM<bitset<32>, bitset<64>> * SdMEM = new DRAM<bitset<32>, bitset<64>>("SdMEM", config->children["SeedMemory"], false);
    SdMEM->load(ioDir);

    DRAM<bitset<32>, bitset<64>> * OcMEM = new DRAM<bitset<32>, bitset<64>>("OcMEM", config->children["OccMemory"], true);
    OcMEM->load(ioDir);
    cout << "2" << endl;
    Core * CORE = new Core("00", ioDir, config->children["Core"]);
    CORE->connect(SdMEM, OcMEM);
    cout << "3" << endl;

    int cycle_count = 0;

    while (!CORE->halted) {
        cout << "===================================================================================================" << endl << endl;
        cout << "New Cycle: " << cycle_count++ << endl;
        CORE->step();

        SdMEM->step();
        OcMEM->step();
    }
}