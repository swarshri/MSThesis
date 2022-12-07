#include<Config.h>

using namespace std;

Config::Config(string id) {
    this->id = id;
}

void Config::add_parameters(string pName, int pVal) {
    this->parameters[pName] = pVal;
}

void Config::add_children(string cName) {
    this->children[cName] = new Config(cName);
}

string Config::get_name() {
    return this->id;
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

Config * ConfigParser::parse(string confDir) {
    ifstream config;
    string line;
    
#ifdef _WIN32
    string filepath = confDir + "\\Model.cfg";
#else
    string filepath = confDir + "Model.cfg";
#endif
    cout << "Parsing Config file: " << filepath << endl;

    Config * Model = new Config("Model");
    
    config.open(filepath);

    if (config.is_open()) {
        Config * cfg = Model;

        stack<Config *> compStack;
        compStack.push(Model);

        while (getline(config, line)) {

            this->remove_whitespaces(&line);

            if (line.find('{') != string::npos) {
                string cName = line.substr(0, line.find('{'));
                cfg->add_children(cName);
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
                int pVal = stoi(line.substr(line.find(':') + 1,  string::npos));
                cfg->add_parameters(pName, pVal);
            }
        }
        config.close();
    }
    else cout<<"Unable to open the config file at "<< filepath << endl;
    return Model;
}