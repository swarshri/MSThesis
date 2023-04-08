#include<Config.h>

using namespace std;

SysConfig::SysConfig(string id) {
    this->id = id;
}

void SysConfig::add_parameter(string pName, string pVal) {
    // Just check the first character of parVal to determine if it is an int or a string.
    if (all_of(pVal.begin(), pVal.end(), ::isdigit)) {
        this->parameters[pName] = stoi(pVal);
        // cout << "Added parameter: " << this->id << " " << pName  << " " << pVal << endl;
    }
    else {
        this->str_parameters[pName] = pVal;
        // cout << "Added string parameter: " << this->id << " " << pName  << " " << pVal << endl;
    }
}

void SysConfig::add_child(string cName) {
    unsigned int pos = cName.find("Pipeline");
    this->children[cName] = new SysConfig(cName);
    // cout << "Added child: " << this->children[cName]->get_name() << endl;

    if (pos < cName.size()) {
        string pName = "Pipeline";
        char basename = *cName.substr(pos + 8, 1).c_str();
        int pVal = (int) this->BaseMap[basename];
        this->children[cName]->add_parameter(pName, to_string(pVal));
    }
}

string SysConfig::get_name() {
    return this->id;
}

bool SysConfig::has_child(string compName) {
    return this->children.count(compName);
}

bool SysConfig::has_parameter(string paramName) {
    return this->parameters.count(paramName) || this->str_parameters.count(paramName);
}

void ConfigParser::remove_whitespaces(string * strval) {
    list<char> to_find = {' ', '\t'};
    for (char c : to_find) {
        unsigned int pos = strval->find(c);
        unsigned int size = strval->size();
        while (pos < size) {
            strval->erase(pos, 1);
            pos = strval->find(c);
            size = strval->size();
        }
    }
}

SysConfig * ConfigParser::parse(string confDir) {
    ifstream config;
    string line;
    
#ifdef _WIN32
    string filepath = confDir + "\\Model.cfg";
#else
    string filepath = confDir + "Model.cfg";
#endif
    cout << "Parsing Config file: " << filepath << endl;

    SysConfig * Model = new SysConfig("Model");
    
    config.open(filepath);

    if (config.is_open()) {
        SysConfig * cfg = Model;

        stack<SysConfig *> compStack;
        compStack.push(Model);

        while (getline(config, line)) {
            this->remove_whitespaces(&line);
            if (line.find('{') != string::npos) {
                string cName = line.substr(0, line.find('{'));
                cfg->add_child(cName);
                compStack.push(cfg->children[cName]);
                cfg = compStack.top();
            }
            else if (line.find('}') != string::npos) {
                compStack.pop();
                cfg = compStack.top();
            }
            else if (line.find(':') != string::npos) {
                line = line.substr(0, line.find('('));
                string pName = line.substr(0, line.find(':'));
                string pVal = line.substr(line.find(':') + 1,  string::npos);
                cfg->add_parameter(pName, pVal);
            }
        }
        config.close();
    }
    else cout<<"Unable to open the config file at "<< filepath << endl;
    return Model;
}