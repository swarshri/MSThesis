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

typedef enum {
    PL_A = 0,
    PL_C = 1,
    PL_G = 2,
    PL_T = 3
} BaseConfig;

class SysConfig {
    public:
        map<string, int> parameters;
        map<string, string> str_parameters;
        map<string, SysConfig *> children;
        map<char, BaseConfig> BaseMap = {{'A', PL_A}, {'C', PL_C}, {'G', PL_G}, {'T', PL_T}};

        SysConfig(string);
        void add_parameter(string, string);
        void add_child(string);

        string get_name();
        bool has_child(string);
        bool has_parameter(string);
    
    private:
        string id;
};

class ConfigParser {
    public:
        string confDir;

        SysConfig * parse(string);

    private:
        void remove_whitespaces(string *);
};

#endif