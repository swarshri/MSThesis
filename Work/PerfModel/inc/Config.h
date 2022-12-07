#include<stdint.h>
#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include<list>
#include<stack>
#include<regex>

using namespace std;

#ifndef _Config_h
#define _Config_h

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

        Config * parse(string);

    private:
        void remove_whitespaces(string *);
};

#endif