#include "FCVTool.h"

Reference::Reference(char* path, bool bwtpath) {
    if (!bwtpath) {
        this->faPath = path;
        // Copied from bwa_index() method from bwtindex.c in bwa tool source code.
        int algo_type = BWTALGO_AUTO, block_size = 10000000;
        bwa_idx_build(this->faPath, this->faPath, algo_type, block_size);
        this->bwtPath = path;
        strcat(this->bwtPath, ".bwt");
        printf("BWT path: %s\n", this->bwtPath);
    }
    else {
        this->faPath = "Invalid";
        this->bwtPath = path;
    }
    // printf("restoring bwt.\n");
    this->bwaBwt = bwt_restore_bwt(bwtPath);
    // printf("restored bwt.\n");
    this->seqLen = this->bwaBwt->seq_len + 1; // +1 for '$' added to the ref before BWT.
    this->occLen = this->bwaBwt->seq_len + 2; // an extra column due to the added '$'.
    this->Count = {{'A', 0}, {'C', 0}, {'G', 0}, {'T', 0}};
    this->Occ = {{'A', vector<bwtint_t>{}}, {'C', vector<bwtint_t>{}}, {'G', vector<bwtint_t>{}}, {'T', vector<bwtint_t>{}}};
    this->SA.resize(this->seqLen);
    // printf("Resized vector variables\n");
}

void Reference::construct_fmdi() {
    // printf("in here1.");
    for (int i = 0; i < 4; i++) {
        // printf("In here2");
        char base = this->int_base[i];
        this->Count[base] = this->bwaBwt->L2[i] + 1; // +1 to account for '$'.
        this->Occ[base].resize(this->occLen);
        this->Occ[base][0] = 0;
        for (int j = 1; j < this->occLen; j++) {
            this->Occ[base][j] = bwt_occ(this->bwaBwt, j-1, i);
            // printf("In here3");
        }
    }
    this->SA[0] = this->bwaBwt->seq_len; // indicates the permutation that starts with '$'.
    bwt_cal_sa(this->bwaBwt, 1); // this should be called before calling bwt_sa().
    for (int i = 1; i < this->seqLen; i++)
        this->SA[i] = bwt_sa(this->bwaBwt, i);
}

vector<vector<uint64_t>> Reference::find_seed(string seed) {
    uint64_t low = 0;
    uint64_t high = this->seqLen;
    for (int i = seed.size() - 1; i >= 0; i--) {
        char base = seed[i];
        low = this->Count[base] + this->Occ[base][low];
        high = this->Count[base] + this->Occ[base][high];
        if (low >= high)
            break;
    }
    // cout << "Seed: " << seed << " Low: " << low << " High: " << high << endl;
    vector<uint64_t> sivals{low, high};

    vector<uint64_t> savals{};
    // get Suffix array values in the interval low to high (excluding the one at high)
    if (low < high) {
        for (int i = low; i < high; i++)
            savals.push_back(this->SA[i]);
    }

    return vector<vector<uint64_t>>{sivals, savals};
}

void Reference::print_fmdi() {
    cout << endl;
    cout << "-----------------------------------------" << endl;
    cout << "this->seqLen: " << this->seqLen << endl;
    cout << "this->occLen: " << this->occLen << endl;
    cout << "Count size: " << this->Count.size() << endl;
    cout << "Count Values:";
    for (auto cnt: this->Count)
        cout << " " << cnt.second;
        // printf("this->Count[%c]: %ld\n", cnt.first, cnt.second);
    cout << endl;
    int inc;
    if (this->seqLen < 500)
        inc = 1;
    else if (this->seqLen < 10000)
        inc = this->seqLen/10;
    else
        inc = this->seqLen/100;
    cout << "Occ Values at inc: " << inc;
    for (auto occ: this->Occ) {
        // printf("Base: %c---------------\n", occ.first);
        cout << endl << "Base - " << occ.first << ":";
        for (int j = 0; j < this->occLen; j+=inc)
            cout << " " << occ.second[j];
            // printf("Occ[%d]: %ld\n", j, occ.second[j]);
    }
    cout << endl;
    cout << "SA Values: ";
    for (int i = 0; i < this->SA.size(); i+=inc)
        cout << " " << this->SA[i];
        // printf("SA[%d]: %d\n", i, this->SA[i]);
    cout << endl << "-------------------------------------------------" << endl;
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
    }
    else cout<<"Unable to open input file for Reads: " << this->fqPath << endl;
}

void Reads::make_seeds(int seedLen) {
    printf("\nReads----------------------\n");
    for (auto read: this->reads) {
        cout << read << endl;
        for (int i = 0; i < read.size(); i += seedLen) {
            // cout << "initial: " << i << " end: " << e << endl;
            string seed = read.substr(i, seedLen);
            // cout << "Seed: " << seed << endl;
            if (seed.find('N') == string::npos)
                this->seeds.push_back(seed);
        }
    }
    // cout << "Seeds: " << endl;
    // for (auto seed: this->seeds)
    //     cout << seed << endl;
}

string Reads::get_seed(int idx) {
    if (idx >= this->seeds.size()) {
        cout << "Invalid seed index: " << idx << endl;
        cout << "Seed vector size: " << this->seeds.size() << endl;
    }
    else
        return this->seeds[idx];
}

uint64_t Reads::get_seedsCount() {
    return this->seeds.size();
}

void Reads::save_result(string seed, vector<uint64_t> si, vector<uint64_t> sa) {
    SeedResult sr;
    sr.seed = seed;
    sr.si_values = si;
    sr.sa_values = sa;
    this->seedresults.push_back(sr);
    // cout << "Saved result: seed results size: " << this->seedresults.size() << endl;
}

void Reads::print_seedresults() {
    cout << endl << "Seeds and results: " << this->seedresults.size() << endl;
    for (auto sr: this->seedresults) {
        cout << sr.seed << "\t\t";
        for (auto sival: sr.si_values)
            cout << " " << sival; 
        cout << "\t\t";
        for (auto saval: sr.sa_values)
            cout << " " << saval;
        cout << endl;
    }
}

int main(int argc, char * argv[]) {    
    char* fasta_path = "";
    int fasta_arg = 0;
    char* bwt_path = "";
    int bwt_arg = 1;
    char* fastq_path = "";

    if (argc != 5) {
        cout << "Invalid number of arguments." << endl;
        cout << "Expected path for the input fasta and fastq files." << endl;
        cout << "Machine stopped." << endl;
        return -1;
    }
    else {
        for (int i = 1; i < argc; i++) {
            // cout << "init i: " << i << endl;
            cout << argv[i] << endl;
            if (strcmp(argv[i], "--ref") == 0) {
                fasta_path = argv[++i];
                fasta_arg = i;
                cout << "Found reference file: " << fasta_path << fasta_arg << endl;
            }
            else if (strcmp(argv[i], "--bwt") == 0) {
                bwt_path = argv[++i];
                cout << "Found BWT file: " << bwt_path << endl;
            }
            else if (strcmp(argv[i], "--reads") == 0) {
                fastq_path = argv[++i];
                cout << "Found reads file: " << fastq_path << endl;
            }
            // cout << "fin i: " << i << endl;
        }
        // TODO: Check file path extensions to make sure they are fasta and fastq files.
        cout << "FASTA Reference file: " << fasta_path << endl;
        cout << "BWTIndexed file path: " << bwt_path << endl;
        cout << "FASTQ Read file path: " << fastq_path << endl;
    }

    Reference * ref;
    if (bwt_path == "")
        ref = new Reference(fasta_path, false);
    else
        ref = new Reference(bwt_path, true);
    
    ref->construct_fmdi();
    ref->print_fmdi();

    Reads * reads = new Reads(fastq_path);
    reads->make_seeds(20);

    // vector<vector<uint64_t>> seed_pos = ref->find_seed("CGTA");
    // vector<uint64_t> seed_si = seed_pos[0];
    // vector<uint64_t> seed_sa = seed_pos[1];
    
    // cout << "SI values: ";
    // for (auto si: seed_si)
    //     cout << si << " ";
    // cout << endl;
    
    // cout << "SA values: ";
    // for (auto sa: seed_sa)
    //     cout << sa << " ";
    // cout << endl;

    // cout << "reads->get_seedsCount(): " << reads->get_seedsCount() << endl;
    for (int i = 0; i < reads->get_seedsCount(); i++) {
        string seed = reads->get_seed(i);
        // cout << "New seed: " << seed << endl;
        vector<vector<uint64_t>> result_pos = ref->find_seed(seed);        
        vector<uint64_t> seed_si = result_pos[0];
        vector<uint64_t> seed_sa = result_pos[1];
        reads->save_result(seed, seed_si, seed_sa);
    }

    reads->print_seedresults();

    // bwtint_t sab, sae;

    // const ubyte_t read[] = {2,3};
    // bwt_match_exact(bwt, 2, read, &sab, &sae);
    // printf("sab: %ld; sae: %ld\n", sab, sae);

    // printf("Ref seq: %s\n", bwt->bwt);
    // SA[0] = 0
}