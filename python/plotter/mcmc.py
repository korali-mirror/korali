#! /usr/bin/env python3

import os
import re
import sys
import time
import json
import numpy as np
import matplotlib.pyplot as plt

from korali.plotter.helpers import plt_pause_light

# Plot histogram of sampes in diagonal
def plot_histogram(ax, theta):
    dim = theta.shape[1]
    num_bins = 50
    
    for i in range(dim):

        if (dim == 1): 
            ax_loc = ax
        else: 
            ax_loc = ax[i,i]
 
        hist, bins, _ = ax_loc.hist(theta[:, i], num_bins, density=True,
                                    color='lightgreen', ec='black')
        
        if i == 0:
           
            # Rescale hist to scale of theta -> get correct axis titles
            widths = np.diff(bins)
            if (dim > 1) :  
                hist = hist / np.max(hist) * (ax_loc.get_xlim()[1] -
                                                ax_loc.get_xlim()[0])
                bottom = ax_loc.get_xlim()[0]
                ax_loc.cla()
                ax_loc.bar(bins[:-1], hist, widths,
                    color='lightgreen', ec='black', bottom=bottom)
                ax_loc.set_ylim(ax_loc.get_xlim())
                ax_loc.set_xticklabels([])
            else:
                ax_loc.cla()
                ax_loc.bar(bins[:-1], hist, widths,
                     color='lightgreen', ec='black')

        elif i == theta.shape[1] - 1:
            ax_loc.set_yticklabels([])
        
        else:
            ax_loc.set_xticklabels([])
            ax_loc.set_yticklabels([])
        ax_loc.tick_params(axis='both', which='both', length=0)


#Plot scatter plot in upper triangle of figure
def plot_upper_triangle(ax, theta, lik=False):
    dim = theta.shape[1]
    if (dim == 1): return
    
    for i in range(dim):
        for j in range(i + 1, dim):
            if lik:
                ax[i, j].scatter(theta[:, j], theta[:, i], marker='o', s=10,
                    c=theta, alpha=0.5)
            else:
                ax[i, j].plot(theta[:, j], theta[:, i], '.', markersize=1)
            ax[i, j].set_xticklabels([])
            ax[i, j].set_yticklabels([])
            ax[i, j].grid(b=True, which='both')


#Plot 2d histogram in lower triangle of figure
def plot_lower_triangle(ax, theta):
    dim = theta.shape[1]
    if (dim == 1): return

    for i in range(dim):
        for j in range(i):
            # returns bin values, bin edges and bin edges
            H, xe, ye = np.histogram2d(theta[:, j], theta[:, i], 10, density=True)
            # plot and interpolate data
            ax[i, j].imshow(H.T, aspect="auto", interpolation='spline16',
                            origin='lower', extent=np.hstack((
                                                ax[j, j].get_xlim(),
                                                ax[i, i].get_xlim())),
                                                cmap=plt.get_cmap('jet'))
            
            if i < theta.shape[1]-1:
                ax[i, j].set_xticklabels([])
            if j > 0:
                ax[i, j].set_yticklabels([])


# Plot MCMC result file
def plot_samples(path, idx=None):
    with open(path) as f:
        data    = json.load(f)
        dims    = data['Problem']['Variables']
        numdim  = len(dims)
        pop     = data['Solver']['State']['Database Entries']
        fitness = data['Solver']['State']['DatabaseFitness']
        samples = np.reshape( data['Solver']['State']['DatabasePoints'], (pop,numdim) )

        plt.style.use('seaborn-dark')
        fig, ax = plt.subplots(samples.shape[1], samples.shape[1], figsize=(8,8))
        fig.show()
        
        if idx is None: 
            fig.canvas.set_window_title(path)
            idx = int(re.findall(r'[0-9]+', path)[-1])
        else:           
            fig.canvas.set_window_title('s{0}.json'.format(str(idx).zfill(5)))
        
        plt.suptitle( 'MCMC\nNumber of Samples {0}\n'.format(str(pop)),
                      fontweight='bold',
                      fontsize  = 12)

        plot_histogram(ax, samples)
        plot_upper_triangle(ax, samples, False)
        plot_lower_triangle(ax, samples)

# Plot MCMC results (read from .json files)
def plot_mcmc(src, live=False):
     
    idx      = 0
    path = '{0}/s{1}.json'.format(src, str(idx).zfill(5))

    while (os.path.isfile(path)):
        idx = idx+1
        path = '{0}/s{1}.json'.format(src, str(idx).zfill(5))
        if ( live == True ):
            print("TODO")

    if (idx == 0):
        print("TODO, file not found")

    path = '{0}/s{1}.json'.format(src, str(idx-1).zfill(5))
    plot_samples(path, idx)
    plt_pause_light(3600) 

    plt.show()