print 'Importing...'
import pandas as pd
import sys
from math import sqrt, pi, exp
import numpy as np

from xplot import *

MS = 10 # markersize

def readtable(name):
  '''read the table'''

  df = pd.read_csv(name, sep=' ')
  # df = df[df['RunNumber'] > xlim]
  df = df.set_index('RunNumber')

  # intensity as area of the peak = amplitude * sigma * sqrt(2 pi) / nlive / tt8
  df['IntensityPeak'] = df['GausAmp'] * df['GausSigma'] * sqrt(2*pi) / df['NLivePmts'] / df['NTT8']
  # intensity as total number of entries
  df['IntensityTotal'] = df['Nentries'].divide(df['NLivePmts'])
  df['IntensityTotal'] = df['IntensityTotal'].divide(df['NTT8'])
  return df



def plot_init():
  ''' create plot objects '''
  ### --------------- plot --------------- ###
  plots = []
  figsize = (10,8)
  # figsize = (15,8)
  for i in range(2):
    plots.append(XPlot(figsize))#, 2-i))

  plots.append(XPlot((12,8)))

  # first plot: sigmas
  plots[0].fig.suptitle('Peak sigma')
  plots[0].axes[0].set_ylabel('Time [ns]')
  # second plot: intensity
  plots[1].fig.suptitle('Intensity')
  plots[1].ax.set_ylabel('Entries / # live PMTs / # TT8 events')
  # third plot: bad channels
  plots[2].ax.set_ylabel('Number of channels')
  plots[2].fig.suptitle('Bad channels')
  return plots



def plot_table(df, plots):
  ''' plot one table '''
  print df.head()
  # first plot: sigmas
  sigmas = ['GausSigma', 'SkewLeftSigma','SkewRightSigma']
  df[sigmas].plot(style='.-', ax = plots[0].axes[0], ms=MS)

  # second plot: intensity
  df[['IntensityTotal','IntensityPeak']].plot(style='.-', ax = plots[1].ax, ms=MS)

  # third plot: bad channels
  df['BadChannels'].plot(ax = plots[2].axes[0], style='.', ms=10, color='b', legend=False, label='_nolegend_')
  dfbad = df['BadChannels'][df['BadChannels'] > 70]
  if len(dfbad): dfbad.plot(ax = plots[2].axes[0], style='.', ms=10, color='r', legend=False, label='_nolegend_')



def plot_tables(tables, op=False, xlim=26000, save=False):
  ''' plot many tables '''
  ## set up plot objects
  plots = plot_init()

  ## plot each table
  for tab in tables:
      print 'Reading:', tab
      df = readtable(tab)
      print 'Plotting:', tab
      plot_table(df, plots)

  for i in range(len(plots)):
    if xlim: plots[i].ax.set_xlim(xmin=xlim)
    if op:
      plots[i].xpoints(xlim=xlim, date=False)
    if i != 2:
        plots[i].legend(out=op, ncol=2)
    plots[i].pretty()


  if save:
    for i in range(len(plots)):
      imgname = 'canvas' + str(i) + '_' + str(xlim)
      if op: imgname += '_op'
      plots[i].fig.savefig(imgname + '.png')
      print(imgname)
  else:
    plt.show()



if __name__ == '__main__':
  names = sys.argv[1:] # table names
  xlim = 26000
  for s in sys.argv:
    if s.isdigit():
        xlim = int(s)
  # xlim = int(sys.argv[-1]) if len(sys.argv) > 2 else 26000
  op = 'op' in sys.argv
  save = 'save' in sys.argv
  names = np.setdiff1d(names, ['op', 'save', str(xlim)])

  plot_tables(names, op, xlim, save)
