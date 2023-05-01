#include<iostream>
#include<bitset>
#include<vector>
#include<math.h>

#include<Config.h>

using namespace std;

#ifndef CACHE_H
#define CACHE_H

struct IncomingCacheStruct {
    uint64_t address;
    int basePointer;
    vector<uint64_t> data;

    friend std::ostream& operator <<(std::ostream& os, IncomingCacheStruct const& e) {
        return os << e.address << "\t"
                  << e.basePointer << "\t"
                  << e.data[0];
    }
};

struct CacheStruct {
    uint64_t tag; // all 32 will never be used. Using the max to not make this a template.
    int lowestBasePointer;
    unsigned int accessCount; // keeps a count of how many times this has been accessed before.
    vector<uint64_t> data; // length = blocksize - usually 1 because there is no spatial locality with the current data layout in OccMemory.
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
        Cache(string, SysConfig *);

        pair<bool, uint64_t> read(uint64_t);
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