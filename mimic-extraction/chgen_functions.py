# These functions are taken from https://git.rwth-aachen.de/jrc-combine/chgen
# And then modified to fit our needs (output datasets of planar points to compute convex hull on)
import numpy as np
import pandas as pd

from sklearn.cluster import DBSCAN
from sklearn.preprocessing import StandardScaler
from sklearn.utils import resample

def run_dbscan(points, eps = 0.5, min_samples = 4):
    '''
    Detect outliers using DBSCAN

    Args:
        points: numpy array of all points of one dataframe
        xy: a list of strings, a combination of parameters
        df_name: a string, a dataframe name
        eps: float, start value for epsilon hyperparameter of dbscan algorithm
        min_samples: int, start value for min_samples hyperparameter of dbscan algorithm
        not_noise: float, minimal number of points that are considered to be with correct values
    Return:
        bools: a boolean list, True - a point belongs to a cluster, False - an outlier
        end_eps: float, end value for eps-parameter
        end_min_samples: int, end value for min_samples-parameter
    '''
    scaled = StandardScaler().fit_transform(points)

    bools=[] # container for points that form a cluster

    # Some simplifications have been made here compared to chgen in order to speed up generation and avoid infinite loops. 
    db = DBSCAN(eps=eps, min_samples=min_samples).fit(scaled)

    #case: set(db.labels_)==2
    bools = [True if x>(-1) else False for x in db.labels_]

    return bools


def find_loss(combo, df, eps = 0.5, min_samples = 4):
    '''
    Wrapper function to detect outliers using DBSCAN

    Args:
        combo: a list, pair of parameters of interest
        df: dataframe with values
        eps: float, a DBSCAN parameter to define a neighborhood of a point
        min_samples: int, minimal number of points in the neighborhood of a point
        not_noise: float, minimal (relative) amount of instances of a dataframe to be considered as a cluster
    Return:
        bools: a boolean list, True - a point belongs to a cluster, False - an outlier
        combo: string, pair of parameters of interest
        pat_loss: int, number of points considered as an outlier
        eps: float, end value for eps-parameter
        smpl: int, end value for min_samples-parameter
    '''
    points = df.loc[:,combo]
    points = np.array(points)

    bools = run_dbscan(points = points, eps = eps, min_samples = min_samples)

    return bools


def full_intersections(df_1, df_2, resample_iterations = 10):
    '''
    Calculates intersections of two lists of points of the same dimension

    Args:
        df_1, df_2: numpy array of points of a clinic which contains only parameters of interest
        df_names: a list of strings, hospitals' names (e.g. ['df_1','df_2']), in the same order as df_1 and df_1 arguments
        pair: a list, a combination of parameters
    Returns:
        df: a pandas dataframe, a table with intersection values for a combination of parameters and hospital cases
    '''
    datasets = []
    for i in np.arange(resample_iterations):
        # create a random sample set of a certain size for both clinics
        boot_1 = resample(df_1, replace=True, n_samples=len(df_1), random_state=i+i)
        boot_2 = resample(df_2, replace=True, n_samples=len(df_2), random_state=i+i)

        #Normally would check #points in boot_1 that are in conv(boot_2) and vice versa.
        datasets.append(boot_1.tolist())
        datasets.append(boot_2.tolist())


    return datasets

def extrant_convex_hull_calls(combo, df_1, df_2, bools_1 = [], bools_2 = [], resample_iterations = 10):
    '''
    Copy of chgen.compute_intersections, but we extract what sets of 2D points it would compute convex hull of.
    '''

    points_1 = df_1.loc[:,combo]
    points_2 = df_2.loc[:,combo]
    points_1 = np.array(points_1)
    points_2 = np.array(points_2)

    if (len(bools_1)!=0) and (len(bools_2)!=0):
        #keep only clustered point (w/o outliers) for current combo
        points_1=points_1[bools_1]
        points_2=points_2[bools_2]

    return full_intersections(df_1 = points_1, df_2 = points_2, resample_iterations = resample_iterations)
