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
     SBMLParser.h
     
     Authors:
     Vincent Noel <vincent-noel@curie.fr>
     Marco Ruscone <marco.ruscone@curie.fr>
     
     Date:
     June 2021
*/

#if defined SBML_COMPAT && !defined _SBML_PARSER_H_
#define _SBML_PARSER_H_
#include "BooleanNetwork.h"
#include "BooleanGrammar.h"
#include <sbml/packages/qual/extension/QualModelPlugin.h>
#include <sbml/SBMLTypes.h>

LIBSBML_CPP_NAMESPACE_USE

class SBMLParser
{
  public:
  
  Network* network;
  bool useSBMLNames;
  Model* model;
  QualModelPlugin* qual_model;
  std::map<std::string, int> maxLevels;
  std::map<std::string, std::vector<std::string> > fixedNames;
  
  SBMLParser(Network* network, const char* file, bool useSBMLNames) : network(network), useSBMLNames(useSBMLNames) {
    
    SBMLDocument* document;
    SBMLReader reader;
    
    document = reader.readSBML(file);
    // unsigned int errors = document->getNumErrors();
    
    // if (errors > 0) {
    //     for (int i=0; i < document->getNumErrors(); i++) {
    //         std::cout << "Error #" << i << " : " << document->getError(i)->getMessage() << std::endl;
    //     }
    //     throw BNException("There are errors in the sbml file");  
    // }
    
    SBasePlugin* qual = document->getPlugin("qual");
    if (qual == NULL) {
        throw BNException("This SBML model is not a qualitative sbml");
    }
    
    this->model = document->getModel();
    this->qual_model = static_cast<QualModelPlugin*>(model->getPlugin("qual"));

    for (unsigned int i=0; i < qual_model->getNumQualitativeSpecies(); i++) {
        QualitativeSpecies* specie = qual_model->getQualitativeSpecies(i);
        std::string new_name;
        if (useSBMLNames && specie->isSetName()) {
            new_name = specie->getName();
            new_name.erase(std::remove_if(
                new_name.begin(), new_name.end(), 
                [](char c) { return !std::isalnum(c) && c != '_'; }
                ), new_name.end()
            );
        } else {
            new_name = specie->getId();
        }
        
        this->maxLevels[specie->getId()] = specie->isSetMaxLevel() ? specie->getMaxLevel() : 1;
        std::vector<std::string> t_fixed_names;
        if (this->maxLevels[specie->getId()] > 1) {
            for (int j=1; j <= this->maxLevels[specie->getId()]; j++) {
                t_fixed_names.push_back(new_name + "_b" + std::to_string(j));
            }
        } else t_fixed_names.push_back(new_name);
        
        this->fixedNames[specie->getId()] = t_fixed_names;
    }
  }
  std::string getName(std::string id, int level) {
    return this->fixedNames[id][level-1];
  }
  
  void build() {
      
    for (unsigned int i=0; i < qual_model->getNumTransitions(); i++) {
        Transition* transition = qual_model->getTransition(i);
        parseTransition(transition);
    }
    
    for (unsigned int i=0; i < qual_model->getNumQualitativeSpecies(); i++) {
        QualitativeSpecies* specie = qual_model->getQualitativeSpecies(i);
       
        for (int j=0; j < this->maxLevels[specie->getId()]; j++){
            if (!this->network->isNodeDefined(getName(specie->getId(), j+1))) {
            
                Expression* input_node = new NodeExpression(this->network->getOrMakeNode(getName(specie->getId(), j+1)));
                // Here we also need to declare lower nodes if lvl > 1
                
                // Here we add terms to enforce : 
                // - To activate one level, the lower one needs to be active : exp & level(i-1)
                for(int k=1; k <= j; k++) {
                    Expression* lower_outputs = new NodeExpression(
                        this->network->getOrMakeNode(getName(specie->getId(), k))
                    );
                    
                    input_node = new AndLogicalExpression(
                        input_node,
                        lower_outputs
                    );
                }
                
                NodeDeclItem* decl_item = new NodeDeclItem("logic", input_node);
                std::vector<NodeDeclItem*>* decl_item_v = new std::vector<NodeDeclItem*>();
                decl_item_v->push_back(decl_item);

                NodeDecl* truc = new NodeDecl(getName(specie->getId(), j+1), decl_item_v, this->network);

                for (std::vector<NodeDeclItem*>::iterator it = decl_item_v->begin(); it != decl_item_v->end(); ++it) {
                    delete *it;
                }
                
                delete decl_item_v;
                delete truc;
            }
        }
    }
  } 
  
  void parseTransition(Transition* transition) 
  {
    unsigned int num_outputs = transition->getNumOutputs();
    std::vector<std::string> t_outputs;
    int max_level = 0;
    
    for (unsigned int j=0; j < num_outputs; j++) {
        std::string name = transition->getOutput(j)->getQualitativeSpecies();
        t_outputs.push_back(name);
        max_level = std::max(max_level, maxLevels[name]);
        
        if (j < (transition->getNumOutputs()-1)) {
            std::cout << ",";
        }
    }
    
    std::vector<FunctionTerm*> fun_terms(max_level);
    DefaultTerm* def_term = NULL;
    int i_fun_term = -1;
    int nb_fun_term = 0;
    while(i_fun_term < ((int) transition->getNumFunctionTerms())) {
        if (i_fun_term == -1) {
            def_term = transition->getDefaultTerm();
            
        } else {
            fun_terms[transition->getFunctionTerm(i_fun_term)->getResultLevel()-1] = transition->getFunctionTerm(i_fun_term);
            nb_fun_term++;
        }
        
        i_fun_term++;
    }
    
    if (nb_fun_term == 0 && def_term == NULL) {
        throw BNException("Could not find the activating expression");
    }
    
    Expression* exp = NULL;
    
    if (def_term != NULL && nb_fun_term == 0){
        
        // Here what we miss is where nb_fun_term > 0 and def_term != null
        // In this case, for now, def_term is ignored. Which is ok, since most of the time def_term.resultLevel is 0
        // But if it's not, then we fail...
        
        if (def_term->getResultLevel() <= 1) {
            createNodes(t_outputs, new ConstantExpression((double) def_term->getResultLevel()));
        } else {
            std::vector<std::string> new_outputs;
            for (auto t_output: t_outputs) {
                for (int i=0; i < def_term->getResultLevel(); i++)
                {
                    new_outputs.push_back(getName(t_output, i+1));
                }
            }
            createNodes(t_outputs, new ConstantExpression(1.0));
        }
    }
    
    else {
        for (int j=0; j < max_level; j++) {
            if (fun_terms[j] != NULL && fun_terms[j]->getMath() != NULL) {
                exp = parseASTNode(fun_terms[j]->getMath());
                
                std::vector<std::string> new_outputs;
                for (auto new_output: t_outputs) {
                    new_outputs.push_back(getName(new_output, j+1));
                }
                
                // Here we need to modify lower terms, because exp(level_i) = exp(level_i) | exp(level_i+1) | ... | exp(level_n)
                for(int k=j+1; k < max_level; k++) {
                    if (fun_terms[k] != NULL && fun_terms[k]->getMath() != NULL) {
                        exp = new OrLogicalExpression(
                            exp,
                            parseASTNode(fun_terms[k]->getMath())
                        );
                    }
                }
                
                // And here we also need &(!lvl_i | lvl_i&!lvl_i+1)
                if ((j+1) < max_level) {
                    Node* node_i = this->network->getOrMakeNode(getName(t_outputs[0], fun_terms[j]->getResultLevel()));
                    Node* node_ip1 = this->network->getOrMakeNode(getName(t_outputs[0], fun_terms[j]->getResultLevel()+1));
                    exp = new AndLogicalExpression(
                        exp,
                        new OrLogicalExpression(
                            new NotLogicalExpression(
                                new NodeExpression(
                                    node_i
                                )
                            ),
                            new AndLogicalExpression(
                                new NodeExpression(
                                    node_i
                                ),
                                new NotLogicalExpression(
                                    new NodeExpression(
                                        node_ip1
                                    )
                                )
                                
                            )
                        )
                    );
                }
                
                // Here we add terms to enforce : 
                // - To activate one level, the lower one needs to be active : exp & level(i-1)
                for(int k=1; k <= j; k++) {
                    Expression* lower_outputs = new NodeExpression(
                        this->network->getOrMakeNode(getName(t_outputs[0], k))
                    );
                    for (size_t l=1; l < t_outputs.size(); l++) {
                        lower_outputs = new AndLogicalExpression(
                            lower_outputs,
                            new NodeExpression(
                                this->network->getOrMakeNode(getName(t_outputs[l], k))
                            )
                        );
                    }
                    
                    exp = new AndLogicalExpression(
                        exp,
                        lower_outputs
                    );
                }
            
                // - To inactivate one level, the upper one needs to be inactive. so we add | level(i)&level(i+1)
                // This is only if j+1 < max_level
                for (int k=j+1; k < max_level; k++) {
                    Expression* higher_outputs = new AndLogicalExpression(
                        new NodeExpression(this->network->getOrMakeNode(getName(t_outputs[0], k))),
                        new NodeExpression(this->network->getOrMakeNode(getName(t_outputs[0], k+1)))
                    );
                    for (size_t l=1; l < t_outputs.size(); l++) {
                        higher_outputs = new AndLogicalExpression(
                            higher_outputs,
                            new AndLogicalExpression(
                                new NodeExpression(this->network->getOrMakeNode(getName(t_outputs[l], k))),
                                new NodeExpression(this->network->getOrMakeNode(getName(t_outputs[l], k+1)))
                            )
                        );
                    }
                    
                    exp = new OrLogicalExpression(
                        exp,
                        higher_outputs
                    );
                }
                
                createNodes(new_outputs, exp);
            }  
        }
    
    } 
        
  }
  
  Expression* parseASTNode(const ASTNode* tree) 
  {
    std::string name;
    int value;
    Expression* ret = NULL;
 
    switch(tree->getType()) {
        case AST_LOGICAL_AND:
        {
            if (tree->getNumChildren() == 1) {
                return parseASTNode(tree->getChild(0));
            }
            
            AndLogicalExpression* children = new AndLogicalExpression(
                parseASTNode(tree->getChild(0)),
                parseASTNode(tree->getChild(1))
            );
            
            for (unsigned int n = 2; n < tree->getNumChildren(); n++) {
                children = new AndLogicalExpression(
                children, 
                parseASTNode(tree->getChild(n))
                );
            }
            
            return children;
        }
        
        case AST_LOGICAL_OR:
        {  
            if (tree->getNumChildren() == 1) {
                return parseASTNode(tree->getChild(0));
            }
            
            OrLogicalExpression* children = new OrLogicalExpression(
                parseASTNode(tree->getChild(0)),
                parseASTNode(tree->getChild(1))
            );
            
            for (unsigned int n = 2; n < tree->getNumChildren(); n++) {
                children = new OrLogicalExpression(
                children, 
                parseASTNode(tree->getChild(n))
                );
            }
            
            return children;
        }
        
        case AST_LOGICAL_XOR:
        {
            if (tree->getNumChildren() == 1) {
                return parseASTNode(tree->getChild(0));
            }
            
            XorLogicalExpression* children = new XorLogicalExpression(
                parseASTNode(tree->getChild(0)),
                parseASTNode(tree->getChild(1))
            );
            
            for (unsigned int n = 2; n < tree->getNumChildren(); n++) {
                children = new XorLogicalExpression(
                children, 
                parseASTNode(tree->getChild(n))
                );
            }
            
            return children;
        }
        
        case AST_LOGICAL_NOT:
            return new NotLogicalExpression(parseASTNode(tree->getChild(0)));

        case AST_RELATIONAL_EQ:
        {
            
            if (tree->getChild(0)->getType() == AST_NAME && tree->getChild(1)->getType() == AST_INTEGER) {
                name = tree->getChild(0)->getName();
                value = tree->getChild(1)->getValue();
            } else if (tree->getChild(1)->getType() == AST_NAME && tree->getChild(0)->getType() == AST_INTEGER) {
                name = tree->getChild(1)->getName();
                value = tree->getChild(0)->getValue();
            } else throw BNException("Not Implemented : SBML qual multivalued formulas nodeA == nodeB");
            
            // Here when we ask for a specific level i, what we mean in boolean is A_b1 & A_b2 & ... & A_bi & !A_b(i+1) We don't need the following ones, 
            // because they already are forbidden if A_b(i+1) is false.
            
            // All those up to the value
            for (int i=0; i < value; i++) {
                if (i == 0) {
                    ret = new NodeExpression(this->network->getOrMakeNode(getName(name, 1)));
                } else {
                    ret = new AndLogicalExpression(
                        ret, 
                        new NodeExpression(this->network->getOrMakeNode(getName(name, i+1)))
                    );
                }
            }
            
            // And none of the next ones
            for (int i=value; i < std::min(this->maxLevels[name], value+1); i++) {
                if (i == 0) {
                    ret = new NotLogicalExpression(new NodeExpression(this->network->getOrMakeNode(getName(name, i+1))));
                } else {
                    ret = new AndLogicalExpression(
                        ret, 
                        new NotLogicalExpression(new NodeExpression(this->network->getOrMakeNode(getName(name, i+1))))
                    );
                }
            }
                    
            return ret;
       
        }
        case AST_RELATIONAL_LEQ:
        // Here we have a multivalued model. The idea is to modify the formula, to replace it by a pure boolean one
        // Ex: Suppose we have A <= i , with max(A) = n. It means that we can change it to : !A_p1 | (A_p1 | ... | A_pi & !A_pi+1)
        {
            if (tree->getChild(0)->getType() == AST_NAME && tree->getChild(1)->getType() == AST_INTEGER) {
                name = tree->getChild(0)->getName();
                value = tree->getChild(1)->getValue();
            } else if (tree->getChild(1)->getType() == AST_NAME && tree->getChild(0)->getType() == AST_INTEGER) {
                name = tree->getChild(1)->getName();
                value = tree->getChild(0)->getValue();
            } else throw BNException("Not Implemented : SBML qual multivalued formulas nodeA <= nodeB");
            
            // First one :
            Expression* first = new NotLogicalExpression(new NodeExpression(this->network->getOrMakeNode(getName(name, 1))));
            
            // All those up to the value
            for (int i=1; i <= value; i++) {
                if (i == 1) {
                    ret = new NodeExpression(this->network->getOrMakeNode(getName(name, 1)));
                } else {
                    ret = new OrLogicalExpression(
                        ret, 
                        new NodeExpression(this->network->getOrMakeNode(getName(name, i)))
                    );
                }
            }
            
            // And none of the next ones
            for (int i=value; i < std::min(this->maxLevels[name], value+1); i++) {
                ret = new AndLogicalExpression(
                    ret, 
                    new NotLogicalExpression(new NodeExpression(this->network->getOrMakeNode(getName(name, i+1))))
                );
            
            }
                    
            return new OrLogicalExpression(first, ret);
        }
        
        case AST_RELATIONAL_GEQ:
        // Here we have a multivalued model. The idea is to modify the formula, to replace it by a pure boolean one
        // Ex: Suppose we have A >= i, with max(A) = n. It means that we can change it to : A_i
        
        {
            if (tree->getChild(0)->getType() == AST_NAME && tree->getChild(1)->getType() == AST_INTEGER) {
                name = tree->getChild(0)->getName();
                value = tree->getChild(1)->getValue();
            } else if (tree->getChild(1)->getType() == AST_NAME && tree->getChild(0)->getType() == AST_INTEGER) {
                name = tree->getChild(1)->getName();
                value = tree->getChild(0)->getValue();
            } else throw BNException("Not Implemented : SBML qual multivalued formulas nodeA >= nodeB");
            
            if (value == 0) {
                // This one is always true
                return new ConstantExpression(1.0);
                 
            } else {             
                return new NodeExpression(
                    this->network->getOrMakeNode(getName(name, value))
                );
            }
        }
        default:
            throw BNException("Not Implemented : Unknown sbml operator : " + std::string(tree->getName()));
    }
  }
  
  void createNodes(std::vector<std::string> names, Expression* exp) 
  {
    for (auto name: names) {
        NodeDeclItem* decl_item = new NodeDeclItem("logic", exp);
        std::vector<NodeDeclItem*>* decl_item_v = new std::vector<NodeDeclItem*>();
        decl_item_v->push_back(decl_item);

        NodeDecl* truc = new NodeDecl(name, decl_item_v, this->network);

        for (std::vector<NodeDeclItem*>::iterator it = decl_item_v->begin(); it != decl_item_v->end(); ++it) {
            delete *it;
        }
        
        delete decl_item_v;
        delete truc;
        }
  }
};

#endif