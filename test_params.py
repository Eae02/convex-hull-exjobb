DATASETS = ["circle", "disk", "square"]
RUNS = 5

#IMPL_NAME = "qh_hybrid_esx"
#PARAM_NAME = "D"
#PARAM_LABEL = "depth to sort at"
#PARAM_RANGE = range(0, 15, 1)

#IMPL_NAME = "qh_bf"
#PARAM_NAME = "D"
#PARAM_LABEL = "depth to start compacting at"
#PARAM_RANGE = range(0, 20)

#IMPL_NAME = "qh_bf"
#PARAM_NAME = "P"
#PARAM_LABEL = "#points to perform compacting at"
#PARAM_RANGE = range(0, 300, 10)

IMPL_NAME = "qh_recpar_nxp"
PARAM_NAME = "T"
PARAM_LABEL = "number of threads"
PARAM_RANGE = [0, 1, 2, 4, 8, 16, 32, 64, 128]

import testlib
import sys
import json
import time

try:
	import matplotlib.pyplot as plt
except:
	pass

def plot(data, name):
	fig, ax = plt.subplots(figsize=(16,12))
	for label, measurements in data:
		ax.plot([m[0] for m in measurements], [m[1] for m in measurements], linewidth=2.0, label=label)
	ax.set_xlabel(PARAM_LABEL or PARAM_NAME)
	ax.set_ylabel("time (ms)")
	ax.set_xticks([m[0] for m in measurements])
	ax.grid(True)
	plt.legend()
	plt.savefig("results/" + name)

if "onlyplot" in sys.argv:
	data = json.loads(sys.stdin.read())
	plot(data, "plotFromJson_" + str(int(time.time())))
else:
	datasetMeasurements = []
	for dataset in DATASETS:
		datasetMeasurements.append((dataset, []))
		for s in list(PARAM_RANGE):
			runName = f"{IMPL_NAME}:{PARAM_NAME}{s}"
			print(f"running {runName} on {dataset}...", end="", flush=True)
			t = testlib.runOnAllFiles([dataset], runName, runs=RUNS)
			print(f" done! ({t:.2f}ms)")
			datasetMeasurements[-1][1].append((s, t))
	
	if "dump" in sys.argv:
		with open(f"results/dump_{IMPL_NAME}:{PARAM_NAME}.json", "w") as fp:
			json.dump(datasetMeasurements, fp)
	else:
		plot(datasetMeasurements, f"{IMPL_NAME}:{PARAM_NAME}")
