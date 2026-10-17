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
     ProbaDist.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

class Network;

#include "ProbaDist.h"
#include "BooleanNetwork.h"
#include "RunConfig.h"
#include "Utils.h"
#include "StatDistDisplayer.h"
#include <iomanip>
#include <math.h>
#include <float.h>

/*
static double abs(double d)
{
  return d > 0. ? d : -d;
}
*/

void ProbaDistCluster::add(unsigned int index, const ProbaDist<NetworkState>& proba_dist)
{
  proba_dist_map[index] = proba_dist;
  factory->setClusterized(index);
}

void ProbaDistCluster::complete(double threshold, unsigned int statdist_traj_count)
{
  for (;;) {
    unsigned int added_proba_dist_cnt = 0;
    std::vector<unsigned int> toadd_map;
    // MAP<unsigned int, ProbaDist<NetworkState> >::iterator begin = proba_dist_map.begin();
    // MAP<unsigned int, ProbaDist<NetworkState> >::iterator end = proba_dist_map.end();
    // while (begin != end) {
    for (const auto & proba_dist_entry : proba_dist_map) {
      unsigned int nn1 = proba_dist_entry.first;
      const ProbaDist<NetworkState>& proba_dist1 = proba_dist_entry.second;
#ifdef CLUSTER_OPTIM
      // MAP<unsigned int, bool>::const_iterator not_clusterized_iter = factory->getNotClusterizedMap().begin();
      // MAP<unsigned int, bool>::const_iterator not_clusterized_end = factory->getNotClusterizedMap().end();
      // while (not_clusterized_iter != not_clusterized_end) {
      for (const auto & not_clusterized : factory->getNotClusterizedMap()) {
        unsigned int nn2 = not_clusterized.first;
        const ProbaDist<NetworkState>& proba_dist2 = factory->getProbaDist(nn2);
        double simil = similarity(nn1, proba_dist1, nn2, proba_dist2, factory->getSimilarityCache());
        if (simil >= threshold) {
          toadd_map.push_back(nn2);
          added_proba_dist_cnt++;
        }
      }
#else
      for (unsigned int nn2 = 0; nn2 < statdist_traj_count; ++nn2) { // optimizatin: should avoid to scan all proba_dist, using a complement map
	if (!factory->isClusterized(nn2)) {
	  const ProbaDist& proba_dist2 = factory->getProbaDist(nn2);
	  double simil = similarity(nn1, proba_dist1, nn2, proba_dist2, factory->getSimilarityCache());
	  if (simil >= threshold) {
	    add(nn2, proba_dist2);
	    added_proba_dist_cnt++;
	  }
	}
      }
#endif
      // ++begin;
    }
#ifdef CLUSTER_OPTIM
    for (const unsigned int& nn2 : toadd_map) {
      add(nn2, factory->getProbaDist(nn2));
    }
#endif
    if (!added_proba_dist_cnt) {
      break;
    }
  }
}

void ProbaDistClusterFactory::makeClusters(RunConfig* runconfig)
{
  if (statdist_traj_count <= runconfig->getStatDistSimilarityCacheMaxSize()) {
    cacheSimilarities();
  }
  for (unsigned int nn1 = 0; nn1 < statdist_traj_count; ++nn1) { // optimization: should avoid to scan all proba_dist, using a complement map
    if (!isClusterized(nn1)) {
      ProbaDistCluster* cluster = newCluster();
      cluster->add(nn1, getProbaDist(nn1));
      cluster->complete(runconfig->getStatdistClusterThreshold(), statdist_traj_count);
    }
  }
}

void ProbaDistClusterFactory::cacheSimilarities()
{
  similarity_cache = new double*[statdist_traj_count];
  for (unsigned int nn1 = 0; nn1 < statdist_traj_count; ++nn1) {
    similarity_cache[nn1] = new double[statdist_traj_count];
  }

  for (unsigned int nn1 = 0; nn1 < statdist_traj_count; ++nn1) {
    for (unsigned int nn2 = nn1; nn2 < statdist_traj_count; ++nn2) {
      similarity_cache[nn1][nn2] = ProbaDistCluster::similarity(nn1, getProbaDist(nn1), nn2, getProbaDist(nn2), NULL);
    }
  }
}

void ProbaDistCluster::computeStationaryDistribution()
{
  for (const auto & proba_dist_entry : proba_dist_map) {
    const ProbaDist<NetworkState>& proba_dist = proba_dist_entry.second;
    ProbaDist<NetworkState>::Iterator iter(proba_dist);
    while (iter.hasNext()) {
      double proba;
      const NetworkState& state = iter.next2(proba);
      auto iter = stat_dist_map.find(state);
      if (iter == stat_dist_map.end()) {
	      stat_dist_map[state] = Proba(proba, proba*proba);
      } else {
        iter->second.proba += proba;
        iter->second.probaSquare += proba*proba;
      }
    }
  }
}

void ProbaDistClusterFactory::computeStationaryDistribution()
{
  unsigned int size = proba_dist_cluster_v.size();
  for (unsigned int nn = 0; nn < size; ++nn) {
    ProbaDistCluster* cluster = proba_dist_cluster_v[nn];
    cluster->computeStationaryDistribution();
  }
}

double ProbaDistCluster::similarity(unsigned int nn1, const ProbaDist<NetworkState>& proba_dist1, unsigned int nn2, const ProbaDist<NetworkState>& proba_dist2, double** similarity_cache)
{
  if (NULL != similarity_cache) {
    return nn2 > nn1 ? similarity_cache[nn1][nn2] : similarity_cache[nn2][nn1];
  }

  ProbaDist<NetworkState>::Iterator proba_dist1_iter = proba_dist1.iterator();
  double simil1 = 0.0;
  double simil2 = 0.0;
  while (proba_dist1_iter.hasNext()) {
    double proba1, proba2;
    const NetworkState& state = proba_dist1_iter.next2(proba1);

    if (proba_dist2.hasState(state, proba2)) {
      simil1 += proba1;
      simil2 += proba2;
    } 
  }

  return simil1 * simil2;
}

void ProbaDistCluster::display(StatDistDisplayer* displayer) const
{
  for (const auto & proba_dist_entry : proba_dist_map) {
    unsigned int nn = proba_dist_entry.first;
    const ProbaDist<NetworkState>& proba_dist = proba_dist_entry.second;
    displayer->beginStateProba(nn+1);
    proba_dist.display(displayer);
    displayer->endStateProba();
  }
}

void ProbaDistClusterFactory::display(StatDistDisplayer* displayer) const
{
  unsigned int size = proba_dist_cluster_v.size();
  displayer->beginFactoryCluster();
  for (unsigned int nn = 0; nn < size; ++nn) {
    ProbaDistCluster* cluster = proba_dist_cluster_v[nn];

    displayer->beginCluster(nn+1, cluster->size());
    cluster->display(displayer);
    displayer->endCluster();
  }
  displayer->endFactoryCluster();
}

void ProbaDistCluster::displayStationaryDistribution(StatDistDisplayer* displayer) const
{
  unsigned int sz = size();
  const double minsquaredouble = DBL_MIN*DBL_MIN;
  for (const auto & stat_dist_entry : stat_dist_map) {
    const NetworkState& state = stat_dist_entry.first;
    const Proba& pb = stat_dist_entry.second;
    double proba = pb.proba/sz;
    double probaSquare = pb.probaSquare/sz;
    double vr = (probaSquare-proba*proba)/(sz-1); // EV 2014-10-07: in case of sz == 1, vr is nan
    double variance;
    if (vr < minsquaredouble || sz <= 1) { // EV 2014-10-07: sz <= 1 to avoid nan values
      variance = 0.0;
    } else {
      variance = sqrt(vr);
    }
    displayer->addProbaVariance(state.getState(), proba, variance);
  }
}

void ProbaDistClusterFactory::displayStationaryDistribution(StatDistDisplayer* displayer) const
{
  unsigned int size = proba_dist_cluster_v.size();
  displayer->beginClusterFactoryStationaryDistribution();
  for (unsigned int nn = 0; nn < size; ++nn) {
    ProbaDistCluster* cluster = proba_dist_cluster_v[nn];
    displayer->beginClusterStationaryDistribution(nn+1);
    cluster->displayStationaryDistribution(displayer);
    displayer->endClusterStationaryDistribution();
  }
  displayer->endClusterFactoryStationaryDistribution();
}
