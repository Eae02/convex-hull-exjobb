import csv
import matplotlib
import matplotlib.pyplot as plt
import sys
import testlib
import os

assert len(sys.argv) > 1
csvname = sys.argv[-1]

TICKS_FONT_SIZE=12
LABELS_FONT_SIZE=20

matplotlib.rc('xtick', labelsize=TICKS_FONT_SIZE)
matplotlib.rc('ytick', labelsize=TICKS_FONT_SIZE)
matplotlib.rc('axes', labelsize=LABELS_FONT_SIZE)
matplotlib.rc('legend', fontsize=LABELS_FONT_SIZE)

with open(csvname, 'r') as file:
	csvData = list(csv.reader(file, delimiter='\t'))

if "--nxh" in sys.argv:
	xValues = list(range(len(csvData[0]) - 1))
else:
	xValues = list(map(float, csvData.pop(0)[1:]))

fig, ax = plt.subplots(figsize=(16,12))
for row in csvData:
	ax.plot(xValues, list(map(float, row[1:])), linewidth=2.0, label=row[0])
ax.set_ylabel("Time (ms)")
ax.set_xlabel(testlib.getcmdarg("xlabel", "X label missing"))
ax.set_xticks(xValues)
if "--xlog" in sys.argv:
	ax.set_xscale("log", base=2)
ax.grid(True)
plt.legend()

outname = "results/" + os.path.basename(csvname) + ".png"
plt.savefig(outname)
print(f"saved to {outname}")
