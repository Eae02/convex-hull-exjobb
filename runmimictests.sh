#!/bin/bash

# Make sure to run createtests.sh first

implementation=$@

touch .mimicdata/with_outliers/times.txt
for f in .mimicdata/with_outliers/*.in; do
    # echo $f
    pre=${f%.in}
    out=$pre.out
    ./ch.bin $implementation < $f > $out 2>> .mimicdata/with_outliers/times.txt
done
echo "With outliers done"
python3 testtools/analyze_times.py 12438 < .mimicdata/with_outliers/times.txt
rm .mimicdata/with_outliers/times.txt

touch .mimicdata/without_outliers/times.txt
for f in .mimicdata/without_outliers/*.in; do
    # echo $f
    pre=${f%.in}
    out=$pre.out
    ./ch.bin $implementation < $f > $out 2>> .mimicdata/without_outliers/times.txt
done
echo "Without outliers done"
python3 testtools/analyze_times.py 12418.663 < .mimicdata/without_outliers/times.txt
rm .mimicdata/without_outliers/times.txt

echo "Done running all tests on $implementation"
