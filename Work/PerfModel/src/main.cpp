#include<stdio.h>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>
#include<thread>

#include<sys/types.h>
#include<sys/stat.h>

#include<Config.h>
#include<DRAMWrapper.h>
#include<Core.h>

using namespace std;

template<int alen, int dlen>
void createMem(string name, string ioDir, string configName, SysConfig* config, DRAMW<alen, dlen>* retObj) {
    retObj = new DRAMW<alen, dlen>(name, ioDir, config->children[configName], config->children["Core"], true);
}

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

    DRAMW<32, 64> * SdMEM = new DRAMW<32, 64>("SdMEM", ioDir, config->children["SeedMemory"], config->children["Core"], true);

    map<char, DRAMW<32, 32>*> OcMEMs;

    DRAMW<32, 32> * OccAMEM = new DRAMW<32, 32>("OccAMEM", ioDir, config->children["OccMemory"], config->children["Core"], true);
    OcMEMs['A'] = OccAMEM;

    DRAMW<32, 32> * OccCMEM = new DRAMW<32, 32>("OccCMEM", ioDir, config->children["OccMemory"], config->children["Core"], true);
    OcMEMs['C'] = OccCMEM;

    DRAMW<32, 32> * OccGMEM = new DRAMW<32, 32>("OccGMEM", ioDir, config->children["OccMemory"], config->children["Core"], true);
    OcMEMs['G'] = OccGMEM;

    DRAMW<32, 32> * OccTMEM = new DRAMW<32, 32>("OccTMEM", ioDir, config->children["OccMemory"], config->children["Core"], true);
    OcMEMs['T'] = OccTMEM;

    int pos2 = confDir.find_last_of('/');
    int pos1 = confDir.substr(0, pos2).find_last_of('/');
    string confName = confDir.substr(pos1 + 1, pos2 - pos1 - 1);
    cout << "confName: " << confName << endl;
    DRAMW<32, 64> * SiMEM = new DRAMW<32, 64>("SiMEM_"+confName, ioDir, config->children["SIMemory"], config->children["Core"], false);

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
        OcMEMs['A']->step();
        OcMEMs['C']->step();
        OcMEMs['G']->step();
        OcMEMs['T']->step();
        SiMEM->step();
    }

    SiMEM->output();
}