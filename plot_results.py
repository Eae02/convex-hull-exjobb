
import pandas as pd
import matplotlib.pyplot as plt

def plot_results():
    df = pd.read_csv('results.csv')

    # Plot from square dataset
    square_times = df.loc[df['test_generator']=='gensquare.bin'].drop(columns=['seed','test_generator'])
    square_times = square_times.groupby(by=['implementation_name','n']).mean().reset_index()
    square_times['time_per_point'] = square_times['compute_time(ms)']/square_times['n']*1000
    print(square_times.head())
    print(square_times.columns)

    fig, ax = plt.subplots(figsize=(8,6),)
    for label, df_2 in square_times.groupby(['implementation_name']):
        df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True)
    plt.title("Running time for different planar convex hull algorithms on square dataset.")
    plt.ylabel("Computation time per input point (us)")
    plt.savefig('square_results')

    # Plot from circle dataset
    circ_times = df.loc[df['test_generator']=='gencirc.bin'].drop(columns=['seed','test_generator'])
    circ_times = circ_times.groupby(by=['implementation_name','n']).mean().reset_index()
    circ_times['time_per_point'] = circ_times['compute_time(ms)']/circ_times['n']*1000
    print(circ_times.head())
    print(circ_times.columns)

    fig, ax = plt.subplots(figsize=(8,6),)
    for label, df_2 in circ_times.groupby(['implementation_name']):
        df_2.plot(x = 'n', y = 'time_per_point',ax=ax, label=label, logx=True)
    plt.title("Running time for different planar convex hull algorithms on circ dataset.")
    plt.ylabel("Computation time per input point (us)")
    plt.savefig('circ_results')

if __name__ == "__main__":
    plot_results()