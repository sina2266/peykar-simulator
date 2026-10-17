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
     popmaboss_res.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     March 2021
*/

#define PY_SSIZE_T_CLEAN
#ifdef PYTHON_API
#include <Python.h>
#include <fstream>
#include <stdlib.h>
#include <set>
#include "src/BooleanNetwork.h"
#include "src/PopMaBEstEngine.h"
// #include "src/PopProbTrajDisplayer.h"

typedef struct {
  PyObject_HEAD
  PopNetwork* network;
  RunConfig* runconfig;
  PopMaBEstEngine* engine;
  time_t start_time;
  time_t end_time;
} cPopMaBoSSResultObject;

static void cPopMaBoSSResult_dealloc(cPopMaBoSSResultObject *self)
{
    free(self->engine);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * cPopMaBoSSResult_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  cPopMaBoSSResultObject* res;
  res = (cPopMaBoSSResultObject *) type->tp_alloc(type, 0);

  return (PyObject*) res;
}

static PyObject* cPopMaBoSSResult_get_fp_table(cPopMaBoSSResultObject* self) {

  PyObject *dict = PyDict_New();

  for (auto& result: self->engine->getFixPointsDists()) {
    PyObject *tuple = PyTuple_Pack(2, 
      PyFloat_FromDouble(result.second.second),
      PyUnicode_FromString(result.second.first.getName(self->network).c_str())
    );

    PyDict_SetItem(dict, PyLong_FromUnsignedLong(result.first), tuple);
  }

  return dict;
}

static PyObject* cPopMaBoSSResult_get_probtraj(cPopMaBoSSResultObject* self) {
  return self->engine->getMergedCumulator()->getNumpyStatesDists(self->network);
}

// static PyObject* cMaBoSSResult_get_last_probtraj(cMaBoSSResultObject* self) {
//   return self->engine->getMergedCumulator()->getNumpyLastStatesDists(self->network);
// }

// static PyObject* cMaBoSSResult_get_nodes_probtraj(cMaBoSSResultObject* self) {
//   return self->engine->getMergedCumulator()->getNumpyNodesDists(self->network);
// }

// static PyObject* cMaBoSSResult_get_last_nodes_probtraj(cMaBoSSResultObject* self) {
//   return self->engine->getMergedCumulator()->getNumpyLastNodesDists(self->network);
// }

static PyObject* cPopMaBoSSResult_display_fp(cPopMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_fp = new std::ofstream(filename);
  CSVFixedPointDisplayer * fp_displayer = new CSVFixedPointDisplayer(self->network, *output_fp, hexfloat);

  self->engine->displayFixpoints(fp_displayer);
  ((std::ofstream*) output_fp)->close();
  delete output_fp;

  return Py_None;
}

static PyObject* cPopMaBoSSResult_display_probtraj(cPopMaBoSSResultObject* self, PyObject *args) 
{
  char * filename = NULL;
  int hexfloat = 0;
  if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
    return NULL;
    
  std::ostream* output_probtraj = new std::ofstream(filename);
  
  CSVProbTrajDisplayer<PopNetworkState> * pop_probtraj_displayer = new CSVProbTrajDisplayer<PopNetworkState>(self->network, *output_probtraj, hexfloat);
  self->engine->displayPopProbTraj(pop_probtraj_displayer);
  
  ((std::ofstream*) output_probtraj)->close();
  delete output_probtraj;

  return Py_None;
}

// static PyObject* cPopMaBoSSResult_display_statdist(cPopMaBoSSResultObject* self, PyObject *args) 
// {
//   char * filename = NULL;
//   int hexfloat = 0;
//   if (!PyArg_ParseTuple(args, "s|i", &filename, &hexfloat))
//     return NULL;
    
//   std::ostream* output_statdist = new std::ofstream(filename);
//   self->engine->displayStatDist(*output_statdist, (bool) hexfloat);
//   ((std::ofstream*) output_statdist)->close();
//   delete output_statdist;

//   return Py_None;
// }

static PyObject* cPopMaBoSSResult_display_run(cPopMaBoSSResultObject* self, PyObject* args) 
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

static PyMethodDef cPopMaBoSSResult_methods[] = {
    {"get_fp_table", (PyCFunction) cPopMaBoSSResult_get_fp_table, METH_NOARGS, "gets the fixpoints table"},
    {"get_probtraj", (PyCFunction) cPopMaBoSSResult_get_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    // {"get_last_probtraj", (PyCFunction) cMaBoSSResult_get_last_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    // {"get_nodes_probtraj", (PyCFunction) cMaBoSSResult_get_nodes_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    // {"get_last_nodes_probtraj", (PyCFunction) cMaBoSSResult_get_last_nodes_probtraj, METH_NOARGS, "gets the raw states probability trajectories of the simulation"},
    {"display_fp", (PyCFunction) cPopMaBoSSResult_display_fp, METH_VARARGS, "prints the fixpoints to a file"},
    {"display_probtraj", (PyCFunction) cPopMaBoSSResult_display_probtraj, METH_VARARGS, "prints the probtraj to a file"},
    // {"display_statdist", (PyCFunction) cMaBoSSResult_display_statdist, METH_VARARGS, "prints the statdist to a file"},
    {"display_run", (PyCFunction) cPopMaBoSSResult_display_run, METH_VARARGS, "prints the run of the simulation to a file"},
    {NULL}  /* Sentinel */
};

static PyTypeObject cPopMaBoSSResult = []{
  PyTypeObject res{PyVarObject_HEAD_INIT(NULL, 0)};

  res.tp_name = "cmaboss.cPopMaBoSSResultObject";
  res.tp_basicsize = sizeof(cPopMaBoSSResultObject);
  res.tp_itemsize = 0;
  res.tp_flags = Py_TPFLAGS_DEFAULT;// | Py_TPFLAGS_BASETYPE;
  res.tp_doc = "cPopMaBoSSResultobject";
  res.tp_new = cPopMaBoSSResult_new;
  res.tp_dealloc = (destructor) cPopMaBoSSResult_dealloc;
  res.tp_methods = cPopMaBoSSResult_methods;
  return res;
}();

#endif