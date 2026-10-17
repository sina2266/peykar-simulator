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
# Tool: MBSS_PrepareProjectFilePieChart.py
# Author: Barthélémy Caron, caron.barth@gmail.com
# Date: Jan 2017
#

# In[12]:

import pandas as pd
import matplotlib.pylab as plt
import sys
from matplotlib import cm
import xlsxwriter
import numpy as np
import seaborn as sns
import matplotlib.patches as mpatches
import matplotlib.gridspec as gridspec
sns.set_style("white")

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

def new_labels (label_list):
    for label_index, label in enumerate(label_list):
        label_list[label_index] = label.replace("Prob[", "").replace("]", "")
    return label_list



if sys.argv[1] == "help":
    print("MBSS_PrepareProjectFilePieChart.py takes one argument: the name of the foldername_probtraj_table.csv file")
    print("The script saves a piechart representing the final state")
    print("Moreover, it creates a folder_name_final.xlsx file containing the final probability of each node.")
    sys.exit("")

fname = sys.argv[1]
file_name = "{}".format(fname)


#maboss_probability_table = "{}/{}_probtraj_table.csv".format(fname, fname)
maboss_probability_table = "{}_probtraj_table.csv".format(fname)
probability_dataframe = {}
probability_dataframe = pd.read_csv(maboss_probability_table, "\t")
probability_dataframe = remove_dataframe_columns(probability_dataframe)
labels = new_labels(probability_dataframe.columns.values)

#sort the dataframe
my_color_palette = sns.color_palette(palette="terrain", n_colors=(probability_dataframe.shape)[1])
plotting_dataframe = probability_dataframe.iloc[-1].rename("")

plotting_labels = []
for value_index, value in enumerate(plotting_dataframe):
    if value >= 0.1:
        plotting_labels.append(plotting_dataframe.index.values[value_index])
    else:
        plotting_labels.append("")

ax = plt.subplot(111)
ax.pie(plotting_dataframe, startangle=90, colors=my_color_palette, labels=plotting_labels, radius=1.2, labeldistance=0.4)
ax.set(ylabel='', aspect='equal')

writer = pd.ExcelWriter("{}_pie.xlsx".format(fname))
plotting_dataframe.to_frame(name='Final Probability').to_excel(writer, sheet_name='final_values')
writer.save()

my_legends = ax.legend(plotting_dataframe.index.values, loc=(0.9, 0.8), fontsize = 8)
sns.despine(top=True, right=True, left=True, bottom=True)
plt.savefig("{}_pie.pdf".format(fname), dpi=300, bbox_extra_artiste=(my_legends,), bbox_inches='tight')





