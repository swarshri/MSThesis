#include <PerfRecorder.h>

PerformanceRecorder::PerformanceRecorder(string iodir, string cfgName, SysConfig* config) {

#ifdef _WIN32
    string filename = "\\OP\\PerfMetrics_" + cfgName;
#else
    string filename = "OP/PerfMetrics_" + cfgName;
#endif
    filename += ".csv";

    // TODO add current date and time to the output file name - to run multiple times.
    this->opFilePath = iodir + filename;
    this->writeCycle = config->parameters["WriteCycle"];
    this->clkcount = 0;
    queue<string> firstclk;
    firstclk.push(to_string(this->clkcount));
    this->data["ClockCount"] = firstclk;
}

void PerformanceRecorder::addMetrics(vector<string> metrics) {
    for (string metric : metrics) {
        // cout << "PR: Adding new metric: " << metric << endl;
        queue<string> newMetric;
        newMetric.push("NewMetric");
        this->data[metric] = newMetric;
    }
}

void PerformanceRecorder::record(uint64_t clkcycle, string metric, string data) {
    if (this->data.find(metric) != this->data.end()) {
        if (this->clkcount == clkcycle) {
            // cout << "PR: Adding data with Clock count: " << clkcycle << " PR's clock count: " << this->clkcount << endl;
            this->data[metric].push(data);
            // cout << "PR: added data: " << this->data[metric].front() << endl;
        }
        else
            cout << "PR: Clock count: " << clkcycle << " doesn't match the current record clock count: " << this->clkcount << endl;
    }
    else
        cout << "PR: There are no metrics named: " << metric << " added to the performance recorder." << endl;
}

void PerformanceRecorder::step() {
    ofstream csvop;
    if (this->clkcount == 0) {
        csvop.open(this->opFilePath, ios::trunc);
        string opline_title = "";
        string opline = "";
        for (auto kv = this->data.begin(); kv != this->data.end(); kv++) {
            opline_title += kv->first;
            opline_title += ",";
            // cout << "PR: kv first: " << kv->first << " kv second size before popping: " << kv->second.size() << endl;
            if (kv->second.front() == "NewMetric")
                kv->second.pop();
            opline += kv->second.front();
            opline += ",";
            // cout << "PR: kv second size before popping: " << kv->second.size() << endl;
            kv->second.pop();
            // cout << "PR: kv second size after popping: " << kv->second.size() << endl;
        }
        cout << "PR: opline title: " << opline_title << endl;
        cout << "PR: opline: " << opline << endl;
        opline_title += "\n";
        csvop << opline_title;
        opline += "\n";
        csvop << opline;
    }
    else if (this->clkcount % this->writeCycle == 0) {
        csvop.open(this->opFilePath, ios::app);
        for (int i = 0; i < this->writeCycle; i++) {
            string opline;
            for (auto kv = this->data.begin(); kv != this->data.end(); kv++) {
                // cout << "PR: kv first: " << kv->first << " kv second size before popping: " << kv->second.size() << endl;
                opline += kv->second.front();
                opline += ",";
                kv->second.pop();
                // cout << "PR: kv second size after popping: " << kv->second.size() << endl;
            }
            cout << "PR: opline: " << opline << endl;
            opline += "\n";
            csvop << opline;
        }
    }
    // Incrementing for next step cycle.
    this->clkcount++;
    cout << "PR: Incremented clock cycle count: " << this->clkcount << endl;
    this->data["ClockCount"].push(to_string(this->clkcount));
    cout << "PR: Pushed back clock cycle count: " << to_string(this->clkcount) << endl;
}

void PerformanceRecorder::dump() {
    ofstream csvop;
    csvop.open(this->opFilePath, ios::app);
    for (int i = 0; i < (this->clkcount - 1) % this->writeCycle; i++) {
        string opline;
        for (auto kv = this->data.begin(); kv != this->data.end(); kv++) {
            opline += kv->second.front();
            opline += ",";
            // cout << "PR: opline: " << opline << endl;
            // cout << "PR: kv second size before popping: " << kv->second.size() << endl;
            kv->second.pop();
            // cout << "PR: kv second size after popping: " << kv->second.size() << endl;
        }
        opline += "\n";
        csvop << opline;
    }
}