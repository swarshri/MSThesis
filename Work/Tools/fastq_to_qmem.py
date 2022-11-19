import os
import argparse

if __name__ == "__main__":
     
    #parse arguments for input file location
    parser = argparse.ArgumentParser(description='FastQToQMem')
    parser.add_argument('--fastq', default="", type=str, help='Path to the input fastq file.')
    parser.add_argument('--qmem', default="", type=str, help='Path to the output qmem file.')
    args = parser.parse_args()

    fastq_file = os.path.abspath(args.fastq)
    print("Fastq Location:", fastq_file)
    qmem_file = os.path.abspath(args.qmem)
    print("QMem Location:", qmem_file)

    fastq_reads = []
    with open(fastq_file, "r") as fastq:
        sample_name = fastq.readline().split('.')[0].replace('@', '')
        fastq_reads = [line.upper().strip() for line in fastq.readlines() if line[0] not in ['@', '+', '?', '\'', '5']]

    #print(fastq_reads, len(fastq_reads), len(fastq_reads[-1]))

    base_mem_dict = {'A': '000',
                     'C': '001',
                     'G': '010',
                     'T': '011',
                     'N': '100',
                     'E': '111'} #End Of Read

    with open(qmem_file, "w") as qmem:
        for read in fastq_reads:
            write_bitstr = ""
            for base in read[::-1]:
                if base in base_mem_dict.keys():
                    write_bitstr += base_mem_dict[base] + '\n'
                else:
                    write_bitstr += base_mem_dict['N'] + '\n'
            write_bitstr += base_mem_dict['E'] + '\n'
            qmem.writelines(write_bitstr)