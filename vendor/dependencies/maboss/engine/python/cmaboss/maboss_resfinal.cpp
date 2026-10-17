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
     maboss_resfinal.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#define PY_SSIZE_T_CLEAN
#ifdef PYTHON_API
#include <Python.h>
#include <structmember.h>
#include <fstream>
#include <stdlib.h>
#include <set>
#include "src/BooleanNetwork.h"
#include "src/FinalStateSimulationEngine.h"
#include "maboss_commons.h"

typedef struct {
  PyObject_HEAD
  Network* network;
  RunConfig* runconfig;
  FinalStateSimulationEngine* engine;
  time_t start_time;
  time_t end_time;
  PyObject* last_probtraj;
} cMaBoSSResultFinalObject;

static void cMaBoSSResultFinal_dealloc(cMaBoSSResultFinalObject *self)
{
    free(self->engine);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * cMaBoSSResultFinal_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  cMaBoSSResultFinalObject* res;
  res = (cMaBoSSResultFinalObject *) type->tp_alloc(type, 0);  
  res->last_probtraj = Py_None;
  return (PyObject*) res;
}

static PyObject* cMaBoSSResultFinal_get_last_probtraj(cMaBoSSResultFinalObject* self) 
{
  if (self->last_probtraj == Py_None) {
    self->last_probtraj = self->engine->getNumpyLastStatesDists();
  }
  Py_INCREF(self->last_probtraj);

  return self->last_probtraj;
}

static PyObject* cMaBoSSResultFinal_get_last_nodes_probtraj(cMaBoSSResultFinalObject* self, PyObject* args) {
  
  std::vector<Node*> list_nodes;
  PyObject* pList = Py_None;
  
  if (!PyArg_ParseTuple(args, "|O", &pList)) {
    PyErr_SetString(PyExc_TypeError, "Error parsing arguments");
    return NULL;
  }
  
  if (pList != Py_None) {
  
    PyObject* pItem;
    int n = PyList_Size(pList);
    
    for (int i=0; i<n; i++) {
        pItem = PyList_GetItem(pList, i);
        list_nodes.push_back(self->network->getNode(std::string(PyUnicode_AsUTF8(pItem))));
    }
  }  
  
  return self->engine->getNumpyLastNodesDists(list_nodes);
}

static PyObject* cMaBoSSResultFinal_display_final_states(cMaBoSSResultFinalObject* self, PyObject* args) {

  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;

  std::ostream* output_final = new std::ofstream(filename);
  FinalStateDisplayer * final_displayer = new CSVFinalStateDisplayer(
    self->network, *output_final, PyObject_IsTrue(PyBool_FromLong(hexfloat))
  );

  self->engine->displayFinal(final_displayer);

  ((std::ofstream*) output_final)->close();
  delete output_final;

  return Py_None;
}

static PyObject* cMaBoSSResultFinal_get_final_time(cMaBoSSResultFinalObject* self) {
  return PyFloat_FromDouble(self->engine->getFinalTime());
}


static PyObject* cMaBoSSResultFinal_display_run(cMaBoSSResultFinalObject* self, PyObject* args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_run = new std::ofstream(filename);
  self->engine->displayRunStats(*output_run, self->start_time, self->end_time);
  ((std::ofstream*) output_run)->close();
  delete output_run;

  return Py_None;
}

static PyMemberDef cMaBoSSResultFinal_members[] = {
    {(char*)"network", T_OBJECT_EX, offsetof(cMaBoSSResultFinalObject, network), 0, (char*)"network"},
    {(char*)"runconfig", T_OBJECT_EX, offsetof(cMaBoSSResultFinalObject, runconfig), 0, (char*)"runconfig"},
    {(char*)"engine", T_OBJECT_EX, offsetof(cMaBoSSResultFinalObject, engine), 0, (char*)"engine"},
    {(char*)"start_time", T_LONG, offsetof(cMaBoSSResultFinalObject, start_time), 0, (char*)"start_time"},
    {(char*)"end_time", T_LONG, offsetof(cMaBoSSResultFinalObject, end_time), 0, (char*)"end_time"},
    {(char*)"last_probtraj", T_OBJECT_EX, offsetof(cMaBoSSResultFinalObject, last_probtraj), 0, (char*)"last_probtraj"},
    {NULL}  /* Sentinel */
};

static PyMethodDef cMaBoSSResultFinal_methods[] = {
    {"get_final_time", (PyCFunction) cMaBoSSResultFinal_get_final_time, METH_NOARGS, "gets the final time of the simulation"},
    {"get_last_probtraj", (PyCFunction) cMaBoSSResultFinal_get_last_probtraj, METH_NOARGS, "gets the last probtraj of the simulation"},
    {"display_final_states", (PyCFunction) cMaBoSSResultFinal_display_final_states, METH_VARARGS, "display the final state"},
    {"get_last_nodes_probtraj", (PyCFunction) cMaBoSSResultFinal_get_last_nodes_probtraj, METH_VARARGS, "gets the last nodes probtraj of the simulation"},
    {"display_run", (PyCFunction) cMaBoSSResultFinal_display_run, METH_VARARGS, "prints the run of the simulation to a file"},
    {NULL}  /* Sentinel */
};

#if ! defined (MAXNODES) || MAXNODES <= 64 
    static char result_final_name[50] = "cmaboss";
#else
    static char result_final_name[50] = STR(MODULE_NAME);
#endif


static PyTypeObject cMaBoSSResultFinal = []{
  PyTypeObject res{PyVarObject_HEAD_INIT(NULL, 0)};

  res.tp_name = strcat(result_final_name, ".cMaBoSSResultFinal");
  res.tp_basicsize = sizeof(cMaBoSSResultFinalObject);
  res.tp_itemsize = 0;
  res.tp_dealloc = (destructor) cMaBoSSResultFinal_dealloc;
  res.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
  res.tp_doc = "cMaBoSSResultFinalObject";
  res.tp_methods = cMaBoSSResultFinal_methods;
  res.tp_members = cMaBoSSResultFinal_members;
  res.tp_new = cMaBoSSResultFinal_new;

  return res;
}();

#endif