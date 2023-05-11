#!/bin/bash

# Make sure to put tests in .mimicdata/medical{A/B} first
# Use scripts and instructions in mimic-extraction folder.

implementation=$@

touch .mimicdata/timesA.txt
for f in .mimicdata/medicalA/*.in; do
    # echo $f
    pre=${f%.in}
    out=$pre.out
    ./ch.bin $implementation < $f > $out 2>> .mimicdata/timesA.txt
done
echo "MedicalA done"
python3 testtools/analyze_times.py 11839 < .mimicdata/timesA.txt
rm .mimicdata/timesA.txt

touch .mimicdata/timesB.txt
for f in .mimicdata/medicalB/*.in; do
    # echo $f
    pre=${f%.in}
    out=$pre.out
    ./ch.bin $implementation < $f > $out 2>> .mimicdata/timesB.txt
done
echo "MedicalB done"
python3 testtools/analyze_times.py 13037 < .mimicdata/timesB.txt
rm .mimicdata/timesB.txt

echo "Done running all tests on $implementation"
