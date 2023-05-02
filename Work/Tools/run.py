import argparse

class DirIP(object):
    def __init__(self, cfgd, refd, readd, opdir):
        self.cfgd = cfgd
        self.refd = refd
        self.readd = readd
        self.opdir = opdir

class Files(object):
    def __init__(self, files_line):
        files = [file.strip() for file in files_line.split(',')]
        self.cfgf = files[0]
        self.reff = files[1]
        self.readf = files[2]

class ModelIO(object):
    def __init__(self, dirs, files):
        self.cfgpath = dirs.cfgd + '/' + files.cfgf
        self.refpath = dirs.refd + '/' + files.reff
        self.readpath = dirs.readd + '/' + files.readf

class Run(object):
    run_id = -1
    def __init__(self, dirs, line):
        MyClass.run_id += 1
        self.model_io = ModelIO(dirs, Files(line))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Runs performance model and fcv and collects results.')
    parser.add_argument('--cfgd', default="", type=str, help='Path to the input fasta file.')
    parser.add_argument('--refd', default="", type=str, help='Path to the input fasta file.')
    parser.add_argument('--readd', default="", type=str, help='Path to the input fastq file.')
    parser.add_argument('--opdir', default="", type=str, help='Path to place the result files.')
    parser.add_argument('--runs', default="", type=str, help='Path to the list of runs in .txt format.')
    args = parser.parse_args()

    dirs = Directory(args.cfgd, args.refd, args.readd, args.opdir)
    runsf = args.runs

    with open(runsf, "r"):
        runs = [Run(dirs, runline) for line in runsf]
