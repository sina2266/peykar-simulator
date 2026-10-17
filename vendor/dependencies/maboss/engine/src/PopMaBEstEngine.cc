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
     PopMaBEstEngine.cc

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     March 2021
*/

#include "PopMaBEstEngine.h"
#include "Probe.h"
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#include <iostream>
#include "Utils.h"
#ifndef WINDOWS
#include <dlfcn.h>
#else
#include <windows.h>
#endif

static const char *MABOSS_USER_FUNC_INIT = "maboss_user_func_init";

const std::string PopMaBEstEngine::VERSION = "0.0.1";
int PopMaBEstEngine::verbose = 0;
// extern size_t RandomGenerator::generated_number_count;

void PopMaBEstEngine::setVerbose(int level) {
  PopMaBEstEngine::verbose = level;
}

PopMaBEstEngine::PopMaBEstEngine(PopNetwork *pop_network, RunConfig *runconfig) : pop_network(pop_network), runconfig(runconfig),
                                                                           time_tick(runconfig->getTimeTick()),
                                                                           max_time(runconfig->getMaxTime()),
                                                                           sample_count(runconfig->getSampleCount()),
                                                                           statdist_trajcount(runconfig->getStatDistTrajCount()),
                                                                           discrete_time(runconfig->isDiscreteTime()),
                                                                           thread_count(runconfig->getThreadCount())
{
  elapsed_core_runtime = user_core_runtime = elapsed_statdist_runtime = user_statdist_runtime = elapsed_epilogue_runtime = user_epilogue_runtime = 0;

  if (thread_count > sample_count)
  {
    thread_count = sample_count;
  }

  if (thread_count > 1 && !runconfig->getRandomGeneratorFactory()->isThreadSafe())
  {
    std::cerr << "Warning: non reentrant random, may not work properly in multi-threaded mode\n";
  }

  const std::vector<Node *> &nodes = pop_network->getNodes();
  NetworkState internal_state;
  bool has_internal = false;
  refnode_count = 0;
  for (const auto * node : nodes)
  {
    if (node->isInternal())
    {
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

  for (unsigned int nn = 0; nn < thread_count; ++nn)
  {
    Cumulator<PopNetworkState> *cumulator = new Cumulator<PopNetworkState>(runconfig, runconfig->getTimeTick(), runconfig->getMaxTime(), (nn == 0 ? firstcount : count), (nn == 0 ? first_scount : scount ));
    if (has_internal)
    {
#ifdef USE_STATIC_BITSET
      NetworkState_Impl state2 = ~internal_state.getState();
      cumulator->setOutputMask(PopNetworkState(state2, 1));
#else
      cumulator->setOutputMask(PopNetworkState(~internal_state.getState(), 1));
#endif
    }
    cumulator_v[nn] = cumulator;
  }
}

PopNetworkState PopMaBEstEngine::getTargetNode(RandomGenerator *random_generator, const PopNetworkStateMap& popNodeTransitionRates, double total_rate) const
{
  double U_rand2 = random_generator->generate();
  double random_rate = U_rand2 * total_rate;
  PopNetworkState result;
  for (const auto & node_tr_rate : popNodeTransitionRates) 
  {
    double rate = node_tr_rate.second;
    random_rate -= rate;
    result = node_tr_rate.first;
    
    if (random_rate <= 0)
      break;
  }

  return result;
}

double PopMaBEstEngine::computeTH(const MAP<NodeIndex, double> &nodeTransitionRates, double total_rate) const
{
  if (nodeTransitionRates.size() == 1)
  {
    return 0.;
  }

  MAP<NodeIndex, double>::const_iterator begin = nodeTransitionRates.begin();

  double TH = 0.;
  double rate_internal = 0.;

  while (begin != nodeTransitionRates.end())
  {
    NodeIndex index = (*begin).first;
    double rate = (*begin).second;
    if (pop_network->getNode(index)->isInternal())
    {
      rate_internal += rate;
    }
    ++begin;
  }

  double total_rate_non_internal = total_rate - rate_internal;

  begin = nodeTransitionRates.begin();

  while (begin != nodeTransitionRates.end())
  {
    NodeIndex index = (*begin).first;
    double rate = (*begin).second;
    if (!pop_network->getNode(index)->isInternal())
    {
      double proba = rate / total_rate_non_internal;
      TH -= log2(proba) * proba;
    }
    ++begin;
  }

  return TH;
}

struct ArgWrapper
{
  PopMaBEstEngine *mabest;
  unsigned int start_count_thread;
  unsigned int sample_count_thread;
  Cumulator<PopNetworkState> *cumulator;
  RandomGeneratorFactory *randgen_factory;
  int seed;
  STATE_MAP<NetworkState_Impl, unsigned int> *fixpoint_map;
  std::ostream *output_traj;

  ArgWrapper(PopMaBEstEngine *mabest, unsigned int start_count_thread, unsigned int sample_count_thread, Cumulator<PopNetworkState> *cumulator, RandomGeneratorFactory *randgen_factory, int seed, STATE_MAP<NetworkState_Impl, unsigned int> *fixpoint_map, std::ostream *output_traj) : mabest(mabest), start_count_thread(start_count_thread), sample_count_thread(sample_count_thread), cumulator(cumulator), randgen_factory(randgen_factory), seed(seed), fixpoint_map(fixpoint_map), output_traj(output_traj) {}
};

void *PopMaBEstEngine::threadWrapper(void *arg)
{
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif
  ArgWrapper *warg = (ArgWrapper *)arg;
  try
  {
    warg->mabest->runThread(warg->cumulator, warg->start_count_thread, warg->sample_count_thread, warg->randgen_factory, warg->seed, warg->fixpoint_map, warg->output_traj);
  }
  catch (const BNException &e)
  {
    std::cerr << e;
  }
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
#endif
  return NULL;
}

void PopMaBEstEngine::runThread(Cumulator<PopNetworkState> *cumulator, unsigned int start_count_thread, unsigned int sample_count_thread, RandomGeneratorFactory *randgen_factory, int seed, STATE_MAP<NetworkState_Impl, unsigned int> *fixpoint_map, std::ostream *output_traj)
{
  const std::vector<Node *> &nodes = pop_network->getNodes();
  PopNetworkState pop_network_state;

  RandomGenerator *random_generator = randgen_factory->generateRandomGenerator(seed);
  for (unsigned int nn = 0; nn < sample_count_thread; ++nn)
  {
    random_generator->setSeed(seed + start_count_thread + nn);
    cumulator->rewind();
    if (verbose > 1) 
      std::cout << "> New simulation" << std::endl;
  
    pop_network->initPopStates(pop_network_state, random_generator, runconfig->getInitPop());
    
    if (verbose > 1) {
      std::cout << ">> Initial state : ";
      pop_network_state.displayOneLine(std::cout, pop_network);
      std::cout << std::endl;
    }
      
    double tm = 0.;
    if (NULL != output_traj) {
      (*output_traj) << "\nTrajectory #" << (nn+1) << '\n';
      (*output_traj) << " istate\t";
      pop_network_state.displayOneLine(*output_traj, pop_network);
      (*output_traj) << '\n';
    }
    while (tm < max_time)
    {
      double total_rate = 0.;
      
      PopNetworkStateMap popNodeTransitionRates;
      // forall S ∈ Σ such that ψ(S) > 0 do
      for (auto pop : pop_network_state.getMap())
      {
        if (pop.second > 0)
        {

          NetworkState t_network_state(pop.first);

          double total_pop_rate = 0.;
          // forall S' such that there is only one different node state compare to S do
          for (auto node : nodes)
          {

            double nodeTransitionRate = 0;
            if (node->getNodeState(t_network_state))
            {
              double r_down = node->getRateDown(t_network_state, pop_network_state);
              if (r_down != 0.0)
              {
                total_pop_rate += r_down;
                nodeTransitionRate = r_down;
              }
            }
            else
            {
              double r_up = node->getRateUp(t_network_state, pop_network_state);
              if (r_up != 0.0)
              {
                total_pop_rate += r_up;
                nodeTransitionRate = r_up;
              }
            }

            // if ρS→S' > 0 then
            if (nodeTransitionRate > 0.0)
            {
              // Construct ψ' from (ψ, S and S')
              // ψ'(S'') ≡ ψ(S'' ), ∀S'' != (S, S')
              PopNetworkState new_pop_network_state = PopNetworkState(pop_network_state);
              
              // ψ'(S) ≡ ψ(S) − 1
              new_pop_network_state.decr(t_network_state);
              
              // ψ'(S') ≡ ψ(S') + 1
              NetworkState new_network_state = t_network_state;
              new_network_state.flipState(node);
              new_pop_network_state.incr(new_network_state); 
              
              // Compute the transition rate ρψ→ψ' using
              // ρψ→ψ' ≡ ψ(S)ρS→S'
              nodeTransitionRate *= pop.second;

              total_rate += nodeTransitionRate;
              // Put (ψ' , ρψ→ψ' ) in Ω
              popNodeTransitionRates.insert(std::pair<PopNetworkState, double>(new_pop_network_state, nodeTransitionRate));
            }
          }
          
          // forall Possible division do
          for (auto division_rule: pop_network->getDivisionRules()) {
            
            // Compute the transition rate of cell division from MaBoSS language, ρS→division
            double division_rate = division_rule.getRate(pop.first, pop_network_state);
            
            // if ρS→division > 0 then
            if (division_rate > 0){
              
              
              // Construct the two daughter cell states S' and S'' using MaBoSS language
              // Construct the new state ψ 0 
              PopNetworkState new_pop_network_state = PopNetworkState(pop_network_state);
              new_pop_network_state.decr(t_network_state);
              
              NetworkState state_daughter1 = division_rule.applyRules(DivisionRule::DAUGHTER_1, pop.first, pop_network_state);
              new_pop_network_state.incr(state_daughter1);
              
              NetworkState state_daughter2 = division_rule.applyRules(DivisionRule::DAUGHTER_2, pop.first, pop_network_state);
              new_pop_network_state.incr(state_daughter2);
              
              // Compute the transition rate ρψ→ψ' = ψ(S)ρS→division
              division_rate *= pop.second;
              
              // Put (ψ' , ρψ→ψ' ) in Ω
              popNodeTransitionRates.insert(std::pair<PopNetworkState, double>(new_pop_network_state, division_rate));
              total_rate += division_rate;
              
            }
          }
          double rate_death = pop_network->getDeathRate(pop.first, pop_network_state);
          if (rate_death > 0){
            rate_death *= pop.second;
            total_rate += rate_death;
            
            PopNetworkState new_pop_network_state = PopNetworkState(pop_network_state);
            new_pop_network_state.decr(t_network_state);
            popNodeTransitionRates.insert(std::pair<PopNetworkState, double>(new_pop_network_state, rate_death));
          }
          
          if (total_pop_rate == 0)
          {
            STATE_MAP<NetworkState_Impl, unsigned int>::iterator iter = fixpoint_map->find(t_network_state.getState());
            if (iter == fixpoint_map->end())
            {
              (*fixpoint_map)[t_network_state.getState()] = 1;
            }
            else
            {
              iter->second++;
            }
          }
        }
      }
      
      if (verbose > 2) {
        for (const auto &transition : popNodeTransitionRates)
        {
          std::cout << ">>> Transition : ";
          PopNetworkState t_state(transition.first);
          t_state.displayOneLine(std::cout, pop_network);
          std::cout << ", proba=" << (int)(100*transition.second/total_rate) << std::endl;
        }
      }
      double TH;
      if (total_rate == 0)
      {
        tm = max_time;
        TH = 0.;
      }
      else
      {
        double transition_time;
        if (discrete_time)
        {
          transition_time = time_tick;
        }
        else
        {
          double U_rand1 = random_generator->generate();
          transition_time = -log(U_rand1) / total_rate;
        }

        tm += transition_time;
        // Commenting for now
        TH = 0.;
        // TH = computeTH(nodeTransitionRates, total_rate);
      }

      if (NULL != output_traj) {
        (*output_traj) << std::setprecision(10) << tm << '\t';
        pop_network_state.displayOneLine(*output_traj, pop_network);
        (*output_traj) << '\t' << TH << '\n';
      }

      cumulator->cumul(pop_network_state, tm, TH);

      if (tm >= max_time)
      {
        break;
      }

      pop_network_state = getTargetNode(random_generator, popNodeTransitionRates, total_rate);

      if (verbose > 0) 
        std::cout << "> time = " << tm << std::endl;
        
      if (verbose > 1) {
        std::cout << ">> Present state : ";
        pop_network_state.displayOneLine(std::cout, pop_network);
        std::cout << std::endl;
      }
      
    }
    if (verbose > 0)
      std::cout << std::endl;
      
    cumulator->trajectoryEpilogue();
  }
  delete random_generator;
}

void PopMaBEstEngine::run(std::ostream *output_traj)
{
  pthread_t *tid = new pthread_t[thread_count];
  RandomGeneratorFactory *randgen_factory = runconfig->getRandomGeneratorFactory();
  int seed = runconfig->getSeedPseudoRandom();
  unsigned int start_sample_count = 0;
  Probe probe;
  for (unsigned int nn = 0; nn < thread_count; ++nn)
  {
    STATE_MAP<NetworkState_Impl, unsigned int> *fixpoint_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
    fixpoint_map_v.push_back(fixpoint_map);
    ArgWrapper *warg = new ArgWrapper(this, start_sample_count, cumulator_v[nn]->getSampleCount(), cumulator_v[nn], randgen_factory, seed, fixpoint_map, output_traj);
    pthread_create(&tid[nn], NULL, PopMaBEstEngine::threadWrapper, warg);
    arg_wrapper_v.push_back(warg);

    start_sample_count += cumulator_v[nn]->getSampleCount();
  }
  for (unsigned int nn = 0; nn < thread_count; ++nn)
  {
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
  delete[] tid;
}

STATE_MAP<NetworkState_Impl, unsigned int> *PopMaBEstEngine::mergeFixpointMaps()
{
  if (1 == fixpoint_map_v.size())
  {
    return new STATE_MAP<NetworkState_Impl, unsigned int>(*fixpoint_map_v[0]);
  }

  STATE_MAP<NetworkState_Impl, unsigned int> *fixpoint_map = new STATE_MAP<NetworkState_Impl, unsigned int>();
  for (auto * fp_map : fixpoint_map_v)
  {
    for (const auto & fp : *fp_map) 
    {
      const NetworkState_Impl &state = fp.first;
      STATE_MAP<NetworkState_Impl, unsigned int>::iterator iter = fixpoint_map->find(state);
      if (iter == fixpoint_map->end())
      {
        (*fixpoint_map)[state] = fp.second;
      }
      else
      {
        iter->second += fp.second;
      }
   }
  }
  return fixpoint_map;
}


void PopMaBEstEngine::mergePairOfFixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2)
{
  for (const auto& fixpoint: *fixpoints_2) {
    
    STATE_MAP<NetworkState_Impl, unsigned int>::iterator iter = fixpoints_1->find(fixpoint.first);
    if (iter == fixpoints_1->end()) {
      (*fixpoints_1)[fixpoint.first] = fixpoint.second;
    
    } else {
      iter->second += fixpoint.second;
    
    }
  }
  delete fixpoints_2; 
}

struct PopMergeWrapper {
  Cumulator<PopNetworkState>* cumulator_1;
  Cumulator<PopNetworkState>* cumulator_2;
  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1;
  STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2;
  
  PopMergeWrapper(Cumulator<PopNetworkState>* cumulator_1, Cumulator<PopNetworkState>* cumulator_2, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2) :
    cumulator_1(cumulator_1), cumulator_2(cumulator_2), fixpoints_1(fixpoints_1), fixpoints_2(fixpoints_2) { }
};

void* PopMaBEstEngine::threadMergeWrapper(void *arg)
{
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::init_pthread();
#endif
  PopMergeWrapper* warg = (PopMergeWrapper*)arg;
  try {
    Cumulator<PopNetworkState>::mergePairOfCumulators(warg->cumulator_1, warg->cumulator_2);
    PopMaBEstEngine::mergePairOfFixpoints(warg->fixpoints_1, warg->fixpoints_2);
  } catch(const BNException& e) {
    std::cerr << e;
  }
#ifdef USE_DYNAMIC_BITSET
  MBDynBitset::end_pthread();
#endif
  return NULL;
}


std::pair<Cumulator<PopNetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> PopMaBEstEngine::mergeResults(std::vector<Cumulator<PopNetworkState>*> cumulator_v, std::vector<STATE_MAP<NetworkState_Impl, unsigned int>*> fixpoint_map_v)
{
size_t size = cumulator_v.size();
  
  if (size == 0) {
    return std::make_pair((Cumulator<PopNetworkState>*) NULL, new STATE_MAP<NetworkState_Impl, unsigned int>());
  }
  
  if (size > 1) {
    
    
    unsigned int lvl=1;
    unsigned int max_lvl = ceil(log2(size));

    while(lvl <= max_lvl) {      
    
      unsigned int step_lvl = pow(2, lvl-1);
      unsigned int width_lvl = floor(size/(step_lvl*2)) + 1;
      pthread_t* tid = new pthread_t[width_lvl];
      unsigned int nb_threads = 0;
      std::vector<PopMergeWrapper*> wargs;
      for(unsigned int i=0; i < size; i+=(step_lvl*2)) {
        
        if (i+step_lvl < size) {
          PopMergeWrapper* warg = new PopMergeWrapper(cumulator_v[i], cumulator_v[i+step_lvl], fixpoint_map_v[i], fixpoint_map_v[i+step_lvl]);
          pthread_create(&tid[nb_threads], NULL, PopMaBEstEngine::threadMergeWrapper, warg);
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

void PopMaBEstEngine::epilogue()
{
  std::pair<Cumulator<PopNetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> res = mergeResults(cumulator_v, fixpoint_map_v);

  merged_cumulator = res.first;
  merged_cumulator->epilogue(pop_network, reference_state);

  fixpoints = *(res.second);
}

PopMaBEstEngine::~PopMaBEstEngine()
{
  delete fixpoint_map_v[0];
  
  for (auto t_arg_wrapper : arg_wrapper_v)
    delete t_arg_wrapper;

  delete merged_cumulator;
}

void PopMaBEstEngine::init()
{
  extern void builtin_functions_init();
  builtin_functions_init();
}

void PopMaBEstEngine::loadUserFuncs(const char *module)
{
  init();

#ifndef WINDOWS
  void *dl = dlopen(module, RTLD_LAZY);
#else
  void *dl = LoadLibrary(module);
#endif

  if (NULL == dl)
  {
#ifndef WINDOWS
    std::cerr << dlerror() << std::endl;
#else
    std::cerr << GetLastError() << std::endl;
#endif
    exit(1);
  }

#ifndef WINDOWS
  void *sym = dlsym(dl, MABOSS_USER_FUNC_INIT);
#else
  typedef void(__cdecl * MYPROC)(std::map<std::string, Function *> *);
  MYPROC sym = (MYPROC)GetProcAddress((HINSTANCE)dl, MABOSS_USER_FUNC_INIT);
#endif

  if (sym == NULL)
  {
    std::cerr << "symbol " << MABOSS_USER_FUNC_INIT << "() not found in user func module: " << module << "\n";
    exit(1);
  }
  typedef void (*init_t)(std::map<std::string, Function *> *);
  init_t init_fun = (init_t)sym;
  init_fun(Function::getFuncMap());
}

void PopMaBEstEngine::displayFixpoints(FixedPointDisplayer *displayer) const
{
  displayer->begin(fixpoints.size());

  size_t nn = 0;
  for (const auto & fp : fixpoints)
  {
    const NetworkState &network_state = fp.first;
    displayer->displayFixedPoint(nn + 1, network_state, fp.second, sample_count);
    nn++;
  }
  displayer->end();
}

void PopMaBEstEngine::displayPopProbTraj(ProbTrajDisplayer<PopNetworkState> *displayer) const
{
  merged_cumulator->displayProbTraj(pop_network, refnode_count, displayer);
}

void PopMaBEstEngine::display(ProbTrajDisplayer<PopNetworkState> *pop_probtraj_displayer, FixedPointDisplayer *fp_displayer) const
{
  displayPopProbTraj(pop_probtraj_displayer);
  displayFixpoints(fp_displayer);
}


void PopMaBEstEngine::displayRunStats(std::ostream& os, time_t start_time, time_t end_time) const {
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
  
  // os << "Number of PopNetworkState_Impl created : " << PopNetworkState_Impl::generated_number_count << std::endl << std::endl;
  
  runconfig->display(pop_network, start_time, end_time, os);
}

const std::map<unsigned int, std::pair<NetworkState, double> > PopMaBEstEngine::getFixPointsDists() const {
  
  std::map<unsigned int, std::pair<NetworkState, double> > res;
  if (0 == fixpoints.size()) {
    return res;
  }

  size_t nn = 0;
  for (const auto & fp : fixpoints) {
    const NetworkState& network_state = fp.first;
    res[nn++] = std::make_pair(network_state,(double) fp.second / sample_count);
  }
  return res;
}
