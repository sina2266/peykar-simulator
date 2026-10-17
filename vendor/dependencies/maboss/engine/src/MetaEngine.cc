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
     MetaEngine.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#include "MetaEngine.h"
#include "Probe.h"
#include "Utils.h"
#ifndef WINDOWS
  #include <dlfcn.h>
#else
  #include <windows.h>
#endif

static const char* MABOSS_USER_FUNC_INIT = "maboss_user_func_init";

void MetaEngine::init()
{
  extern void builtin_functions_init();
  builtin_functions_init();
}

void MetaEngine::loadUserFuncs(const char* module)
{
  init();

#ifndef WINDOWS
  void* dl = dlopen(module, RTLD_LAZY);
#else
  void* dl = LoadLibrary(module);
#endif

  if (NULL == dl) {
#ifndef WINDOWS    
    std::cerr << dlerror() << std::endl;
#else
    std::cerr << GetLastError() << std::endl;
#endif
    exit(1);
  }

#ifndef WINDOWS
  void* sym = dlsym(dl, MABOSS_USER_FUNC_INIT);
#else
  typedef void (__cdecl *MYPROC)(std::map<std::string, Function*>*);
  MYPROC sym = (MYPROC) GetProcAddress((HINSTANCE) dl, MABOSS_USER_FUNC_INIT);
#endif

  if (sym == NULL) {
    std::cerr << "symbol " << MABOSS_USER_FUNC_INIT << "() not found in user func module: " << module << "\n";
    exit(1);
  }
  typedef void (*init_t)(std::map<std::string, Function*>*);
  init_t init_fun = (init_t)sym;
  init_fun(Function::getFuncMap());
}

NodeIndex MetaEngine::getTargetNode(Network* _network, RandomGenerator* random_generator, const std::vector<double>& nodeTransitionRates, double total_rate) const
{
  double U_rand2 = random_generator->generate();
  double random_rate = U_rand2 * total_rate;
  NodeIndex node_idx = INVALID_NODE_INDEX;
  
  for (unsigned int i=0; i < nodeTransitionRates.size() && random_rate >= 0.; i++) {
    node_idx = i;
    double rate = nodeTransitionRates[i];
    random_rate -= rate;
  }

  assert(node_idx != INVALID_NODE_INDEX);
  assert(_network->getNode(node_idx)->getIndex() == node_idx);
  return node_idx;
}

double MetaEngine::computeTH(Network* _network, const std::vector<double>& nodeTransitionRates, double total_rate) const
{
  if (nodeTransitionRates.size() == 1) {
    return 0.;
  }


  double TH = 0.;
  double rate_internal = 0.;

  for (unsigned int i = 0; i < nodeTransitionRates.size(); i++) {
    NodeIndex index = i;
    double rate = nodeTransitionRates[i];
    if (rate != 0.0 && _network->getNode(index)->isInternal()) {
      rate_internal += rate;
    }
  }

  double total_rate_non_internal = total_rate - rate_internal;

  for (unsigned int i = 0; i < nodeTransitionRates.size(); i++){

    NodeIndex index = i;
    double rate = nodeTransitionRates[i];
    if (rate != 0.0 && !_network->getNode(index)->isInternal()) {
      double proba = rate / total_rate_non_internal;
      TH -= log2(proba) * proba;
    }
  }

  return TH;
}