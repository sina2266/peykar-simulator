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
     EnsembleEngine.cc

   Authors:
     Vincent Noel <contact@vincent-noel.fr>
 
   Date:
     March 2019
*/

#include "EnsembleEngine.h"
#include "Probe.h"
#include "Utils.h"
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#ifndef WINDOWS
  #include <dlfcn.h>
#else
  #include <windows.h>
#endif
#include <iostream>

const std::string EnsembleEngine::VERSION = "1.2.0";

EnsembleEngine::EnsembleEngine(std::vector<Network*> networks, RunConfig* runconfig, bool save_individual_result, bool _random_sampling) :
  ProbTrajEngine(networks[0], runconfig), networks(networks), save_individual_result(save_individual_result), random_sampling(_random_sampling) {

  if (thread_count > 1 && !runconfig->getRandomGeneratorFactory()->isThreadSafe()) {
    std::cerr << "Warning: non reentrant random, may not work properly in multi-threaded mode\n";
  }
  
  const std::vector<Node*>& nodes = networks[0]->getNodes();
  
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

  simulation_indices_v.resize(thread_count); // Per thread
  std::vector<unsigned int> simulations_per_model(networks.size(), 0);

  // Here we write a dict with the number of simulation by model
  unsigned int network_index;
  if (random_sampling)
  {
    // Here we need the random generator to compute the list of simulations
    
#ifdef MPI_COMPAT
    // If MPI, then we only do it in node 0, then we broadcast it
    if (world_rank == 0)
    {
#endif
    
      RandomGeneratorFactory* randgen_factory = runconfig->getRandomGeneratorFactory();
      int seed = runconfig->getSeedPseudoRandom();
      RandomGenerator* random_generator = randgen_factory->generateRandomGenerator(seed);
    
#ifdef MPI_COMPAT
      for (unsigned int nn = 0; nn < global_sample_count; nn++) {
#else
      for (unsigned int nn = 0; nn < sample_count; nn++) {
#endif
        // This will need sample_count random numbers... maybe there is another way ?
        // TODO : Actually we can, by generating the number of simulation per model, 
        // so only needing network.size() random numbers. 
        // Should be generate()*2*(sample_count/networks.size()) (since expectancy of generate() is 0.5)
        network_index = (unsigned int) floor(random_generator->generate()*networks.size());
        simulations_per_model[network_index] += 1;
      }
      
      delete random_generator;
      
#ifdef MPI_COMPAT
      MPI_Bcast(simulations_per_model.data(), networks.size(), MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    
    } else {
      MPI_Bcast(simulations_per_model.data(), networks.size(), MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    }
#endif
  } else{
    for (unsigned int nn = 0; nn < networks.size(); ++nn) {
      if (nn == 0) {
#ifdef MPI_COMPAT    
        simulations_per_model[nn] = floor(global_sample_count/networks.size()) + (global_sample_count % networks.size());
#else
        simulations_per_model[nn] = floor(sample_count/networks.size()) + (sample_count % networks.size());
#endif
      } else {
#ifdef MPI_COMPAT
        simulations_per_model[nn] = floor(global_sample_count/networks.size());
#else
        simulations_per_model[nn] = floor(sample_count/networks.size());
#endif
      }
    }
  }
  
  
#ifdef MPI_COMPAT

  std::vector<unsigned int> local_simulations_per_model(networks.size(), 0);
  unsigned int start_model = 0;
  unsigned int start_sim = world_rank * (global_sample_count / world_size) + (world_rank > 0 ? global_sample_count % world_size : 0);
  unsigned int end_sim = start_sim + sample_count - 1;
  
  unsigned int i=0;
  unsigned int local_sum = 0;
  
  
  for (auto nb_sim: simulations_per_model) {
    
    unsigned int end_model = start_model + nb_sim - 1;
    if (end_model >= start_sim && start_model <= end_sim) {
    
      unsigned int start = start_model < start_sim ? start_sim : start_model;
      unsigned int end = end_model >= end_sim ? end_sim : end_model;
      
      local_simulations_per_model[i] += end - start + 1;
      local_sum += end - start + 1;
      
    }
    start_model += nb_sim;
    i++;
  }
  
#endif 
  cumulator_models_v.resize(thread_count); // Per thread
  fixpoints_models_v.resize(thread_count);

  if (save_individual_result) {
    cumulators_thread_v.resize(networks.size());
    fixpoints_threads_v.resize(networks.size());
  }

  unsigned int count = sample_count / thread_count;
  unsigned int firstcount = count + sample_count - count * thread_count;
  
  unsigned int scount = statdist_trajcount / thread_count;
  unsigned int first_scount = scount + statdist_trajcount - scount * thread_count;

  unsigned int position = 0;
  unsigned int offset = 0;
  for (unsigned int nn = 0; nn < thread_count; ++nn) {
    unsigned int t_count = (nn == 0 ? firstcount : count);
    Cumulator<NetworkState>* cumulator = new Cumulator<NetworkState>(runconfig, runconfig->getTimeTick(), runconfig->getMaxTime(), t_count, (nn == 0 ? first_scount : scount ));
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

    // Setting the size of the list of indices to the thread's sample count
    simulation_indices_v[nn].resize(t_count); 

    // Here we build the indice of simulation for this thread
    // We have two counters : 
    // - nnn, which is the counter of simulation indices
    // - j, k which are the index of the model, and the counter of repetition of this model
    unsigned int nnn = 0; // Simulation indice (< t_count, thus inside a thread)
    unsigned int j = position; // Model indice
    unsigned int k = offset; // Indice of simulation for a model

    // Aside from the simulation indice, we are building
    // a dict with the number of model simulations by model
    int m = 0;
    std::vector<unsigned int> count_by_models;
    
    while(nnn < t_count) {
      assert(j <= networks.size());
      // If we assigned all the simulation of the model j
#ifdef MPI_COMPAT
      if (k == local_simulations_per_model[j])
#else
      if (k == simulations_per_model[j])
#endif
      {
        if (k > 0) { // If we indeed assigned something

          // We add another element to the vector
          // count_by_models.resize(count_by_models.size()+1); 
     
          // If this is the first model assigned in this thread
          if (m == 0) {
            // Then we need to count those who were assigned in the previous thread
            // count_by_models[m] = k - offset;
            count_by_models.push_back(k - offset);
          } else {
            // Otherwise we did assigned them all in this thread
            // count_by_models[m] = k;
            count_by_models.push_back(k);
          }
          
          m++; // One model assigned ! 
        }
        
        j++; // One model completely assigned
        k = 0; // We reset the model simulation counter
      // }
      // Otherwise, we keep assigning them
      } else {
        simulation_indices_v[nn][nnn] = j;
        k++; // One model simulation assigned
        nnn++; // One thread simulation assigned
      }
    }

    // If we didn't finished assigning this model, 
    // then we need to put in the counts up to where we went
    if (k > 0) {
      // We add another element to the vector
      // count_by_models.resize(count_by_models.size()+1); 

      // // If this is the first model assigned in this thread
      if (m == 0) {
        // Then we need to substract those who were assigned in the previous thread
        // count_by_models[m] = k - offset;
        count_by_models.push_back(k - offset);
      } else {
        // Otherwise we did assigned them all in this thread
        // count_by_models[m] = k;
        count_by_models.push_back(k);
      }
    }

    // Here we update the position and offset for the next thread
    // If we just finished a model, the position will be set to the next model
#ifdef MPI_COMPAT
    if (k == local_simulations_per_model[j]) {
#else
    if (k == simulations_per_model[j]) {
#endif
      offset = 0;
      position = ++j;

    // Otherwise we keep with this model
    } else {
      offset = k;
      position = j;
    }

    // If we want to save the individual trajectories, then we 
    // initialize the cumulators, and we add them to the vector of 
    // cumulators by model
    unsigned int c = 0;
    if (save_individual_result) {
      cumulator_models_v[nn].resize(count_by_models.size());
      fixpoints_models_v[nn].resize(count_by_models.size());

      for (nnn = 0; nnn < count_by_models.size(); nnn++) {
        if (count_by_models[nnn] > 0) {
          Cumulator<NetworkState>* t_cumulator = new Cumulator<NetworkState>(
            runconfig, runconfig->getTimeTick(), runconfig->getMaxTime(), count_by_models[nnn], statdist_trajcount
          );
          if (has_internal) {

#ifdef USE_STATIC_BITSET
          NetworkState_Impl state2 = ~internal_state.getState();
          t_cumulator->setOutputMask(state2);
#else
          t_cumulator->setOutputMask(~internal_state.getState());
#endif
          }
          t_cumulator->setRefnodeMask(refnode_mask.getState());          
          cumulator_models_v[nn][nnn] = t_cumulator;
          cumulators_thread_v[simulation_indices_v[nn][c]].push_back(t_cumulator);
        

          STATE_MAP<NetworkState_Impl, unsigned int>* t_fixpoints_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
          fixpoints_models_v[nn][nnn] = t_fixpoints_map;
          fixpoints_threads_v[simulation_indices_v[nn][c]].push_back(t_fixpoints_map);
        }
        c += count_by_models[nnn];
      }
    }
  }
}

struct EnsembleArgWrapper {
  EnsembleEngine* mabest;
  unsigned int start_count_thread;
  unsigned int sample_count_thread;
  Cumulator<NetworkState>* cumulator;

  std::vector<unsigned int> simulations_per_model;
  std::vector<Cumulator<NetworkState>*> models_cumulators;
  std::vector<STATE_MAP<NetworkState_Impl, unsigned int>* > models_fixpoints;
  
  RandomGeneratorFactory* randgen_factory;
  int seed;
  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map;
  std::ostream* output_traj;

  EnsembleArgWrapper(
    EnsembleEngine* mabest, unsigned int start_count_thread, unsigned int sample_count_thread, 
    Cumulator<NetworkState>* cumulator, std::vector<unsigned int> simulations_per_model, 
    std::vector<Cumulator<NetworkState>*> models_cumulators, std::vector<STATE_MAP<NetworkState_Impl, unsigned int>* > models_fixpoints,
    RandomGeneratorFactory* randgen_factory, int seed, 
    STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map, std::ostream* output_traj) :

      mabest(mabest), start_count_thread(start_count_thread), sample_count_thread(sample_count_thread), 
      cumulator(cumulator), simulations_per_model(simulations_per_model), 
      models_cumulators(models_cumulators), models_fixpoints(models_fixpoints),
      randgen_factory(randgen_factory), seed(seed), 
      fixpoint_map(fixpoint_map), output_traj(output_traj) {

    }
};

void* EnsembleEngine::threadWrapper(void *arg)
{
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif
  EnsembleArgWrapper* warg = (EnsembleArgWrapper*)arg;
  try {
    warg->mabest->runThread(
      warg->cumulator, warg->start_count_thread, warg->sample_count_thread, 
      warg->randgen_factory, warg->seed, warg->fixpoint_map, warg->output_traj, 
      warg->simulations_per_model, warg->models_cumulators, warg->models_fixpoints);
  } catch(const BNException& e) {
    std::cerr << e;
  }
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
#endif
  return NULL;
}

void EnsembleEngine::runThread(Cumulator<NetworkState>* cumulator, unsigned int start_count_thread, unsigned int sample_count_thread, 
  RandomGeneratorFactory* randgen_factory, int seed, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map, std::ostream* output_traj, 
  std::vector<unsigned int> simulation_ind, std::vector<Cumulator<NetworkState>*> t_models_cumulators, std::vector<STATE_MAP<NetworkState_Impl, unsigned int>* > t_models_fixpoints)
{
  NetworkState network_state; 
  
  int model_ind = 0;
  RandomGenerator* random_generator = randgen_factory->generateRandomGenerator(seed);
  const std::vector<Node*>& nodes = networks[0]->getNodes();
  std::vector<double> nodeTransitionRates(nodes.size(), 0.0);
  for (unsigned int nn = 0; nn < sample_count_thread; ++nn) {

#ifdef MPI_COMPAT
    random_generator->setSeed(seed+(world_rank * (global_sample_count/world_size) + (world_rank > 0 ? global_sample_count % world_size : 0)) + start_count_thread+nn);
#else
    random_generator->setSeed(seed+start_count_thread+nn);
#endif
    unsigned int network_index = simulation_ind[nn];

    if (nn > 0 && network_index != simulation_ind[nn-1]) {
      model_ind++;
    }

    Network* network = networks[network_index];
    const std::vector<Node*>& nodes = network->getNodes();
    std::vector<Node*>::const_iterator begin = nodes.begin();
    // std::vector<Node*>::const_iterator end = nodes.end();
  
    cumulator->rewind();
    if (save_individual_result){
      t_models_cumulators[model_ind]->rewind();
    }
    
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

      while (begin != nodes.end()) {
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

        if (save_individual_result) {
          STATE_MAP<NetworkState_Impl, unsigned int>* t_fixpoint_map = t_models_fixpoints[model_ind];
          STATE_MAP<NetworkState_Impl, unsigned int>::iterator iter = t_fixpoint_map->find(network_state.getState());
          if (iter == t_fixpoint_map->end()) {
            (*t_fixpoint_map)[network_state.getState()] = 1;
          } else {
            iter->second++;
          }
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
      if (save_individual_result){
        t_models_cumulators[model_ind]->cumul(network_state, tm, TH);
      }

      if (tm >= max_time) {
	      break;
      }

      NodeIndex node_idx = getTargetNode(network, random_generator, nodeTransitionRates, total_rate);
      network_state.flipState(network->getNode(node_idx));
    }

    cumulator->trajectoryEpilogue();
    if (save_individual_result){
      t_models_cumulators[model_ind]->trajectoryEpilogue();
    }
  }
  delete random_generator;
}

void EnsembleEngine::run(std::ostream* output_traj)
{
  pthread_t* tid = new pthread_t[thread_count];
  RandomGeneratorFactory* randgen_factory = runconfig->getRandomGeneratorFactory();
  int seed = runconfig->getSeedPseudoRandom();
  unsigned int start_sample_count = 0;
  Probe probe;
  for (unsigned int nn = 0; nn < thread_count; ++nn) {
    STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
    fixpoint_map_v.push_back(fixpoint_map);
    EnsembleArgWrapper* warg = new EnsembleArgWrapper(this, start_sample_count, cumulator_v[nn]->getSampleCount(), cumulator_v[nn], simulation_indices_v[nn], cumulator_models_v[nn], fixpoints_models_v[nn], randgen_factory, seed, fixpoint_map, output_traj);
    pthread_create(&tid[nn], NULL, EnsembleEngine::threadWrapper, warg);
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
  delete [] tid;
}  

#ifdef MPI_COMPAT

void EnsembleEngine::mergeMPIIndividual(bool pack) 
{
  if (world_size > 1) {
    for (unsigned int model=0; model < networks.size(); model++) {
      
      std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> results = mergeMPIResults(runconfig, cumulators_per_model[model], fixpoints_per_model[model], world_size, world_rank);
      cumulators_per_model[model] = results.first;
      fixpoints_per_model[model] = results.second;

      if (world_rank == 0)
        cumulators_per_model[model]->epilogue(networks[model], reference_state);
    }
  }
}
#endif


void EnsembleEngine::epilogue()
{
  std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> results = mergeResults(cumulator_v, fixpoint_map_v);
  merged_cumulator = results.first;
  fixpoints = *(results.second);

#ifdef MPI_COMPAT
  std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> mpi_results = mergeMPIResults(runconfig, merged_cumulator, &fixpoints, world_size, world_rank);
  merged_cumulator = mpi_results.first;
  fixpoints = *(mpi_results.second);
  
  if (world_rank == 0){
#endif

    merged_cumulator->epilogue(networks[0], reference_state);

#ifdef MPI_COMPAT
  }
#endif
  
  if (save_individual_result) 
  {
    mergeIndividual();
    
#ifdef MPI_COMPAT
    mergeMPIIndividual();
#endif    
  } 

}

void EnsembleEngine::mergeIndividual() {
  cumulators_per_model.resize(networks.size(), NULL);
  fixpoints_per_model.resize(networks.size(), NULL);

  for (unsigned int i=0; i < networks.size(); i++) {
    
    std::pair<Cumulator<NetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> results = mergeResults(cumulators_thread_v[i], fixpoints_threads_v[i]);
    cumulators_per_model[i] = results.first;
    fixpoints_per_model[i] = results.second;
    
    if (cumulators_per_model[i] != NULL)
      cumulators_per_model[i]->epilogue(networks[i], reference_state);
  }
}

void EnsembleEngine::displayIndividualFixpoints(unsigned int model_id, FixedPointDisplayer* displayer) const 
{
#ifdef MPI_COMPAT
  if (world_rank == 0) {
#endif
  
  if (fixpoints_per_model[model_id] != NULL) {
    displayer->begin(fixpoints_per_model[model_id]->size());

    size_t nn = 0;
    for (const auto & fixpoint: *fixpoints_per_model[model_id]) {
      displayer->displayFixedPoint(nn+1, fixpoint.first, fixpoint.second, sample_count);
      ++nn;
    }
  } else {
    displayer->begin(0);
  }
  displayer->end();
#ifdef MPI_COMPAT
  }
#endif
}

void EnsembleEngine::displayIndividual(unsigned int model_id, ProbTrajDisplayer<NetworkState>* probtraj_displayer, StatDistDisplayer* statdist_displayer, FixedPointDisplayer* fp_displayer) const
{
#ifdef MPI_COMPAT
if (world_rank == 0){
#endif

  if (cumulators_per_model[model_id] != NULL) {
    cumulators_per_model[model_id]->displayProbTraj(networks[model_id], refnode_count, probtraj_displayer);
    cumulators_per_model[model_id]->displayStatDist(networks[model_id], refnode_count, statdist_displayer);
  }

  displayIndividualFixpoints(model_id, fp_displayer);

#ifdef MPI_COMPAT
}
#endif
}

EnsembleEngine::~EnsembleEngine()
{
  delete fixpoint_map_v[0];
  
  for (auto * t_arg_wrapper: arg_wrapper_v)
    delete t_arg_wrapper;
  
  delete merged_cumulator;

  for (auto * t_cumulator: cumulators_per_model) 
    delete t_cumulator;

  for (auto * t_fixpoint: fixpoints_per_model)
    delete t_fixpoint;
}
