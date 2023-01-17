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

    map<char, DRAM<bitset<32>, bitset<32>>*> OcMEMs;
    DRAM<bitset<32>, bitset<32>> * OccAMEM = new DRAM<bitset<32>, bitset<32>>("OccAMEM", config->children["OccMemory"], true);
    OccAMEM->load(ioDir);
    OcMEMs['A'] = OccAMEM;

    DRAM<bitset<32>, bitset<32>> * OccCMEM = new DRAM<bitset<32>, bitset<32>>("OccCMEM", config->children["OccMemory"], true);
    OccCMEM->load(ioDir);
    OcMEMs['C'] = OccCMEM;

    DRAM<bitset<32>, bitset<32>> * OccGMEM = new DRAM<bitset<32>, bitset<32>>("OccGMEM", config->children["OccMemory"], true);
    OccGMEM->load(ioDir);
    OcMEMs['G'] = OccGMEM;

    DRAM<bitset<32>, bitset<32>> * OccTMEM = new DRAM<bitset<32>, bitset<32>>("OccTMEM", config->children["OccMemory"], true);
    OccTMEM->load(ioDir);
    OcMEMs['T'] = OccTMEM;

    Core * CORE = new Core("00", ioDir, config->children["Core"]);
    CORE->connect(SdMEM, OcMEMs);

    int cycle_count = 0;

    while (!CORE->halted) {
        cout << "===================================================================================================" << endl << endl;
        cout << "New Cycle: " << cycle_count++ << endl;
        CORE->step();

        SdMEM->step();
        OccAMEM->step();
        OccCMEM->step();
        OccGMEM->step();
        OccTMEM->step();
    }
}