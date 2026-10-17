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
     BooleanNetwork.cc

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
     updated October 2014
*/

#include "BooleanNetwork.h"
#include "BooleanGrammar.h"
#include "RunConfig.h"
#include "Utils.h"
#include <iostream>

#ifdef SBML_COMPAT
#include "SBMLParser.h"
#endif


extern FILE* CTBNDLin;
extern void CTBNDL_scan_expression(const char *);
extern int CTBNDLparse();
extern void CTBNDLlex_destroy();
const bool backward_istate = getenv("MABOSS_BACKWARD_ISTATE") != NULL;
bool MaBoSS_quiet = false;

bool Node::override = false;
bool Node::augment = false;
size_t Network::MAX_NODE_SIZE = 0;

const int DivisionRule::DAUGHTER_1 = 1;
const int DivisionRule::DAUGHTER_2 = 2;

// Number of generated PopNetworkState_Impl
// long PopNetworkState_Impl::generated_number_count = 0;

Node::Node(const std::string& label, const std::string& description, NodeIndex index) : label(label), description(description), istate_set(false), is_internal(false), is_reference(false), referenceState(false), logicalInputExpr(NULL), rateUpExpr(NULL), rateDownExpr(NULL), index(index)
{
#if !defined(USE_STATIC_BITSET) && !defined(USE_DYNAMIC_BITSET)
  node_bit = NetworkState::nodeBit(index);
#endif
}


void Node::reset()
{
  description = "";
  istate_set = false;
  is_internal = false;
  is_reference = false;
  referenceState = false;
  delete logicalInputExpr;
  logicalInputExpr = NULL;
  delete rateUpExpr;
  rateUpExpr = NULL;
  delete rateDownExpr;
  rateDownExpr = NULL;
}

void Node::setLogicalInputExpression(const Expression* logicalInputExpr) {
  delete this->logicalInputExpr;
  this->logicalInputExpr = logicalInputExpr;
}

void Node::setRateUpExpression(const Expression* expr) {
  delete this->rateUpExpr;
  this->rateUpExpr = expr;
}

void Node::setRateDownExpression(const Expression* expr) {
  delete this->rateDownExpr;
  this->rateDownExpr = expr;
}

bool Node::isInputNode() const
{
  return getLogicalInputExpression() == NULL && getRateUpExpression() == NULL && getRateDownExpression() == NULL;
}

Network::Network() : last_index(0U)
{
  istate_group_list = new std::vector<IStateGroup*>();
  symbol_table = new SymbolTable();
  set_current_network(NULL);
  set_pop_network(NULL);
}

PopNetwork::PopNetwork() : Network() 
{ 
  deathRate = NULL; 
  divisionRules.clear();
  pop_istate_group_list = new std::vector<PopIStateGroup*>();
}

int Network::parseExpression(const char* content, std::map<std::string, NodeIndex>* nodes_indexes){
  
  set_current_network(this);
  CTBNDL_scan_expression(content);

  try 
  {
    int r = CTBNDLparse();
    set_current_network(NULL);

    if (r) {
      CTBNDLlex_destroy();
      return 1;
    }
    compile(nodes_indexes);
    CTBNDLlex_destroy();
    return 0;
  }
  catch (const BNException& e) 
  {
    set_current_network(NULL);
    CTBNDLlex_destroy();

    throw;
  }
}


int Network::parse(const char* file, std::map<std::string, NodeIndex>* nodes_indexes, bool is_temp_file, bool useSBMLNames)
{
#ifdef SBML_COMPAT
  if (hasEnding(std::string(file), ".xml") || hasEnding(std::string(file), ".sbml")) {
    return this->parseSBML(file, nodes_indexes, useSBMLNames);
  }
#endif

  if (NULL != file) {
    CTBNDLin = fopen(file, "r");
    if (CTBNDLin == NULL) {
      throw BNException("network parsing: cannot open file:" + std::string(file) + " for reading");
    }
    if (is_temp_file) {
      unlink(file);
    }
  }

  set_current_network(this);

  try{
    int r = CTBNDLparse();

    set_current_network(NULL);

    if (r) {
      if (NULL != file)
        fclose(CTBNDLin);
      CTBNDLlex_destroy();

      return 1;
    }
    compile(nodes_indexes);

    if (NULL != file)
      fclose(CTBNDLin);
    
    CTBNDLlex_destroy();

    return 0;
  }
  catch (const BNException& e) 
  {  
    if (NULL != file)
      fclose(CTBNDLin);
    
    set_current_network(NULL);
    CTBNDLlex_destroy();

    throw;
  }

}
#ifdef SBML_COMPAT

int Network::parseSBML(const char* file, std::map<std::string, NodeIndex>* nodes_indexes, bool useSBMLNames) 
{  
  SBMLParser* parser = new SBMLParser(this, file, useSBMLNames);
  
  // try{
    parser->build();
  // } catch (BNException e) {
  //   std::cerr << "ERROR : " << e.getMessage() << std::endl;
  //   return 1;
  // }
  compile(nodes_indexes);

  return 0;
}

#endif



int PopNetwork::parse(const char* file, std::map<std::string, NodeIndex>* nodes_indexes, bool is_temp_file)
{
  set_pop_network(this);
  int res = Network::parse(file, nodes_indexes, is_temp_file);
  
  set_pop_network(NULL);
  return res;
}


int PopNetwork::parseExpression(const char* content, std::map<std::string, NodeIndex>* nodes_indexes){
  
  set_pop_network(this);
  int res = Network::parseExpression(content, nodes_indexes);
  
  set_pop_network(NULL);
  return res;
}

void DivisionRule::setRate(Expression* rate) {  
  this->rate = rate->clone();
}

double DivisionRule::getRate(const NetworkState& state, const PopNetworkState& pop) {
  return rate->eval(NULL, state, pop);
}

NetworkState DivisionRule::applyRules(int daughter, const NetworkState& state, const PopNetworkState& pop) {
  NetworkState res(state);
  
  for (auto daughter_rule : daughters[daughter]) {
    res.setNodeState(daughter_rule.first, (bool)daughter_rule.second->eval(NULL, state, pop));
  }
  
  return res;
}

double PopNetwork::getDeathRate(const NetworkState& state, const PopNetworkState& pop) const {
  if (deathRate != NULL)
    return deathRate->eval(NULL, state, pop);
  else
    return 0.;
} 


void SymbolTable::display(std::ostream& os, bool check) const
{
  for (const auto & symb_entry : symb_map) {
    double value = getSymbolValue(symb_entry.second, check);
    os << symb_entry.first << " = " << value << ";\n";
  }
}

void SymbolTable::checkSymbols() const
{
  std::string str;
  for (const auto & symb_entry : symb_map) {
    const Symbol* symbol = symb_entry.second;
    SymbolIndex idx = symbol->getIndex();
    if (!symb_def[idx]) {
      str += std::string("\n") + "symbol " + symbol->getName() + " is not defined";
    }
  }

  if (str.length() != 0) {
    throw BNException(str);
  }
}

void Network::compile(std::map<std::string, NodeIndex>* nodes_indexes)
{
  MAP<std::string, Node*>::iterator begin = node_map.begin();

#if 0
  // checks for cycles
  // actually, not really pertinent...
  while (begin != node_map.end()) {
    Node* node = (*begin).second;
    if (!node->isInputNode()) {
      if (node->getLogicalInputExpr()->getRateUpExpression()->hasCycle(node)) {
	//std::cerr << "cycle detected for node " << node->getLabel() << '\n';
      }
    }
    ++begin;
  }
#endif

  // looks for input and non input nodes
  begin = node_map.begin();
  while (begin != node_map.end()) {
    Node* node = (*begin).second;
    if (!isNodeDefined(node->getLabel())) {
      throw BNException("node " + node->getLabel() + " used but not defined"); 
    }
    ++begin;
  }

  begin = node_map.begin();
  nodes.resize(node_map.size());
  while (begin != node_map.end()) {
    Node* node = (*begin).second;
    if (nodes_indexes != NULL) {
      node->setIndex((*nodes_indexes)[node->getLabel()]);
    }
    
    if (node->isInputNode()) {
      input_nodes.push_back(node);
    } else {
      non_input_nodes.push_back(node);
    }
    nodes[node->getIndex()] = node;
    ++begin;
  }
}

Node* Network::defineNode(const std::string& label, const std::string& description)
{
  if (isNodeDefined(label)) {
    throw BNException("node " + label + " already defined");
  }
  checkNewNode();
  Node* node = new Node(label, description, last_index++); // node factory
  setNodeAsDefined(label);
  node_map[label] = node;
  return node;
}

Node* Network::getNode(const std::string& label)
{
  if (node_map.find(label) == node_map.end()) {
    throw BNException("network: node " + label + " not defined");
  }
  return node_map[label];
}

void Network::initStates(NetworkState& initial_state, RandomGenerator * randgen)
{
  if (backward_istate) {
    for (const auto * node : nodes) {
      initial_state.setNodeState(node, node->getIState(this, randgen));
    }
  } else {
    IStateGroup::initStates(this, initial_state, randgen);
  }
}

void PopNetwork::initPopStates(PopNetworkState& initial_pop_state, RandomGenerator* randgen, unsigned int pop) {
  PopIStateGroup::initPopStates(this, initial_pop_state, randgen, pop);
}


void Network::display(std::ostream& os) const
{
  int nn = 0;
  for (const auto * node : nodes) {
    if (0 != nn) {
      os << '\n';
    }
    node->display(os);
    nn++;
  }
}

double Node::getRateUp(const NetworkState& network_state) const
{
  if (getRateUpExpression() == NULL) {
    if (NULL != getLogicalInputExpression()) {
      double d = getLogicalInputExpression()->eval(this, network_state);
      return (0.0 != d) ? 1.0 : 0.0;
    }
    return 0.0;
  }
  return getRateUpExpression()->eval(this, network_state);
}

double Node::getRateUp(const NetworkState& network_state, const PopNetworkState& pop) const
{
  if (getRateUpExpression() == NULL) {
    if (NULL != getLogicalInputExpression()) {
      double d = getLogicalInputExpression()->eval(this, network_state, pop);
      return (0.0 != d) ? 1.0 : 0.0;
    }
    return 0.0;
  }
  return getRateUpExpression()->eval(this, network_state, pop);
}


double Node::getRateDown(const NetworkState& network_state) const
{
  if (getRateDownExpression() == NULL) {
    if (NULL != getLogicalInputExpression()) {
      double d = getLogicalInputExpression()->eval(this, network_state);
      return (0.0 != d) ? 0.0 : 1.0;
    }
    return 0.0;
  }
  return getRateDownExpression()->eval(this, network_state);
}

double Node::getRateDown(const NetworkState& network_state, const PopNetworkState& pop) const
{
  if (getRateDownExpression() == NULL) {
    if (NULL != getLogicalInputExpression()) {
      double d = getLogicalInputExpression()->eval(this, network_state, pop);
      return (0.0 != d) ? 0.0 : 1.0;
    }
    return 0.0;
  }
  return getRateDownExpression()->eval(this, network_state, pop);
}


void Node::mutate(double value) 
{
    logicalInputExpr = new ConstantExpression(value);
    rateUpExpr = NULL;
    rateDownExpr = NULL;
}

NodeState Node::getNodeState(const NetworkState& network_state) const
{
  return network_state.getNodeState(this);
}

void Node::setNodeState(NetworkState& network_state, NodeState state)
{
  network_state.setNodeState(this, state);
}

bool Node::computeNodeState(NetworkState& network_state, NodeState& node_state) const
{
  return network_state.computeNodeState(this, node_state);
}

NodeState Node::getIState(const Network* network, RandomGenerator* rangen) const
{
  if (!istate_set) {
    double rand = rangen->generate();
    istate = rand > 0.5; // >= 0.5 ?
  }
  return istate;
}

void Node::display(std::ostream& os) const
{
  os << "node " << label << " {\n";
  if (description.length() > 0) {
    os << "  description = \"" << description << "\";\n";
  }
  if (NULL != logicalInputExpr) {
    os << "  logic = ";
    logicalInputExpr->display(os);
    os << ";\n";
  }
  if (NULL != rateUpExpr) {
    os << "  rate_up = ";
    rateUpExpr->display(os);
    os << ";\n";
  }
  if (NULL != rateDownExpr) {
    os << "  rate_down = ";
    rateDownExpr->display(os);
    os << ";\n";
  }

  if (attr_expr_map.size() || attr_str_map.size()) {
    os << "\n  // extra attributes\n";
    for (const auto & attr_expr : attr_expr_map) {
      os << "  " << attr_expr.first << " = ";
      attr_expr.second->display(os);
      os << ";\n";
    }

    for (const auto & attr_str : attr_str_map) {
      os << "  " << attr_str.first << " = \"" << attr_str.second << "\";\n";
    }
  }
  os << "}\n";
}

bool NetworkState::computeNodeState(const Node* node, NodeState& node_state)
{
  const Expression* expr = node->getLogicalInputExpression();
  if (NULL != expr) {
    double d = expr->eval(node, *this);
    node_state = d != 0.;
    setNodeState(node, node_state);
    return true;
  }
  return false;
}

#define HAMMING_METHOD2

unsigned int NetworkState::hamming(Network* network, const NetworkState_Impl& state2) const
{
  unsigned int hd = 0;
#ifdef HAMMING_METHOD1
  // faster way
  unsigned long s = (state ^ (state2 & state));
  unsigned int node_count = network->getNodes().size();
  for (unsigned int nn = 0; nn < node_count; ++nn) {
    if ((1ULL << nn) & s) {
      hd++;
    }
  }
  return hd;
#endif

#ifdef HAMMING_METHOD2
  NetworkState network_state2(state2, 1);
  const std::vector<Node*>& nodes = network->getNodes();
  
  for (const auto * node : nodes) {
    if (node->isReference()) {
      NodeState node_state1 = getNodeState(node);
      NodeState node_state2 = network_state2.getNodeState(node);
      hd += 1 - (node_state1 == node_state2);
    }
  }

  return hd;
#endif
}

unsigned int NetworkState::hamming(Network* network, const NetworkState& state2) const
{
  return hamming(network, state2.getState());
}

void Network::displayHeader(std::ostream& os) const
{
  int nn = 0;
  for (const auto * node : nodes) {
    os << (nn > 0 ? "\t" : "") << node->getLabel();
    nn++;
  }
  os << '\n';
}


void NetworkState::display(std::ostream& os, Network* network) const
{
  const std::vector<Node*>& nodes = network->getNodes();
  int nn = 0;
  for (const auto * node : nodes) {
    os << (nn > 0 ? "\t" : "") << getNodeState(node);
    nn++;
  }
  os << '\n';
}

std::string NetworkState::getName(Network* network, const std::string& sep) const {
#if defined(USE_STATIC_BITSET) || defined(USE_DYNAMIC_BITSET)
  if (state.none()) {
    return "<nil>";
  }
#else
  if (!state) {
    return "<nil>";
  }
#endif

  std::string result = "";
  const std::vector<Node*>& nodes = network->getNodes();
  
  bool displayed = false;
  for (const auto * node : nodes) {
    if (getNodeState(node)) {
      if (displayed) {
	      result += sep;
      } else {
	      displayed = true;
      }
      result += node->getLabel();
    }
  }
  return result;
  }


void NetworkState::displayOneLine(std::ostream& os, Network* network, const std::string& sep) const
{
  os << getName(network, sep);
}

void NetworkState::displayJSON(std::ostream& os, Network* network, const std::string& sep) const
{
  os << getName(network, sep);
}

std::string PopNetworkState::getName(Network * network, const std::string& sep) const {
  
  std::string res = "[";
  
  size_t i = mp.size();
  for (auto pop : mp) {
    NetworkState t_state(pop.first);
    res += "{" + t_state.getName(network) + ":" + std::to_string(pop.second) + "}";
    if (--i > 0) {
      res += ",";
    }
  }
  res += "]";
  return res;
}

void PopNetworkState::displayOneLine(std::ostream &strm, Network* network, const std::string& sep) const 
{    
  strm << getName(network, sep);
}

void PopNetworkState::displayJSON(std::ostream &strm, Network* network, const std::string& sep) const 
{    
  strm << "[";
  size_t i = mp.size();
  for (auto pop : mp) {
    NetworkState t_state(pop.first);
    strm << "{'" << t_state.getName(network) << "':" << pop.second << "}";
    if (--i > 0) {
      strm << ",";
    }
  }
  strm << "]";
}

unsigned int PopNetworkState::count(Expression * expr) const
{
  unsigned int res = 0;
  
  for (auto network_state_proba : mp) {
    NetworkState network_state = NetworkState(network_state_proba.first);
    if (expr == NULL || (bool)expr->eval(NULL, network_state)) {
      res += network_state_proba.second;
    }
  }
  
  return res;
}


unsigned int PopNetworkState::hamming(Network* network, const NetworkState& state2) const
{
  unsigned int hd = 0;
// #ifdef HAMMING_METHOD1
//   // faster way
//   unsigned long s = (state ^ (state2 & state));
//   unsigned int node_count = network->getNodes().size();
//   for (unsigned int nn = 0; nn < node_count; ++nn) {
//     if ((1ULL << nn) & s) {
//       hd++;
//     }
//   }
//   return hd;
// #endif

// #ifdef HAMMING_METHOD2
//   NetworkState network_state2(state2, 1);
//   const std::vector<Node*>& nodes = network->getNodes();
//   std::vector<Node*>::const_iterator begin = nodes.begin();
//   std::vector<Node*>::const_iterator end = nodes.end();

//   while (begin != end) {
//     Node* node = *begin;
//     if (node->isReference()) {
//       NodeState node_state1 = getNodeState(node);
//       NodeState node_state2 = network_state2.getNodeState(node);
//       hd += 1 - (node_state1 == node_state2);
//     }
//     ++begin;
//   }
// #endif

  return hd;
}

std::ostream& operator<<(std::ostream& os, const BNException& e)
{
  os << "BooleanNetwork exception: " << e.getMessage() << '\n';
  return os;
}

void IStateGroup::checkAndComplete(Network* network)
{
  std::map<std::string, bool> node_label_map;
  for (auto * istate_group : *(network->getIStateGroup())) {
    std::vector<const Node*>* nodes = istate_group->getNodes();

    for (const auto * node : *nodes) {
      if (node_label_map.find(node->getLabel()) != node_label_map.end()) {
	      throw BNException("duplicate node istate declaration: " + node->getLabel());
      }
      node_label_map[node->getLabel()] = true;
    }
  }

  const std::vector<Node*>& nodes = network->getNodes();
  for (const auto * node : nodes) {
    if (node_label_map.find(node->getLabel()) == node_label_map.end()) {
      new IStateGroup(network, node);
    }
  }

  // now complete missing nodes
}

void IStateGroup::initStates(Network* network, NetworkState& initial_state, RandomGenerator* randgen)
{
  for (auto * istate_group : *(network->getIStateGroup())) {
    
    std::vector<const Node*>* nodes = istate_group->getNodes();
    std::vector<ProbaIState*>* proba_istates = istate_group->getProbaIStates();

    if (proba_istates->size() == 1) {
      ProbaIState* proba_istate = (*proba_istates)[0];
      std::vector<double>* state_value_list = proba_istate->getStateValueList();
      size_t nn = 0;
      for (const auto & value : *state_value_list) {
        const Node* node = (*nodes)[nn++];
        initial_state.setNodeState(node, value != 0. ? true : false);
      }
    } else {
      double rand = randgen->generate();
      assert(rand >= 0. && rand <= 1.);
      size_t proba_istate_size = proba_istates->size();
      double proba_sum = 0;
      size_t jj = 0;
      for (auto * proba_istate : *proba_istates) {
        proba_sum += proba_istate->getProbaValue();
        //std::cerr << "rand: " << rand << " " << proba_sum << '\n';
        if (rand < proba_sum || jj == proba_istate_size - 1) {
          std::vector<double>* state_value_list = proba_istate->getStateValueList();
          size_t nn = 0;
          //std::cerr << "state #" << jj << " choosen\n";
          for (const auto & value : *state_value_list) {
            const Node* node = (*nodes)[nn++];
            initial_state.setNodeState(node, value != 0. ? true : false);
          }
          break;
        }
      }
    }
  }
}

void PopIStateGroup::initPopStates(PopNetwork* network, PopNetworkState& initial_state, RandomGenerator* randgen, unsigned int pop)
{
  PopNetwork* pop_network = static_cast<PopNetwork*>(network);
  initial_state.clear();
  if (pop_network->getPopIStateGroup()->size() > 0) 
  {
    // std::cout << "Creating a pop state from " << pop_network->getPopIStateGroup()->size() << " istate groups" << std::endl;
    for (auto * istate_group: *(pop_network->getPopIStateGroup())) 
    {  
      std::vector<const Node*>* nodes = istate_group->getNodes();
      std::vector<PopProbaIState*>* proba_istates = istate_group->getPopProbaIStates();  
    
      if (proba_istates->size() == 1)
      {
        PopProbaIState* proba_istate = (*proba_istates)[0];
        
        std::vector<PopIStateGroup::PopProbaIState::PopIStateGroupIndividual*>* individual_list = proba_istate->getIndividualList();
                
        for (auto * individual: *individual_list) 
        {   
          int i=0;
          NetworkState network_state;
          for (const auto & value: individual->getStateValueList()) {
            const Node* node = (*nodes)[i++];
            network_state.setNodeState(node, value != 0. ? true : false);
          }
          
          initial_state.addStatePop(network_state.getState(), individual->getPopSize());
        }
      } else {
        double rand = randgen->generate();
        assert(rand >= 0. && rand <= 1.);
        
        NetworkState network_state;
        double pop_size = 0;
        double proba_sum = 0;
        
        // std::cout << "Creating a random initial state from " << proba_istates->size() << " possibilities " << std::endl;
        for (auto * proba_istate: *proba_istates) 
        {
          proba_sum += proba_istate->getProbaValue();
          if (rand < proba_sum)
          {
            for (auto * individual: *(proba_istate->getIndividualList())) 
            {
              pop_size = individual->getPopSize();
              int i=0;
              for (const auto & value: individual->getStateValueList()) 
              {
                const Node* node = (*nodes)[i++];
                network_state.setNodeState(node, value != 0. ? true : false);
              }
              initial_state.addStatePop(network_state.getState(), pop_size); 

            }

            break; 
          }
        }
      }
    }
  } else {
    NetworkState state;
    IStateGroup::initStates(network, state, randgen);
    initial_state = PopNetworkState(state.getState(), pop);
          // std::cout << "State chosen : " << initial_state.getName(network) << std::endl << std::endl;

  }
}


void IStateGroup::display(Network* network, std::ostream& os)
{
  for (auto * istate_group: *network->getIStateGroup()) {
    std::vector<const Node*>* nodes = istate_group->getNodes();
    std::vector<ProbaIState*>* proba_istates = istate_group->getProbaIStates();

    if (nodes->size() == 1 && proba_istates->size() == 1) {
      std::vector<double>* state_value_list = (*proba_istates)[0]->getStateValueList();
      os << (*nodes)[0]->getLabel() << ".istate = " << ((*state_value_list)[0] != 0. ? "1" : "0") << ";\n";
      continue;
    }
    os << '[';
    size_t nn = 0;
    for (const auto * node : *nodes) {
      os << (nn > 0 ? ", " : "") << node->getLabel();
      nn++;
    }
    os << "].istate = ";

    size_t jj = 0;
    for (auto * proba_istate: *proba_istates) {
      os << (jj > 0 ? ", " : "") << proba_istate->getProbaValue() << " [";
      std::vector<double>* state_value_list = proba_istate->getStateValueList();
      size_t ii = 0;
      for (auto & value : *state_value_list) {
        os << (ii > 0 ? ", " : "") << value;
        ii++;
      }
      os << "]";
      jj++;
    }

    os << ";\n";
  }
}

void IStateGroup::reset(Network * network) {
  network->getIStateGroup()->clear();
}

Node::~Node()
{
  delete logicalInputExpr;
  delete rateUpExpr;
  delete rateDownExpr;

  for (const auto & attr_expr : attr_expr_map) {
    delete attr_expr.second;
  }
}

Network::Network(const Network& network)
{
  *this = network;
}

Network& Network::operator=(const Network& network)
{
  node_map = network.node_map;
  last_index = network.last_index;
  input_nodes = network.input_nodes;
  non_input_nodes = network.non_input_nodes;
  nodes = network.nodes;
  symbol_table = network.symbol_table;
  return *this;
}

Network::~Network()
{
  delete symbol_table;
  
  for (auto * istate_group: *istate_group_list) {
    delete istate_group;
  }
  delete istate_group_list;
  
  for (auto & node : node_map) {
    delete node.second;
  }
  
}

void Network::cloneIStateGroup(std::vector<IStateGroup*>* _istate_group_list) 
{
  for (auto istate_group: *_istate_group_list) 
  {
    new IStateGroup(istate_group, this);
  }
}

PopNetwork::PopNetwork(const PopNetwork& pop_network) {
  *this = pop_network;
}

PopNetwork& PopNetwork::operator=(const PopNetwork& pop_network) {
  Network::operator=(pop_network);
  deathRate = pop_network.getDeathRate()->clone();
  for (auto division_rule: pop_network.getDivisionRules()) {
    divisionRules.push_back(division_rule);
  }
  return *this;
}

void SymbolTable::reset()
{
  symb_map.clear();
  symb_value.clear();
  symb_def.clear();
  symb_dont_set.clear();
  last_symb_idx = 0;
}

int setConfigVariables(Network* network, const std::string& prog, std::vector<std::string>& runvar_v)
{
  SymbolTable* symtab = network->getSymbolTable();
  for (const auto & var_values : runvar_v) {
    size_t o_var_value_pos = 0;
    for (;;) {
      if (o_var_value_pos == std::string::npos) {
	break;
      }
      size_t var_value_pos = var_values.find(',', o_var_value_pos);
      std::string var_value = var_value_pos == std::string::npos ? var_values.substr(o_var_value_pos) : var_values.substr(o_var_value_pos, var_value_pos-o_var_value_pos);
      o_var_value_pos = var_value_pos + (var_value_pos == std::string::npos ? 0 : 1);
      size_t pos = var_value.find('=');
      if (pos == std::string::npos) {
	std::cerr << '\n' << prog << ": invalid var format [" << var_value << "] VAR=BOOL_OR_DOUBLE expected\n";
	return 1;
      }
      std::string ovar = var_value.substr(0, pos);
      std::string var = ovar[0] != '$' ? "$" + ovar : ovar;
      const Symbol* symbol = symtab->getOrMakeSymbol(var);
      std::string value = var_value.substr(pos+1);
      if (!strcasecmp(value.c_str(), "true")) {
	symtab->overrideSymbolValue(symbol, 1);
      } else if (!strcasecmp(value.c_str(), "false")) {
	symtab->overrideSymbolValue(symbol, 0);
      } else {
	double dval;
	int r = sscanf(value.c_str(), "%lf", &dval);
	if (r != 1) {
	  std::cerr << '\n' << prog << ": invalid value format [" << var_value << "] " << ovar << "=BOOL_OR_DOUBLE expected\n";
	  return 1;
	}
	symtab->overrideSymbolValue(symbol, dval);
      }
    }
  }
  return 0;
}

int setConfigVariables(Network* network, const std::string& prog, const std::string& runvar)
{
  std::vector<std::string> runvar_v;
  runvar_v.push_back(runvar);
  return setConfigVariables(network, prog, runvar_v);
}

void SymbolTable::unsetSymbolExpressions() {
  for (auto * exp : symbolExpressions) {
    exp->unset();
  }
}
