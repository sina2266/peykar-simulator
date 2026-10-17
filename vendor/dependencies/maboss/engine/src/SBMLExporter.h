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
     SBMLExporter.h
     
     Authors:
     Vincent Noel <vincent-noel@curie.fr>
     
     Date:
     April 2024
*/

#if defined SBML_COMPAT && !defined _SBML_EXPORTER_H_
#define _SBML_EXPORTER_H_
#include "BooleanNetwork.h"
#include "BooleanGrammar.h"
#include <sbml/packages/qual/extension/QualModelPlugin.h>
#include <sbml/SBMLTypes.h>
#include <sstream>

LIBSBML_CPP_NAMESPACE_USE

class SBMLExporter
{
  public:
  
  Network* network;
  RunConfig* runconfig;
//   bool useSBMLNames;
//   Model* model;
//   QualModelPlugin* qual_model;
//   std::map<std::string, int> maxLevels;
//   std::map<std::string, std::vector<std::string> > fixedNames;
  
  SBMLExporter(Network* network, RunConfig* runconfig, const char* file) : network(network), runconfig(runconfig) {
    
    SBMLNamespaces sbmlns(3,1,"qual",1);
  
    // create the document
    SBMLDocument *document = new SBMLDocument(&sbmlns);
 
    // mark qual as required
    document->setPackageRequired("qual", true);
    
    Model* model = document->createModel();
    
    // create the Compartment
    Compartment* compartment = model->createCompartment();
    compartment->setId("c");
    compartment->setConstant(true);
 
    //
    // Get a QualModelPlugin object plugged in the model object.
    //
    // The type of the returned value of SBase::getPlugin() function is
    // SBasePlugin*, and thus the value needs to be casted for the
    // corresponding derived class.
    //
    QualModelPlugin* mplugin = static_cast<QualModelPlugin*>(model->getPlugin("qual"));
 
    
    for (auto* node: network->getNodes())
    {
      // create the QualitativeSpecies
      QualitativeSpecies* qs = mplugin->createQualitativeSpecies();
      node->writeSBML(qs);
      
      Transition* t = mplugin->createTransition();
      t->setId("tr_" + std::to_string(node->getIndex()));
      t->setSBOTerm(1);
      
      std::stringstream ss;
      LogicalExprGenContext genctx(network, node, ss);
      Expression* expr = node->generateRawLogicalExpression();
      ASTNode* math = expr->writeSBML(genctx);
      // std::cout << SBML_formulaToL3String(math) << std::endl;
    
      for (auto* input: expr->getNodes())
      {
        Input* i = t->createInput();
        i->setId("tr_" + std::to_string(node->getIndex()) + "_in_" + std::to_string(input->getIndex()));
        i->setQualitativeSpecies(input->getLabel());
        i->setTransitionEffect(INPUT_TRANSITION_EFFECT_NONE);
        i->setSign(INPUT_SIGN_POSITIVE);
        i->setThresholdLevel(1);
        i->setName("");
      }
      
      Output* o = t->createOutput();
      o->setId("tr_" + std::to_string(node->getIndex()) + "_out");
      o->setQualitativeSpecies(node->getLabel());
      o->setTransitionEffect(OUTPUT_TRANSITION_EFFECT_PRODUCTION);
      o->setOutputLevel(2);
      o->setName("");
    
      DefaultTerm* dt = t->createDefaultTerm();
      dt->setResultLevel(0) ;
    
      FunctionTerm* ft = t->createFunctionTerm();
      ft->setResultLevel(1);
      ft->setMath(math);
    }
    
    SBMLWriter sbml_writer = SBMLWriter();
    sbml_writer.writeSBMLToFile(document, file);
    
  }

  
};

#endif