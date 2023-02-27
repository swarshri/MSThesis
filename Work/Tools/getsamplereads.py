import random
import argparse

if __name__ == "__main__":
    ipreffile = r"/home/sc8781/Thesis/git/Work/PerfModel/data/sample/trial_Ref237920Base/GRCh38_237920.fasta"
    with open(ipreffile, 'r') as ip:
        ref = ''.join(ip.readlines()[1:]).replace('\n', '')

    for i in range(10000000):
        rn = random.randint(0, len(ref))
        print(rn)
        line = "@SAMPLEX."+str(i)+" "+str(i)+"/1"+"\n"
        line += ref[rn:rn+151].upper()
        line += '\n'+'+'+'\n'
        line += '?'*151+'\n'
        print(line)
        
    opfile = r"/home/sc8781/Thesis/git/Work/PerfModel/data/sample/trial_Ref237920Base/sample_reads_100000.fastq"
    with open(opfile, 'w') as op:
        op.writelines(outlines)