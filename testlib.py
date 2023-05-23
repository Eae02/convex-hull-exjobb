import subprocess
import os, sys

def getcmdarg(name, default=None):
	for i in range(1, len(sys.argv)):
		if sys.argv[i].startswith(f"-{name}="):
			return sys.argv[i].split("=", maxsplit=1)[1]
	if default is None:
		print(f"missing command line argument -{name}")
		exit(1)
	return default

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

def findResult(output: str, label: str):
	labelPos = output.index(label)
	value = output[labelPos + len(label):].strip().split(maxsplit=1)[0]
	if value.endswith("%"):
		value = value[:-1]
	return float(value.replace(",", ""))

def run(inputFile, implementation, extraArgs=[], timeout = 10, maxThreads=None, metric="time"):
	if implementation == "qhull":
		return runQhull(inputFile, timeout)
	command = ['./ch.bin', '-q', implementation] + extraArgs
	if metric in ["cacheMisses", "cacheMissRate"]:
		command = ["valgrind", "--tool=cachegrind", "--cachegrind-out-file=/dev/null"] + command
	if maxThreads is not None:
		command = ["taskset", "--cpu-list", "0-" + str(maxThreads - 1)] + command
	with open(inputFile, "r") as f:
		proc = subprocess.run(command, stdin=f, stderr=subprocess.PIPE, stdout=subprocess.PIPE, timeout = timeout)
		output = proc.stderr.decode("utf-8")
	return findResult(output, {
		"cacheMisses": "LL misses:",
		"cacheMissRate": "LL miss rate:",
		"time": "compute time:"
	}[metric])

def runOnAllFiles(datasets, implementation, runs=1, datasetSize="large", extraArgs=[], maxThreads=None, metric="time"):
	if type(datasets) != type([]):
		datasets = [datasets]
	times = []
	for dataset in datasets:
		dirpath = f".testcases/{dataset}/{datasetSize}"
		files = list(filter(lambda f: f.endswith(".in"), os.listdir(dirpath)))
		for r in range(runs):
			for file in files:
				time = run(dirpath + "/" + file, implementation, extraArgs, maxThreads=maxThreads, metric=metric)
				times.append(time)
	return sum(times) / len(times)
