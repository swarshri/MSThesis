#include <DataInput.h>

Reference::Reference(char* path, bool bwtpath) {
    if (!bwtpath) {
        // Make .bwt file before restoring it.
        this->faPath = path;
        int algo_type = BWTALGO_AUTO, block_size = 10000000; // Copied from bwa_index() method from bwtindex.c in bwa tool source code.
        bwa_idx_build(this->faPath, this->faPath, algo_type, block_size);
        this->bwtPath = path;
        strcat(this->bwtPath, ".bwt");
        printf("BWT path: %s\n", this->bwtPath);
    }
    else {
        // .bwt file already exists.
        this->faPath = "Invalid";
        this->bwtPath = path;
    }
    this->bwaBwt = bwt_restore_bwt(bwtPath);
    bwt_cal_sa(this->bwaBwt, 1); // this should be called before calling bwt_sa().
    this->seqLen = this->bwaBwt->seq_len + 1; // +1 for '$' added to the ref before BWT.
    this->occLen = this->bwaBwt->seq_len + 2; // an extra column due to the added '$'.
}

bwt_t * Reference::getBWT() {
    return this->bwaBwt;
}

uint64_t Reference::get_seqLen() {
    return this->seqLen;
}

uint64_t Reference::get_occLen() {
    return this->occLen;
}

uint64_t Reference::getCount(char base) {
    if (this->base_int.find(base) != this->base_int.end()) {
        int i = this->base_int[base];
        return this->bwaBwt->L2[i] + 1; // +1 to account for '$'.
    }
    else {
        cout << "Count Invalid base character: " << base << endl;
        return 0;
    }
}

uint64_t Reference::getCount(int base) {
    if (base < 4)
        return this->bwaBwt->L2[base] + 1; // +1 to account for '$'.
    else
        cout << "Count Invalid base character: " << base << endl;
    return 0;
}

uint64_t Reference::getOcc(char base, uint64_t idx) {
    if (this->base_int.find(base) != this->base_int.end()) {
        int i = this->base_int[base];
        if (idx == 0)
            return 0;
        else if(idx < this->occLen)
            return bwt_occ(this->bwaBwt, idx-1, i);
        else
            cout << "Occ Index: " << idx << "should be less than " << this->occLen << endl;
    }
    else
        cout << "Occ Invalid base character: " << base << endl;
}

uint64_t Reference::getOcc(int base, uint64_t idx) {
    if (base < 4) {
        if (idx == 0)
            return 0;
        else if(idx < this->occLen)
            return bwt_occ(this->bwaBwt, idx-1, base);
        else
            cout << "Occ Index: " << idx << "should be less than " << this->occLen << endl;
    }
    else
        cout << "Occ Invalid base character: " << base << endl;
}

uint64_t Reference::getSA(uint64_t idx) {
    if (idx == 0)
        return this->bwaBwt->seq_len; // indicates the permutation that starts with '$'.
    else if(idx < this->seqLen)
        return bwt_sa(this->bwaBwt, idx);
    else
        cout << "SA Index: " << idx << "should be less than " << this->seqLen << endl;
}

Reads::Reads(string fqpath) {
    this->fqPath = fqpath;
    this->maxSeedLen = 20; // seed length cannot exceed this at the moment because our seeds are fixed length of 64 bits and each base takes 3 bits.

    ifstream readf;
    string line;
    readf.open(this->fqPath);

    if (readf.is_open()) {
        cout << endl << "File opened: " << this->fqPath << endl;
        bool readline = false;
        while (getline(readf, line)) {
            if (readline) {                
                this->reads.push_back(line);
                readline = false;
            }
            if (line[0] == '@') // If this line starts with '@' - next line contains the read
                readline = true;
        }
        readf.close();
        cout << "Found " << this->reads.size() << " reads." << endl;
    }
    else cout<<"Unable to open input file for Reads: " << this->fqPath << endl;
}

void Reads::make_seeds(int seedLen) {
    // printf("\nReads----------------------\n");
    cout << "Making seeds of maximum length: " << seedLen << endl;
    for (auto read: this->reads) {
        // cout << read << endl;
        for (int i = 0; i < read.size(); i += seedLen) {
            // cout << "initial: " << i << " end: " << e << endl;
            string seed = read.substr(i, seedLen);
            // cout << "Seed: " << seed << endl;
            if (seed.find('N') == string::npos)
                this->seeds.push_back(seed);
        }
    }
    // cout << "Seeds: " << this->seeds.size() << endl;
    // for (auto seed: this->seeds)
    //     cout << seed << endl;
}

string Reads::get_seed(int idx) {
    if (idx >= this->seeds.size()) {
        cout << "Invalid seed index: " << idx << endl;
        cout << "Seed vector size: " << this->seeds.size() << endl;
        return "Invalid";
    }
    else
        return this->seeds[idx];
}

uint64_t Reads::get_seedsCount() {
    return this->seeds.size();
}