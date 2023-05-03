#include <DataInput.h>

Reference::Reference(string fapath) {
    string path = fapath;
    this->faPath = (char*)malloc(sizeof(char) * (path.size() + 1));
    strcpy(this->faPath, path.c_str());
    cout << "DIP: this->fapath: " << this->faPath << endl;
    int algo_type = BWTALGO_AUTO, block_size = 10000000; // Copied from bwa_index() method from bwtindex.c in bwa tool source code.
    bwa_idx_build(this->faPath, this->faPath, algo_type, block_size);
    
    this->bwtPath = (char*)malloc(sizeof(char) * (path.size() + 5));
    strcpy(this->bwtPath, path.c_str());
    strcat(this->bwtPath, ".bwt");
    printf("BWT file path: %s\n", this->bwtPath);

    this->saPath = (char*)malloc(sizeof(char) * (path.size() + 4));
    strcpy(this->saPath, path.c_str());
    strcat(this->saPath, ".sa");
    printf("SA file path: %s\n", this->saPath);
    this->restore_bwt_sa();
}

Reference::Reference(string bwtpath, string sapath) {
    // .bwt file already exists.
    this->faPath = "Invalid";

    // expects two paths in the input argument.
    // first path is BWT file path.
    this->bwtPath = (char*)malloc(sizeof(char) * (bwtpath.size() + 1));
    strcpy(this->bwtPath, bwtpath.c_str());

    // second path is SA file path.
    this->saPath = (char*)malloc(sizeof(char) * (sapath.size() + 1));
    strcpy(this->saPath, sapath.c_str());
    this->restore_bwt_sa();
}

Reference::Reference(vector<string> paths, bool bwtpath) {
    if (!bwtpath) {
        // Make .bwt file before restoring it.
        // The first argument just has a single fasta file path.
        string path = paths[0];
        this->faPath = (char*)malloc(sizeof(char) * (path.size() + 1));
        strcpy(this->faPath, path.c_str());
        cout << "DIP: this->fapath: " << this->faPath << endl;
        int algo_type = BWTALGO_AUTO, block_size = 10000000; // Copied from bwa_index() method from bwtindex.c in bwa tool source code.
        bwa_idx_build(this->faPath, this->faPath, algo_type, block_size);
        
        this->bwtPath = (char*)malloc(sizeof(char) * (path.size() + 5));
        strcpy(this->bwtPath, path.c_str());
        strcat(this->bwtPath, ".bwt");
        printf("BWT file path: %s\n", this->bwtPath);

        this->saPath = (char*)malloc(sizeof(char) * (path.size() + 4));
        strcpy(this->saPath, path.c_str());
        strcat(this->saPath, ".sa");
        printf("SA file path: %s\n", this->saPath);
    }
    else {
        // .bwt file already exists.
        this->faPath = "Invalid";
        // expects two paths in the input argument.
        // first path is BWT file path.
        string path = paths[0];
        this->bwtPath = (char*)malloc(sizeof(char) * (path.size() + 1));
        strcpy(this->bwtPath, path.c_str());

        // second path is SA file path.
        path = paths[1];
        this->saPath = (char*)malloc(sizeof(char) * (path.size() + 1));
        strcpy(this->saPath, path.c_str());
    }
    this->restore_bwt_sa();
}

void Reference::restore_bwt_sa() {
    // cout << "DIP: Restoring BWT." << endl;
    // auto start = high_resolution_clock::now();
    this->bwaBwt = bwt_restore_bwt(this->bwtPath);
    // auto time_elapsed = duration_cast<seconds>(high_resolution_clock::now() - start);
    cout << "DIP: Done restoring BWT." << endl; // - Time taken: " << time_elapsed.count() << " seconds." << endl;
    // cout << "DIP: Calculating SA." << endl;
    auto start = high_resolution_clock::now();
    // bwt_cal_sa(this->bwaBwt, 1); // this should be called before calling bwt_sa().
    bwt_restore_sa(this->saPath, this->bwaBwt);
    auto time_elapsed = duration_cast<seconds>(high_resolution_clock::now() - start);
    cout << "DIP: Done calculating SA." << endl; // - Time taken: " << time_elapsed.count() << " seconds." << endl;
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
    uint64_t seedwithnoise = 0;
    for (auto read: this->reads) {
        // cout << read << endl;
        for (int i = 0; i < read.size(); i += seedLen) {
            // cout << "initial: " << i << " end: " << e << endl;
            string seed = read.substr(i, seedLen);
            // cout << "Seed: " << seed << endl;
            if (seed.find('N') == string::npos)
                this->seeds.push_back(seed);
            else
                seedwithnoise++;
        }
    }
    this->seeds.push_back("EOS");
    // cout << "Seeds: " << this->seeds.size() << endl;
    // for (auto seed: this->seeds)
    //     cout << seed << endl;
    cout << "Seeds with Noise (not in the final set): " << seedwithnoise << endl;
    cout << "Seeds Count: " << this->seeds.size() << endl;
}

string Reads::get_seed(uint64_t idx) {
    if (idx >= this->seeds.size()) {
        cout << "Invalid seed index: " << idx << endl;
        cout << "Seed vector size: " << this->seeds.size() << endl;
        return "Invalid";
    }
    else
        return this->seeds[idx];
}

bitset<64> Reads::get_seed_bitset(uint64_t idx) {
    string seed = this->get_seed(idx);
    if (seed == "Invalid") {
        cout << "Invalid seed access." << endl;
        return bitset<64>(0xFFFFFFFFFFFFFFFF); // say end of seeds.
    }

    if (seed == "EOS")
        return bitset<64>(0xFFFFFFFFFFFFFFFF);

    map<char, string> base_bitstr_map = {{'A', "000"},
                                         {'C', "001"},
                                         {'G', "010"},
                                         {'T', "011"}};
    string seedbits = "111";
    for (char b: seed)
        seedbits += base_bitstr_map[b];
    
    cout << "idx: " << idx << " seed: " << seed << " seedbits: " << seedbits << endl;

    if (seedbits.size() > 64) {
        cout << "Seed bits more than 64. Exiting" << endl;
        exit(-1);
    }
    return bitset<64>(seedbits);
}

uint64_t Reads::get_seedsCount() {
    return this->seeds.size() - 1;
}