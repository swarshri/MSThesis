#include "EMEFunc.h"

FMDI::FMDI(Reference * ref) {
    cout << "In FMDI Constructor." << endl;
    this->ref = ref;
    this->seqLen = this->ref->get_seqLen();
    this->occLen = this->ref->get_occLen();
    this->Count = {{'A', 0}, {'C', 0}, {'G', 0}, {'T', 0}};
    this->full_fmdi_available = false;
    this->get_counts();
    cout << "Done constructing FMDI." << endl;
    // this->construct_fmdi();
}

void FMDI::get_counts() {
    for (char base: {'A', 'C', 'G', 'T'})
        this->Count[base] = this->ref->getCount(base);
}

void FMDI::construct_fmdi() {
    this->Occ = {{'A', vector<bwtint_t>{}}, {'C', vector<bwtint_t>{}}, {'G', vector<bwtint_t>{}}, {'T', vector<bwtint_t>{}}};
    this->SA.resize(this->seqLen);
    for (char base: {'A', 'C', 'G', 'T'}) {
        this->Count[base] = this->ref->getCount(base);
        this->Occ[base].resize(this->occLen);
        for (int j = 0; j < this->occLen; j++) {
            this->Occ[base][j] = this->ref->getOcc(base, j);
        }
    }
    for (int i = 0; i < this->seqLen; i++)
        this->SA[i] = this->ref->getSA(i);
    this->full_fmdi_available = true;
}

SeedResult FMDI::find_seed(string seed) {
    uint64_t low = 0;
    uint64_t high = this->seqLen;
    for (int i = seed.size() - 1; i >= 0; i--) {
        char base = seed[i];
        uint64_t occ_low, occ_high;
        if (this->full_fmdi_available) {
            occ_low = this->Occ[base][low];
            occ_high = this->Occ[base][high];
        }
        else {
            occ_low = this->ref->getOcc(base, low);
            occ_high = this->ref->getOcc(base, high);
        }
        low = this->Count[base] + occ_low;
        high = this->Count[base] + occ_high;
        if (low >= high)
            break;
    }
    // cout << "Seed: " << seed << " Low: " << low << " High: " << high << endl;
    vector<uint64_t> savals{};
    // get Suffix array values in the interval low to high (excluding the one at high)
    // if (low < high) {
    //     for (int i = low; i < high; i++) {
    //         uint64_t saval;
    //         if (this->full_fmdi_available)
    //             saval = this->SA[i];
    //         else
    //             saval = this->ref->getSA(i);
    //         savals.push_back(saval);
    //         if (saval == 10000) {
    //             cout << "Found 10000. Can RIP lol." << endl;
    //             break;
    //         }
    //     }
    // }
    SeedResult ret_sr;
    ret_sr.seed = seed;
    ret_sr.si_values = {low, high};
    ret_sr.sa_values = savals;
    if (low < high) ret_sr.present = true;
    else ret_sr.present = false;

    return ret_sr;
}

void FMDI::print_fmdi() {
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
    if (this->full_fmdi_available) {
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
    }
    cout << endl << "-------------------------------------------------" << endl;
}

ExactMatchEngine::ExactMatchEngine(FMDI * idxed_ref, string opdir) {
    this->iref = idxed_ref;
    this->sropfile = opdir + "/FSEMResults.txt";
}

void ExactMatchEngine::find_exact_matches(Reads * reads) {
    uint64_t seedsCount = reads->get_seedsCount();
    cout << "Finding exact matches - Seed Count: " << seedsCount << endl;
    for (uint64_t i = 0; i < seedsCount; i++) {
        string seed = reads->get_seed(i);
        if (seed != "EOS") {
            SeedResult em_result = this->iref->find_seed(seed);
            this->seedresults.push_back(em_result);
        }
    }
}

void ExactMatchEngine::output_seedresults() {
    ofstream srop;
    srop.open(this->sropfile, std::ios_base::out);
	if (srop.is_open()) {
        cout << "Seed Result output file opened: " << this->sropfile << endl;
		for (auto sr: this->seedresults) {
            string line = to_string(sr.si_values[0]) + "\t" + to_string(sr.si_values[1]);
			srop << line << endl;
        }
	}
	else cout << "Unable to open output file: " << this->sropfile << endl;
	srop.close();
}

void ExactMatchEngine::print_seedresults() {
    cout << endl << "Seeds and results - Count: " << this->seedresults.size() << endl;
    for (auto sr: this->seedresults) {
        cout << sr.seed << "\t\t";
        for (auto sival: sr.si_values)
            cout << " " << sival; 
        cout << "\t\t" << sr.present;
        // for (auto saval: sr.sa_values)
        //     cout << " " << saval;
        // cout << *min_element(sr.sa_values.begin(), sr.sa_values.end());
        cout << endl;
    }
}

int main(int argc, char * argv[]) {    
    char* fasta_path = "";
    char* bwt_path = "";
    char* sa_path = "";
    char* fastq_path = "";
    string op_dir = "";

    cout << "Received " << argc << " arguments." << endl;
    if (argc != 7 and argc != 9) {
        cout << "Invalid number of arguments." << endl;
        cout << "Expected path for the input fasta (or bwt, sa pair), fastq files, and op file path." << endl;
        cout << "Valid commands are, " << endl;
        cout << "funcsim --ref <refpath> --reads <readpath> --op <opfilepath>" << endl;
        cout << "funcsim --bwt <bwtpath> --sa <sapath> --reads <readpath> --op <opfilepath>" << endl;
        cout << "Machine stopped." << endl;
        return -1;
    }
    else {
        for (int i = 1; i < argc; i++) {
            // cout << "init i: " << i << endl;
            cout << argv[i] << endl;
            if (strcmp(argv[i], "--ref") == 0) {
                fasta_path = argv[++i];
                cout << "Found reference file: " << fasta_path << endl;
            }
            else if (strcmp(argv[i], "--bwt") == 0) {
                bwt_path = argv[++i];
                cout << "Found BWT file: " << bwt_path << endl;
            }
            else if (strcmp(argv[i], "--sa") == 0) {
                sa_path = argv[++i];
                cout << "Found SA file: " << sa_path << endl;
            }
            else if (strcmp(argv[i], "--reads") == 0) {
                fastq_path = argv[++i];
                cout << "Found reads file: " << fastq_path << endl;
            }
            else if (strcmp(argv[i], "--op") == 0) {
                op_dir = argv[++i];
                cout << "Found op directory path: " << op_dir << endl;
            }
            // cout << "fin i: " << i << endl;
        }
        // TODO: Check file path extensions to make sure they are fasta and fastq files.
        cout << "FASTA Reference file: " << fasta_path << endl;
        cout << "BWTIndexed file path: " << bwt_path << endl;
        cout << "Suffix Array file path: " << sa_path << endl;
        cout << "FASTQ Read file path: " << fastq_path << endl;
        cout << "Output directory path: " << op_dir << endl;
    }

    Reference * ref;
    if (bwt_path == "")
        ref = new Reference(fasta_path);
    else
        ref = new Reference(bwt_path, sa_path);
    
    FMDI * idxed_ref = new FMDI(ref);
    idxed_ref->print_fmdi();

    Reads * reads = new Reads(fastq_path);
    reads->make_seeds(20);

    ExactMatchEngine * EMEngine = new ExactMatchEngine(idxed_ref, op_dir);
    EMEngine->find_exact_matches(reads);
    EMEngine->output_seedresults();
    EMEngine->print_seedresults();
}