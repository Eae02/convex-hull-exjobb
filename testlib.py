import subprocess
import os

def run(inputFile, implementation, extraArgs=[]):
	command = ['./ch.bin', '-q', implementation] + extraArgs
	with open(inputFile, "r") as f:
		proc = subprocess.run(command, stdin=f, stderr=subprocess.PIPE, stdout=subprocess.PIPE, timeout = 1000000)
		output = proc.stderr.decode("utf-8")
	computeTime = None
	for line in output.split("\n"):
		if line.startswith("compute time:"):
			computeTime = float(line.split(":")[1][:-2])
	return computeTime

def runOnAllFiles(dataset, implementation, runs=1, datasetSize="large", extraArgs=[]):
	times = []
	dirpath = f".testcases/{dataset}/{datasetSize}"
	files = list(filter(lambda f: f.endswith(".in"), os.listdir(dirpath)))
	for r in range(runs):
		for file in files:
			time = run(dirpath + "/" + file, implementation, extraArgs)
			times.append(time)
	return sum(times) / len(times)
