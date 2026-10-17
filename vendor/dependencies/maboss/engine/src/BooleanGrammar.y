
%{
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
     BooleanGrammar.y

   Authors:
     Eric Viara <viara@sysra.com>
     Gautier Stoll <gautier.stoll@curie.fr>
     Vincent Noël <vincent.noel@curie.fr> 
   Date:
     January-March 2011
*/

#include "BooleanGrammar.h"

extern int yylex();
static void yyerror(const char *s);
static Network* current_network;
static PopNetwork* current_pop_network;
%}

%union {
  std::vector<NodeDecl*>* node_decl_list;
  NodeDecl* node_decl;
  std::vector<NodeDeclItem*>* node_decl_item_list;
  NodeDeclItem* node_decl_item;
  DivisionDecl* division_decl;
  std::vector<DivisionDaughterDecl*>* division_list_daughter;
  DivisionDaughterDecl* division_decl_daughter;
  Expression* expr;
  ArgumentList* arg_list;
  char* str;
  double d;
  long long l;
}

%type<node_decl> node_decl
%type<node_decl_item_list> node_decl_item_list
%type<node_decl_item> node_decl_item
%type<division_decl> division_decl
%type<expr> division_decl_rate
%type<division_list_daughter> division_list_daughter
%type<division_decl_daughter> division_decl_daughter
%type<expr> death_decl
%type<expr> death_decl_rate
%type<expr> cell_number
%type<expr> primary_expression 
%type<expr> postfix_expression 
%type<expr> unary_expression 
%type<expr> multiplicative_expression 
%type<expr> additive_expression 
%type<expr> relational_expression 
%type<expr> equality_expression
%type<expr> logical_and_expression
%type<expr> logical_or_expression
%type<expr> logical_xor_expression
%type<expr> conditional_expression
%type<expr> expression
%type<arg_list> argument_expression_list

%token<str> IDENTIFIER VARIABLE STRING
%token<d> DOUBLE
%token<l> INTEGER

%token LOGAND LOGOR LOGXOR LOGNOT EQUAL NOT_EQUAL NODE GTEQ LTEQ DIVISION DEATH RATE DAUGHTER1 DAUGHTER2 CELL_NUMBER

%%

translation_unit: decl
{
}
| translation_unit decl
{
}
;

decl: node_decl
{
}
| division_decl
{
}
| death_decl
{
}
;

node_decl: NODE IDENTIFIER '{' node_decl_item_list '}'
{
  NodeDecl* node_decl = new NodeDecl($2, $4);
  free($2);
  for (auto * node_decl_item : *($4)) {
    delete node_decl_item;
  }
  delete $4;
  delete node_decl;
}
| NODE IDENTIFIER '{' '}'
{
  NodeDecl* node_decl = new NodeDecl($2, NULL);
  free($2);
  delete node_decl;
}
| IDENTIFIER ':' expression term_opt
{
  NodeDeclItem* decl_item = new NodeDeclItem("logic", $3);
  std::vector<NodeDeclItem*>* decl_item_v = new std::vector<NodeDeclItem*>();
  decl_item_v->push_back(decl_item);

  NodeDecl* node_decl = new NodeDecl($1, decl_item_v);
  free($1);
  for (auto * decl_item : *decl_item_v) {
    delete decl_item;
  }
  delete decl_item_v;
  delete node_decl;
}
| IDENTIFIER ',' expression term_opt
{
  if (strcmp($1, "targets") == 0 && strcmp($3->toString().c_str(), "factors") == 0) {
    current_network->removeLastNode($3->toString());
    free($3);
  } else {
    NodeDeclItem* decl_item = new NodeDeclItem("logic", $3);
    std::vector<NodeDeclItem*>* decl_item_v = new std::vector<NodeDeclItem*>();
    decl_item_v->push_back(decl_item);

    NodeDecl* node_decl = new NodeDecl($1, decl_item_v);
    free($1);
    for (auto * node_decl_item : *decl_item_v) {
      delete node_decl_item;
    }
    delete decl_item_v;
    delete node_decl;
  }
}
;

node_decl_item_list: node_decl_item
{
  $$ = new std::vector<NodeDeclItem*>();
  $$->push_back($1);
} 
| node_decl_item_list node_decl_item
{
  $1->push_back($2);
  $$ = $1;
}
;

node_decl_item: IDENTIFIER '=' expression ';'
{
  $$ = new NodeDeclItem($1, $3);
  free($1);
}
| IDENTIFIER '=' STRING ';'
{
  $$ = new NodeDeclItem($1, $3);
  free($1);
}
;

division_decl: DIVISION '{' division_decl_rate division_list_daughter '}'
{
  $$ = new DivisionDecl($4, $3);
}
| DIVISION '{' division_decl_rate '}'
{
  std::vector<DivisionDaughterDecl*>* empty_daughters = NULL;
  $$ = new DivisionDecl(empty_daughters, $3);
}
| DIVISION '{' division_list_daughter '}'
{
  Expression* empty_rate = NULL;
  $$ = new DivisionDecl($3, empty_rate);
}
;

division_decl_rate: RATE '=' expression ';'
{
  $$ = $3;
}
;

division_list_daughter: division_decl_daughter
{
  $$ = new std::vector<DivisionDaughterDecl*>();
  $$->push_back($1);
}
| division_list_daughter division_decl_daughter
{ 
  $1->push_back($2);
  $$ = $1; 
}
;

division_decl_daughter: IDENTIFIER '.' DAUGHTER1 '=' expression ';'
{  
  $$ = new DivisionDaughterDecl(current_network->getOrMakeNode($1), DivisionRule::DAUGHTER_1, $5);
}
| IDENTIFIER '.' DAUGHTER2 '=' expression ';'
{
  $$ = new DivisionDaughterDecl(current_network->getOrMakeNode($1), DivisionRule::DAUGHTER_2, $5);  
}
;

death_decl: DEATH '{' death_decl_rate '}'
{
}
;

death_decl_rate: RATE '=' expression ';'
{
  if (current_pop_network == NULL) 
      throw BNException("Please use PopMaBoSS to simulate PopMaBoSS models");

  current_pop_network->setDeathRate($3);
}
;

primary_expression: IDENTIFIER
{
  Node* node = current_network->getOrMakeNode($1);
  $$ = new NodeExpression(node);
  free($1);
}
| '@' IDENTIFIER
{
  $$ = new AliasExpression($2);
  free($2);
}
| VARIABLE
{
  $$ = new SymbolExpression(current_network->getSymbolTable(), current_network->getSymbolTable()->getOrMakeSymbol($1));
  free($1);
}
| INTEGER
{
  $$ = new ConstantExpression($1);
  
}
| DOUBLE
{
  $$ = new ConstantExpression($1);
}
| '(' expression ')'
{
  $$ = new ParenthesisExpression($2);
}
| '#' CELL_NUMBER '(' cell_number ')'
{
  $$ = new PopExpression($4);
}
;

cell_number:
{
  $$ = NULL;
}
| expression
{
  $$ = $1;
}

postfix_expression: primary_expression
{
  $$ = $1;
}
| IDENTIFIER '(' argument_expression_list ')'
{
  $$ = new FuncCallExpression($1, $3);
  free($1);
}
| IDENTIFIER '(' ')'
{
  $$ = new FuncCallExpression($1, NULL);
  free($1);
}
;

argument_expression_list: conditional_expression
{
  $$ = new ArgumentList();
  $$->push_back($1);
}
| argument_expression_list ',' conditional_expression
{
  $$ = $1;
  $$->push_back($3);
}
;

unary_expression: postfix_expression
{
  $$ = $1;
}
| '+' unary_expression
{
  $$ = $2;
}
| '-' unary_expression
{
  $$ = new SubExpression(new ConstantExpression(0.0), $2);
}
| '!' unary_expression
{
  $$ = new NotLogicalExpression($2);
}
| LOGNOT unary_expression
{
  $$ = new NotLogicalExpression($2);
}
;

multiplicative_expression: unary_expression
{
  $$ = $1;
}
| multiplicative_expression '*' unary_expression
{
  $$ = new MulExpression($1, $3);
}
| multiplicative_expression '/' unary_expression
{
  $$ = new DivExpression($1, $3);
}
;

additive_expression: multiplicative_expression
{
  $$ = $1;
}
| additive_expression '+' multiplicative_expression
{
  $$ = new AddExpression($1, $3);
}
| additive_expression '-' multiplicative_expression
{
  $$ = new SubExpression($1, $3);
}
;

relational_expression: additive_expression
{
  $$ = $1;
}
| relational_expression '<' additive_expression
{
  $$ = new LetterExpression($1, $3);
}
| relational_expression '>' additive_expression
{
  $$ = new GreaterExpression($1, $3);
}
| relational_expression LTEQ additive_expression
{
  $$ = new LetterOrEqualExpression($1, $3);
}
| relational_expression GTEQ additive_expression
{
  $$ = new GreaterOrEqualExpression($1, $3);
}
;

equality_expression: relational_expression
{
  $$ = $1;
}
| equality_expression EQUAL relational_expression
{
  $$ = new EqualExpression($1, $3);
}
| equality_expression NOT_EQUAL relational_expression
{
  $$ = new NotEqualExpression($1, $3);
}
;

logical_and_expression: equality_expression
{
  $$ = $1;
}
| equality_expression LOGAND logical_and_expression
{
  $$ = new AndLogicalExpression($1, $3);
}
;

logical_or_expression: logical_and_expression
{
  $$ = $1;
}
| logical_and_expression LOGOR logical_or_expression
{
  $$ = new OrLogicalExpression($1, $3);
}
;

logical_xor_expression: logical_or_expression
{
  $$ = $1;
}
| logical_xor_expression LOGXOR logical_or_expression
{
  $$ = new XorLogicalExpression($1, $3);
}
| logical_xor_expression '^' logical_or_expression
{
  $$ = new XorLogicalExpression($1, $3);
}
;

conditional_expression: logical_xor_expression
{
  $$ = $1;
}
| logical_xor_expression '?' expression ':' conditional_expression
{
  $$ = new CondExpression($1, $3, $5);
}
;

expression: conditional_expression
{
  $$ = $1;
}
;

term_opt :
| ';'
;

%%

#include "lex.CTBNDL.cc"

void set_current_network(Network* network)
{
  current_network = network;
}

Network* get_current_network()
{
  return current_network;
}

void set_pop_network(PopNetwork* pop_network)
{
  current_pop_network = pop_network;
}

PopNetwork* get_pop_network()
{
  return current_pop_network;
}