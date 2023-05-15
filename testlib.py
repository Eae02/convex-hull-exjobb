import subprocess
import os

def runQhull(inputFile, timeout = 10):
	command = ['qhull']
	with open(inputFile, "r") as f:
		proc = subprocess.run(command, stdin=f, stderr=subprocess.PIPE, stdout=subprocess.PIPE, timeout = timeout)
		output = proc.stdout.decode("utf-8")
	computeTime = None
	for line in output.split("\n"):
		if "CPU seconds" in line:
			computeTime = float(line.split(":")[1])*1000 #convert to ms
	return computeTime

def run(inputFile, implementation, extraArgs=[], timeout = 10):
	if implementation == "qhull":
		return runQhull(inputFile, timeout)
	command = ['./ch.bin', '-q', implementation] + extraArgs
	with open(inputFile, "r") as f:
		proc = subprocess.run(command, stdin=f, stderr=subprocess.PIPE, stdout=subprocess.PIPE, timeout = timeout)
		output = proc.stderr.decode("utf-8")
	computeTime = None
	for line in output.split("\n"):
		if line.startswith("compute time:"):
			computeTime = float(line.split(":")[1][:-2])
	return computeTime

def runOnAllFiles(datasets, implementation, runs=1, datasetSize="large", extraArgs=[]):
	if type(datasets) != type([]):
		datasets = [datasets]
	times = []
	for dataset in datasets:
		dirpath = f".testcases/{dataset}/{datasetSize}"
		files = list(filter(lambda f: f.endswith(".in"), os.listdir(dirpath)))
		for r in range(runs):
			for file in files:
				time = run(dirpath + "/" + file, implementation, extraArgs)
				times.append(time)
	return sum(times) / len(times)
