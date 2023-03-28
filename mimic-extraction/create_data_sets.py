# Code is taken from https://git.rwth-aachen.de/jrc-combine/chgen
# And then modified to fit our needs (output datasets of planar points to compute convex hull on)

### An example of intersection values computation for the case when datasets are taken at their full size

import numpy as np
import pandas as pd
import random
from itertools import combinations, repeat
import multiprocessing
import logging
logging.basicConfig(filename='output/log_ch.txt',level=logging.INFO)
import os

#from chgen.wrapper import find_loss, compute_intersections
from chgen_functions import find_loss, extrant_convex_hull_calls

OUTPUT_LOCATION = 'output'
MAX_NO_DATASETS = 1000

# define a dimension (number of features in a combination) and names to the dataframes
# that are used for calculations. The same names must be used for intersection value
# analysis in further_analysis.py script
dim = 2
df_names = ['first_df', 'second_df']
logging.info(f'CURRENT PAIR: {df_names[0]} and {df_names[1]}')
logging.info(f'CURRENT DIMENSION: {dim}')

df_1 = pd.read_csv('output/intermediate-mimiciii.csv')
df_2 = pd.read_csv('output/intermediate-mimiciv.csv')
logging.info(df_1.head())
logging.info(df_2.head())
# remove categorical data
df_1.drop(columns = ['icustay_id'], inplace = True)
df_2.drop(columns = ['icustay_id'], inplace = True)

# find common parameters for both dataframes
common_params = sorted(list(set(df_1.columns).intersection(df_2.columns)))
logging.info(f'Number of common parameters: {len(common_params)} for {df_names[0]} and {df_names[1]}')
logging.info(f'Common parameters for {df_names[0]} and {df_names[1]}: {common_params}')

df_1 = df_1[common_params]
df_2 = df_2[common_params]

# create a list with all combinations of parameters for a defined dimension
combos = list(combinations(common_params, dim))
logging.info(f'Number of combos: {len(combos)} for {df_names[0]} and {df_names[1]}')


# computation of intersection values with all points (outliers included) without sampling of instances
datasets_with_outliers = []
for combo in combos:
    logging.info(f'Creating dataset for features {combo}')
    newdatasets = extrant_convex_hull_calls(combo = combo, df_1 = df_1, df_2 = df_2, bools_1 = [], bools_2 = [])
    datasets_with_outliers.extend(newdatasets)
logging.info(f'BEFORE DBSCAN, {len(datasets_with_outliers)} data sets created')

filename = f'{OUTPUT_LOCATION}/with_outliers/1.in'
os.makedirs(os.path.dirname(filename), exist_ok=True)
for i, dataset in enumerate(datasets_with_outliers):
    if i >= MAX_NO_DATASETS:
        break
    n = len(dataset)
    with open(f'{OUTPUT_LOCATION}/with_outliers/{i+1}.in', 'w') as f:
        f.write('2\n')
        f.write(f'{n}\n')
        for x,y in dataset:
            f.write(f'{x} {y}\n')
    

datasets_without_outliers = []
# find outliers
eps = 0.5 #neighborhood distance of a point
min_samples = 4 #minimal number of neighbors of a point
not_noise = 0.9 #amount of points that form a cluster, i.e. it is allowed to have 10% of ouliers at most

# computation of intersection values after outlier removal without sampling of instances
for combo in combos:
    logging.info(f'Creating outlier free dataset for features {combo}')
    bools_1 = find_loss(combo, df_1, df_names[0], eps, min_samples)
    bools_2 = find_loss(combo, df_2, df_names[1], eps, min_samples)
    newdatasets = extrant_convex_hull_calls(combo = combo, df_1 = df_1, df_2 = df_2, bools_1 = bools_1, bools_2 = bools_2)
    datasets_without_outliers.extend(newdatasets)
logging.info(f'AFTER DBSCAN, {len(datasets_without_outliers)} data sets created')

filename = f'{OUTPUT_LOCATION}/without_outliers/1.in'
os.makedirs(os.path.dirname(filename), exist_ok=True)
for i, dataset in enumerate(datasets_without_outliers):
    if i >= MAX_NO_DATASETS:
        break
    n = len(dataset)
    with open(f'{OUTPUT_LOCATION}/without_outliers/{i+1}.in','w') as f:
        f.write('2\n')
        f.write(f'{n}\n')
        for x,y in dataset:
            f.write(f'{x} {y}\n')