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
    ProbaDist.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#ifndef _PROBADIST_H_
#define _PROBADIST_H_

#include <assert.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>

#ifdef MPI_COMPAT
#include <mpi.h>

#include <stdint.h>
#include <limits.h>

#if SIZE_MAX == UCHAR_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define my_MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
   #error "what is happening here?"
#endif

#endif

#include "BooleanNetwork.h"
#include "Utils.h"
#include "StatDistDisplayer.h"

#define CLUSTER_OPTIM


template <typename S>
class ProbaDist {
  STATE_MAP<S, double> mp;

 public:
  size_t size() const {
    return mp.size();
  }

  void incr(const S& state, double tm_slice) {
    typename STATE_MAP<S, double>::iterator proba_iter = mp.find(state);
    if (proba_iter == mp.end()) {
      mp[state] = tm_slice;
    } else {
      (*proba_iter).second += tm_slice;
    }
  }

  void clear() {
    mp.clear();
  }

  void set(const S& state, double tm_slice) {
    mp[state] = tm_slice;
  }

  bool hasState(const S& state, double& tm_slice) const {
    typename STATE_MAP<S, double>::const_iterator iter = mp.find(state);
    if (iter != mp.end()) {
      tm_slice = (*iter).second;
      return true;
    }
    return false;
  }

#ifdef MPI_COMPAT
  size_t my_MPI_Size() {
    return sizeof(size_t) + size() * (NetworkState::my_MPI_Pack_Size() + sizeof(double));
  }
    
  void my_MPI_Pack(void* buff, unsigned int size_pack, int* position) 
  {
    size_t t_proba_dist_map_size = size();
    MPI_Pack(&t_proba_dist_map_size, 1, my_MPI_SIZE_T, buff, size_pack, position, MPI_COMM_WORLD);

    ProbaDist::Iterator t_proba_dist1_iter = iterator();
    while (t_proba_dist1_iter.hasNext()) {
      double proba;
      const NetworkState& state = t_proba_dist1_iter.next2(proba);
      state.my_MPI_Pack(buff, size_pack, position);
      MPI_Pack(&proba, 1, MPI_DOUBLE, buff, size_pack, position, MPI_COMM_WORLD);
    } 
  }
  
  void my_MPI_Unpack(void* buff, unsigned int buff_size, int* position) 
  {
    size_t t_proba_dist_map_size;
    MPI_Unpack(buff, buff_size, position, &t_proba_dist_map_size, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);

    for (size_t iii = 0; iii < t_proba_dist_map_size; iii++) {
      
      NetworkState state;
      double value = 0;
      state.my_MPI_Unpack(buff, buff_size, position);
      MPI_Unpack(buff, buff_size, position, &value, 1, MPI_DOUBLE, MPI_COMM_WORLD);

      set(state.getState(), value);      
    }
  }

  void my_MPI_Recv(int source)
  {
    size_t t_proba_dist_map_size;
    MPI_Recv(&t_proba_dist_map_size, 1, my_MPI_SIZE_T, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (size_t iii = 0; iii < t_proba_dist_map_size; iii++) {
      
      NetworkState state;
      double value;
      
      // MPI_Recv(&state, 1, MPI_UNSIGNED_LONG_LONG, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      state.my_MPI_Recv(source);
      MPI_Recv(&value, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      set(state.getState(), value);      
    }
  }
  void my_MPI_Send(int dest) 
  {
    size_t t_proba_dist_map_size = size();
    MPI_Send(&t_proba_dist_map_size, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);

    ProbaDist::Iterator t_proba_dist1_iter = iterator();
    while (t_proba_dist1_iter.hasNext()) {
      double proba;
      const NetworkState& state = t_proba_dist1_iter.next2(proba);
      state.my_MPI_Send(dest);
      // MPI_Send(&state, 1, MPI_UNSIGNED_LONG_LONG, dest, 0, MPI_COMM_WORLD);
      MPI_Send(&proba, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);  
    } 
  }
#endif
  class Iterator {
    
    const ProbaDist& proba_dist_map;
    typename STATE_MAP<S, double>::const_iterator iter, end;

  public:
  Iterator(const ProbaDist& proba_dist_map) : proba_dist_map(proba_dist_map) {
      iter = proba_dist_map.mp.begin();
      end = proba_dist_map.mp.end();
    }
	
    void rewind() {
      iter = proba_dist_map.mp.begin();
    }

    bool hasNext() const {
      return iter != end;
    }

    void next(S& state, double& tm_slice) {
      state = (*iter).first;
      tm_slice = (*iter).second;
      ++iter;
    }

    void next(S& state) {
      state = (*iter).first;
      ++iter;
    }

    const S& next2(double& tm_slice) {
      tm_slice = (*iter).second;
      return (*iter++).first;
    }

    const S& next2() {
      return (*iter++).first;
    }

    void next(double& tm_slice) {
      tm_slice = (*iter).second;
      ++iter;
    }
  };	

  void display(StatDistDisplayer* displayer) const {
    ProbaDist<S>::Iterator proba_dist_iter = iterator();
    while (proba_dist_iter.hasNext()) {
        NetworkState state;
        double proba;
        proba_dist_iter.next(state, proba);
        displayer->addStateProba(state.getState(), proba);
    }
  }

  Iterator iterator() {return Iterator(*this);}
  Iterator iterator() const {return Iterator(*this);}
};

class ProbaDistClusterFactory;

class ProbaDistCluster {

  MAP<unsigned int, ProbaDist<NetworkState> > proba_dist_map;
  ProbaDistClusterFactory* factory;
  struct Proba {
    double proba;
    double probaSquare;
    Proba() : proba(0.), probaSquare(0.) { }
    Proba(double proba, double probaSquare) : proba(proba), probaSquare(probaSquare) { }
  };
  // state -> Proba
  STATE_MAP<NetworkState, Proba> stat_dist_map;

 public:
  ProbaDistCluster(ProbaDistClusterFactory* factory) : factory(factory) { }

  void add(unsigned int index, const ProbaDist<NetworkState>& proba_dist);
  static double similarity(unsigned int nn1, const ProbaDist<NetworkState>& proba_dist1, unsigned int nn2, const ProbaDist<NetworkState>& proba_dist2, double** similarity_cache);

  size_t size() const {return proba_dist_map.size();}

  void complete(double threshold, unsigned int statdist_traj_count);
  void computeStationaryDistribution();

  void display(StatDistDisplayer* displayer) const;
  void displayStationaryDistribution(StatDistDisplayer* displayer) const;
};

class ProbaDistClusterFactory {

  std::vector<ProbaDistCluster*> proba_dist_cluster_v;
  MAP<unsigned int, bool> proba_dist_clusterized;
#ifdef CLUSTER_OPTIM
  MAP<unsigned int, bool> proba_dist_not_clusterized;
#endif
  const std::vector<ProbaDist<NetworkState> > proba_dist_v;
  unsigned int statdist_traj_count;
  double** similarity_cache;

 public:
  ProbaDistClusterFactory(const std::vector<ProbaDist<NetworkState> >& proba_dist_v, unsigned int statdist_traj_count) : proba_dist_v(proba_dist_v), statdist_traj_count(statdist_traj_count), similarity_cache(NULL) {
#ifdef CLUSTER_OPTIM
    for (unsigned int nn = 0; nn < statdist_traj_count; ++nn) {
      proba_dist_not_clusterized[nn] = true;
    }
    //std::cout << "SIZE: " << proba_dist_not_clusterized.size() << '\n';
#endif
  }

  ProbaDistCluster* newCluster() {
    ProbaDistCluster* proba_dist_cluster = new ProbaDistCluster(this);
    proba_dist_cluster_v.push_back(proba_dist_cluster);
    return proba_dist_cluster;
  }

  void setClusterized(unsigned int index) {
    proba_dist_clusterized[index] = true;
#ifdef CLUSTER_OPTIM
    MAP<unsigned int, bool>::iterator iter = proba_dist_not_clusterized.find(index);
    if (iter != proba_dist_not_clusterized.end()) {
      proba_dist_not_clusterized.erase(iter);
    }
#endif
  }

  double** getSimilarityCache() {
    return similarity_cache;
  }

#ifdef CLUSTER_OPTIM
  const MAP<unsigned int, bool>& getNotClusterizedMap() const {
    return proba_dist_not_clusterized;
  }
#endif

  bool isClusterized(unsigned int index) const {
    return proba_dist_clusterized.find(index) != proba_dist_clusterized.end();
  }

  const ProbaDist<NetworkState>& getProbaDist(unsigned int index) const {
    //assert(index < proba_dist_v.size());
    return proba_dist_v[index];
  }

  void cacheSimilarities();

  void makeClusters(RunConfig* runconfig);
  void computeStationaryDistribution();
  
  void displayStationaryDistribution(StatDistDisplayer* displayer) const;
  void display(StatDistDisplayer* displayer) const;

  ~ProbaDistClusterFactory() {
    
    for (unsigned int nn1 = 0; nn1 < statdist_traj_count; ++nn1) {
      delete [] similarity_cache[nn1];
    }
    delete [] similarity_cache;
    for (auto& proba_dist_cluster: proba_dist_cluster_v) {
      delete proba_dist_cluster;
    }
  }
};

#endif
