DATASETS = ["circle", "disk", "square"]
METRIC = "time"

import testlib
import sys
import math

runs = int(testlib.getcmdarg("r", 5))

implName = testlib.getcmdarg("i")

maxThreadsLog = int(testlib.getcmdarg("lt", -1))

if "-t80" in sys.argv:
	threads = [1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80]
elif maxThreadsLog != -1:
	threads = [2 ** i for i in range(math.floor(math.log2(maxThreadsLog)) + 1)]
else:
	maxThreads = int(testlib.getcmdarg("t", 8))
	threads = [i for i in range(1, maxThreads + 1)]

datasetSize = "large"
if "-med" in sys.argv:
	datasetSize = "medium"

measurements = []
for dataset in DATASETS:
	measurements.append([])
	for mi, th in enumerate(threads):
		print(f"running {implName} on {dataset}/{datasetSize} with {th} threads...", end="", flush=True, file=sys.stderr)
		t = testlib.runOnAllFiles([dataset], implName, runs=runs, maxThreads=th, metric=METRIC, datasetSize=datasetSize)
		print(f" done! ({t:.2f})", file=sys.stderr)
		measurements[-1].append(t)

print("data\t" + "\t".join(map(str, threads)))
for dataset, m in zip(DATASETS, measurements):
	print(dataset + "\t" + "\t".join(map(str, m)))
