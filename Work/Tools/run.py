import os
import argparse
import subprocess

class DirIP(object):
    def __init__(self, args):
        self.cfgd = os.path.abspath(args.cfgd)
        self.refd = os.path.abspath(args.refd)
        self.readd = os.path.abspath(args.readd)
        self.fsd = os.path.abspath(args.fsd)
        self.pmd = os.path.abspath(args.pmd)
        self.opdir = os.path.abspath(args.opdir)

    def print(self):
        print("Directory path CLAs ----------------")
        print("Config Directory: ", self.cfgd)
        print("Reference Directory: ", self.refd)
        print("Read Directory: ", self.readd)
        print("Functional Simulator Directory: ", self.fsd)
        print("Performance Model Directory: ", self.pmd)
        print("Output Directory: ", self.opdir)
        print()

class Files(object):
    def __init__(self, files_line):
        # print("Files - files_line:", files_line)
        files = [file.strip() for file in files_line.split(',')]
        self.cfgf = files[0]
        self.reff = files[1]
        self.readf = files[2]

    @property
    def cfgid(self):
        return self.cfgf.split('.')[0]

    @property
    def refid(self):
        return self.reff.split('/')[-2]

    @property
    def readid(self):
        return self.readf.split('/')[-1].split('.')[0]

    def print(self):
        print(self.cfgf, self.reff, self.readf)

class Run(object):
    runcount = 0
    def __init__(self, dirs, line, runsfid):
        self.id = self.runcount
        self.files = Files(line)
        self.cfgpath = os.path.join(dirs.cfgd, self.files.cfgf)
        self.refpath = os.path.join(dirs.refd, self.files.reff)
        self.bwtpath = self.refpath + ".bwt"
        self.sapath = self.refpath + ".sa"
        self.readpath = os.path.join(dirs.readd, self.files.readf)
        self.fsbinary = os.path.join(dirs.fsd, "funcsim")
        self.pmbinary = os.path.join(dirs.pmd, "model")

        opsubdir1 = "ip_" + runsfid
        opsubdir2 = "op_cfg_" + self.files.cfgid + '_ref_' + self.files.refid + '_read_' + self.files.readid
        self.opdir = os.path.join(dirs.opdir, opsubdir1, opsubdir2)
        if not os.path.isdir(self.opdir):
            os.makedirs(self.opdir)

        perfcsv_file = os.path.join(self.opdir, "..", "Perf.csv")
        print("perf csv file:", perfcsv_file)
        if os.path.isfile(perfcsv_file):
            os.remove(perfcsv_file)

        self.pmconsoleoppath = os.path.join(self.opdir, "runperfmodel.out")
        self.fsconsoleoppath = os.path.join(self.opdir, "runfuncsim.out")
        
        Run.runcount += 1

    def funcsim(self):
        if os.path.isfile(self.bwtpath) and os.path.isfile(self.sapath):
            command = [self.fsbinary, "--bwt", self.bwtpath,
                                      "--sa", self.sapath,
                                      "--reads", self.readpath,
                                      "--op", self.opdir]
        else:
            command = [self.fsbinary, "--ref", self.refpath,
                                      "--reads", self.readpath,
                                      "--op", self.opdir]
        
        print("\nRunning Functional Simulator with command:")
        print(' '.join(command))
        with open(self.fsconsoleoppath, "w") as outfile:
            subprocess.run(command, stdout=outfile)

    def perfmodel(self):
        # print("BWT:", self.bwtpath, os.path.isfile(self.bwtpath))
        # print("SA:", self.sapath, os.path.isfile(self.sapath))
        if os.path.isfile(self.bwtpath) and os.path.isfile(self.sapath):
            command = [self.pmbinary, "--cfg", self.cfgpath, 
                                      "--bwt", self.bwtpath,
                                      "--sa", self.sapath,
                                      "--reads", self.readpath,
                                      "--op", self.opdir]
        else:
            command = [self.pmbinary, "--cfg", self.cfgpath, 
                                      "--ref", self.refpath,
                                      "--reads", self.readpath,
                                      "--op", self.opdir]
        
        print("\nRunning Performance Model with command:")
        print(' '.join(command))
        with open(self.pmconsoleoppath, "w") as outfile:
            subprocess.run(command, stdout=outfile)
    
    def verify_functional_correctness(self):
        fsopfile = self.opdir + "/FSEMResults.txt"
        with open(fsopfile, 'r') as fsop:
            fsresults = fsop.readlines()
            # print("FS Results:", fsresults)

        pmopfile = self.opdir + "/SiMEM.mem"
        with open(pmopfile, 'r') as pmop:
            pmresults = [line for line in pmop if line != "0\t0\n"]
            # print("PM Results:", pmresults)

        if len(fsresults) == len(pmresults) and fsresults == pmresults:
            fcresult = "Passed"
        else:
            fcresult = "Failed"
        print("\nFunctional Correctness Verification:", fcresult)

    def print(self):
        print("\n" + '-'*40)
        print("Run ID:", self.id)
        print("Run inputs:", self.files.print())

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Runs performance model and fcv and collects results.')
    parser.add_argument('--cfgd', default="", type=str, help='Path to the config directory.')
    parser.add_argument('--refd', default="", type=str, help='Path to the input fasta directory.')
    parser.add_argument('--readd', default="", type=str, help='Path to the input fastq directory.')
    parser.add_argument('--fsd', default="", type=str, help='Path containing the functional simulator.')
    parser.add_argument('--pmd', default="", type=str, help='Path containing the performance model.')
    parser.add_argument('--opdir', default="", type=str, help='Path to place the result files.')
    parser.add_argument('--runs', default="", type=str, help='Path to the list of runs in .txt format.')
    args = parser.parse_args()

    print("args:", args)

    dirs = DirIP(args)
    dirs.print()

    runsfp = os.path.abspath(args.runs)
    runsfid = os.path.split(runsfp)[1].split('.')[0]
    print("Input path:", runsfp)
    with open(runsfp, "r") as runsf:
        runs = [Run(dirs, line, runsfid) for line in runsf]

    for run in runs:
        run.print()
        # Run Functional Simulator
        run.funcsim()
        # Run Performance Model
        run.perfmodel()
        # Compare si outputs from both for verifying functional correctness
        print("\nVerifying Functional Correctness.")
        run.verify_functional_correctness()