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
     maboss_net.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#ifndef MABOSS_NETWORK
#define MABOSS_NETWORK

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <set>
#include "src/BooleanNetwork.h"
#include "src/MaBEstEngine.h"

typedef struct {
  PyObject_HEAD
  Network* network;
} cMaBoSSNetworkObject;

static void cMaBoSSNetwork_dealloc(cMaBoSSNetworkObject *self)
{
    free(self->network);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static Network* cMaBoSSNetwork_getNetwork(cMaBoSSNetworkObject* self) 
{
  return self->network;
}

static PyObject* cMaBoSSNetwork_getListNodes(cMaBoSSNetworkObject* self)
{
  PyObject *list = PyList_New(self->network->getNodes().size());

  size_t index = 0;
  for (auto* node: self->network->getNodes()) {
    PyList_SetItem(list, index, PyUnicode_FromString(node->getLabel().c_str()));
    index++;
  }

  return list;
}

static PyObject * cMaBoSSNetwork_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  char * network_file;
  static const char *kwargs_list[] = {"network", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "s", const_cast<char **>(kwargs_list), 
    &network_file
  ))
    return NULL;
  
  cMaBoSSNetworkObject* pynetwork;
  pynetwork = (cMaBoSSNetworkObject *) type->tp_alloc(type, 0);
  pynetwork->network = new Network();
  
  try{
    pynetwork->network->parse(network_file);
  
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  
  return (PyObject*) pynetwork;
}


static PyMethodDef cMaBoSSNetwork_methods[] = {
    {"getNetwork", (PyCFunction) cMaBoSSNetwork_getNetwork, METH_NOARGS, "returns the network object"},
    {"getListNodes", (PyCFunction) cMaBoSSNetwork_getListNodes, METH_NOARGS, "returns the list of nodes"},
    {NULL}  /* Sentinel */
};

static PyTypeObject cMaBoSSNetwork = []{
    PyTypeObject net{PyVarObject_HEAD_INIT(NULL, 0)};

    net.tp_name = "cmaboss.cMaBoSSNetworkObject";
    net.tp_basicsize = sizeof(cMaBoSSNetworkObject);
    net.tp_itemsize = 0;
    net.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    net.tp_doc = "cMaBoSS Network object";
    net.tp_new = cMaBoSSNetwork_new;
    net.tp_dealloc = (destructor) cMaBoSSNetwork_dealloc;
    net.tp_methods = cMaBoSSNetwork_methods;
    return net;
}();
#endif