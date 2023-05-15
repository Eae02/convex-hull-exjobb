
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
import sys

# Polynomial hash to get consistent label colors. Author: https://stackoverflow.com/a/70262376
def myHash(text:str):
  hash=0
  for ch in text:
    hash = ( hash*281  ^ ord(ch)*997) & 0xFFFFFFFF
  return hash

cmap = plt.get_cmap('tab20')
num_colors = 20
                        #(3,1)     #(5,1,1,1) 
linestyles = ['solid', 'dashed', 'dashdot', (0,(10,3)), (0,(5,1)), (0,(8,1)), (0,(10,1)), (0,(3,1,1,1)), (0,(3,1,1,1,1,1)), (0,(8,1,1,1)), (0,(5,1,3,1))]

def plot_results(file_name = 'results/times.csv', label_colors  = None):
    # Plots for thesis
    #label_colors = {'chan':('Chan','red'), 'chan_refined': ('Refined Chan','green'), 'merge_hull': ('MergeHull','orange')}
    label_colors = {'chan_refined': ('Refined Chan','limegreen'), 'mc': ('Monotone Chain','red'), 'cgal_graham': ('CGAL Graham','darkred'), 'cgal_akl_toussaint':('CGAL Akl Toussaint','purple'),'qhull':('Qhull','navy'), 'qh_rec_esx': ('Depth Quickhull Initial Sort','deeppink'),'qh_rec_nxp': ('Depth Quickhull No x partition','blue')}
    
    # Plot more chan variants
    #label_colors = {'chan':('chan','red'), 'chan_refined': ('chan_refined','orange'), 'merge_hull': ('merge_hull','blue'), 'merge_hull_chan_trick' : ('merge_hull_chan_trick', 'green')}
    
    # Plot more qh variants
    # label_colors = {'qh_rec_esx': ('depth_quickhull_sort_initially','red'), 'qh_rec_ss': ('depth_quickhull_single_scan','pink'), 'qh_bf_ss': ('breadth_quickhull_single_scan','purple'),'qh_rec_nxp': ('depth_quickhull_no_xpartition','deeppink'),'qh_bf_nxp': ('breadth_quickhull_no_xpartition','indigo')}
    df = pd.read_csv(file_name)
    #Filter out implementations not plotted
    if label_colors != None:
        df = df[df.apply(lambda row: row['implementation_name'] in label_colors, axis=1)]

    # Plot from square dataset
    square_times = df.loc[df['test_generator']=='gensquare.bin'].drop(columns=['seed','test_generator'])
    square_times = square_times.groupby(by=['implementation_name','n']).mean().reset_index()
    square_times['time_per_point'] = square_times['compute_time(ms)']/square_times['n']*1000
    print(square_times.head())
    print(square_times.columns)
    fig, ax = plt.subplots(figsize=(8,6))
    for label, df_2 in square_times.groupby(['implementation_name']):
        if label_colors == None:
            df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = cmap((myHash(label)%num_colors)/float(num_colors)), linestyle = linestyles[myHash(label)%len(linestyles)])
        elif label in label_colors:
            label,color = label_colors[label]
            df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = color)
    plt.title("Running time for different planar convex hull algorithms on square dataset on POWER")
    plt.ylabel("Computation time per input point (us)")
    plt.legend(handlelength = 10)
    ax.set_ylim(ymin=0, ymax = min(0.5, 1.05 * square_times['time_per_point'].max()))
    plt.savefig('results/square_plot')

    # Plot from circle dataset
    circ_times = df.loc[df['test_generator']=='gencirc.bin'].drop(columns=['seed','test_generator'])
    circ_times = circ_times.groupby(by=['implementation_name','n']).mean().reset_index()
    circ_times['time_per_point'] = circ_times['compute_time(ms)']/circ_times['n']*1000
    print(circ_times.head())
    print(circ_times.columns)

    doubleplot = True
    if not doubleplot:
        # Just one plot
        fig, ax = plt.subplots(figsize=(8,6))
    else:
        # Broken Axis to display outlier
        fig, (ax2, ax) = plt.subplots(2, 1, sharex=True, figsize=(8,6))
    for label, df_2 in circ_times.groupby(['implementation_name']):
        if label_colors == None:
            df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = cmap((myHash(label)%num_colors)/float(num_colors)), linestyle = linestyles[myHash(label)%len(linestyles)])
            if doubleplot:
                df_2.plot(x = 'n', y = 'time_per_point',ax=ax2, label=label, logx=True, color = cmap((myHash(label)%num_colors)/float(num_colors)), linestyle = linestyles[myHash(label)%len(linestyles)])
        elif label in label_colors:
            label,color = label_colors[label]
            df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = color)
            if doubleplot:
                df_2.plot(x = 'n', y = 'time_per_point',ax=ax2, label=label, logx=True, color = color)
    fig.suptitle("Running time for different planar convex hull algorithms on circle dataset on POWER")
    plt.ylabel("Computation time per input point (us)")
    plt.legend(handlelength = 10)
    ax.set_ylim(ymin=0, ymax = min(0.8, 1.05 * circ_times['time_per_point'].max()))
    #For double plotting
    if doubleplot:
        ax.get_legend().remove()
        ax2.set_ylim(ymin=2.2, ymax = 3)
        ax.spines['top'].set_visible(False)
        ax2.spines['bottom'].set_visible(False)
        ax2.xaxis.tick_top()
        ax2.tick_params(labeltop=False)  # don't put tick labels at the top
        ax.xaxis.tick_bottom()
        d = .015  # how big to make the diagonal lines in axes coordinates
        # arguments to pass to plot, just so we don't keep repeating them
        kwargs = dict(transform=ax2.transAxes, color='k', clip_on=False)
        ax2.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
        ax2.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

        kwargs.update(transform=ax.transAxes)  # switch to the bottom axes
        ax.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
        ax.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal

    plt.savefig('results/circ_plot')

    # Plot from disk dataset
    disk_times = df.loc[df['test_generator']=='gendisk.bin'].drop(columns=['seed','test_generator'])
    disk_times = disk_times.groupby(by=['implementation_name','n']).mean().reset_index()
    disk_times['time_per_point'] = disk_times['compute_time(ms)']/disk_times['n']*1000
    print(disk_times.head())
    print(disk_times.columns)

    fig, ax = plt.subplots(figsize=(8,6))
    for label, df_2 in disk_times.groupby(['implementation_name']):
        if label_colors == None:
            df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = cmap((myHash(label)%num_colors)/float(num_colors)), linestyle = linestyles[myHash(label)%len(linestyles)])
        elif label in label_colors:
            label,color = label_colors[label]
            df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True, color = color)
    plt.title("Running time for different planar convex hull algorithms on disk dataset on POWER")
    plt.ylabel("Computation time per input point (us)")
    plt.legend(handlelength = 10)
    ax.set_ylim(ymin=0, ymax = min(0.75, 1.05 * disk_times['time_per_point'].max()))
    plt.savefig('results/disk_plot')

if __name__ == "__main__":
    if len(sys.argv)>1:
        plot_results(sys.argv[1])
    else:
        plot_results()