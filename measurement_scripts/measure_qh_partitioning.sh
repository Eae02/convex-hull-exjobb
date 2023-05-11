#!/bin/bash
REP=5
OUT_FILE=partitioning_measurements.csv

. ./measure_pcm_utils.sh

pcm_init

pcm_run qh_rec_nxp square
pcm_run qh_rec_xp square
pcm_run qh_rec_ss square
pcm_run qh_rec_esx square
pcm_run qh_hybrid_esx:D3 square

pcm_run qh_rec_nxp disk
pcm_run qh_rec_xp disk
pcm_run qh_rec_ss disk
pcm_run qh_rec_esx disk
pcm_run qh_hybrid_esx:D3 disk

pcm_run qh_rec_nxp circle
pcm_run qh_rec_xp circle
pcm_run qh_rec_ss circle
pcm_run qh_rec_esx circle
pcm_run qh_hybrid_esx:D3 circle

pcm_write_sdev
