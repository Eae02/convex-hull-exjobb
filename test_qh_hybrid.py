DATASET = "circle"
RUNS = 3

import testlib
import matplotlib.pyplot as plt
import matplotlib as mpl

measurements = []
for s in range(1, 30, 2):
	t = testlib.runOnAllFiles(DATASET, f"qh_hybrid_jw:P{s}", runs=RUNS)
	print(f"size={s}: {t:.2f}ms")
	measurements.append((s, t))

fig, ax = plt.subplots(figsize=(16,12))
ax.plot([m[0] for m in measurements], [m[1] for m in measurements], linewidth=2.0)
plt.show()
