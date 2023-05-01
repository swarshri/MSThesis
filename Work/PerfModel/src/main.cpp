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

int main(int argc, char * argv[]) {
    string confFilePath = "";
    string refPath = "";
    string bwtPath = "";
    string saPath = "";
    string readPath = "";
    string opDir = "";

    if (argc == 5) {
        confFilePath = argv[1];
        refPath = argv[2];
        readPath = argv[3];
        opDir = argv[4];
        cout << "Config File Path: " << confFilePath << endl;
        cout << "Reference File Path: " << refPath << endl;
        cout << "Read File Path: " << readPath << endl;
        cout << "OP Directory: " << opDir << endl;
    }
    else if (argc == 6) {
        confFilePath = argv[1];
        bwtPath = argv[2];
        saPath = argv[3];
        readPath = argv[4];
        opDir = argv[5];
        cout << "Config File Path: " << confFilePath << endl;
        cout << "BWT File Path: " << bwtPath << endl;
        cout << "SA File Path: " << saPath << endl;
        cout << "Read File Path: " << readPath << endl;
        cout << "OP Directory: " << opDir << endl;
    }
    else {
        cout << "Invalid number of arguments: " << argc << endl;
        cout << "Expected path containing the config file, the reference (bwt and sa) and read file paths, and the directory path for the output in order." << endl;
        cout << "Machine stopped." << endl;
        return -1;
    }

    SysConfig * config = ConfigParser().parse(confFilePath);
    cout << "Config file parsed" << endl;
    
    Reference * RefObj;    
    if (refPath != "")
        RefObj = new Reference(refPath);
    else if (bwtPath != "" and saPath != "")
        RefObj = new Reference(bwtPath, saPath);
    
    Reads * ReadsObj = new Reads(readPath);

    SeedMemory * SdMEM = new SeedMemory("SdMEM", opDir, config->children["SeedMemory"], config->children["Core"]);
    SdMEM->input(ReadsObj);

    map<char, OccMemory*> OcMEMs;
    OcMEMs['A'] = new OccMemory("A", opDir, config->children["OccMemory"], config->children["Core"]);
    OcMEMs['C'] = new OccMemory("C", opDir, config->children["OccMemory"], config->children["Core"]);
    OcMEMs['G'] = new OccMemory("G", opDir, config->children["OccMemory"], config->children["Core"]);
    OcMEMs['T'] = new OccMemory("T", opDir, config->children["OccMemory"], config->children["Core"]);

    for (auto ocm: OcMEMs)
        ocm.second->input(RefObj);

    int pos1 = confFilePath.find_last_of('/');
    int pos2 = confFilePath.find_last_of('.');
    string confName = confFilePath.substr(pos1 + 1, pos2 - pos1 - 1);
    cout << "confName: " << confName << endl;
    SiMemory * SiMEM = new SiMemory("SiMEM_"+confName, opDir, config->children["SIMemory"], config->children["Core"]);
    // SiMEM->allocate();

    // int pauseip;
    // cout << "Pause for input: ";
    // cin >> pauseip;

    PerformanceRecorder * perf = new PerformanceRecorder(opDir, confName, config->children["PerformanceRecorder"]);

    Core * CORE = new Core("00", opDir, config->children["Core"], perf, RefObj);
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