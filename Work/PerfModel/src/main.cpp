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

    DRAM * SdMEM = new DRAM("SdMEM", config->children["SeedMemory"], false);
    SdMEM->load(ioDir);

    DRAM * OcMEM = new DRAM("OcMEM", config->children["OccMemory"], true);
    OcMEM->load(ioDir);

    bool break_flag = false;
    vector<bitset<64>> readData;

    while (true) {
        if (SdMEM->isFree())
            SdMEM->readAccess(bitset<32>(0)); 

        SdMEM->step();
        if (SdMEM->readDone) {
            readData = SdMEM->lastReadData;
            SdMEM->readDone = false;
            cout << "Read Data" << endl;
            for (int i=0; i < 4; i++)
                cout << "Data at " << i << " " << readData[i] << endl;

            if (break_flag) break;

            if (SdMEM->isFree())
                SdMEM->writeAccess(bitset<32>(4), readData);
        }
        if (SdMEM->writeDone) {
            if (SdMEM->isFree())
                SdMEM->readAccess(bitset<32>(4));
            SdMEM->writeDone = false;
            break_flag = true;
        }
    }
}