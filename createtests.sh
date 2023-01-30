#!/bin/bash

rm -r .testcases
mkdir .testcases

# Unit square tests
echo "Creating unit square tests"
mkdir .testcases/square
mkdir .testcases/square/small
for testn in {1..10}
do
    python3 testtools/gensquare.py $testn 100 > .testcases/square/small/$testn.in
    ./ch.bin impl1 < .testcases/square/small/$testn.in > .testcases/square/small/$testn.ans
done
echo "Small done"
mkdir .testcases/square/medium
for testn in {11..20}
do
    python3 testtools/gensquare.py $testn 10000 > .testcases/square/medium/$testn.in
    ./ch.bin impl1 < .testcases/square/medium/$testn.in > .testcases/square/medium/$testn.ans
done
echo "Medium done"
mkdir .testcases/square/large
for testn in {21..30}
do
    python3 testtools/gensquare.py $testn 1000000 > .testcases/square/large/$testn.in
    time ./ch.bin impl1 < .testcases/square/large/$testn.in > .testcases/square/large/$testn.ans
done
echo "Large done"


# unit circle tests
echo "Creating unit circle tests"
mkdir .testcases/circle
mkdir .testcases/circle/small
for testn in {1..10}
do
    python3 testtools/gencirc.py $testn 100 > .testcases/circle/small/$testn.in
    ./ch.bin impl1 < .testcases/circle/small/$testn.in > .testcases/circle/small/$testn.ans
done
echo "Small done"
mkdir .testcases/circle/medium
for testn in {11..20}
do
    python3 testtools/gencirc.py $testn 10000 > .testcases/circle/medium/$testn.in
    ./ch.bin impl1 < .testcases/circle/medium/$testn.in > .testcases/circle/medium/$testn.ans
done
echo "Medium done"
mkdir .testcases/circle/large
for testn in {21..30}
do
    python3 testtools/gencirc.py $testn 1000000 > .testcases/circle/large/$testn.in
    time ./ch.bin impl1 < .testcases/circle/large/$testn.in > .testcases/circle/large/$testn.ans
done
echo "Large done"