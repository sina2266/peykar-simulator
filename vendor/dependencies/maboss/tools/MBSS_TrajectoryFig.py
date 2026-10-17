#!/usr/bin/env python3
# coding: utf-8
#
#############################################################################
#                                                                           #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)   #
#                                                                           #
# Copyright (c) 2011-2020 Institut Curie, 26 rue d'Ulm, Paris, France       #
# All rights reserved.                                                      #
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions are    #
# met:                                                                      #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright notice, #
# this list of conditions and the following disclaimer.                     #
#                                                                           #
# 2. Redistributions in binary form must reproduce the above copyright      #
# notice, this list of conditions and the following disclaimer in the       #
# documentation and/or other materials provided with the distribution.      #
#                                                                           #
# 3. Neither the name of the copyright holder nor the names of its          #
# contributors may be used to endorse or promote products derived from this #
# software without specific prior written permission.                       #
#                                                                           #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED #
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           #
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER #
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       #
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        #
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    #
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              #
#                                                                           #
#############################################################################
#
# Tool: MBSS_TrajectoryFig.py
# Author: Barthélémy Caron, caron.barth@gmail.com
# Date: Jan 2017
#

import pandas as pd
import matplotlib.pylab as plt
import sys
from matplotlib import cm
import xlsxwriter
import numpy as np
import seaborn as sns
sns.set(style="darkgrid")

def remove_dataframe_columns (panda_dataframe):
    dataframe = panda_dataframe
    dataframe = dataframe.drop("Time", 1)
    dataframe = dataframe.drop("TH", 1)
    dataframe = dataframe.drop("ErrTH", 1)
    dataframe = dataframe.drop("H", 1)
    
    for item in dataframe:
            if "ErrProb" in item or "Unnamed" in item:
                dataframe = dataframe.drop(item,1)
    return(dataframe)


def get_time_array (panda_dataframe):
    time_array = []
    time_array = np.asarray(probability_dataframe["Time"]).T
    return(time_array)


def remove_array_points ( my_array , one_in):
    to_delete = []
    total = np.arange(0, len(my_array), 1)
    for value in total:
        if value%one_in != 0:
            to_delete.append(value)
    my_array = np.delete(my_array, to_delete)
    return(my_array)


def remove_dataframe_points ( my_dataframe, one_in):
    to_delete = []
    total = np.arange(0, my_dataframe.shape[0], 1)
    for value in total:
        if value%one_in != 0:
            to_delete.append(value)
    my_dataframe = my_dataframe.drop(to_delete,0)
    return(my_dataframe)

def new_labels (label_list):
    for label_index, label in enumerate(label_list):
        label_list[label_index] = label.replace("Prob[", "").replace("]", "")
    return label_list

if sys.argv[1] == "help":
    print("MBSS_TrajectoryFig.py takes one argument: the name of the folder containing the foldername_probtraj_table.csv file")
    print("The script saves a figure representing the evolution of probability through timeof every external node")
    print("Moreover, it creates a folder_name.xlsx file containing the same data that the foldername_probtraj_table.csv file after removing all the entropy and Error columns.")
    sys.exit("")

model = sys.argv[1]
file_name = "{}".format(model)

maboss_probability_table = "{}/{}_probtraj_table.csv".format(file_name, file_name)
probability_dataframe = {}
probability_dataframe = pd.read_csv(maboss_probability_table, "\t")
time_array = get_time_array(probability_dataframe)
probability_dataframe = remove_dataframe_columns(probability_dataframe)
plotting_array = np.zeros((np.asarray(probability_dataframe.shape)[1], np.asarray(probability_dataframe.shape)[0]))

for column_index in np.arange((probability_dataframe.shape)[1]):
    plotting_array[column_index, :] = np.asarray(probability_dataframe.iloc[:, column_index])

label_list = new_labels(probability_dataframe.columns.values)

plotting_array = plotting_array.T
color_pallette = sns.color_palette("Dark2", (probability_dataframe.shape)[1])

fig = plt.figure(1, dpi=200)
ax = fig.add_subplot(111)

for column_index in np.arange((probability_dataframe.shape)[1]):
    maximum_argument = np.amax(plotting_array[:, column_index])
    ax.plot(time_array, plotting_array[:, column_index],c=color_pallette[column_index], linewidth=2, linestyle='-', label=label_list[column_index])

writer = pd.ExcelWriter("{}_traj.xlsx".format(file_name))
probability_dataframe.to_excel(writer, 'Sheet1')
writer.save()

ax.set_xlabel("Time(s)")
ax.set_ylabel("Probability")
my_legends = ax.legend(loc='center right', fontsize=8)
fig.tight_layout()
ax.set_title("{}".format(file_name))
plt.savefig("{}_traj.pdf".format(file_name), dpi=300, bbox_extra_artiste=(my_legends,), bbox_inches='tight')
