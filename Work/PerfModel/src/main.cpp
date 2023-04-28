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
#include<PerfRecorder.h>

#include "../../Common/inc/DataInput.h"

using namespace std;

template<int alen, int dlen>
void createMem(string name, string ioDir, string configName, SysConfig* config, DRAMW<alen, dlen>* retObj) {
    retObj = new DRAMW<alen, dlen>(name, ioDir, config->children[configName], config->children["Core"], true);
}

int main(int argc, char * argv[]) {
    string confFilePath = "";
    string refPath = "";
    string readPath = "";
    string opDir = "";

    if (argc != 5) {
        cout << "Invalid number of arguments." << endl;
        cout << "Expected path containing the config file, the reference and read file paths, and the dir4ectory path for the output in order." << endl;
        cout << "Machine stopped." << endl;
        return -1;
    }
    else {
        confFilePath = argv[1];
        refPath = argv[2];
        readPath = argv[3];
        opDir = argv[4];
        cout << "Config File Path: " << confFilePath << endl;
        cout << "Reference File Path: " << refPath << endl;
        cout << "Read File Path: " << readPath << endl;
        cout << "OP Directory: " << opDir << endl;
    }

    SysConfig * config = ConfigParser().parse(confFilePath);
    cout << "Config file parsed" << endl;
    Reference * RefObj;
    // if (refPath.find_last_of(".bwt") == refPath.size() - 4)
    //     RefObj = new Reference(refPath, true);
    // else
    RefObj = new Reference(refPath, false);
    Reads * ReadsObj = new Reads(readPath);

    SeedMemory<32, 64> * SdMEM = new SeedMemory<32, 64>("SdMEM", opDir, config->children["SeedMemory"], config->children["Core"]);
    SdMEM->input(ReadsObj);

    map<char, OccMemory<32, 32>*> OcMEMs;
    OcMEMs['A'] = new OccMemory<32, 32>("A", opDir, config->children["OccMemory"], config->children["Core"]);
    OcMEMs['C'] = new OccMemory<32, 32>("C", opDir, config->children["OccMemory"], config->children["Core"]);
    OcMEMs['G'] = new OccMemory<32, 32>("G", opDir, config->children["OccMemory"], config->children["Core"]);
    OcMEMs['T'] = new OccMemory<32, 32>("T", opDir, config->children["OccMemory"], config->children["Core"]);

    for (auto ocm: OcMEMs)
        ocm.second->input(RefObj);

    int pos1 = confFilePath.find_last_of('/');
    int pos2 = confFilePath.find_last_of('.');
    string confName = confFilePath.substr(pos1 + 1, pos2 - pos1 - 1);
    cout << "confName: " << confName << endl;
    DRAMW<32, 64> * SiMEM = new DRAMW<32, 64>("SiMEM_"+confName, opDir, config->children["SIMemory"], config->children["Core"], false);
    SiMEM->allocate();

    // int pauseip;
    // cout << "Pause for input: ";
    // cin >> pauseip;

    PerformanceRecorder * perf = new PerformanceRecorder(opDir, confName, config->children["PerformanceRecorder"]);

    Core * CORE = new Core("00", opDir, config->children["Core"], perf);
    CORE->connect(SdMEM, OcMEMs, SiMEM);
    
// #ifdef _WIN32
//     string opDir = opDir + "\\OP";
// #else
//     string opDir = opDir + "OP";
// #endif

//     struct stat info;
//     if (stat(opDir.c_str(), &info) != 0) {
//         cout << opDir << " doesn't exist." << endl;
//         mkdir(opDir.c_str(), S_IWUSR);
//     }
//     else
//         cout << opDir << " exists." << endl;
        
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

        perf->step();

        if (cycle_count == 1000)
            CORE->halted = true;
    }
    perf->dump();
    SiMEM->output();
}