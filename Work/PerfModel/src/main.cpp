#include<stdio.h>
#include<bitset>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<iostream>

#include<Config.h>
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

    DRAM<bitset<64>, bitset<32>> * SdMEM = new DRAM<bitset<64>, bitset<32>>("SdMEM", config->children["SeedMemory"], false);
    SdMEM->load(ioDir);

    DRAM<bitset<64>, bitset<32>> * OcMEM = new DRAM<bitset<64>, bitset<32>>("OcMEM", config->children["OccMemory"], false);
    OcMEM->load(ioDir);

    DRAM<bitset<64>, bitset<32>> * SiMEM = new DRAM<bitset<64>, bitset<32>>("SiMEM", config->children["SAIMemory"], false);
    SiMEM->load(ioDir);
}