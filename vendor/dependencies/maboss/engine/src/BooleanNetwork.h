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
     BooleanNetwork.h

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2011
*/

#ifndef _BOOLEANNETWORK_H_
#define _BOOLEANNETWORK_H_

#define EV_OPTIM_2021_10

// #include <iostream>
// #include <functional>

#include "maboss-config.h"

#ifdef USE_DYNAMIC_BITSET

#undef MAXNODES
#define MAXNODES 0xFFFFFFF

#elif MAXNODES>64

#define USE_STATIC_BITSET

#endif

// To be defined only when comparing bitset with ulong implementation
//#define COMPARE_BITSET_AND_ULONG

#ifdef HAS_UNORDERED_MAP
#define USE_UNORDERED_MAP
#endif


#ifdef SBML_COMPAT
#include <sbml/SBMLTypes.h>
#include "sbml/packages/qual/common/QualExtensionTypes.h"
 
LIBSBML_CPP_NAMESPACE_USE
#endif


#define MAP std::map

#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <strings.h>
#include <string.h>
#ifdef USE_STATIC_BITSET
#include <bitset>
#elif defined(USE_DYNAMIC_BITSET)
#include "MBDynBitset.h"
#endif
#include "Function.h"

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


const std::string LOGICAL_AND_SYMBOL = " & ";
const std::string LOGICAL_OR_SYMBOL = " | ";
const std::string LOGICAL_NOT_SYMBOL = "!";
const std::string LOGICAL_XOR_SYMBOL = " ^ ";

extern bool MaBoSS_quiet;

class Expression;
class NotLogicalExpression;
class SymbolExpression;
class ConstantExpression;
class NetworkState;
class PopNetworkState;
class Network;
class Node;
class RandomGenerator;
class RunConfig;
class IStateGroup;
class PopIStateGroup;

class LogicalExprGenContext {

  const Network* network;
  const Node* node;
  std::ostream& os;
  unsigned int level;

 public:
  LogicalExprGenContext(const Network* network, const Node* node, std::ostream& os) : network(network), node(node), os(os), level(0) { }

  const Network* getNetwork() const {return network;}
  std::ostream& getOStream() const {return os;}

  const Node* getNode() {return node;}

  unsigned int getLevel() const {return level;}
  void incrLevel() {level++;}
  void decrLevel() {level--;}

  class LevelManager {

    LogicalExprGenContext& genctx;
    unsigned int level;

  public:
    LevelManager(LogicalExprGenContext& genctx) : genctx(genctx) {
      level = genctx.getLevel();
      genctx.incrLevel();
    }

    unsigned int getLevel() const {return level;}

    ~LevelManager() {
      genctx.decrLevel();
    }
  };
};


typedef unsigned int NodeIndex;
typedef bool NodeState; // for now... could be a class
typedef unsigned int SymbolIndex;

#ifdef USE_STATIC_BITSET

#ifdef USE_UNORDERED_MAP
typedef std::bitset<MAXNODES> NetworkState_Impl;

namespace std {
  template <> struct HASH_STRUCT<bitset<MAXNODES> >
  {
    size_t operator()(const bitset<MAXNODES>& val) const {
#ifdef COMPARE_BITSET_AND_ULONG
      return val.to_ulong();
#else
      static const bitset<MAXNODES> MASK(0xFFFFFFFFUL);
      return (val & MASK).to_ulong();
#endif
    }
  };

  template <> struct equal_to<bitset<MAXNODES> >
  {
    size_t operator()(const bitset<MAXNODES>& val1, const bitset<MAXNODES>& val2) const {
      return val1 == val2;
    }
  };

  // Added less operator, necessary for maps, sets. Code from https://stackoverflow.com/a/21245301/11713763
  template <> struct less<bitset<MAXNODES> >
  {
    size_t operator()(const bitset<MAXNODES>& val1, const bitset<MAXNODES>& val2) const {
    for (int i = MAXNODES-1; i >= 0; i--) {
        if (val1[i] ^ val2[i]) return val2[i];
    }
    return false;

    }
  };
}

#else

template <int N> class sbitset : public std::bitset<N> {

 public:
  sbitset() : std::bitset<N>() { }
  sbitset(const sbitset<N>& sbitset) : std::bitset<N>(sbitset) { }
  sbitset(const std::bitset<N>& bitset) : std::bitset<N>(bitset) { }

  int operator<(const sbitset<N>& bitset1) const {
#ifdef COMPARE_BITSET_AND_ULONG
    return this->to_ulong() < bitset1.to_ulong();
#else
    for (int nn = N-1; nn >= 0; --nn) {
      int delta = this->test(nn) - bitset1.test(nn);
      if (delta < 0) {
	return 1;
      }
      if (delta > 0) {
	return 0;
      }
    }
    return 0;
#endif
  }
};

typedef sbitset<MAXNODES> NetworkState_Impl;
#endif
#elif defined(USE_DYNAMIC_BITSET)

typedef MBDynBitset NetworkState_Impl;

#else
typedef unsigned long long NetworkState_Impl;
#endif

static const std::string ATTR_RATE_UP = "rate_up";
static const std::string ATTR_RATE_DOWN = "rate_down";
static const std::string ATTR_LOGIC = "logic";
static const std::string ATTR_DESCRIPTION = "description";
static const NodeIndex INVALID_NODE_INDEX = (NodeIndex)~0U;

class BNException {

  std::string msg;

public:
  BNException(const std::string& msg) : msg(msg) { }

  const std::string& getMessage() const {return msg;}
};

extern std::ostream& operator<<(std::ostream& os, const BNException& e);

class Node {
  static bool override;
  static bool augment;
  std::string label;
  std::string description;
  bool istate_set;
  bool is_internal;
  bool is_reference;
  NodeState referenceState;
  const Expression* logicalInputExpr;
  const Expression* rateUpExpr;
  const Expression* rateDownExpr;
  mutable NodeState istate;
  NodeIndex index;
  
  MAP<std::string, const Expression*> attr_expr_map;
  MAP<std::string, std::string> attr_str_map;
  Expression* rewriteLogicalExpression(Expression* rateUpExpr, Expression* rateDownExpr) const;
  NetworkState_Impl node_bit;

 public:
  Node(const std::string& label, const std::string& description, NodeIndex index);

  void setIndex(NodeIndex new_index) {
    index = new_index;
#if !defined(USE_STATIC_BITSET) && !defined(USE_DYNAMIC_BITSET)
    node_bit = 1ULL << new_index;
#endif
  }

  const std::string& getLabel() const {
    return label;
  }

  void setDescription(const std::string& description) {
    this->description = description;
  }

  const std::string& getDescription() const {
    return description;
  }

  void setLogicalInputExpression(const Expression* logicalInputExpr);

  void setRateUpExpression(const Expression* expr);

  void setRateDownExpression(const Expression* expr);

  const Expression* getLogicalInputExpression() const {
    return logicalInputExpr;
  }

  const Expression* getRateUpExpression() const {
    return rateUpExpr;
  }

  const Expression* getRateDownExpression() const {
    return rateDownExpr;
  }

  void setAttributeExpression(const std::string& attr_name, const Expression* expr) {
    if (attr_name == ATTR_RATE_UP) {
      setRateUpExpression(expr);
      return;
    }
    if (attr_name == ATTR_RATE_DOWN) {
      setRateDownExpression(expr);
      return;
    }
    if (attr_name == ATTR_LOGIC) {
      setLogicalInputExpression(expr);
      return;
    }
    attr_expr_map[attr_name] = expr;
  }

  void mutate(double value);

  NodeState getIState(const Network* network, RandomGenerator* randgen) const;

  void setIState(NodeState istate) {
    istate_set = true;
    this->istate = istate;
  }

  bool istateSetRandomly() const {
    return !istate_set;
  }

  bool isInternal() const {
    return is_internal;
  }

  void isInternal(bool is_internal) {
    this->is_internal = is_internal;
  }

  bool isReference() const {
    return is_reference;
  }

  void setReference(bool is_reference) {
    this->is_reference = is_reference;
  }

  NodeState getReferenceState() const {
    return referenceState;
  }

  void setReferenceState(NodeState referenceState) {
    this->is_reference = true;
    this->referenceState = referenceState;
  }

  const Expression* getAttributeExpression(const std::string& attr_name) const {
    MAP<std::string, const Expression*>::const_iterator iter = attr_expr_map.find(attr_name);
    if (iter == attr_expr_map.end()) {
      if (attr_name == ATTR_RATE_UP) {
	return getRateUpExpression();
      }
      if (attr_name == ATTR_RATE_DOWN) {
	return getRateDownExpression();
      }
      if (attr_name == ATTR_LOGIC) {
	return getLogicalInputExpression();
      }
      return NULL;
    }
    return (*iter).second;
  }

  void setAttributeString(const std::string& attr_name, const std::string& str) {
    if (attr_name == ATTR_DESCRIPTION) {
      setDescription(str);
      return;
    }

    attr_str_map[attr_name] = str;
  }

  std::string getAttributeString(const std::string& attr_name) const {
    MAP<std::string, std::string>::const_iterator iter = attr_str_map.find(attr_name);
    if (iter == attr_str_map.end()) {
      if (attr_name == ATTR_DESCRIPTION) {
	return getDescription();
      }
      return "";
    }
    return (*iter).second;
  }

  NodeIndex getIndex() const {return index;}

#if !defined(USE_STATIC_BITSET) && !defined(USE_DYNAMIC_BITSET)
  NetworkState_Impl getNodeBit() const {return node_bit;}
#endif

  const MAP<std::string, const Expression*>& getAttributeExpressionMap() const {
    return attr_expr_map;
  }

  const MAP<std::string, std::string>& getAttributeStringMap() const {
    return attr_str_map;
  }

  bool isInputNode() const; // true if node state does not depend on other node states

  double getRateUp(const NetworkState& network_state) const;
  double getRateDown(const NetworkState& network_state) const;
  double getRateUp(const NetworkState& network_state, const PopNetworkState& pop) const;
  double getRateDown(const NetworkState& network_state, const PopNetworkState& pop) const;
  NodeState getNodeState(const NetworkState& network_state) const;
  void setNodeState(NetworkState& network_state, NodeState state);

  // returns true if and only if there is a logical input expression that allows to compute state from input nodes
  bool computeNodeState(NetworkState& network_state, NodeState& node_state) const;

  std::string toString() const {
    std::ostringstream ostr;
    display(ostr);
    return ostr.str();
  }

  void display(std::ostream& os) const;
  Expression* generateRawLogicalExpression() const;
  void generateLogicalExpression(LogicalExprGenContext& gen) const;

  static void setOverride(bool override) {
    Node::override = override;
  }

  static bool isOverride() {return override;}

  static void setAugment(bool augment) {
    Node::augment = augment;
  }

  static bool isAugment() {return augment;}

#ifdef SBML_COMPAT
  void writeSBML(QualitativeSpecies* qs) 
  {
    qs->setId(this->getLabel());
    qs->setCompartment("c");
    qs->setConstant(false);
    qs->setInitialLevel(1);
    qs->setMaxLevel(1);
    qs->setName(this->getLabel());
  }
#endif

  void reset();

  ~Node();
};

// symbol entry (i.e. variables)
class Symbol {
  std::string symb;
  SymbolIndex symb_idx;

public:
  Symbol(const std::string& symb, SymbolIndex symb_idx) : symb(symb), symb_idx(symb_idx) { }
  const std::string& getName() const {return symb;}
  SymbolIndex getIndex() const {return symb_idx;}
};

//The symbol table
class SymbolTable {
  SymbolIndex last_symb_idx;
  MAP<std::string, Symbol*> symb_map;
  std::vector<double> symb_value;
  std::vector<bool> symb_def;
  std::map<SymbolIndex, bool> symb_dont_set;
  
  std::vector<SymbolExpression *> symbolExpressions;

public:
  SymbolTable() : last_symb_idx(0) { }
  
  const Symbol* getSymbol(const std::string& symb) {
    if (symb_map.find(symb) == symb_map.end()) {
      return NULL;
    }
    return symb_map[symb];
  }

  const Symbol* getOrMakeSymbol(const std::string& symb) {
    if (symb_map.find(symb) == symb_map.end()) {
      symb_def.push_back(false);
      symb_value.push_back(0.0);
      symb_map[symb] = new Symbol(symb, last_symb_idx++);
      assert(symb_value.size() == last_symb_idx);
      assert(symb_def.size() == last_symb_idx);
    }
    return symb_map[symb];
  }

  double getSymbolValue(const Symbol* symbol, bool check = true) const {
    SymbolIndex idx = symbol->getIndex();
    if (!symb_def[idx]) {
      if (check) {
	throw BNException("symbol " + symbol->getName() + " is not defined"); 
      }
      return 0.;
   }
    return symb_value[idx];
  }

  size_t getSymbolCount() const {return symb_map.size();}

  void setSymbolValue(const Symbol* symbol, double value) {
    SymbolIndex idx = symbol->getIndex();
    if (symb_dont_set.find(idx) == symb_dont_set.end()) {
      symb_def[idx] = true;
      symb_value[idx] = value;
    }
  }

  void overrideSymbolValue(const Symbol* symbol, double value) {
    setSymbolValue(symbol, value);
    symb_dont_set[symbol->getIndex()] = true;
  }

  void display(std::ostream& os, bool check = true) const;
  void checkSymbols() const;

  std::vector<std::string> getSymbolsNames() const {
    std::vector<std::string> result;
    for (auto& symb : symb_map) {
      result.push_back(symb.first);
    }
    return result;
  }
  void reset();

  void addSymbolExpression(SymbolExpression * exp) {
    symbolExpressions.push_back(exp);
  }

  void unsetSymbolExpressions();

  ~SymbolTable() {
    for (auto& symbol : symb_map) {
      delete symbol.second;
    }
  }
};

// the boolean network (also used as a Node factory)
class Network {
  MAP<std::string, Node*> node_map;
  NodeIndex last_index;
  std::vector<Node*> input_nodes;
  std::vector<Node*> non_input_nodes;
  std::vector<Node*> nodes;

  MAP<std::string, bool> node_def_map;
  std::vector<IStateGroup*>* istate_group_list;
  SymbolTable* symbol_table;
  static size_t MAX_NODE_SIZE;

  // must be call before creating a new node
  void checkNewNode() {
    size_t size = node_map.size();
    if (size >= MAXNODES) {
      std::ostringstream ostr;
      ostr << "maximum node count exceeded: maximum allowed is " << MAXNODES;
      throw BNException(ostr.str());
    }
    if (size >= MAX_NODE_SIZE) {
      MAX_NODE_SIZE = size+1;
    }
  }

public:

  Network();

  Network(const Network& network);
  Network& operator=(const Network& network);

  int parse(const char* file = NULL, std::map<std::string, NodeIndex>* nodes_indexes = NULL, bool is_temp_file = false, bool useSBMLNames = false);
  int parseExpression(const char* content = NULL, std::map<std::string, NodeIndex>* nodes_indexes = NULL);
  
  #ifdef SBML_COMPAT
  int parseSBML(const char* file, std::map<std::string, NodeIndex>* nodes_indexes = NULL, bool useSBMLNames = false);
  #endif
  
  std::vector<IStateGroup*>* getIStateGroup() {
    return istate_group_list;
  }

  void cloneIStateGroup(std::vector<IStateGroup*>* _istate_group_list);

  SymbolTable* getSymbolTable() { 
    return symbol_table;
  };

  Node* defineNode(const std::string& label, const std::string& description = "");

  Node* getNode(const std::string& label);

  Node* getNode(NodeIndex node_idx) {
    assert(node_idx < getNodeCount());
    return nodes[node_idx];
  }

  Node* getOrMakeNode(const std::string& label) {
    if (node_map.find(label) != node_map.end()) {
      return node_map[label];
    }
    checkNewNode();
    Node* node = new Node(label, "", last_index++); // node factory
    node_map[label] = node;
    return node;
  }

  size_t getNodeCount() const {return node_map.size();}

  static size_t getMaxNodeSize() {
    //MAX_NODE_SIZE = 508; // for testing
    //static bool msg_displayed = false;
    static bool msg_displayed = true;
    if (!msg_displayed) {
      if (!MaBoSS_quiet) {
	std::cerr << "\nMaBoSS notice:\n";
	std::cerr << "  Using dynamic bitset implementation (any number of nodes): this version is not fully optimized and may use a large amount of memory\n";
	std::cerr << "  For this " << MAX_NODE_SIZE << " node network, preferably used ";
	if (MAX_NODE_SIZE <= 64) {
	  std::cerr << "the standard 'MaBoSS' program\n";
	} else {
	  std::cerr << "the static bitset implementation program 'MaBoSS_" << MAX_NODE_SIZE << "n' built using: make MAXNODES=" << MAX_NODE_SIZE << "\n";
	}
      }
      msg_displayed = true;
    }
    return MAX_NODE_SIZE;
  }

  void compile(std::map<std::string, NodeIndex>* nodes_indexes = NULL);

  // vector of nodes which do not depend on other nodes
  const std::vector<Node*>& getInputNodes() const {return input_nodes;}

  // vector of the other nodes
  const std::vector<Node*>& getNonInputNodes() const {return non_input_nodes;}

  // vector of all nodes
  const std::vector<Node*>& getNodes() const {return nodes;}

  std::string toString() const {
    std::ostringstream ostr;
    display(ostr);
    return ostr.str();
  }
  
  void initStates(NetworkState& initial_state, RandomGenerator* randgen);

  void displayHeader(std::ostream& os) const;

  void display(std::ostream& os) const;

  void generateLogicalExpressions(std::ostream& os) const;

  bool isNodeDefined(const std::string& identifier) {
    return node_def_map.find(identifier) != node_def_map.end();
  }

  void setNodeAsDefined(const std::string& identifier) {
    node_def_map[identifier] = true;
  }

  void resetNodeDefinition() {
    node_def_map.clear();
  }

  void removeLastNode(const std::string& identifier) {
    /* This function was created to remove the last node which was just created. 
       It is only used when parsing a bnet file, to remove the "target, factors" which is part of the header
       Changes are that you should NEVER use it for another purpose.
    */
    if (node_map.find(identifier) != node_map.end()) {
      Node* to_delete = node_map[identifier];
      node_map.erase(identifier);
      if (to_delete->getIndex() == (last_index-1)) {
        last_index--;
      }
      free(to_delete);
    }
  }

  ~Network();
};

class DivisionRule {
  
  public:

  //During a division, you remove the cell and create two new cells
  static const int DAUGHTER_1;
  // and 
  static const int DAUGHTER_2;
  
  // Each one has it's own map which will change a node value according to an expression
  std::map<Node*, Expression*> daughter1;
  std::map<Node*, Expression*> daughter2;
  
  // And we have an map to select the map of each daughter
  std::map<int, std::map<Node*, Expression*> > daughters = {{DAUGHTER_1, daughter1}, {DAUGHTER_2, daughter2}};
  
  // Division also have a rate
  Expression* rate;
  
  
  DivisionRule() {
    daughter1.clear();
    daughter2.clear();
    rate = NULL;
  }
  
  void setRate(Expression* rate);
  double getRate(const NetworkState& state, const PopNetworkState& pop);
  
  void addDaughterNode(int daughter, Node* node, Expression* expression) {
    daughters[daughter][node] = expression;
  }
  
  // This will return a new state based on the mother cell, properly modified according to the maps
  NetworkState applyRules(int daughter, const NetworkState& state, const PopNetworkState& pop);
};


class PopNetwork : public Network {
  public:
  
  // Population networks have two extra fields :
  
  // Rules for division (speed, state modifications)
  std::vector<DivisionRule> divisionRules;
  
  // Death rate
  Expression* deathRate;
  
  std::vector<PopIStateGroup*>* pop_istate_group_list;

  PopNetwork();
  ~PopNetwork() { delete pop_istate_group_list; }
  PopNetwork(const PopNetwork& network);
  PopNetwork& operator=(const PopNetwork& network);

  int parse(const char* file = NULL, std::map<std::string, NodeIndex>* nodes_indexes = NULL, bool is_temp_file = false);
  int parseExpression(const char* content, std::map<std::string, NodeIndex>* nodes_indexes);

  void initPopStates(PopNetworkState& initial_pop_state, RandomGenerator* randgen, unsigned int pop);

  void addDivisionRule(DivisionRule rule) { divisionRules.push_back(rule); }
  void setDeathRate(Expression* expr) { deathRate = expr; }
  
  const std::vector<DivisionRule> getDivisionRules() const { return divisionRules; }
  const Expression* getDeathRate() const { return deathRate; }
  
  // Evaluation of the death rate according to the state
  double getDeathRate(const NetworkState& state, const PopNetworkState& pop) const;
  
  std::vector<PopIStateGroup*>* getPopIStateGroup() { return pop_istate_group_list; }
};

// global state of the boolean network
class NetworkState {
  NetworkState_Impl state;

#if !defined(USE_STATIC_BITSET) && !defined(USE_DYNAMIC_BITSET)
  static NetworkState_Impl nodeBit(const Node* node) {
    return node->getNodeBit();
  }

public:
  static NetworkState_Impl nodeBit(NodeIndex node_index) {
    return (1ULL << node_index);
  }
#endif

public:
  NetworkState(const NetworkState_Impl& state) : state(state) { }
  NetworkState(const NetworkState& state) : state(state.getState()) {}
#ifdef USE_DYNAMIC_BITSET
  NetworkState(const NetworkState_Impl& state, int copy) : state(state, 1) { }
  NetworkState(const NetworkState& state, int copy) : state(state.getState(), 1) {}
#else
  NetworkState(const NetworkState_Impl& state, int copy) : state(state) { }
  NetworkState(const NetworkState& state, int copy) : state(state.getState()) {}
#endif

  NetworkState operator&(const NetworkState& mask) const { 
    return NetworkState(state & mask.getState());
  }
  
  NetworkState applyMask(const NetworkState& mask) const {
    return NetworkState(state & mask.getState());
  }

#ifdef USE_STATIC_BITSET
  NetworkState() { }
#elif defined(USE_DYNAMIC_BITSET)
  // EV: 2020-10-23
  //NetworkState() : state(MAXNODES) { }
  NetworkState() : state(Network::getMaxNodeSize()) { }
  // EV: 2020-12-01 would be better to create a 0-size state and then call resize dynamically
  //NetworkState() : state(0) { }
#else
  NetworkState() : state(0ULL) { }
#endif

  void set() {
#ifdef USE_STATIC_BITSET
    state.set();
#elif defined(USE_DYNAMIC_BITSET)
    // EV: 2020-10-23
    //output_mask.resize(MAXNODES);
    state.resize(Network::getMaxNodeSize());
    state.set();
#else
    state = ~0ULL;
#endif
  }

  void reset() {
#ifdef USE_STATIC_BITSET
    state.reset();
#elif defined(USE_BOOST_BITSET) || defined(USE_DYNAMIC_BITSET)
    // EV: 2020-10-23
    //output_mask.resize(MAXNODES);
    state.resize(Network::getMaxNodeSize());
    state.reset();
#else
    state = 0ULL;
#endif
  }

  NodeState getNodeState(const Node* node) const {
#if defined(USE_STATIC_BITSET) || defined(USE_DYNAMIC_BITSET)
    return state.test(node->getIndex());
#else
    return state & nodeBit(node);
#endif
  }

  void setNodeState(const Node* node, NodeState node_state) {
#if defined(USE_STATIC_BITSET) || defined(USE_DYNAMIC_BITSET)
    state.set(node->getIndex(), node_state);
#else
    if (node_state) {
      state |= nodeBit(node);
    } else {
      state &= ~nodeBit(node);
    }
#endif
  }

  void flipState(const Node* node) {
#if defined(USE_STATIC_BITSET) || defined(USE_DYNAMIC_BITSET)
    //state.set(node->getIndex(), !state.test(node->getIndex()));
    state.flip(node->getIndex());
#else
    state ^= nodeBit(node);
#endif
  }

  // returns true if and only if there is a logical input expression that allows to compute state from input nodes
  bool computeNodeState(const Node* node, NodeState& node_state);

  static bool isPopState() { return false; }
  std::set<NetworkState_Impl>* getNetworkStates() {
    return new std::set<NetworkState_Impl>({state});
  }
  
#ifdef USE_DYNAMIC_BITSET
  NetworkState_Impl getState(int copy) const {return NetworkState_Impl(state, copy);}
#endif
  NetworkState_Impl getState() const {return state;}


  void display(std::ostream& os, Network* network) const;

  std::string getName(Network * network, const std::string& sep=" -- ") const;
 
  void displayOneLine(std::ostream& os, Network* network, const std::string& sep = " -- ") const;
  void displayJSON(std::ostream& os, Network* network, const std::string& sep = " -- ") const;

#ifndef USE_UNORDERED_MAP
  bool operator<(const NetworkState& network_state) const {
    return state < network_state.state;
  }
#endif
  unsigned int hamming(Network* network, const NetworkState_Impl& state) const;
  unsigned int hamming(Network* network, const NetworkState& state) const;


#ifdef MPI_COMPAT
  static size_t my_MPI_Pack_Size() {
#ifdef USE_STATIC_BITSET
    return (MAXNODES/64 + (MAXNODES%64 > 0 ? 1 : 0)) * sizeof(unsigned long long) + sizeof(size_t);

#elif defined(USE_DYNAMIC_BITSET)
    return 0;
    
#else
    return sizeof(unsigned long long);
    
#endif
    
  }

  void my_MPI_Pack(void* buff, unsigned int size_pack, int* position) const {
#ifdef USE_STATIC_BITSET
    
    std::vector<unsigned long long> arr = to_ullongs(state);
    size_t nb_ullongs = arr.size();
    MPI_Pack(&nb_ullongs, 1, my_MPI_SIZE_T, buff, size_pack, position, MPI_COMM_WORLD);
    
    for (size_t i = 0; i < arr.size(); i++) {
      MPI_Pack(&(arr[i]), 1, MPI_UNSIGNED_LONG_LONG, buff, size_pack, position, MPI_COMM_WORLD);
    }
    
#elif defined(USE_DYNAMIC_BITSET)
    
    
#else
    MPI_Pack(&state, 1, MPI_UNSIGNED_LONG_LONG, buff, size_pack, position, MPI_COMM_WORLD);
    
#endif
  }
  
  void my_MPI_Unpack(void* buff, unsigned int buff_size, int* position) {
#ifdef USE_STATIC_BITSET
    std::vector<unsigned long long> v;
    size_t nb_ullongs;
    MPI_Unpack(buff, buff_size, position, &nb_ullongs, 1, my_MPI_SIZE_T, MPI_COMM_WORLD);
    
    for (size_t i = 0; i < nb_ullongs; i++) {
      unsigned long long t_ullong;
      MPI_Unpack(buff, buff_size, position, &t_ullong, 1, MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
      v.push_back(t_ullong);
    }
    
    state = to_bitset(v);

#elif defined(USE_DYNAMIC_BITSET)
    
    
#else
    MPI_Unpack(buff, buff_size, position, &state, 1, MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
    
#endif
    
  }
  void my_MPI_Recv(int source) 
  {
#ifdef USE_STATIC_BITSET
    std::vector<unsigned long long> v;
    size_t nb_ullongs;
    MPI_Recv(&nb_ullongs, 1, my_MPI_SIZE_T, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    for (size_t i = 0; i < nb_ullongs; i++) {
      unsigned long long t_ullong;
      MPI_Recv(&t_ullong, 1, MPI_UNSIGNED_LONG_LONG, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      v.push_back(t_ullong);
    }
    
    state = to_bitset(v);
    
#elif defined(USE_DYNAMIC_BITSET)
    
#else
    MPI_Recv(&state, 1, MPI_UNSIGNED_LONG_LONG, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#endif
  }
  
  void my_MPI_Send(int dest) const
  {
#ifdef USE_STATIC_BITSET
    std::vector<unsigned long long> arr = to_ullongs(state);
    size_t nb_ullongs = arr.size();
    MPI_Send(&nb_ullongs, 1, my_MPI_SIZE_T, dest, 0, MPI_COMM_WORLD);
    
    for (size_t i = 0; i < arr.size(); i++) {
      MPI_Send(&(arr[i]), 1, MPI_UNSIGNED_LONG_LONG, dest, 0, MPI_COMM_WORLD);
    }
    
#elif defined(USE_DYNAMIC_BITSET)

#else
    MPI_Send(&state, 1, MPI_UNSIGNED_LONG_LONG, dest, 0, MPI_COMM_WORLD);

#endif
  }
#endif

  static NodeState getState(Node* node, const NetworkState_Impl &state) {
#if defined(USE_STATIC_BITSET) || defined(USE_DYNAMIC_BITSET)
    return state.test(node->getIndex());
#else
    return state & nodeBit(node);
#endif
  }

#ifdef USE_STATIC_BITSET
  
  static std::vector<unsigned long long> to_ullongs(std::bitset<MAXNODES> bs) {
    std::vector<unsigned long long> ret;
    ret.clear();

    unsigned int nb_arrays = MAXNODES/64 + (MAXNODES%64 > 0 ? 1 : 0);
    for (unsigned int i=0; i < nb_arrays; i++) {
        ret.push_back(((bs<<(i*64))>>(MAXNODES-64)).to_ullong());   
    }
  
    return ret;
  }

  static std::bitset<MAXNODES> to_bitset(std::vector<unsigned long long> arr) {
    std::bitset<MAXNODES> ret;
    
    for (unsigned int i=0; i < (arr.size()-1); i++) 
    {
        ret |= arr[i];
    
        if (i < (arr.size()-2)) {
            ret = ret << 64;
        } else {
            ret = ret << (MAXNODES-((i+1)*64));        
            ret |= (arr[i+1] >> (MAXNODES-((i+1)*64)));
        } 
    } 
    return ret;
  }
#endif

};
namespace std {
  template <> struct HASH_STRUCT<NetworkState>
  {
    size_t operator()(const NetworkState& val) const {
      return std::hash<NetworkState_Impl>{}(val.getState());
    }
  };
  
  template <> struct equal_to<NetworkState>
  {
    size_t operator()(const NetworkState& val1, const NetworkState& val2) const {
      return std::equal_to<NetworkState_Impl>{}(val1.getState(), val2.getState());
    }
  };

  // Added less operator, necessary for maps, sets. Code from https://stackoverflow.com/a/21245301/11713763
  template <> struct less<NetworkState>
  {
    size_t operator()(const NetworkState& val1, const NetworkState& val2) const {
      return std::less<NetworkState_Impl>{}(val1.getState(), val2.getState());
    }
  };
}

// global state of the population boolean network
class PopNetworkState {
  
  std::map<NetworkState_Impl, unsigned int> mp;
  mutable size_t hash;
  mutable bool hash_init;

public:

  const std::map<NetworkState_Impl, unsigned int>& getMap() const {
    return mp;
  }

  size_t getHash() const { 
    // EV 2021-08-23: invalid comparison as hash == 0 can be a valid computed hash; replaced by hash_init
    //if (hash == 0) {
    if (!hash_init) {
      hash = compute_hash();
      hash_init = true;
    }
    return hash; 
  }

 PopNetworkState() : mp(std::map<NetworkState_Impl, unsigned int>()), hash(0) , hash_init(false) { }
 PopNetworkState(const PopNetworkState &p ) : hash(0), hash_init(false) { *this = p; }
 PopNetworkState(const PopNetworkState &p , int copy) : hash(0), hash_init(false) { *this = p; }
 PopNetworkState(std::map<NetworkState_Impl, unsigned int> mp ) : mp(mp), hash(0), hash_init(false) { }

 PopNetworkState(NetworkState_Impl state, unsigned int value) : mp(std::map<NetworkState_Impl, unsigned int>()), hash(0) , hash_init(false) {
    mp[state] = value;
  }
  
  void set() {
    mp.clear();
    hash_init = false;
    hash = 0;
    NetworkState new_state;
    new_state.set();
    mp[new_state.getState()] = 1;
  }
  
  PopNetworkState& operator=(const PopNetworkState &p ) 
  {     
    mp = std::map<NetworkState_Impl, unsigned int>(p.getMap());
    // EV 2021-10-28
    hash = 0;
    hash_init = false;
    return *this;
  }

  PopNetworkState applyMask(const PopNetworkState& mask) const {
    std::map<NetworkState_Impl, unsigned int> new_map;
    NetworkState networkstate_mask = mask.getMap().begin()->first;
        
    for (const auto & elem: mp) {
      NetworkState_Impl new_state = elem.first & networkstate_mask.getState();
      new_map[new_state] = elem.second;
    }
    return PopNetworkState(new_map);
  }

  void addStatePop(const NetworkState_Impl& state, unsigned int pop) {
    auto iter = mp.find(state);
    if (iter == mp.end()) {
      mp[state] = pop;
    } else {
      iter->second += pop;
    }
    // EV 2021-10: the following code was missing
    hash = 0;
    hash_init = false;
  }
  
  // & operator for applying the mask
  PopNetworkState operator&(const NetworkState_Impl& mask) const { 
    
    PopNetworkState masked_pop_state;
    for (const auto &network_state_pop : mp) {
      NetworkState_Impl masked_network_state = network_state_pop.first & mask;
      masked_pop_state.addStatePop(masked_network_state, network_state_pop.second);
    }
    
    return masked_pop_state; 
  }
  
  // & operator for applying the mask
  PopNetworkState operator&(const NetworkState& mask) const { 
    
    PopNetworkState masked_pop_state;
    for (const auto &network_state_pop : mp) {
      NetworkState_Impl masked_network_state = network_state_pop.first & mask.getState();
      masked_pop_state.addStatePop(masked_network_state, network_state_pop.second);
    }
    
    return masked_pop_state; 
  }
  
  bool operator==(const PopNetworkState& pop_state) const {

    // So when are two PopNetworkState inequals ?
    // First, if they don't have the same length of states in the population
    const std::map<NetworkState_Impl, unsigned int>& other_mp = pop_state.getMap();

    if (mp.size() != other_mp.size()) {
      return false;
    }
    
   // EV 2021-10-28: std::map are ordered, so it is just necessary to compare
    // return std::equal(mp.begin(), mp.end(), other_mp.begin());
    
    std::map<NetworkState_Impl, unsigned int>::const_iterator iter = mp.begin();
    std::map<NetworkState_Impl, unsigned int>::const_iterator other_iter = other_mp.begin();
    for ( ; iter != mp.end(); ++iter, ++other_iter) {
      if ((iter->first != other_iter->first) || (iter->second != other_iter->second)) {
	return false;
      }
    }
    return true;
  }
  
  // Increases the population of the state
  void incr(const NetworkState& net_state) {
    NetworkState_Impl t_state = net_state.getState();
    auto iter = mp.find(t_state);
    if (iter == mp.end()) {
      mp[t_state] = 1;
    } else {
      iter->second++;
    }
    hash = 0;
    hash_init = false;
  }

  // Decreases the population of the state
  void decr(const NetworkState& net_state) {
    NetworkState_Impl t_state = net_state.getState();
    auto iter = mp.find(t_state);
    assert(iter != mp.end());
    if (iter->second > 1) {
      iter->second--;
    } else {
      mp.erase(t_state);
    }
    hash = 0;
    hash_init = false;
  }
  
  // Returns if the state exists
  bool exists(const NetworkState& net_state) const {
    return mp.find(net_state.getState()) != mp.end();
  }
  
  // EV 2021-10-28: useful in case of using std::map<PopNetworkState, ...>
  bool operator<(const PopNetworkState& pop_state) const {

    const std::map<NetworkState_Impl, unsigned int>& other_mp = pop_state.getMap();

    if (mp.size() != other_mp.size()) {
      return mp.size() < other_mp.size();
    }
    
    // EV 2021-10-28: std::map are ordered => this code is ok
    std::map<NetworkState_Impl, unsigned int>::const_iterator iter = mp.begin();
    std::map<NetworkState_Impl, unsigned int>::const_iterator other_iter = other_mp.begin();
    for ( ; iter != mp.end(); ++iter, ++other_iter) {
      if (iter->first != other_iter->first) {
	return std::less<NetworkState_Impl>{}(iter->first, other_iter->first);
      } else if (iter->second != other_iter->second) {
	return iter->second < other_iter->second;
      }
    }
    return false;
  }

  size_t compute_hash() const {
    
    // New one : for all state:pop, compute sum_i = state_i * pop_i;
    // Expensive, but should be a good one ?
    
    size_t result = 1;
    for (auto &network_state_pop: mp) {
      NetworkState_Impl t_state = network_state_pop.first;
      const unsigned char* p = (const unsigned char*)&t_state;
      for (size_t nn = 0; nn < sizeof(t_state); nn++) {
        unsigned char val = *p++;
        if (val) {
          result *= val;
          result ^= result >> 8;
        }
      }
      p = (const unsigned char*)&network_state_pop.second;
      if (*p) {
	result *= *p;
	result ^= result >> 8;
      }
    }
    return result;
    // EV: 2021-11-24 note: returning another hash code changes the results, for instance:
    // return (size_t)(result*1.1);
  }
  
  // Count the population satisfying an expression
  unsigned int count(Expression * expr) const;
  void clear() { mp.clear(); hash_init = false; }
  std::string getName(Network * network, const std::string& sep=" -- ") const;
  void displayOneLine(std::ostream& os, Network* network, const std::string& sep = " -- ") const;
  void displayJSON(std::ostream& os, Network* network, const std::string& sep = " -- ") const;

  unsigned int hamming(Network* network, const NetworkState& state) const;
  
  static bool isPopState() { return true; }
  std::set<NetworkState_Impl>* getNetworkStates() {
    std::set<NetworkState_Impl>* result = new std::set<NetworkState_Impl>();
    for (auto network_state_pop : mp) {
      result->insert(network_state_pop.first);
    }
    return result;
  }
};

namespace std {
  template <> struct hash<PopNetworkState>
  {
    size_t operator()(const PopNetworkState & x) const
    {
      return x.getHash();
    }
  };
}

// abstract base class used for expression evaluation
class Expression {

public:
  virtual double eval(const Node* this_node, const NetworkState& network_state) const = 0;
  virtual double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop_state) const = 0;
  
  virtual bool hasCycle(Node* node) const = 0;

  std::string toString() const {
    std::ostringstream ostr;
    display(ostr);
    return ostr.str();
  }

  virtual Expression* clone() const = 0; 

  virtual Expression* cloneAndShrink(bool& shrinked) const {
    return clone();
  }

  virtual void display(std::ostream& os) const = 0;
  virtual bool isConstantExpression() const {return false;}
  virtual bool isLogicalExpression() const {return false;}
  virtual std::vector<Node*> getNodes() const{return std::vector<Node*>(); }
#ifdef SBML_COMPAT
  virtual ASTNode* writeSBML(LogicalExprGenContext& genctx) const { return new ASTNode(AST_CONSTANT_TRUE); }
#endif
  virtual void generateLogicalExpression(LogicalExprGenContext& genctx) const = 0;
  virtual bool generationWillAddParenthesis() const {return false;}

  bool evalIfConstant(double& value) const;
  bool evalIfConstant(bool& value) const;

  static Expression* cloneAndShrinkRecursive(Expression* expr);

  virtual const NotLogicalExpression* asNotLogicalExpression() const {return NULL;}

  virtual ~Expression() {
  }
};

class NodeExpression : public Expression {
  Node* node;

public:
  NodeExpression(Node* node) : node(node) { }

  Expression* clone() const {return new NodeExpression(node);}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return (double)node->getNodeState(network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop_state) const {
    return (double)node->getNodeState(network_state);
  }

  bool hasCycle(Node* node) const {
    return this->node == node;
  }

  void display(std::ostream& os) const {
    os << node->getLabel();
  }

  bool isLogicalExpression() const {return true;}
  
  std::vector<Node*> getNodes() const{
    std::vector<Node*> vec;
    vec.push_back(node);
    return vec;
  }

#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {
    ASTNode* equ = new ASTNode(AST_RELATIONAL_EQ);
    ASTNode* a_node = new ASTNode(AST_NAME);
    a_node->setId(node->getLabel());
    a_node->setName(node->getLabel().c_str());
    ASTNode* one = new ASTNode(AST_INTEGER);
    one->setValue(1);
    
    equ->addChild(a_node);
    equ->addChild(one);
    return equ;
  }
#endif
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

  ~NodeExpression() {
    //delete node;
  }
};

class PopExpression : public Expression {
  Expression* expr;

public:
  PopExpression(Expression* expr) : expr(expr) { }

  Expression* clone() const {return new PopExpression(expr);}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return 0.;
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop_state) const {
    return (double) pop_state.count(expr);
  }
  
  bool hasCycle(Node* node) const {
    return false;
  }

  void display(std::ostream& os) const {
    os << "#cell(";
    expr->display(os);
    os << ")";
  }

  bool isLogicalExpression() const {return true;}
  std::vector<Node*> getNodes() const{
    return std::vector<Node*>();
  }
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

  ~PopExpression() {
  }
};

// concrete classes used for expression evaluation
class BinaryExpression : public Expression {

protected:
  Expression* left;
  Expression* right;

public:
  BinaryExpression(Expression* left, Expression* right) : left(left), right(right) { }

  bool hasCycle(Node* node) const {
    return left->hasCycle(node) || right->hasCycle(node);
  }

  virtual bool isConstantExpression() const {
    return left->isConstantExpression() && right->isConstantExpression();
  }
 
  std::vector<Node*> getNodes() const{
    std::vector<Node*> vec1 = left->getNodes();
    std::vector<Node*> vec2 = right->getNodes();
    std::vector<Node*> vec(vec1.begin(), vec1.end());
    for (auto* node : vec2) {
      if (std::find(vec.begin(), vec.end(), node) == vec.end()) {
        vec.push_back(node);
      }
    }
    return vec;
  }
  
  virtual ~BinaryExpression() {
    delete left;
    delete right;
  }
};

class MulExpression : public BinaryExpression {

public:
  MulExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new MulExpression(left->clone(), right->clone());}
  Expression* cloneAndShrink(bool& shrinked) const;
  
  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) * right->eval(this_node, network_state);
  }
  
  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) * right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " * ";
    right->display(os);
    os <<  ")";
  }

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class DivExpression : public BinaryExpression {

public:
  DivExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new DivExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) / right->eval(this_node, network_state);
  }
  
  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) / right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " / ";
    right->display(os);
    os << ")";
  }

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class AddExpression : public BinaryExpression {

public:
  AddExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new AddExpression(left->clone(), right->clone());}
  Expression* cloneAndShrink(bool& shrinked) const;

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) + right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) + right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " + ";
    right->display(os);
    os << ")";
  }

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class SubExpression : public BinaryExpression {

public:
  SubExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new SubExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) - right->eval(this_node, network_state);
  }
  
  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) - right->eval(this_node, network_state, pop);
  }
  
  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " - ";
    right->display(os);
    os << ")";
  }

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class EqualExpression : public BinaryExpression {

public:
  EqualExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new EqualExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) == right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) == right->eval(this_node, network_state, pop);
  }
  
  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " == ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class NotEqualExpression : public BinaryExpression {

public:
  NotEqualExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new NotEqualExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) != right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) != right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " != ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class LetterExpression : public BinaryExpression {

public:
  LetterExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new LetterExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) < right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) < right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " < ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class LetterOrEqualExpression : public BinaryExpression {

public:
  LetterOrEqualExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new LetterOrEqualExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) <= right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) <= right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " <= ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class GreaterExpression : public BinaryExpression {

public:
  GreaterExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new GreaterExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) > right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) > right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " > ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class GreaterOrEqualExpression : public BinaryExpression {

public:
  GreaterOrEqualExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new GreaterOrEqualExpression(left->clone(), right->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return left->eval(this_node, network_state) >= right->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return left->eval(this_node, network_state, pop) >= right->eval(this_node, network_state, pop);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " >= ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class CondExpression : public Expression {

  Expression* cond_expr;
  Expression* true_expr;
  Expression* false_expr;

public:
  CondExpression(Expression* cond_expr, Expression* true_expr, Expression* false_expr) : cond_expr(cond_expr), true_expr(true_expr), false_expr(false_expr) { }

  Expression* clone() const {return new CondExpression(cond_expr->clone(), true_expr->clone(), false_expr->clone());}

  Expression* cloneAndShrink(bool& shrinked) const;
  //  Expression* cloneAndShrink(bool& shrinked) const {return new CondExpression(cond_expr->cloneAndShrink(shrinked), true_expr->cloneAndShrink(shrinked), false_expr->cloneAndShrink(shrinked));}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    if (0. != cond_expr->eval(this_node, network_state)) {
      return true_expr->eval(this_node, network_state);
    }
    return false_expr->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    if (0. != cond_expr->eval(this_node, network_state, pop)) {
      return true_expr->eval(this_node, network_state, pop);
    }
    return false_expr->eval(this_node, network_state, pop);
  }

  bool hasCycle(Node* node) const {
    return cond_expr->hasCycle(node) || true_expr->hasCycle(node) || false_expr->hasCycle(node);
  }

  void display(std::ostream& os) const {
    os <<  "(";
    cond_expr->display(os);
    os <<  " ? ";
    true_expr->display(os);
    os <<  " : ";
    false_expr->display(os);
    os << ")";
  }

  bool isConstantExpression() const {
    return cond_expr->isConstantExpression() && true_expr->isConstantExpression() && false_expr->isConstantExpression();
  }

  bool isLogicalExpression() const {
    return true_expr->isLogicalExpression() && false_expr->isLogicalExpression();
  }

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

  virtual ~CondExpression() {
    delete cond_expr;
    delete true_expr;
    delete false_expr;
  }
};

class ConstantExpression : public Expression {

  double value;

public:
  ConstantExpression(double value) : value(value) { }

  Expression* clone() const {return new ConstantExpression(value);}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return value;
  }
  
  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return value;
  }

  bool hasCycle(Node* node) const {
    return false;
  }

  void display(std::ostream& os) const {
    os << value;
  }

  bool isConstantExpression() const {return true;}

  bool isLogicalExpression() const {return value == 0 || value == 1;}
  
  std::vector<Node*> getNodes() const{
    return std::vector<Node*>();
  }
  
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

};

class SymbolExpression : public Expression {

  SymbolTable* symbol_table;
  const Symbol* symbol;
  mutable bool value_set;
  mutable double value;

public:
  SymbolExpression(SymbolTable* symbol_table, const Symbol* symbol) : symbol_table(symbol_table), symbol(symbol), value_set(false) { 
    symbol_table->addSymbolExpression(this);
  }

  Expression* clone() const {return new SymbolExpression(symbol_table, symbol);}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    if (!value_set) {
      value = symbol_table->getSymbolValue(symbol);
      value_set = true;
    }
    return value;
  }
  
  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    if (!value_set) {
      value = symbol_table->getSymbolValue(symbol);
      value_set = true;
    }
    return value;
  }

  bool hasCycle(Node* node) const {
    return false;
  }

  void display(std::ostream& os) const {
    os << symbol->getName();
  }

  bool isConstantExpression() const {return true;}
  
  std::vector<Node*> getNodes() const{
    return std::vector<Node*>();
  }
  
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

  void unset() { value_set = false; }
};

class AliasExpression : public Expression {
  std::string identifier;

  const Expression* getAliasExpression(const Node* this_node) const {
    if (NULL != this_node) {
      return this_node->getAttributeExpression(identifier);
    }
    return NULL;
  }

  mutable const Expression* alias_expr;

public:
  AliasExpression(const std::string& identifier) : identifier(identifier), alias_expr(NULL) { }

  Expression* clone() const {return new AliasExpression(identifier);}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    if (NULL == alias_expr) {
      alias_expr = getAliasExpression(this_node);
    }
    if (NULL != alias_expr) {
      return alias_expr->eval(this_node, network_state);
    }
    if (NULL != this_node) {
      throw BNException("invalid use of alias attribute @" + identifier + " in node " + this_node->getLabel());
    }
    throw BNException("invalid use of alias attribute @" + identifier + " in unknown node");
  }
  
  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    if (NULL == alias_expr) {
      alias_expr = getAliasExpression(this_node);
    }
    if (NULL != alias_expr) {
      return alias_expr->eval(this_node, network_state, pop);
    }
    
    if (NULL != this_node) {
      throw BNException("invalid use of alias attribute @" + identifier + " in node " + this_node->getLabel());
    }
    throw BNException("invalid use of alias attribute @" + identifier + " in unknown node");
  }

  bool hasCycle(Node* node) const {
    return false;
  }

  void display(std::ostream& os) const {
    os << '@' << identifier;
  }
#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {
    alias_expr = getAliasExpression(genctx.getNode());
    if (NULL != alias_expr) {
      return alias_expr->writeSBML(genctx);
    }
    else return new ASTNode(AST_CONSTANT_FALSE);
  }
#endif
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class OrLogicalExpression : public BinaryExpression {

public:
  OrLogicalExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }
  Expression* cloneAndShrink(bool& shrinked) const;

  Expression* clone() const {return new OrLogicalExpression(left->clone(), right->clone());}

  bool generationWillAddParenthesis() const {return true;}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return (double)((bool)left->eval(this_node, network_state) || (bool)right->eval(this_node, network_state));
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return (double)((bool)left->eval(this_node, network_state, pop) || (bool)right->eval(this_node, network_state, pop));
  }
  
  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " OR ";
    right->display(os);
    os << ")";
  }

  virtual bool isLogicalExpression() const {return true;}

#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {
    ASTNode* op = new ASTNode(AST_LOGICAL_OR);
    
    op->addChild(left->writeSBML(genctx));
    op->addChild(right->writeSBML(genctx));
    return op;
  }
#endif
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class AndLogicalExpression : public BinaryExpression {

public:
  AndLogicalExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new AndLogicalExpression(left->clone(), right->clone());}
  Expression* cloneAndShrink(bool& shrinked) const;

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return (double)((bool)left->eval(this_node, network_state) && (bool)right->eval(this_node, network_state));
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return (double)((bool)left->eval(this_node, network_state, pop) && (bool)right->eval(this_node, network_state, pop));
  }

  bool generationWillAddParenthesis() const {return true;}

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " AND ";
    right->display(os);
    os << ")";
  }

  bool isLogicalExpression() const {return true;}
#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {
    ASTNode* op = new ASTNode(AST_LOGICAL_AND);
    
    op->addChild(left->writeSBML(genctx));
    op->addChild(right->writeSBML(genctx));
    return op;
  }
#endif
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class XorLogicalExpression : public BinaryExpression {

public:
  XorLogicalExpression(Expression* left, Expression* right) : BinaryExpression(left, right) { }

  Expression* clone() const {return new XorLogicalExpression(left->clone(), right->clone());}

  Expression* cloneAndShrink(bool& shrinked) const;

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return (double)((bool)left->eval(this_node, network_state) ^ (bool)right->eval(this_node, network_state));
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return (double)((bool)left->eval(this_node, network_state, pop) ^ (bool)right->eval(this_node, network_state, pop));
  }

  void display(std::ostream& os) const {
    os <<  "(";
    left->display(os);
    os <<  " XOR ";
    right->display(os);
    os << ")";
  }

  bool isLogicalExpression() const {return true;}
#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {
    ASTNode* op = new ASTNode(AST_LOGICAL_XOR);
    
    op->addChild(left->writeSBML(genctx));
    op->addChild(right->writeSBML(genctx));
    return op;
  }
#endif
  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
};

class NotLogicalExpression : public Expression {
  Expression* expr;

public:
  NotLogicalExpression(Expression* expr) : expr(expr) { }

  Expression* clone() const {return new NotLogicalExpression(expr->clone());}
  Expression* cloneAndShrink(bool& shrinked) const;

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return (double)(!((bool)expr->eval(this_node, network_state)));
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return (double)(!((bool)expr->eval(this_node, network_state, pop)));
  }

  bool hasCycle(Node* node) const {
    return expr->hasCycle(node);
  }

  const NotLogicalExpression* asNotLogicalExpression() const {return this;}

  void display(std::ostream& os) const {
    os <<  "NOT ";
    expr->display(os);
  }

  bool isLogicalExpression() const {return true;}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;
#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {
    ASTNode* op = new ASTNode(AST_LOGICAL_NOT);
    
    op->addChild(expr->writeSBML(genctx));
    return op;
  }
#endif
  ~NotLogicalExpression() {
    delete expr;
  }
};

class ParenthesisExpression : public Expression {
  Expression* expr;

public:
  ParenthesisExpression(Expression* expr) : expr(expr) { }

  Expression* clone() const {return new ParenthesisExpression(expr->clone());}

  Expression* cloneAndShrink(bool& shrinked) const {
    return new ParenthesisExpression(expr->cloneAndShrink(shrinked));
  }

  bool generationWillAddParenthesis() const {return true;}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    return expr->eval(this_node, network_state);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    return expr->eval(this_node, network_state, pop);
  }

  const NotLogicalExpression* asNotLogicalExpression() const {return expr->asNotLogicalExpression();}

  bool hasCycle(Node* node) const {
    return expr->hasCycle(node);
  }

  void display(std::ostream& os) const {
    os <<  '(';
    expr->display(os);
    os <<  ')';
  }

  bool isConstantExpression() const {return expr->isConstantExpression();}
  bool isLogicalExpression() const {return expr->isLogicalExpression();}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

#ifdef SBML_COMPAT
  ASTNode* writeSBML(LogicalExprGenContext& genctx) const {

    return expr->writeSBML(genctx);
  }
#endif

  virtual ~ParenthesisExpression() {
    delete expr;
  }
};

class ArgumentList {
  std::vector<Expression*> expr_v;

public:
  ArgumentList() { }

  void push_back(Expression* expr) {expr_v.push_back(expr);}

  ArgumentList* clone() const {
    ArgumentList* arg_list_cloned = new ArgumentList();
    for (const auto * expr : expr_v) {
      arg_list_cloned->push_back(expr->clone());
    }
    return arg_list_cloned;
  }

  bool hasCycle(Node* node) const {
    for (const auto * expr : expr_v) {
      if (expr->hasCycle(node)) {
	return true;
      }
    }
    return false;
  }

  bool isConstantExpression() const {
    for (const auto * expr : expr_v) {
      if (!expr->isConstantExpression()) {
	return false;
      }
    }
    return true;
  }

  void display(std::ostream& os) const {
    unsigned int nn = 0;
    for (const auto * expr : expr_v) {
      os << (nn > 0 ? ", " : "");
      expr->display(os);
      nn++;
    }
  }

  const std::vector<Expression*>& getExpressionList() const { return expr_v; }
  size_t getExpressionListCount() const { return expr_v.size(); }

  ~ArgumentList() {
    for (auto * expr : expr_v)
      delete expr;
    
  }
};

class FuncCallExpression : public Expression {
  std::string funname;
  ArgumentList* arg_list;
  Function* function;
  bool is_const = false;
  double value;

public:
  FuncCallExpression(const std::string& funname, ArgumentList* arg_list) : funname(funname), arg_list(arg_list), function(NULL), value(0.) {
    function = Function::getFunction(funname);

    if (function == NULL) {
      throw BNException("unknown function " + funname);
    }
    function->check(arg_list);
    
    // This part is evaluating the formula, but if there is a parameter its value
    // has not been parsed yet. So this will fail. 
    // is_const = function->isDeterministic() && isConstantExpression();
    // if (is_const) {
    //   NetworkState network_state;
    //   value = function->eval(NULL, network_state, arg_list);
    // }
 }

  Expression* clone() const {return new FuncCallExpression(funname, arg_list->clone());}

  double eval(const Node* this_node, const NetworkState& network_state) const {
    if (is_const) {
      return value;
    }
    return function->eval(this_node, network_state, arg_list);
  }

  double eval(const Node* this_node, const NetworkState& network_state, const PopNetworkState& pop) const {
    if (is_const) {
      return value;
    }
    return function->eval(this_node, network_state, pop, arg_list);
  }
  void display(std::ostream& os) const {
    os <<  funname << '(';
    arg_list->display(os);
    os <<  ')';
  }

  bool hasCycle(Node* node) const {
    return arg_list->hasCycle(node);
  }

  bool isConstantExpression() const {return arg_list->isConstantExpression();}

  void generateLogicalExpression(LogicalExprGenContext& genctx) const;

  virtual ~FuncCallExpression() {
    delete arg_list;
  }
};




class PopIStateGroup {
  
public:
  class PopProbaIState {
    public:

    class PopIStateGroupIndividual {
      public:
      std::vector<double> state_value_list;
      unsigned int pop_size;
      
      PopIStateGroupIndividual(std::vector<Expression*>* state_expr_list, Expression* t_pop_size) 
      {
        NetworkState network_state;

        for (auto state_expr: *state_expr_list) {
          state_value_list.push_back(state_expr->eval(NULL, network_state));
        }
        
        pop_size = (unsigned int) t_pop_size->eval(NULL, network_state);  
      }
      
      std::vector<double> getStateValueList() { return state_value_list; }
      unsigned int getPopSize() { return pop_size; }  
    };



    double proba_value;
    Expression* proba_expr;
    std::vector<PopIStateGroupIndividual*>* individual_list;
    
    PopProbaIState(Expression* proba_expr, std::vector<PopIStateGroupIndividual*>* individual_list) 
    {
      this->proba_expr = proba_expr;
      NetworkState network_state;
      proba_value = proba_expr->eval(NULL, network_state);
      
      this->individual_list = individual_list;
    }
    std::vector<PopIStateGroupIndividual*>* getIndividualList() { return individual_list; }
    double getProbaValue() { return proba_value; }
    
  };
  
  std::vector<const Node*>* nodes;
  std::vector<PopProbaIState*>* proba_istates;
  
  PopIStateGroup(Network* network, std::vector<const Node*>* nodes, std::vector<PopProbaIState*>* proba_istates, std::string& error_msg) : nodes(nodes), proba_istates(proba_istates) 
  {
    epilogue(network); 
  }
    
  void epilogue(Network* network) 
  {
    PopNetwork* pop_network = static_cast<PopNetwork*>(network);
    pop_network->getPopIStateGroup()->push_back(this);
  }
  
  std::vector<const Node*>* getNodes() { return nodes; }
  std::vector<PopProbaIState*>* getPopProbaIStates() { return proba_istates; }
  
  static void initPopStates(PopNetwork* network, PopNetworkState& initial_state, RandomGenerator* randgen, unsigned int pop);

};

class IStateGroup {

public:
  struct ProbaIState {
    double proba_value;
    std::vector<double>* state_value_list;

    ProbaIState(Expression* proba_expr, std::vector<Expression*>* state_expr_list) {
      NetworkState network_state;
      proba_value = proba_expr->eval(NULL, network_state);
      
      state_value_list = new std::vector<double>();
      for (auto * state_expr : *state_expr_list)
      {
        NetworkState network_state;
        state_value_list->push_back(state_expr->eval(NULL, network_state));
	    }
    }
  
    // only one node
    ProbaIState(double proba_value, Expression* state_expr) {
      this->proba_value = proba_value;
      state_value_list = new std::vector<double>();
      NetworkState network_state;
      state_value_list->push_back(state_expr->eval(NULL, network_state));
    }

    ProbaIState(double proba_value, double istate_value) {
      this->proba_value = proba_value;
      state_value_list = new std::vector<double>();
      state_value_list->push_back(istate_value);
    }
    
    ProbaIState(ProbaIState* obj) {
      this->proba_value = obj->getProbaValue();
      this->state_value_list = new std::vector<double>(*(obj->getStateValueList()));
    }

    ~ProbaIState() {
      delete state_value_list;
    }
    double getProbaValue() {return proba_value;}
    std::vector<double>* getStateValueList() {return state_value_list;}
    void normalizeProbaValue(double proba_sum) {proba_value /= proba_sum;}
  };
  
  IStateGroup(Network* network, std::vector<const Node*>* nodes, std::vector<ProbaIState*>* proba_istates, std::string& error_msg) : nodes(nodes), proba_istates(proba_istates) {
    is_random = false;
    size_t node_size = nodes->size();
    for (auto * proba_istate : *proba_istates)
    {
      if (proba_istate->getStateValueList()->size() != node_size) {
        std::ostringstream ostr;
        ostr << "size inconsistency in istate expression: got " <<  proba_istate->getStateValueList()->size() << " states, has " << node_size << " nodes";
        error_msg = ostr.str();
        return;
      }
    }
    epilogue(network);
 }

  IStateGroup(Network * network, const Node* node) {
    is_random = true;
    nodes = new std::vector<const Node*>();
    nodes->push_back(node);
    proba_istates = new std::vector<ProbaIState*>();
    proba_istates->push_back(new ProbaIState(0.5, 0.));
    proba_istates->push_back(new ProbaIState(0.5, 1.));
    epilogue(network);
  }

  IStateGroup(Network * network, const Node* node, Expression* expr) {
    is_random = false;
    nodes = new std::vector<const Node*>();
    nodes->push_back(node);
    proba_istates = new std::vector<ProbaIState*>();
    proba_istates->push_back(new ProbaIState(1., expr));
    epilogue(network);
  }
  
  IStateGroup(IStateGroup* obj, Network* network) {
    this->is_random = obj->isRandom();
    this->nodes = new std::vector<const Node*>();
    for (const auto node: *(obj->getNodes())) {
      this->nodes->push_back(node);
    }
    this->proba_istates = new std::vector<ProbaIState*>();
    for(auto proba_istate: *(obj->getProbaIStates())) {
      this->proba_istates->push_back(new ProbaIState(proba_istate));
    }
    epilogue(network);
  }
  
  ~IStateGroup() {
    delete nodes;
    for (auto * proba_istate : *proba_istates)
      delete proba_istate;

    delete proba_istates;
  }

  std::vector<const Node*>* getNodes() {return nodes;}
  std::vector<ProbaIState*>* getProbaIStates() {return proba_istates;}
  double getProbaSum() const {return proba_sum;}

  bool isRandom() const {return is_random;}

  bool hasNode(Node * node) {
    for (auto * t_node : *nodes)
      if (node == t_node)
        return true;
      
    return false;
  }

  static void checkAndComplete(Network* network);
  static void initStates(Network* network, NetworkState& initial_state, RandomGenerator * randgen);
  static void display(Network* network, std::ostream& os);
  static void reset(Network* network);
  

    static void setNodeProba(Network * network, Node * node, double value) {

    for (auto * istate_group : *(network->getIStateGroup()))
    {
      if (istate_group->hasNode(node)) {

        std::vector<IStateGroup::ProbaIState*>* proba_istates = istate_group->getProbaIStates();
        std::vector<const Node*>* group_nodes = istate_group->getNodes();

        // Simple case, if the node is alone
        if (group_nodes->size() == 1) {

          // Do we also need to free memory for all ProbaIStaste ? 
          for (size_t i=0; i < proba_istates->size(); i++) {
            delete (*proba_istates)[i];
          }
          proba_istates->clear();

          // Again, simple case, we just need one ProbaIState
          if (value == 0.0 || value == 1.0) {            
            proba_istates->push_back(new ProbaIState(1.0, value));

          } else {
            proba_istates->push_back(new ProbaIState(1.0-value, 0.0));
            proba_istates->push_back(new ProbaIState(value, 1.0));
          }
        }
        // Otherwise, the simplest might be to remove it and create a new IStateGroup with this node alone
        else {

          //First, we erase it from nodes, and get its index
          size_t i;
          for(i = 0; i < group_nodes->size(); i++) {
              if (group_nodes->at(i) == node) {
                group_nodes->erase(group_nodes->begin() + (std::ptrdiff_t) i);
                break;
              }
          }

          for (auto * proba_istate : *proba_istates)
          {
            proba_istate->state_value_list->erase(proba_istate->state_value_list->begin() + (std::ptrdiff_t) i);
          }

          // Finally, we add a proba_istate with the desired value
          std::vector<const Node*>* new_nodes = new std::vector<const Node*>();

          new_nodes->push_back(node);

          std::vector<IStateGroup::ProbaIState*>* new_proba_istates = new std::vector<IStateGroup::ProbaIState*>();

          if (value == 0.0 || value == 1.0) {            
            new_proba_istates->push_back(new ProbaIState(1.0, value));

          } else {
            new_proba_istates->push_back(new ProbaIState(1.0-value, 0.0));
            new_proba_istates->push_back(new ProbaIState(value, 1.0));
          }

          std::string message = "";

          new IStateGroup(network, new_nodes, new_proba_istates, message);
        }
      }
    }
  }

static void setInitialState(Network * network, NetworkState * state) {

  for (auto * node : network->getNodes())
  {
    // std::cout << "Setting " << (*it)->getLabel() << " to " << (*it)->getNodeState(*state) << std::endl;
    setNodeProba(network, node, node->getNodeState(*state));
  }
  
}
private:
  std::vector<const Node*>* nodes;
  std::vector<ProbaIState*>* proba_istates;
  double proba_sum;
  bool is_random;

  void computeProbaSum() 
  {  
    proba_sum = 0;
    for (auto * proba_istate : *proba_istates)
      proba_sum += proba_istate->getProbaValue();
    
    for (auto * proba_istate : *proba_istates)
      proba_istate->normalizeProbaValue(proba_sum);
    
  }

  void epilogue(Network* network) {
    computeProbaSum();
    network->getIStateGroup()->push_back(this);
  }
};

extern const bool backward_istate;

extern bool dont_shrink_logical_expressions;
extern int setConfigVariables(Network* network, const std::string& prog, const std::string& runvar);
extern int setConfigVariables(Network* network, const std::string& prog, std::vector<std::string>& runvar_v);

#endif
