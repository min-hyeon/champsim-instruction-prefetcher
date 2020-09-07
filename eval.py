import simviz.load
import simviz.fmt
import os
import collections
import functools
import itertools
import json
import pandas as pd
import matplotlib.pyplot as plt
import scipy.stats
import numpy as np


def main():

    data = simviz.load.dict_from_dir(
        path='./eval/stats/formatted/',
        fileload=functools.partial(simviz.load.dict_from_stats,
            fileparse=functools.partial(simviz.load.parse_json,
                pred='',
                sep='.',
                stop=['roi-stats.{n_cpu}.L1I', 'roi-stats.{n_cpu}.ipc-cumulative'],
                drop=['warmup-instr',
                      'sim-instr',
                      'num-cpu',
                      'llc-set',
                      'llc-way',
                      'dram',
                      'trace',
                      'finished',
                      'roi-stats.{n_cpu}.instr',
                      'roi-stats.{n_cpu}.cycle',
                      'roi-stats.{n_cpu}.L1D',
                      'roi-stats.{n_cpu}.L2C',
                      'roi-stats.{n_cpu}.LLC',
                      'dram-stats',
                      'branch-prediction',
                      'branch-type'])))


    """
    Construct data frame
    """

    """
    Return python dict from given dict, where nested internal dicts
    are flattened and keys that compose each path are compressed into
    tuple.

    Examples
    --------
    >>> d = {'1': 1,
             '2': {'2.1': 2,
                   '2.2': {'2.2.1': 5,
                           '2.2.2' : 10}},
             '3': 7}
    >>> path_compress_to_tuple(d)
    {('1'): 1,
     ('2', '2.1'): 2,
     ('2', '2.2', '2.2.1'): 5,
     ('2', '2.2', '2.2.2'): 10,
     ('3'): 7}
    """
    data = path_compress_to_tuple(data)

    """
    Pads each tuple-keys in the given dict to the longest length.

    Examples
    --------
    >>> d = {('1'): 1,
             ('2', '2.1'): 2,
             ('2', '2.2', '2.2.1'): 5,
             ('2', '2.2', '2.2.2'): 10,
             ('3'): 7}
    >>> pad_tuple_paths(d, pad='#')
    {('1', '#', '#'): 1,
     ('2', '2.1', '#'): 2,
     ('2', '2.2', '2.2.1'): 5,
     ('2', '2.2', '2.2.2'): 10,
     ('3', '#', '#'): 7}
    """
    data = pad_tuple_paths(data, pad='-')
    
    """
    Construct pandas Series from python dict, where keys are tuples for
    creating pandas MultiIndex.

    See pandas.MultiIndex.from_tuples.
    """
    s = pd.Series(data)
    
    """
    Consturct pandas Dataframe by converting last three levels in
    MultiIndex into columns.
    """
    df = s.unstack(level=[3, 4, 5])

    indices = []
    for t in df.index:
        indices.append(tuple([
            simviz.fmt.bin2pref(t[0]),
            t[1],
            simviz.fmt.stats2trace(t[2])
        ]))
    df.index = pd.MultiIndex.from_tuples(indices)
    df.index.names = ['prefetch', 'trace.type', 'trace']


    """
    IPC.
    """
    df1 = df['roi-stats.cpu0.ipc-cumulative']['-']['-'].droplevel('trace.type').astype(float)
    df1 = df1.reset_index().pivot(index='trace', columns='prefetch', values='-')

    """
    Speedup.
    """
    df2 = df1.copy()
    for col in df2:
        if col != 'baseline':
            df2[col] = df2[col] / df2['baseline']
    df2 = df2.drop('baseline', axis=1)

    """
    L1I miss rate.
    """
    df3 = df['roi-stats.cpu0.L1I']['load'].droplevel('trace.type').astype(float)
    df3['miss-rate'] = df3['miss'] / df3['access']
    df3 = df3.reset_index().pivot(index='trace', columns='prefetch', values='miss-rate')

    """
    Prefetch miss rate.
    """
    df4 = df['roi-stats.cpu0.L1I']['prefetch'].droplevel('trace.type').astype(float)
    df4['prefetch-miss-rate'] = df4['miss'] / df4['access']
    df4 = df4.reset_index().pivot(index='trace', columns='prefetch', values='prefetch-miss-rate')


    df1_avg = pd.Series(scipy.stats.hmean(df1, axis=0), index=df1.columns)
    df2_avg = pd.Series(scipy.stats.gmean(df2, axis=0), index=df2.columns)
    df3_avg = np.mean(df3, axis=0)
    df4_avg = np.mean(df4, axis=0)


    print('\033[1;35m' + 'IPC (detailed)' + '\x1b[0m')
    print(df1)
    print('\033[1;35m' + 'Speedup (detailed)' + '\x1b[0m')
    print(df2)
    print('\033[1;35m' + 'Miss rate (detailed)' + '\x1b[0m')
    print(df3)
    print('\033[1;35m' + 'Prefetch miss rate (detailed)' + '\x1b[0m')
    print(df4)

    print('')

    print('\033[1;35m' + 'IPC (average)' + '\x1b[0m')
    print(df1_avg)
    print('\033[1;35m' + 'Speedup (average)' + '\x1b[0m')
    print(df2_avg)
    print('\033[1;35m' + 'Miss rate (average)' + '\x1b[0m')
    print(df3_avg)
    print('\033[1;35m' + 'Prefetch miss rate (average)' + '\x1b[0m')
    print(df4_avg)

    df.to_excel('./eval/summary/stats-all.xlsx')

    if not os.path.exists('./eval/summary/csv/'):
        os.mkdir('./eval/summary/csv/')

    df1.to_csv('./eval/summary/csv/ipc.csv')
    df2.to_csv('./eval/summary/csv/speedup.csv')
    df3.to_csv('./eval/summary/csv/miss-rate.csv')
    df4.to_csv('./eval/summary/csv/prefetch-miss-rate.csv')

    df1_avg.to_csv('./eval/summary/csv/ipc.avg.csv')
    df2_avg.to_csv('./eval/summary/csv/speedup.avg.csv')
    df3_avg.to_csv('./eval/summary/csv/miss-rate.avg.csv')
    df4_avg.to_csv('./eval/summary/csv/prefetch-miss-rate.avg.csv')

    plot_detailed(
        {'IPC': df1,
         'Speedup': df2,
         'L1I miss rate': df3,
         'Prefetch miss rate': df4})
    plot_avg(
        {'IPC': df1_avg,
         'Speedup': df2_avg,
         'L1I miss rate': df3_avg,
         'Prefetch miss rate': df4_avg})


def plot_avg(d):

    names = list(d.keys())
    frames = list(d.values())

    fig, axes = plt.subplots(nrows=1, ncols=len(frames), figsize=(2.5*len(frames), 3))

    if len(frames) == 1:
        axes = [axes]

    for i, df in enumerate(frames):
        df.plot(kind='bar', ax=axes[i], width=0.7, fontsize=9)
        axes[i].set_ylabel(names[i], fontsize=9)
        axes[i].xaxis.label.set_visible(False)

    plt.tight_layout()
    plt.savefig('./eval/summary/plot.avg.pdf', format='pdf')


def plot_detailed(d):

    names = list(d.keys())
    frames = list(d.values())

    fig, axes = plt.subplots(nrows=len(frames), ncols=1, figsize=(15, 3*len(frames)))

    if len(frames) == 1:
        axes = [axes]

    for i, df in enumerate(frames):
        df.plot(kind='bar', ax=axes[i], width=0.7, fontsize=9)
        axes[i].set_ylabel(names[i], fontsize=9)
        axes[i].legend(fontsize=9, loc='center left', bbox_to_anchor=(1, 0.5))
        axes[i].xaxis.label.set_visible(False)

    plt.tight_layout()
    plt.savefig('./eval/summary/plot.pdf', format='pdf')


def pad_tuple_paths(d, pad):
    tuples = []
    for k, v in d.items():
        pad_len = len(max(d, key=len)) - len(k)
        tuples.append((k + tuple([pad]*pad_len), v))
    return dict(tuples)


def path_compress_to_tuple(d, pred=()):
    items = []
    for key, value in d.items():
        new_key = pred + tuple([key]) if pred else tuple([key])
        if isinstance(value, dict):
            items.extend(path_compress_to_tuple(value, new_key).items())
        else:
            items.append((new_key, value))
    return dict(items)


if __name__ == '__main__':
    main()

