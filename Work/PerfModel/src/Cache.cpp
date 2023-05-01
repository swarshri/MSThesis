#include <Cache.h>

Cache::Cache(string base, SysConfig* config) {
    this->name = "Cache";
    this->ways = config->parameters["Associativity"];
    this->size = config->parameters["Size"] * pow(2, 10);
    this->blocksize = config->parameters["BlockSize"];

    this->numentries = this->size/this->blocksize;
    this->numsets = this->numentries/this->ways;

    this->offsetbits = ceil(log2(this->blocksize));
    this->indexbits = ceil(log2(this->numsets));
    this->tagbits = 32 - this->indexbits - this->offsetbits;

    cout << "In Cache constructor: " << base << " array size: " << this->Array.size() << endl;
    for (unsigned int i = 0; i < this->numsets; i++) {
        vector<CacheStruct> newSet;
        for (unsigned int j = 0; j < this->ways; j++) {
            CacheStruct newBlock;
            newBlock.tag = 0;
            newBlock.lowestBasePointer = 21; // Keeping it at the highest until the first write happens.
            newBlock.accessCount = 0;
            vector<uint64_t> newData;
            for (unsigned int k = 0; k < this->blocksize; k++)
                newData.push_back(0);
            newBlock.data = newData;
            newBlock.valid = false;

            newSet.push_back(newBlock);
            // cout << "Cache construction: Pushed a new block into a new set: " << j << endl;
            // cout << " Last added Block: " << newSet[j] << endl;
        }
        // cout << "Set size: " << newSet.size() << endl;
        this->Array.push_back(newSet);
        // cout << "Cache construction: Pushed a new set into this->Array: " << i << endl;
    }
    cout << "In Cache constructor: " << base << " finished construction: " << this->Array.size() << endl;
}

pair<bool, uint64_t> Cache::read(uint64_t address) {
    // extract tag, index, and offset from the address.
    uint64_t offset;
    if (this->offsetbits == 0)
        offset = 0;
    else
        offset = address & (0xFFFFFFFF >> (32 - this->offsetbits));
    unsigned int index = (address >> this->offsetbits) & (0xFFFFFFFF >> (32- this->indexbits));
    unsigned int tag = (address >> (this->offsetbits + this->indexbits));

    cout << "Read Index: " << index << " Tag: " << tag  << " Offset: " << offset << endl;
    // get all ways in the set = Array[index].
    // run through all the ways in the set and compare tag and check validity - valid should always be 1 because, the
    // data never gets written, the blocks only get replaced by other more useful blocks after being loaded from memory.
    for (unsigned int i = 0; i < this->ways; i++) {
        auto way = &this->Array[index][i];
        if (way->valid && way->tag == tag) { // if hit, send the data out.
            cout << "Cache hit!!! for address: " << address << endl;
            cout << "Hit cache data: " << way->data[offset] << endl;
            return pair<bool, uint64_t>(true, way->data[offset]);
        }
    }

    // if miss, send false and 0 out.
    cout << "Cache miss!!! for address: " << address << endl;
    return pair<bool, uint64_t>(false, 0);
}

bool Cache::write(IncomingCacheStruct incoming) {
    cout << this->name << ": Writing incoming cache entry: " << incoming << endl;
    // extract tag, index, and offset from the address.
    uint64_t address = incoming.address;
    // unsigned int offset = address.to_ulong() & (0xFFFFFFFF >> (32- this->offsetbits)); // - unused
    unsigned int index = (address >> this->offsetbits) & (0xFFFFFFFF >> (32- this->indexbits));
    unsigned int tag = (address >> (this->offsetbits + this->indexbits)) & (0xFFFFFFFF >> (32 - this->tagbits));
    cout << this->name << ": Incoming cache entry index: " << index << " tag: " << tag << endl;

    // run through all the ways in the set = Array[index] and compare tag.
    // if there is a tag match, the data is already present in the cache.
    // in this case, we return true after incrementing the accessCount and comparing
    // the lowestBasePointer field. if the incoming basePointer is lesser than the 
    // lowestBasePointer field, we replace the lowestBasePointer field with the incoming one.
    int firstEmptyWay = -1;
    unsigned int highestBasePointer = 0;
    int wayWithHighestBasePointer = -1;
    int count = 0;
    cout << this->name << " this->Array.size(): " << this->Array.size() << endl;
    cout << "this->ways: " << this->ways << endl;
    for (unsigned int i = 0; i < this->ways; i++) {
    // for (auto way = this->Array[index].begin(); way != this->Array[index].end(); way++) {
        auto way = &this->Array[index][i];
        cout << "In for loop 1 - count: " << count << " way->valid: " << way->valid << endl;
        if (way->valid) {
            cout << "In if (way->valid) case. " << way->valid << endl;
            if (way->tag == tag) {
                way->accessCount++;
                if (incoming.basePointer < way->lowestBasePointer)
                    way->lowestBasePointer = incoming.basePointer;
                cout << this->name << ": Cache entry already present in index: " << index << " way: " << count << endl;
                cout << this->name << ": Cache entry: " << *way << endl;
                return true; // data is already in cache. Cache hit on write.
            }
            else if (way->lowestBasePointer > highestBasePointer) {                
                cout << "In else if (way->lowestBasePointer > highestBasePointer) case. " << way->lowestBasePointer << endl;
                wayWithHighestBasePointer = count;
                highestBasePointer = way->lowestBasePointer;
            }
        }
        else if (!way->valid && firstEmptyWay == -1) {
            cout << "In else if (!way->valid) case." << way->valid << endl;
            firstEmptyWay = count;
            break;
        }
        count++;
    }

    cout << this->name << ": firstEmptyWay: " << firstEmptyWay << endl;
    cout << this->name << ": wayWithHighestBasePointer: " << wayWithHighestBasePointer << endl;
    cout << this->name << ": highestBasePointer: " << highestBasePointer << endl;

    int chosenWay = -1;
    // if none of the tags match, look for an empty way in the set with valid = false.
    // if there is an empty way - shove the data into it and return True (success).
    // the first empty way is in firstEmptyWay.
    if (firstEmptyWay != -1) 
        chosenWay = firstEmptyWay;
    // if there are no empty ways in the set, we find the way with highest lowestBasePointer.
    // if the incoming basecount is lesser than that, we replace it with this one and return True (success).
    // only BaseCount based replacement policy for now - doesn't worry about accessCount. 
    else if (wayWithHighestBasePointer != -1)
        chosenWay = wayWithHighestBasePointer;
        
    if (chosenWay != -1) {
        cout << this->name << ": Entry before: " << this->Array[index][chosenWay] << endl;
        this->Array[index][chosenWay].tag = tag;
        this->Array[index][chosenWay].lowestBasePointer = incoming.basePointer;
        this->Array[index][chosenWay].accessCount = 0;
        if (incoming.data.size() == this->Array[index][chosenWay].data.size())
            this->Array[index][chosenWay].data = incoming.data;
        else
            cout << "Cache: " << this->name << " - ERROR: incoming block size mismatch with the intended block size." << endl;
        this->Array[index][chosenWay].valid = true;
        cout << this->name << ": Written in index: " << index << " way: " << chosenWay << endl;
        cout << this->name << ": Written entry: " << this->Array[index][chosenWay] << endl;
        return true;
    }
    
    // return False otherwise. Cache doesn't hold the data.
    return false;
}