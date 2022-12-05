#include<stdint.h>
#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include<list>
#include<stack>
#include<regex>

using namespace std;

class Config {
    public:
        map<string, int> parameters;
        map<string, Config *> children;

        Config(string);
        void add_parameters(string, int);
        void add_children(string);

        string get_name();
    
    private:
        string id;
};

class ConfigParser {
    public:
        string confDir;
        
        ConfigParser(string);

        Config * parse();

    private:
        void remove_whitespaces(string *);
};