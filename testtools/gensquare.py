#Usage: python3 gensquare.py seed n

import sys

import random

n = len(sys.argv)
if n < 3:
    print("No seed and numpts specified, exiting...")
    exit(0)
if n>3:
    print("Too many arguments, exiting...")
    exit(0)
seed = sys.argv[1]
n = int(sys.argv[2])

rand = random.Random(seed)
print(2)
print(n)
for i in range(n):
    print(rand.random(),rand.random())

