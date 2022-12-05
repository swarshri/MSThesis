#include<stdio.h>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>

#include<DRAM.h>
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

    Config * config = ConfigParser(confDir).parse();

    DRAM * SdMEM = new DRAM("SdMEM", config->children["SeedMemory"], false);
    SdMEM->load(ioDir);

    DRAM * OcMEM = new DRAM("OcMEM", config->children["OccMemory"], true);
    OcMEM->load(ioDir);

    DRAM * SiMEM = new DRAM("SiMEM", config->children["SAIMemory"], true);
    SiMEM->load(ioDir);
}