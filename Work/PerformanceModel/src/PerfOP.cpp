#include <PerfOP.h>

PerformanceOutput::PerformanceOutput(string iodir, IOInfo* ioinfo, SysConfig* config, Core* core) {

#ifdef _WIN32
    this->opFilePath = iodir + "\\..\\Perf.csv";
#else
    this->opFilePath = iodir + "/../Perf.csv";
#endif

    this->ioinfo = ioinfo;
    this->coreptr = core;

    FILE * opfile;
    opfile = fopen(this->opFilePath.c_str(), "r");
    if (opfile) {
        cout << "POP: File exists." << endl;
        ifstream csvip(this->opFilePath);
        string line;
        this->id = 0;
        while (getline(csvip, line))
            this->id++;
    }
    else {
        cout << "POP: File doesn't exist." << endl;
        ofstream csvop;
        csvop.open(this->opFilePath, ios::app);
        string title = "#, Config Name, Reference File Name, Reference Length, Seeds Count, ";
        title += this->coreptr->getPerfMetricTitles();
        cout << "TITLE: " << title << endl;
        csvop << title;
        this->id = 1;
    }
}

void PerformanceOutput::output() {
    ofstream csvop;
    csvop.open(this->opFilePath, ios::app);
    // string opline = "new file new line.";
    string delim = ", ";
    string opline = "\n" + to_string(this->id) + delim +
                    this->ioinfo->conffilename + delim +
                    this->ioinfo->reffilename + delim +
                    to_string(this->ioinfo->reflength) + delim +
                    to_string(this->ioinfo->seedscount) + delim;
                    
    // cout << "POP: opline:" << this->opFilePath << opline << *this->coreptr->getPerfMetrics() << endl;
    csvop << opline << *this->coreptr->getPerfMetrics();
}