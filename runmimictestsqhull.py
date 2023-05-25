import testlib

# Medical A

times = []
inputs = 1000
for i in range(inputs):
    # print(f'running test {i+1}')
    inputfile = f'.mimicdata/medicalA/{i+1}.in'
    time = testlib.runQhull(inputfile)
    times.append(time)
avgtime = sum(times)/inputs
n = 11839
avgperpoint = avgtime/n*1000
print ("Medical A: Average compute time:", '%.6f' % (avgtime), "ms,", "time per point:", '%.6f' % (avgperpoint), "us")


# Medical B
times = []
inputs = 1000
for i in range(inputs):
    # print(f'running test {i+1}')
    inputfile = f'.mimicdata/medicalB/{i+1}.in'
    time = testlib.runQhull(inputfile)
    times.append(time)
avgtime = sum(times)/inputs
n = 13037
avgperpoint = avgtime/n*1000
print ("Medical B: Average compute time:", '%.6f' % (avgtime), "ms,", "time per point:", '%.6f' % (avgperpoint), "us")
