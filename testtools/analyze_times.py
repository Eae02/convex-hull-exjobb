#Usage: python3 analyze_times.py n < times.txt

import sys

numtests = 0
totelapsed = 0.0
totcompute = 0.0

n = int(sys.argv[1]) #Number of input points in each test

for line in sys.stdin.readlines():
    if "elapsed time" in line:
        numtests+=1
        totelapsed += float(line.split()[2][:-2])
    elif "compute time" in line:
        totcompute += float(line.split()[2][:-2])
if numtests>0:
    print("Average elapsed time:", '%.6f' % (totelapsed/numtests), "ms,", "time per point:", '%.6f' % (totelapsed/numtests/n*1000), "us")
    print("Average compute time:", '%.6f' % (totcompute/numtests), "ms,", "time per point:", '%.6f' % (totcompute/numtests/n*1000), "us")

