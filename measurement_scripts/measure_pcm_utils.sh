#!/bin/bash

function pcm_init {
	echo "" > .tmp_sdev.csv
	printf "implementation\tdata\tcompute time\tinstr retired\tinstr per clock\tinstr per clock (core)\tmemory bytes read\tmemory bytes written\tL2 hits\tL2 misses\tL2 hit ratio\tL3 hits\tL3 misses\tL3 hit ratio\tpl bad speculation\tpl backend bound\tpl frontend bound\n" > $OUT_FILE
}

function pcm_run {
	sudo python3 run_pcm.py $1 data=$2 rep=$REP --transpose-output > .tmp.csv
	printf "$1\t$2\t" >> $OUT_FILE
	sed '2q;d' .tmp.csv >> $OUT_FILE
	printf "$1 (sdev)\t$2\t" >> .tmp_sdev.csv
	sed '3q;d' .tmp.csv >> .tmp_sdev.csv
	rm .tmp.csv
}

function pcm_write_sdev {
	cat .tmp_sdev.csv >> $OUT_FILE
}
