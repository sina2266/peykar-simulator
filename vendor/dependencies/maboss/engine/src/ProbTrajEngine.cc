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
     ProbTrajEngine.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     March 2021
*/

#include "ProbTrajEngine.h"
#include "Probe.h"
#include "Utils.h"

struct MergeWrapper {
  Cumulator<NetworkState>* cumulator_1;
  Cumulator<NetworkState>* cumulator_2;
  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1;
  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2;
  
  MergeWrapper(Cumulator<NetworkState>* cumulator_1, Cumulator<NetworkState>* cumulator_2, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2) :
    cumulator_1(cumulator_1), cumulator_2(cumulator_2), fixpoints_1(fixpoints_1), fixpoints_2(fixpoints_2) { }
};

void* ProbTrajEngine::threadMergeWrapper(void *arg)
{
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif
  MergeWrapper* warg = (MergeWrapper*)arg;
  try {
    Cumulator<NetworkState>::mergePairOfCumulators(warg->cumulator_1, warg->cumulator_2);
    ProbTrajEngine::mergePairOfFixpoints(warg->fixpoints_1, warg->fixpoints_2);
  } catch(const BNException& e) {
    std::cerr << e;
  }
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
#endif
  return NULL;
}


std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> ProbTrajEngine::mergeResults(std::vector<Cumulator<NetworkState>*>& cumulator_v, std::vector<STATE_MAP<NetworkState_Impl, unsigned int> *>& fixpoint_map_v) {
  
  size_t size = cumulator_v.size();
  
  if (size == 0) {
    return std::make_pair((Cumulator<NetworkState>*) NULL, new STATE_MAP<NetworkState_Impl, unsigned int>());
  }
  
  if (size > 1) {
    
    
    unsigned int lvl=1;
    unsigned int max_lvl = ceil(log2(size));

    while(lvl <= max_lvl) {      
    
      unsigned int step_lvl = pow(2, lvl-1);
      unsigned int width_lvl = floor(size/(step_lvl*2)) + 1;
      pthread_t* tid = new pthread_t[width_lvl];
      unsigned int nb_threads = 0;
      std::vector<MergeWrapper*> wargs;
      for(unsigned int i=0; i < size; i+=(step_lvl*2)) {
        
        if (i+step_lvl < size) {
          MergeWrapper* warg = new MergeWrapper(cumulator_v[i], cumulator_v[i+step_lvl], fixpoint_map_v[i], fixpoint_map_v[i+step_lvl]);
          pthread_create(&tid[nb_threads], NULL, ProbTrajEngine::threadMergeWrapper, warg);
          nb_threads++;
          wargs.push_back(warg);
        } 
      }
      
      for(unsigned int i=0; i < nb_threads; i++) {   
          pthread_join(tid[i], NULL);
          
      }
      
      for (auto warg: wargs) {
        delete warg;
      }
      delete [] tid;
      lvl++;
    }
  
   
  }
  
  return std::make_pair(cumulator_v[0], fixpoint_map_v[0]);
}

#ifdef MPI_COMPAT


std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> ProbTrajEngine::mergeMPIResults(RunConfig* runconfig, Cumulator<NetworkState>* ret_cumul, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints, int world_size, int world_rank, bool pack)
{  
  if (world_size> 1) {
    
    int lvl=1;
    int max_lvl = ceil(log2(world_size));

    while(lvl <= max_lvl) {
    
      int step_lvl = pow(2, lvl-1);
      
      for(int i=0; i < world_size; i+=(step_lvl*2)) {
        
        if (i+step_lvl < world_size) {
          if (world_rank == i || world_rank == (i+step_lvl)){
            ret_cumul = Cumulator<NetworkState>::mergePairOfMPICumulators(ret_cumul, world_rank, i, i+step_lvl, runconfig, pack);
            mergePairOfMPIFixpoints(fixpoints, world_rank, i, i+step_lvl, pack);
          }
        } 
      }
      
      lvl++;
    }
  }
  
  return std::make_pair(ret_cumul, fixpoints); 
  
}
#endif


const double ProbTrajEngine::getFinalTime() const {
  return merged_cumulator->getFinalTime();
}

void ProbTrajEngine::displayProbTraj(ProbTrajDisplayer<NetworkState>* displayer) const {

#ifdef MPI_COMPAT
if (getWorldRank() == 0) {
#endif

  merged_cumulator->displayProbTraj(network, refnode_count, displayer);

#ifdef MPI_COMPAT
}
#endif
}

void ProbTrajEngine::displayStatDist(StatDistDisplayer* statdist_displayer) const {

#ifdef MPI_COMPAT
if (getWorldRank() == 0) {
#endif

  Probe probe;
  merged_cumulator->displayStatDist(network, refnode_count, statdist_displayer);
  probe.stop();
  elapsed_statdist_runtime = probe.elapsed_msecs();
  user_statdist_runtime = probe.user_msecs();

  /*
  unsigned int statdist_traj_count = runconfig->getStatDistTrajCount();
  if (statdist_traj_count == 0) {
    output_statdist << "Trajectory\tState\tProba\n";
  }
  */
#ifdef MPI_COMPAT
}
#endif

}

void ProbTrajEngine::display(ProbTrajDisplayer<NetworkState>* probtraj_displayer, StatDistDisplayer* statdist_displayer, FixedPointDisplayer* fp_displayer) const
{
  displayProbTraj(probtraj_displayer);
  displayStatDist(statdist_displayer);
  displayFixpoints(fp_displayer);
}
