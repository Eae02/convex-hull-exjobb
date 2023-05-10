#!/bin/bash
REP=3
OUT_FILE=qh_bf_par_measurements.csv

. ./measure_pcm_utils.sh

pcm_init

pcm_run qhp square
pcm_run qhp_seq square
pcm_run qhp_nr square

pcm_run qhp disk
pcm_run qhp_seq disk
pcm_run qhp_nr disk

pcm_write_sdev
