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
     MaBEstEngine.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#include "MaBEstEngine.h"
#include "Probe.h"
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#include <iostream>

const std::string MaBEstEngine::VERSION = "2.5.6";

MaBEstEngine::MaBEstEngine(Network* network, RunConfig* runconfig) :
  ProbTrajEngine(network, runconfig)
  {

  if (thread_count > sample_count) {
    thread_count = sample_count;
  }

  if (thread_count > 1 && !runconfig->getRandomGeneratorFactory()->isThreadSafe()) {
    std::cerr << "Warning: non reentrant random, may not work properly in multi-threaded mode\n";
  }

  const std::vector<Node*>& nodes = network->getNodes();
  
  NetworkState internal_state;
  bool has_internal = false;
  refnode_count = 0;
  for (const auto * node : nodes) {
    if (node->isInternal()) {
      has_internal = true;
      internal_state.setNodeState(node, true);
    }
    if (node->isReference()) {
      reference_state.setNodeState(node, node->getReferenceState());
      refnode_mask.setNodeState(node, true);
      refnode_count++;
    }
  }

  merged_cumulator = NULL;
  cumulator_v.resize(thread_count);
  unsigned int count = sample_count / thread_count;
  unsigned int firstcount = count + sample_count - count * thread_count;

  unsigned int scount = statdist_trajcount / thread_count;
  unsigned int first_scount = scount + statdist_trajcount - scount * thread_count;

  for (unsigned int nn = 0; nn < thread_count; ++nn) {
    Cumulator<NetworkState>* cumulator = new Cumulator<NetworkState>(runconfig, runconfig->getTimeTick(), runconfig->getMaxTime(), (nn == 0 ? firstcount : count), (nn == 0 ? first_scount : scount ));
    if (has_internal) {
#ifdef USE_STATIC_BITSET
      NetworkState_Impl state2 = ~internal_state.getState();
      cumulator->setOutputMask(state2);
#else
      cumulator->setOutputMask(~internal_state.getState());
#endif
    }
    cumulator->setRefnodeMask(refnode_mask.getState());
    cumulator_v[nn] = cumulator;
  }
}

struct ArgWrapper {
  MaBEstEngine* mabest;
  unsigned int start_count_thread;
  unsigned int sample_count_thread;
  Cumulator<NetworkState>* cumulator;
  RandomGeneratorFactory* randgen_factory;
  long long int* elapsed_time;
  int seed;
  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map;
  std::ostream* output_traj;

  ArgWrapper(MaBEstEngine* mabest, unsigned int start_count_thread, unsigned int sample_count_thread, Cumulator<NetworkState>* cumulator, RandomGeneratorFactory* randgen_factory, long long int * elapsed_time, int seed, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map, std::ostream* output_traj) :
    mabest(mabest), start_count_thread(start_count_thread), sample_count_thread(sample_count_thread), cumulator(cumulator), randgen_factory(randgen_factory), elapsed_time(elapsed_time), seed(seed), fixpoint_map(fixpoint_map), output_traj(output_traj) { }
};

void* MaBEstEngine::threadWrapper(void *arg)
{
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif
  ArgWrapper* warg = (ArgWrapper*)arg;
  try {
    warg->mabest->runThread(warg->cumulator, warg->start_count_thread, warg->sample_count_thread, warg->randgen_factory, warg->elapsed_time, warg->seed, warg->fixpoint_map, warg->output_traj);
  } catch(const BNException& e) {
    std::cerr << e;
  }
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
#endif
  return NULL;
}

void MaBEstEngine::runThread(Cumulator<NetworkState>* cumulator, unsigned int start_count_thread, unsigned int sample_count_thread, RandomGeneratorFactory* randgen_factory, long long int* elapsed_time, int seed, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map, std::ostream* output_traj)
{
  const std::vector<Node*>& nodes = network->getNodes();
  std::vector<Node*>::const_iterator begin = nodes.begin();
  std::vector<Node*>::const_iterator end = nodes.end();
  NetworkState network_state; 
  Probe probe;
  probe.start();
  std::vector<double> nodeTransitionRates(nodes.size(), 0.0);
  RandomGenerator* random_generator = randgen_factory->generateRandomGenerator(seed);
  for (unsigned int nn = 0; nn < sample_count_thread; ++nn) {

    random_generator->setSeed(seed+start_count_thread+nn);
    cumulator->rewind();
    network->initStates(network_state, random_generator);
    double tm = 0.;
    if (NULL != output_traj) {
      (*output_traj) << "\nTrajectory #" << (nn+1) << '\n';
      (*output_traj) << " istate\t";
      network_state.displayOneLine(*output_traj, network);
      (*output_traj) << '\n';
    }
    while (tm < max_time) {
      double total_rate = 0.;
            nodeTransitionRates.assign(nodes.size(), 0.0);

      begin = nodes.begin();

      while (begin != end) {
	Node* node = *begin;
	NodeIndex node_idx = node->getIndex();
	if (node->getNodeState(network_state)) {
	  double r_down = node->getRateDown(network_state);
	  if (r_down != 0.0) {
	    total_rate += r_down;
	    nodeTransitionRates[node_idx] = r_down;
	  }
	} else {
	  double r_up = node->getRateUp(network_state);
	  if (r_up != 0.0) {
	    total_rate += r_up;
	    nodeTransitionRates[node_idx] = r_up;
	  }
	}
	++begin;
      }

      double TH;
      if (total_rate == 0) {
	tm = max_time;
	TH = 0.;
  STATE_MAP<NetworkState_Impl, unsigned int>::iterator iter = fixpoint_map->find(network_state.getState());
	if (iter == fixpoint_map->end()) {
	  (*fixpoint_map)[network_state.getState()] = 1;
	} else {
	  iter->second++;
	}
      } else {
	double transition_time ;
	if (discrete_time) {
	  transition_time = time_tick;
	} else {
	  double U_rand1 = random_generator->generate();
	  transition_time = -log(U_rand1) / total_rate;
	}
	
	tm += transition_time;
	TH = computeTH(network, nodeTransitionRates, total_rate);
      }

      if (NULL != output_traj) {
	(*output_traj) << std::setprecision(10) << tm << '\t';
	network_state.displayOneLine(*output_traj, network);
	(*output_traj) << '\t' << TH << '\n';
      }

      cumulator->cumul(network_state, tm, TH);

      if (tm >= max_time) {
	break;
      }

      NodeIndex node_idx = getTargetNode(network, random_generator, nodeTransitionRates, total_rate);
      network_state.flipState(network->getNode(node_idx));
    }
    cumulator->trajectoryEpilogue();
  }
  
  probe.stop();
  *elapsed_time = probe.elapsed_msecs();
  
  delete random_generator;
}

void MaBEstEngine::run(std::ostream* output_traj)
{
  pthread_t* tid = new pthread_t[thread_count];
  RandomGeneratorFactory* randgen_factory = runconfig->getRandomGeneratorFactory();
  int seed = runconfig->getSeedPseudoRandom();
#ifdef MPI_COMPAT
  unsigned int start_sample_count = sample_count * world_rank;
#else
  unsigned int start_sample_count = 0;
#endif

#ifdef MPI_COMPAT
  thread_elapsed_runtimes[world_rank].resize(thread_count);
#else
  thread_elapsed_runtimes.resize(thread_count);
#endif

  Probe probe;
  for (unsigned int nn = 0; nn < thread_count; ++nn) {
    STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
    fixpoint_map_v.push_back(fixpoint_map);

#ifdef MPI_COMPAT
    ArgWrapper* warg = new ArgWrapper(this, start_sample_count, cumulator_v[nn]->getSampleCount(), cumulator_v[nn], randgen_factory, &(thread_elapsed_runtimes[world_rank][nn]), seed, fixpoint_map, output_traj);
#else
    ArgWrapper* warg = new ArgWrapper(this, start_sample_count, cumulator_v[nn]->getSampleCount(), cumulator_v[nn], randgen_factory, &(thread_elapsed_runtimes[nn]), seed, fixpoint_map, output_traj);
#endif

    pthread_create(&tid[nn], NULL, MaBEstEngine::threadWrapper, warg);
    arg_wrapper_v.push_back(warg);

    start_sample_count += cumulator_v[nn]->getSampleCount();
  }
  for (unsigned int nn = 0; nn < thread_count; ++nn) {
    pthread_join(tid[nn], NULL);
  }
  probe.stop();
  elapsed_core_runtime = probe.elapsed_msecs();
  user_core_runtime = probe.user_msecs();
  
  probe.start();
  epilogue();
  probe.stop();
  elapsed_epilogue_runtime = probe.elapsed_msecs();
  user_epilogue_runtime = probe.user_msecs();
  
#ifdef MPI_COMPAT
  
  if (world_rank == 0)
  {
    elapsed_core_runtimes.resize(world_size);
    user_core_runtimes.resize(world_size);
    elapsed_epilogue_runtimes.resize(world_size);
    user_epilogue_runtimes.resize(world_size);
  
  }
  
  MPI_Gather(&elapsed_core_runtime, 1, MPI_LONG_LONG_INT, elapsed_core_runtimes.data(), 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
  MPI_Gather(&user_core_runtime, 1, MPI_LONG_LONG_INT, user_core_runtimes.data(), 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);  
  MPI_Gather(&elapsed_epilogue_runtime, 1, MPI_LONG_LONG_INT, elapsed_epilogue_runtimes.data(), 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
  MPI_Gather(&user_epilogue_runtime, 1, MPI_LONG_LONG_INT, user_epilogue_runtimes.data(), 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
  
#endif
  
  
  delete [] tid;
}  

void MaBEstEngine::displayRunStats(std::ostream& os, time_t start_time, time_t end_time) const {

#ifdef MPI_COMPAT
if (getWorldRank() == 0) {
#endif
  
  const char sepfmt[] = "-----------------------------------------------%s-----------------------------------------------\n";
  char bufstr[1024];

  os << '\n';
  snprintf(bufstr, 1024, sepfmt, "--- Run ---");
  os << bufstr;

  os << "MaBoSS version: " << VERSION << " [networks up to " << MAXNODES << " nodes]\n";
  os << "\nRun start time: " << ctime(&start_time);
  os << "Run end time: " << ctime(&end_time);

  os << "\nCore user runtime: " << (getUserCoreRunTime()/1000.) << " secs using " << thread_count << " thread" << (thread_count > 1 ? "s" : "") << "\n";
  os << "Core elapsed runtime: " << (getElapsedCoreRunTime()/1000.) << " secs using " << thread_count << " thread" << (thread_count > 1 ? "s" : "") << "\n\n";

  os << "Epilogue user runtime: " << (getUserEpilogueRunTime()/1000.) << " secs using 1 thread\n";
  os << "Epilogue elapsed runtime: " << (getElapsedEpilogueRunTime()/1000.) << " secs using 1 thread\n\n";

  os << "StatDist user runtime: " << (getUserStatDistRunTime()/1000.) << " secs using 1 thread\n";
  os << "StatDist elapsed runtime: " << (getElapsedStatDistRunTime()/1000.) << " secs using 1 thread\n\n";
  
  runconfig->display(network, start_time, end_time, os);

#ifdef MPI_COMPAT
}
#endif 

}

void MaBEstEngine::epilogue()
{
  std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> results = mergeResults(cumulator_v, fixpoint_map_v);
  merged_cumulator = results.first;
  fixpoints = *(results.second);

#ifdef MPI_COMPAT
  
  std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> mpi_results = mergeMPIResults(runconfig, merged_cumulator, &fixpoints, world_size, world_rank);
  merged_cumulator = mpi_results.first;
  fixpoints = *(mpi_results.second);
  
  if (world_rank == 0)
  {
#endif

  merged_cumulator->epilogue(network, reference_state);
  
#ifdef MPI_COMPAT
  }
#endif 
}

MaBEstEngine::~MaBEstEngine()
{
  delete fixpoint_map_v[0];
  
  for (auto t_arg_wrapper: arg_wrapper_v)
    delete t_arg_wrapper;

  delete merged_cumulator;
}

