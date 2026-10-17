/*
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

   Module:
     MetaEngine.h

   Authors:
     Vincent Noel <contact@vincent-noel.fr>
 
   Date:
     March 2019
*/

#ifndef _PROBTRAJENGINE_H_
#define _PROBTRAJENGINE_H_

#include <string>
#include <map>
#include <vector>
#include <assert.h>

#ifdef MPI_COMPAT
#include <mpi.h>
#endif

#include "BooleanNetwork.h"
#include "FixedPointEngine.h"
#include "Cumulator.h"
#include "RandomGenerator.h"
#include "RunConfig.h"
#include "FixedPointDisplayer.h"
#include "ProbTrajDisplayer.h"

struct EnsembleArgWrapper;

class ProbTrajEngine : public FixedPointEngine {

protected:

  Cumulator<NetworkState>* merged_cumulator;
  std::vector<Cumulator<NetworkState>*> cumulator_v;

  static void* threadMergeWrapper(void *arg);

  static std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> mergeResults(std::vector<Cumulator<NetworkState>*>& cumulator_v, std::vector<STATE_MAP<NetworkState_Impl, unsigned int> *>& fixpoint_map_v);  
  
#ifdef MPI_COMPAT
  static std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> mergeMPIResults(RunConfig* runconfig, Cumulator<NetworkState>* ret_cumul, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints, int world_size, int world_rank, bool pack=true);
  
#endif

public:

  ProbTrajEngine(Network* network, RunConfig* runconfig) : FixedPointEngine(network, runconfig) {}
  
  Cumulator<NetworkState>* getMergedCumulator() {
    return merged_cumulator; 
  }

  int getMaxTickIndex() const {return merged_cumulator->getMaxTickIndex();} 
  const double getFinalTime() const;

  void displayStatDist(StatDistDisplayer* output_statdist) const;
  void displayProbTraj(ProbTrajDisplayer<NetworkState>* displayer) const;
  
  void display(ProbTrajDisplayer<NetworkState>* probtraj_displayer, StatDistDisplayer* statdist_displayer, FixedPointDisplayer* fp_displayer) const;

};

#endif
