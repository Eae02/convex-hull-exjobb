import csv
import matplotlib
import matplotlib.pyplot as plt
import sys
import testlib
import os
import math

assert len(sys.argv) > 1
csvname = sys.argv[-1]

xSpacing = int(testlib.getcmdarg("xsp", -1))
ySpacing = int(testlib.getcmdarg("ysp", -1))

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

maxY = 0

fig, ax = plt.subplots(figsize=(16,12))
for row in csvData:
	yValues = list(map(float, row[1:]))
	maxY = max(maxY, max(yValues))
	label = row[0]
	lineStyle, lineColor = None, None
	if ":" in label:
		labelParts = label.split(":")
		label = labelParts[0]
		lineColor = labelParts[1]
		if len(labelParts) > 2:
			lineStyle = labelParts[2]
	ax.plot(xValues, yValues, linewidth=2.0, label=label, color=lineColor, linestyle=lineStyle)

ax.set_ylabel("Time (ms)")
ax.set_xlabel(testlib.getcmdarg("xlabel", "X label missing"))

if xSpacing != -1:
	ax.set_xticks([xValues[0]] + [x for x in xValues[1:-1] if (x % xSpacing) == 0] + [xValues[-1]])
if "--xlog" in sys.argv:
	ax.set_xscale("log", base=2)

if ySpacing != -1:
	lastTickY = (math.ceil(maxY / ySpacing) + 1) * ySpacing
	ax.set_yticks(range(0, lastTickY, ySpacing))

legend1 = plt.legend()

linesFile = testlib.getcmdarg("lines", "")
if linesFile != "":
	linesLegendTitle = "Lines"
	lines, lineLabels = [], []
	with open(linesFile, "r") as fp:
		for ln in fp.readlines():
			if ln.startswith("title="):
				linesLegendTitle = ln.split("=")[1]
				continue
			label, lineY, color = ln.split(":")
			lineLabels.append(label.strip())
			lines.append(plt.axhline(float(lineY), linestyle="--", dashes=(10, 2), color=color.strip(), linewidth=2))
	linesLegend = plt.legend(lines, lineLabels, title_fontsize=LABELS_FONT_SIZE, loc="center right", title=linesLegendTitle)

ax.add_artist(legend1)

ax.grid(True)

outname = "results/" + os.path.basename(csvname) + ".png"
plt.savefig(outname)
print(f"saved to {outname}")
