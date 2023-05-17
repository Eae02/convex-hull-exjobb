DATASETS = ["circle", "disk", "square"]
RUNS = 5

import testlib
import sys
import math

def getcmdarg(name, default=None):
	for i in range(1, len(sys.argv)):
		if sys.argv[i].startswith(f"-{name}="):
			return sys.argv[i].split("=", maxsplit=1)[1]
	if default is None:
		print(f"missing command line argument -{name}")
		exit(1)
	return default

implName = getcmdarg("i")

maxThreadsLog = int(getcmdarg("lt", -1))
if maxThreadsLog != -1:
	threads = [2 ** i for i in range(math.floor(math.log2(maxThreadsLog)) + 1)]
else:
	maxThreads = int(getcmdarg("t", 8))
	threads = [i for i in range(1, maxThreads + 1)]

measurements = []
for dataset in DATASETS:
	measurements.append([])
	for mi, th in enumerate(threads):
		print(f"running {implName} on {dataset} with {th} threads...", end="", flush=True, file=sys.stderr)
		t = testlib.runOnAllFiles([dataset], implName, runs=RUNS, maxThreads=th)
		print(f" done! ({t:.2f}ms)", file=sys.stderr)
		measurements[-1].append(t)

print("data\t" + "\t".join(map(str, threads)))
for dataset, m in zip(DATASETS, measurements):
	print(dataset + "\t" + "\t".join(map(str, m)))
