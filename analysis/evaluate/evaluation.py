#!/usr/bin/python2
# -*- coding: utf-8 -*-
# 2014-06-11
# test program to plot waveforms from TDMS (labview binary format)

# loading the necessary libraries
# TDMS file handler
import sys
import numpy as np
from numpy.fft import rfft, rfftfreq
from nptdms import TdmsFile
from operator import add
# something i do not know
import numpy as np
# not sure what this is either
#import matplotlib.pyplot as plt
# maybe this is a wrapper to plot in gnuplot
import Gnuplot

def tobin(datax, datay, binsize):
    newx = []
    newy = []
    startbin = datax[0]
    bin_acc = 0.
    newcount = 0
    for count in range(0,len(datax)):
        if(datax[count] >= startbin + binsize):
            newx.append(startbin + binsize/2.)
            newy.append(bin_acc)
            newcount = newcount + 1
            startbin += binsize
            bin_acc = 0.
        bin_acc += abs(datay[count])
    return (newx,newy)


# reading TDMS file
filename = sys.argv[1]
datafile = TdmsFile(filename)  # TdmsFile is a function from the library/class npTDMS??

# get the group names
print filename
list_of_groups = datafile.groups() # groups is a function from npTDMS, what returns the names of the groups
# print list_of_groups[0]     it's only possible to print element 0, so the list has only one element
number_of_groups = len(list_of_groups)
# print number_of_groups   this gives a "1", so there is only one group
for groupname in list_of_groups:
    print groupname  # the groupname is "data"; it means to print every element of a list
    list_of_channels = datafile.group_channels(groupname) # group channels is a function from npTDMS, what returns a list of channel objects
    for channel in list_of_channels:
        print channel

# extracting first waveform
# getting voltages
bin_res_x = []
bin_res_y = []
#for group in ("Cube X1",):
for group in ("Cube X1", "Cube X2", "Cube Y1", "Cube Y2", "Cube Z1", "Cube Z2"):
#for group in ("Cube X2", "Cube Y1", "Cube Y2", "Cube Z1", "Cube Z2"):
    print "Group: " + group
    cubeX1 = datafile.object('data',group)      # we have one group and a list of channels in that group
    # getting time increment and then creating time array
    print cubeX1.properties
    dt = cubeX1.property('wf_increment') # extract the time information of the properties of the group "cubeX1"
    print "sample_period = " + str(dt)
    cubeX1_y = cubeX1.data     # create the y-axis for the plot, so the voltage values
    # print cubeX1_y
    cubeX1_x = [0 for x in range(len(cubeX1_y))]
    print len(cubeX1_y)
    for count in range(0,len(cubeX1_y),1):
        cubeX1_x[count] = count*dt

    #channel = datafile.object('data','Cube X1')
    #data = channel.data

    sp = rfft(cubeX1_y)
    freq = rfftfreq(cubeX1_y.size, d=dt)

    #binning:
    (binx, biny) = tobin(freq, sp, 50.0)
    #normalize to sampling time:
    sampletime = dt*float(len(cubeX1_y))
    for i in range(len(biny)):
        biny[i] = biny[i]/sampletime

    if (bin_res_y == []):
        print "first"
        bin_res_y = biny
        bin_res_x = binx
    else:
        for count in range(len(bin_res_y)):
            bin_res_y[count] = bin_res_y[count] + biny[count]
        #bin_res_y = map(add, bin_res_y, biny)



f = open(sys.argv[2], 'w')
for count in range(0,len(bin_res_x)):
    f.write(str(bin_res_x[count]) + " " + str(abs(bin_res_y[count])) + "\n")


#f = open('workfile', 'w')
#for count in range(0,len(sp)):
#    f.write(str(freq[count]) + " " + str(abs(sp[count])) + "\n")

#g = Gnuplot.Gnuplot()
#g("set linearscale xy 10")
#g("set style data linespoints")
#g.xlabel("time [s]")
#g.ylabel("field [nT]")
#
#d = Gnuplot.Data(freq, sp, title="squid time series X1")
##d = Gnuplot.Data(cubeX1_x, cubeX1_y, title="squid time series X1")
#g.plot(d)
