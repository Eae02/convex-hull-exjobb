DATASETS = ["square"]
RUNS = 5

#IMPL_NAME = "qh_hybrid_mc"
#PARAM_NAME = "D"
#PARAM_RANGE = range(0, 20, 1)

IMPL_NAME = "qh_bf"
PARAM_NAME = "D"
PARAM_LABEL = "depth to start compacting at"
PARAM_RANGE = range(0, 20)

#IMPL_NAME = "qh_bf"
#PARAM_NAME = "P"
#PARAM_LABEL = "#points to perform compacting at"
#PARAM_RANGE = range(0, 300, 10)

import testlib
import matplotlib.pyplot as plt

measurements = []
for s in list(PARAM_RANGE):
	runName = f"{IMPL_NAME}:{PARAM_NAME}{s}"
	print(f"running {runName}...", end="", flush=True)
	t = testlib.runOnAllFiles(DATASETS, runName, runs=RUNS)
	print(f" done! ({t:.2f}ms)")
	measurements.append((s, t))

fig, ax = plt.subplots(figsize=(16,12))
ax.plot([m[0] for m in measurements], [m[1] for m in measurements], linewidth=2.0)
ax.set_xlabel(PARAM_LABEL or PARAM_NAME)
ax.set_ylabel("time (ms)")
ax.set_xticks([m[0] for m in measurements])
ax.grid(True)
plt.savefig(f"results/{IMPL_NAME}:{PARAM_NAME}")
