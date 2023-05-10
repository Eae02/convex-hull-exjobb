#!/bin/bash
REP=3
OUT_FILE=qh_bf_measurements.csv

. ./measure_pcm_utils.sh

pcm_init

pcm_run qh_bf:N square
pcm_run qh_bf square

pcm_run qh_bf:N circle
pcm_run qh_bf circle

pcm_run qh_bf:N disk
pcm_run qh_bf disk

pcm_write_sdev
