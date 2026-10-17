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
     FixedPointEngine.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     March 2021
*/

#include "FixedPointEngine.h"
#include "Utils.h"

void FixedPointEngine::mergePairOfFixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2)
{
  for (auto& fixpoint: *fixpoints_2) {
    
    STATE_MAP<NetworkState_Impl, unsigned int>::iterator t_fixpoint = fixpoints_1->find(fixpoint.first);
    if (fixpoints_1->find(fixpoint.first) == fixpoints_1->end()) {
      (*fixpoints_1)[fixpoint.first] = fixpoint.second;
    
    } else {
      t_fixpoint->second += fixpoint.second;
    
    }
  }
  delete fixpoints_2; 
}


STATE_MAP<NetworkState_Impl, unsigned int>* FixedPointEngine::mergeFixpointMaps()
{
  if (1 == fixpoint_map_v.size()) {
    return new STATE_MAP<NetworkState_Impl, unsigned int>(*fixpoint_map_v[0]);
  }

  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
  for (auto * fp_map : fixpoint_map_v) {
    for (const auto & fp : *fp_map) {
      const NetworkState_Impl& state = fp.first;
      STATE_MAP<NetworkState_Impl, unsigned int>::iterator iter = fixpoint_map->find(state);
      if (iter == fixpoint_map->end()) {
	      (*fixpoint_map)[state] = fp.second;
      } else {
	      iter->second += fp.second;
      }
    }
  }
  return fixpoint_map;
}


#ifdef MPI_COMPAT
void FixedPointEngine::MPI_Unpack_Fixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, char* buff, unsigned int buff_size)
{
        
  int position = 0;
  unsigned int nb_fixpoints;
  MPI_Unpack(buff, buff_size, &position, &nb_fixpoints, 1, MPI_UNSIGNED, MPI_COMM_WORLD);
  
  if (nb_fixpoints > 0) {
    if (fp_map == NULL) {
      fp_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
    }
    for (unsigned int j=0; j < nb_fixpoints; j++) {
      NetworkState state;
      state.my_MPI_Unpack(buff, buff_size, &position);
      unsigned int count = 0;
      MPI_Unpack(buff, buff_size, &position, &count, 1, MPI_UNSIGNED, MPI_COMM_WORLD);
      
      if (fp_map->find(state.getState()) == fp_map->end()) {
        (*fp_map)[state.getState()] = count;
      } else {
        (*fp_map)[state.getState()] += count;
      }
    }
  }
}

char* FixedPointEngine::MPI_Pack_Fixpoints(const STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, int dest, unsigned int * buff_size)
{
  unsigned int nb_fixpoints = fp_map == NULL ? 0 : fp_map->size();
  *buff_size = sizeof(unsigned int) + (sizeof(unsigned int) + NetworkState::my_MPI_Pack_Size()) * nb_fixpoints;
  char* buff = new char[*buff_size];
  int position = 0;
  
  MPI_Pack(&nb_fixpoints, 1, MPI_UNSIGNED, buff, *buff_size, &position, MPI_COMM_WORLD);

  if (nb_fixpoints > 0) {
    for (auto& fixpoint: *fp_map) {  
      NetworkState state(fixpoint.first);
      unsigned int count = fixpoint.second;
      state.my_MPI_Pack(buff, *buff_size, &position);
      MPI_Pack(&count, 1, MPI_UNSIGNED, buff, *buff_size, &position, MPI_COMM_WORLD);
    }
  }
  return buff;
}

void FixedPointEngine::MPI_Send_Fixpoints(const STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, int dest) 
{
  int nb_fixpoints = fp_map->size();
  MPI_Send(&nb_fixpoints, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
  
  for (auto& fixpoint: *fp_map) {
    NetworkState state(fixpoint.first);
    unsigned int count = fixpoint.second;
    
    state.my_MPI_Send(dest);
    MPI_Send(&count, 1, MPI_UNSIGNED, dest, 0, MPI_COMM_WORLD);
    
  } 
}

void FixedPointEngine::MPI_Recv_Fixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, int origin) 
{
  int nb_fixpoints = -1;
  MPI_Recv(&nb_fixpoints, 1, MPI_INT, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  for (int j = 0; j < nb_fixpoints; j++) {
    NetworkState state;
    state.my_MPI_Recv(origin);
    
    unsigned int count = -1;
    MPI_Recv(&count, 1, MPI_UNSIGNED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    if (fp_map->find(state.getState()) == fp_map->end()) {
      (*fp_map)[state.getState()] = count;
    } else {
      (*fp_map)[state.getState()] += count;
    }
  }
}

void FixedPointEngine::mergePairOfMPIFixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints, int world_rank, int dest, int origin, bool pack) 
{
   if (world_rank == dest) 
   {
   
    if (pack) {
      unsigned int buff_size;
      MPI_Recv( &buff_size, 1, MPI_UNSIGNED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
      char* buff = new char[buff_size];
      MPI_Recv( buff, buff_size, MPI_PACKED, origin, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
          
      MPI_Unpack_Fixpoints(fixpoints, buff, buff_size);
      delete buff;
      
    } else {
      MPI_Recv_Fixpoints(fixpoints, origin);
    }
    
  } else if (world_rank == origin) {

    if (pack) {

      unsigned int buff_size;
      char* buff = MPI_Pack_Fixpoints(fixpoints, dest, &buff_size);

      MPI_Send(&buff_size, 1, MPI_UNSIGNED, dest, 0, MPI_COMM_WORLD);
      MPI_Send( buff, buff_size, MPI_PACKED, dest, 0, MPI_COMM_WORLD); 
      delete buff;            
      
    } else {
     
      MPI_Send_Fixpoints(fixpoints, dest);
    }
  }
}

#endif

const std::map<unsigned int, std::pair<NetworkState, double> > FixedPointEngine::getFixPointsDists() const {
  
  std::map<unsigned int, std::pair<NetworkState, double> > res;
  if (0 == fixpoints.size()) {
    return res;
  }

  int nn = 0;
  for (const auto & fp : fixpoints) {
    const NetworkState& network_state = fp.first;
    res[nn++] = std::make_pair(network_state,(double) fp.second / sample_count);
  }
  return res;
}

void FixedPointEngine::displayFixpoints(FixedPointDisplayer* displayer) const 
{
#ifdef MPI_COMPAT
if (getWorldRank() == 0) {
#endif

  displayer->begin(fixpoints.size());
  int nn = 0;
  for (const auto & fp : fixpoints) {
    const NetworkState& network_state = fp.first;
    displayer->displayFixedPoint(nn+1, network_state, fp.second, sample_count);
    nn++;
  }
  displayer->end();

#ifdef MPI_COMPAT
}
#endif

}