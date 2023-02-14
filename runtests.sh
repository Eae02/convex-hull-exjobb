#!/bin/bash

# Make sure to run createtests.sh first

implementation=$@




echo "Run unit square tests"

rm .testcases/square/small/times.txt
touch .testcases/square/small/times.txt

for f in .testcases/square/small/*.in; do
    echo $f
    pre=${f%.in}
    out=$pre.out
    ans=$pre.ans
    ./ch.bin $implementation < $f > $out 2>> .testcases/square/small/times.txt
    diff $out $ans
done
echo "Small done"
python3 testtools/analyze_times.py 100 < .testcases/square/small/times.txt


rm .testcases/square/medium/times.txt
touch .testcases/square/medium/times.txt

for f in .testcases/square/medium/*.in; do
    echo $f
    pre=${f%.in}
    out=$pre.out
    ans=$pre.ans
    ./ch.bin $implementation < $f > $out 2>> .testcases/square/medium/times.txt
    diff $out $ans
done
echo "Medium done"
python3 testtools/analyze_times.py 10000 < .testcases/square/medium/times.txt

rm .testcases/square/large/times.txt
touch .testcases/square/large/times.txt

for f in .testcases/square/large/*.in; do
    echo $f
    pre=${f%.in}
    out=$pre.out
    ans=$pre.ans
    ./ch.bin $implementation < $f > $out 2>> .testcases/square/large/times.txt
    diff $out $ans
done
echo "Large done"
python3 testtools/analyze_times.py 1000000 < .testcases/square/large/times.txt


echo "Run unit circle tests"

rm .testcases/circle/small/times.txt
touch .testcases/circle/small/times.txt
for f in .testcases/circle/small/*.in; do
    echo $f
    pre=${f%.in}
    out=$pre.out
    ans=$pre.ans
    ./ch.bin $implementation < $f > $out 2>> .testcases/circle/small/times.txt
    diff $out $ans
done
echo "Small done"
python3 testtools/analyze_times.py 100 < .testcases/circle/small/times.txt


rm .testcases/circle/medium/times.txt
touch .testcases/circle/medium/times.txt
for f in .testcases/circle/medium/*.in; do
    echo $f
    pre=${f%.in}
    out=$pre.out
    ans=$pre.ans
    ./ch.bin $implementation < $f > $out 2>> .testcases/circle/medium/times.txt
    diff $out $ans
done
echo "Medium done"
python3 testtools/analyze_times.py 10000 < .testcases/circle/medium/times.txt


rm .testcases/circle/large/times.txt
touch .testcases/circle/large/times.txt
for f in .testcases/circle/large/*.in; do
    echo $f
    pre=${f%.in}
    out=$pre.out
    ans=$pre.ans
    ./ch.bin $implementation < $f > $out 2>> .testcases/circle/large/times.txt
    diff $out $ans
done
echo "Large done"
python3 testtools/analyze_times.py 1000000 < .testcases/circle/large/times.txt

echo "Done running all tests on $implementation"
