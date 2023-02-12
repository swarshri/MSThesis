#include<iostream>
#include<bitset>
#include<vector>
#include<math.h>

#include<Config.h>

using namespace std;

#ifndef CACHE_H
#define CACHE_H

struct IncomingCacheStruct {
    bitset<32> address;
    bitset<6> basePointer;
    vector<bitset<32>> data;

    friend std::ostream& operator <<(std::ostream& os, IncomingCacheStruct const& e) {
        return os << e.address << "\t"
                  << e.basePointer << "\t"
                  << e.data[0];
    }
};

struct CacheStruct {
    bitset<32> tag; // all 32 will never be used. Using the max to not make this a template.
    bitset<6> lowestBasePointer;
    unsigned int accessCount; // keeps a count of how many times this has been accessed before.
    vector<bitset<32>> data; // length = blocksize - usually 1 because there is no spatial locality with the current data layout in OccMemory.
    bool valid;

    friend std::ostream& operator <<(std::ostream& os, CacheStruct const& e) {
        return os << e.valid << "\t" 
                  << e.tag << "\t"
                  << e.lowestBasePointer << "\t"
                  << e.accessCount << "\t"
                  << e.data[0];
    }
};

class Cache {
    public:
        Cache(char, Config*);

        pair<bool, bitset<32>> read(bitset<32>);
        bool write(IncomingCacheStruct);
    
    private:
        string name;

        unsigned int ways;
        unsigned int size;
        unsigned int blocksize;
        unsigned int numentries;
        unsigned int numsets;

        unsigned int tagbits;
        unsigned int indexbits;
        unsigned int offsetbits;
        
        vector<vector<CacheStruct>> Array;
};

#endif