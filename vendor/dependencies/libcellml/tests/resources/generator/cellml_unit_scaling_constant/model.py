# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

CONSTANT_COUNT = 9
COMPUTED_CONSTANT_COUNT = 10
ALGEBRAIC_VARIABLE_COUNT = 0

CONSTANT_INFO = [
    {"name": "ind_c_mM", "units": "mM", "component": "constants"},
    {"name": "c_mM", "units": "mM", "component": "constants"},
    {"name": "c", "units": "mM", "component": "constants"},
    {"name": "c_M", "units": "M", "component": "constants"},
    {"name": "ind_c_M", "units": "M", "component": "constants"},
    {"name": "ind_oc_mM", "units": "mM", "component": "other_constants"},
    {"name": "oc_mM", "units": "mM", "component": "other_constants"},
    {"name": "oc_M", "units": "M", "component": "other_constants"},
    {"name": "ind_oc_M", "units": "M", "component": "other_constants"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "comp_c_mM_mM", "units": "mM", "component": "main"},
    {"name": "comp_c_mM_M", "units": "M", "component": "main"},
    {"name": "comp_c_M_mM", "units": "mM", "component": "main"},
    {"name": "comp_c_M_M", "units": "M", "component": "main"},
    {"name": "comp_k_mM", "units": "mM", "component": "main"},
    {"name": "comp_k_M", "units": "M", "component": "main"},
    {"name": "comp_oc_mM_mM", "units": "mM", "component": "main"},
    {"name": "comp_oc_mM_M", "units": "M", "component": "main"},
    {"name": "comp_oc_M_mM", "units": "mM", "component": "main"},
    {"name": "comp_oc_M_M", "units": "M", "component": "main"}
]

ALGEBRAIC_VARIABLE_INFO = [
]


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


def initialise_arrays(constants, computed_constants, algebraic_variables):
    constants[2] = 123.0
    constants[1] = constants[2]
    constants[0] = constants[1]
    constants[3] = constants[2]
    constants[4] = constants[3]
    constants[6] = 0.001*constants[2]
    constants[5] = constants[6]
    constants[7] = constants[2]
    constants[8] = constants[7]


def compute_computed_constants(constants, computed_constants, algebraic_variables):
    computed_constants[0] = constants[1]+constants[0]
    computed_constants[1] = 0.001*constants[1]+0.001*constants[0]
    computed_constants[2] = 1000.0*constants[3]+1000.0*constants[4]
    computed_constants[3] = constants[3]+constants[4]
    computed_constants[4] = constants[2]+constants[2]
    computed_constants[5] = 0.001*constants[2]+0.001*constants[2]
    computed_constants[6] = constants[6]+constants[5]
    computed_constants[7] = 0.001*constants[6]+0.001*constants[5]
    computed_constants[8] = 1000.0*constants[7]+1000.0*constants[8]
    computed_constants[9] = constants[7]+constants[8]


def compute_variables(constants, computed_constants, algebraic_variables):
    pass
