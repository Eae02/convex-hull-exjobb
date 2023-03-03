
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
import sys


cmap = mpl.colormaps['tab20']

def plot_results(file_name = 'results/times.csv'):

    df = pd.read_csv(file_name)

    # Plot from square dataset
    square_times = df.loc[df['test_generator']=='gensquare.bin'].drop(columns=['seed','test_generator'])
    square_times = square_times.groupby(by=['implementation_name','n']).mean().reset_index()
    square_times['time_per_point'] = square_times['compute_time(ms)']/square_times['n']*1000
    print(square_times.head())
    print(square_times.columns)
    fig, ax = plt.subplots(figsize=(16,12))
    for idx,(label, df_2) in enumerate(square_times.groupby(['implementation_name'])):
        df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = cmap(idx))
    plt.title("Running time for different planar convex hull algorithms on square dataset.")
    plt.ylabel("Computation time per input point (us)")
    ax.set_ylim(ymin=0, ymax = min(1, square_times['time_per_point'].max()))
    plt.savefig('results/square_plot')

    # Plot from circle dataset
    circ_times = df.loc[df['test_generator']=='gencirc.bin'].drop(columns=['seed','test_generator'])
    circ_times = circ_times.groupby(by=['implementation_name','n']).mean().reset_index()
    circ_times['time_per_point'] = circ_times['compute_time(ms)']/circ_times['n']*1000
    print(circ_times.head())
    print(circ_times.columns)
    fig, ax = plt.subplots(figsize=(16,12))
    for idx,(label, df_2) in enumerate(circ_times.groupby(['implementation_name'])):
        df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = cmap(idx))
    plt.title("Running time for different planar convex hull algorithms on circ dataset.")
    plt.ylabel("Computation time per input point (us)")
    ax.set_ylim(ymin=0, ymax = min(2.5, circ_times['time_per_point'].max()))
    ax.set_ylim(ymin=0)
    plt.savefig('results/circ_plot')

    # Plot from disk dataset
    disk_times = df.loc[df['test_generator']=='gendisk.bin'].drop(columns=['seed','test_generator'])
    disk_times = disk_times.groupby(by=['implementation_name','n']).mean().reset_index()
    disk_times['time_per_point'] = disk_times['compute_time(ms)']/disk_times['n']*1000
    print(disk_times.head())
    print(disk_times.columns)

    fig, ax = plt.subplots(figsize=(16,12))
    for idx,(label, df_2) in enumerate(disk_times.groupby(['implementation_name'])):
        df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = cmap(idx))
    plt.title("Running time for different planar convex hull algorithms on disk dataset.")
    ax.set_ylim(ymin=0, ymax = min(1, disk_times['time_per_point'].max()))
    ax.set_ylim(ymin=0)
    plt.ylabel("Computation time per input point (us)")
    plt.savefig('results/disk_plot')

if __name__ == "__main__":
    if len(sys.argv)>1:
        plot_results(sys.argv[1])
    else:
        plot_results()