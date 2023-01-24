#!/bin/bash

rm -r .testcases
mkdir .testcases
mkdir .testcases/small
for testn in {1..100}
do
    python3 testtools/gen.py $testn 100 > .testcases/small/$testn.in
    ./ch.bin impl1 < .testcases/small/$testn.in > .testcases/small/$testn.ans
done

mkdir .testcases/medium
for testn in {101..110}
do
    python3 testtools/gen.py $testn 10000 > .testcases/medium/$testn.in
    ./ch.bin impl1 < .testcases/medium/$testn.in > .testcases/medium/$testn.ans
done

mkdir .testcases/large
for testn in {111..120}
do
    python3 testtools/gen.py $testn 1000000 > .testcases/large/$testn.in
    time ./ch.bin impl1 < .testcases/large/$testn.in > .testcases/large/$testn.ans
done