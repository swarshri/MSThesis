#include<stdio.h>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>

#include<sys/types.h>
#include<sys/stat.h>

#include<Config.h>
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

    SysConfig * config = ConfigParser().parse(confDir);

    DRAM<bitset<32>, bitset<64>> * SdMEM = new DRAM<bitset<32>, bitset<64>>("SdMEM", config->children["SeedMemory"], true);
    SdMEM->input(ioDir);

    map<char, DRAM<bitset<32>, bitset<32>>*> OcMEMs;
    DRAM<bitset<32>, bitset<32>> * OccAMEM = new DRAM<bitset<32>, bitset<32>>("OccAMEM", config->children["OccMemory"], true);
    OccAMEM->input(ioDir);
    OcMEMs['A'] = OccAMEM;

    DRAM<bitset<32>, bitset<32>> * OccCMEM = new DRAM<bitset<32>, bitset<32>>("OccCMEM", config->children["OccMemory"], true);
    OccCMEM->input(ioDir);
    OcMEMs['C'] = OccCMEM;

    DRAM<bitset<32>, bitset<32>> * OccGMEM = new DRAM<bitset<32>, bitset<32>>("OccGMEM", config->children["OccMemory"], true);
    OccGMEM->input(ioDir);
    OcMEMs['G'] = OccGMEM;

    DRAM<bitset<32>, bitset<32>> * OccTMEM = new DRAM<bitset<32>, bitset<32>>("OccTMEM", config->children["OccMemory"], true);
    OccTMEM->input(ioDir);
    OcMEMs['T'] = OccTMEM;

    int pos2 = confDir.find_last_of('/');
    int pos1 = confDir.substr(0, pos2).find_last_of('/');
    string confName = confDir.substr(pos1 + 1, pos2 - pos1 - 1);
    cout << "confName: " << confName << endl;
    DRAM<bitset<32>, bitset<64>> * SiMEM = new DRAM<bitset<32>, bitset<64>>("SiMEM_"+confName, config->children["SIMemory"], false);

    Core * CORE = new Core("00", ioDir, config->children["Core"]);
    CORE->connect(SdMEM, OcMEMs, SiMEM);

#ifdef _WIN32
    string opDir = ioDir + "\\OP";
#else
    string opDir = ioDir + "OP";
#endif

    struct stat info;
    if (stat(opDir.c_str(), &info) != 0) {
        cout << opDir << " doesn't exist." << endl;
        mkdir(opDir.c_str(), S_IWUSR);
    }
    else
        cout << opDir << " exists." << endl;
        
    int cycle_count = 0;

    cout << endl;
    cout << "=======================================================" << endl;
    cout << "--------- Configuration and Construction Done ---------" << endl;
    cout << "--------------- Starting the simulator ----------------" << endl;
    cout << "=======================================================" << endl << endl;
    while (!CORE->halted) {
        cout << "===================================================================================================" << endl;
        cout << "New Cycle: " << cycle_count++ << endl;
        CORE->step();

        SdMEM->step();
        OccAMEM->step();
        OccCMEM->step();
        OccGMEM->step();
        OccTMEM->step();
        SiMEM->step();
    }

    SiMEM->output(ioDir);
}