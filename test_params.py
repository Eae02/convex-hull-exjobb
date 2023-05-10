DATASETS = ["circle", "disk", "square"]
RUNS = 5

IMPL_NAME = "qh_hybrid_esx"
PARAM_NAME = "D"
PARAM_LABEL = "depth to sort at"
PARAM_RANGE = range(0, 15, 1)

#IMPL_NAME = "qh_bf"
#PARAM_NAME = "D"
#PARAM_LABEL = "depth to start compacting at"
#PARAM_RANGE = range(0, 20)

#IMPL_NAME = "qh_bf"
#PARAM_NAME = "P"
#PARAM_LABEL = "#points to perform compacting at"
#PARAM_RANGE = range(0, 300, 10)

import testlib
import matplotlib.pyplot as plt

datasetMeasurements = []
for dataset in DATASETS:
	datasetMeasurements.append([])
	for s in list(PARAM_RANGE):
		runName = f"{IMPL_NAME}:{PARAM_NAME}{s}"
		print(f"running {runName} on {dataset}...", end="", flush=True)
		t = testlib.runOnAllFiles([dataset], runName, runs=RUNS)
		print(f" done! ({t:.2f}ms)")
		datasetMeasurements[-1].append((s, t))

fig, ax = plt.subplots(figsize=(16,12))
for measurements, label in zip(datasetMeasurements, DATASETS):
	ax.plot([m[0] for m in measurements], [m[1] for m in measurements], linewidth=2.0, label=label)
ax.set_xlabel(PARAM_LABEL or PARAM_NAME)
ax.set_ylabel("time (ms)")
ax.set_xticks([m[0] for m in measurements])
ax.grid(True)
plt.legend()
plt.savefig(f"results/{IMPL_NAME}:{PARAM_NAME}")
