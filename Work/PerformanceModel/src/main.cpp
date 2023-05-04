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
#include<PerfOP.h>

#include "../../Common/inc/DataInput.h"

using namespace std;

int main(int argc, char * argv[]) {
    string confFilePath = "";
    string refPath = "";
    string bwtPath = "";
    string saPath = "";
    string readPath = "";
    string opDir = "";
    
    cout << "Received " << argc << " arguments." << endl;
    if (argc != 9 and argc != 11) {
        cout << "Invalid number of arguments." << endl;
        cout << "Expected path for the input fasta (or bwt, sa pair), fastq files, and op dir path." << endl;
        cout << "Valid commands are, " << endl;
        cout << "model --cfg <cfgpath> --ref <refpath> --reads <readpath> --op <opdirpath>" << endl;
        cout << "model --cfg <cfgpath> --bwt <bwtpath> --sa <sapath> --reads <readpath> --op <opdirpath>" << endl;
        cout << "Machine stopped." << endl;
        return -1;
    }
    else {
        for (int i = 1; i < argc; i++) {
            // cout << "init i: " << i << endl;
            cout << argv[i] << endl;
            if (strcmp(argv[i], "--cfg") == 0) {
                confFilePath = argv[++i];
                cout << "Found config file: " << confFilePath << endl;
            }
            else if (strcmp(argv[i], "--ref") == 0) {
                refPath = argv[++i];
                cout << "Found reference file: " << refPath << endl;
            }
            else if (strcmp(argv[i], "--bwt") == 0) {
                bwtPath = argv[++i];
                cout << "Found BWT file: " << bwtPath << endl;
            }
            else if (strcmp(argv[i], "--sa") == 0) {
                saPath = argv[++i];
                cout << "Found SA file: " << saPath << endl;
            }
            else if (strcmp(argv[i], "--reads") == 0) {
                readPath = argv[++i];
                cout << "Found reads file: " << readPath << endl;
            }
            else if (strcmp(argv[i], "--op") == 0) {
                opDir = argv[++i];
                cout << "Found op dir path: " << opDir << endl;
            }
            // cout << "fin i: " << i << endl;
        }
        // TODO: Check file path extensions to make sure they are fasta and fastq files.
        cout << "Config file: " << confFilePath << endl;
        cout << "FASTA Reference file: " << refPath << endl;
        cout << "BWTIndexed file path: " << bwtPath << endl;
        cout << "Suffix Array file path: " << saPath << endl;
        cout << "FASTQ Read file path: " << readPath << endl;
        cout << "Output file path: " << opDir << endl;
    }

    SysConfig * config = ConfigParser().parse(confFilePath);
    cout << "Config file parsed" << endl;
    
    Reference * RefObj;    
    if (refPath != "")
        RefObj = new Reference(refPath);
    else if (bwtPath != "" and saPath != "") {
        RefObj = new Reference(bwtPath, saPath);
        refPath = bwtPath.substr(0, bwtPath.size() - 4);
    }
    
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

    SiMemory * SiMEM = new SiMemory("SiMEM", opDir, config->children["SIMemory"], config->children["Core"]);
    // SiMEM->allocate();

    // int pauseip;
    // cout << "Pause for input: ";
    // cin >> pauseip;

    int pos1 = confFilePath.find_last_of('/');
    int pos2 = confFilePath.find_last_of('.');
    string confName = confFilePath.substr(pos1 + 1, pos2 - pos1 - 1);
    cout << "confName: " << confName << endl;
    PerformanceRecorder * perf = new PerformanceRecorder(opDir, confName, config->children["PerformanceRecorder"]);
    
    Core * CORE = new Core("00", opDir, config->children["Core"], perf, RefObj);
    CORE->connect(SdMEM, OcMEMs, SiMEM);
    
    string refName = refPath.substr(refPath.find_last_of('/') + 1);
    cout << "Reference Name: " << refName << endl;
    string readName = refPath.substr(readPath.find_last_of('/') + 1);
    cout << "Read Name: " << readName << endl;

    IOInfo * ioinfo = new IOInfo;
    ioinfo->conffilename = confName;
    ioinfo->reffilename = refName;
    ioinfo->reflength = RefObj->get_seqLen();
    ioinfo->readfilename = readName;
    ioinfo->readscount = ReadsObj->get_readsCount();
    ioinfo->seedscount= ReadsObj->get_seedsCount();
    PerformanceOutput * perfop = new PerformanceOutput(opDir, ioinfo, config, CORE);

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

        // if (cycle_count == 1000)
        //     CORE->halted = true;
    }
    perf->dump();
    SiMEM->output();
    perfop->output();
}