#Usage: python3 gencirc.py seed n

#Generates points in a circle

import sys

import random
import math

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
    angle = rand.random()*2*math.pi

    print(math.cos(angle),math.sin(angle))

