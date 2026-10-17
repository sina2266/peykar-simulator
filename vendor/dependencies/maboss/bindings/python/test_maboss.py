# coding: utf-8
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


# Module: maboss/comm.py
# Authors: Eric Viara <viara@sysra.com>
# Date: May-December 2018

#
# to add in environment before running this script:
# export MABOSS_SERVER=../../engine/src/MaBoSS-server
#

from __future__ import print_function
import maboss.comm, maboss.simul, maboss.result, sys

# MaBoSS client instantiation
# - the MaBoSS client forks a MaBoSS-server (defined as default as "MaBoSS-server" or in MABOSS_SERVER environment variable)
# - this MaBoSS client can be used for multiple simulations
mbcli = maboss.comm.MaBoSSClient()

# create a simulation (network + config)
TESTLOC = "../../engine/tests/"
simulation = maboss.simul.Simulation(bndfile = TESTLOC + "/cellcycle-bad.bnd", cfgfiles = [TESTLOC + "/cellcycle_runcfg.cfg", TESTLOC + "/cellcycle_runcfg-thread_1-simple.cfg"])

# run the simulation, the forked MaBoSS-server will be used
check = True
check = False
verbose = False
augment = False
override = False
augment = True
#override = True
result = mbcli.run(simulation, {"check" : check, "hexfloat" : True, "augment" : augment, "override" : override, "verbose" : verbose}) # will call Result(mbcli, simulation)

# get the returned data (notice the data is not checkd)
result_data = result.getResultData()

# prints the returned data
if result_data.getStatus():
    print("result_data status=", result_data.getStatus(), "errmsg=", result_data.getErrorMessage(), file=sys.stderr)
if result_data.getStatus() == 0: # means Success
    print("FP", result_data.getFP())
    print("Runlog", result_data.getRunLog())
    print("ProbTraj", result_data.getProbTraj())
    print("StatDist", result_data.getStatDist())


