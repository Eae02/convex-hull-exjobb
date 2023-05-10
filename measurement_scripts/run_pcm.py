#!/bin/python
import subprocess
import sys
import os
import statistics

if os.getuid() != 0:
	print("need to run as root")
	exit(1)

implName = None
measurements = [[]]
measurementLabels = ["compute time"]

dataset = "square"
rep = 1
for arg in sys.argv:
	parts = arg.split("=")
	if len(parts) == 1 and not arg.startswith("--"):
		implName = parts[0]
	else:
		if parts[0] == "data":
			dataset = parts[1]
		if parts[0] == "rep":
			rep = int(parts[1])

rootDir = os.path.realpath(os.path.dirname(__file__) + "/..")

files = list(filter(lambda f: f.endswith(".in"), os.listdir(f"{rootDir}/.testcases/{dataset}/large")))
files.sort()
files = files * rep

for i, file in enumerate(files):
	fullPath = f"{rootDir}/.testcases/{dataset}/large/{file}"
	sys.stderr.write(f"\033[1K\rrunning {implName} on {dataset}/large/{file} ({i+1}/{len(files)})...")
	sys.stderr.flush()
	with open(fullPath, 'r') as test_case_file:
		command = [rootDir + '/ch.bin', '-q', "-pcm", implName]
		proc = subprocess.run(command, stdin=test_case_file, stderr=subprocess.PIPE, stdout=subprocess.PIPE, timeout = 1000000)
		output = proc.stderr.decode("utf-8")
		lines = output.split("\n")
		for line in lines:
			if line.startswith("compute time:"):
				measurements[0].append(float(line.split(":")[1][:-2]))
		measurementsIndex = lines.index("pcm measurements:")
		for line in lines[measurementsIndex+1:]:
			if ":" in line:
				label, value = line.split(":")
				label = label.strip()
				if label not in measurementLabels:
					measurementLabels.append(label)
					measurements.append([])
				mi = measurementLabels.index(label)
				measurements[mi].append(float(value.replace(",", "")))

sys.stderr.write("\n")

results = []
for label, values in zip(measurementLabels, measurements):
	avg = statistics.mean(values)
	dev = statistics.stdev(values)
	results.append((label, f"{avg:.5f}", f"{dev:.5f}"))

if "--transpose-output" in sys.argv:
	for i in range(3):
		print("\t".join(map(lambda r: r[i], results)))
else:
	for label, avg, dev in results:
		print(f"{label:<20}\t{avg:<20}\t{dev}")
