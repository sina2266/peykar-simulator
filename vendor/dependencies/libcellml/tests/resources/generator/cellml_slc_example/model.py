# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

CONSTANT_COUNT = 9
COMPUTED_CONSTANT_COUNT = 1
ALGEBRAIC_VARIABLE_COUNT = 0

CONSTANT_INFO = [
    {"name": "P_5", "units": "per_sec3", "component": "SLC_template3_ss"},
    {"name": "q_Ao", "units": "fmol", "component": "SLC_template3_ss"},
    {"name": "q_Ai", "units": "fmol", "component": "SLC_template3_ss"},
    {"name": "P_4", "units": "per_fmol2_sec3", "component": "SLC_template3_ss"},
    {"name": "P_3", "units": "per_fmol_sec3", "component": "SLC_template3_ss"},
    {"name": "P_2", "units": "per_fmol_sec3", "component": "SLC_template3_ss"},
    {"name": "P_1", "units": "per_fmol_sec4", "component": "SLC_template3_ss"},
    {"name": "P_0", "units": "per_fmol_sec4", "component": "SLC_template3_ss"},
    {"name": "E", "units": "fmol", "component": "SLC_template3_ss"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "v", "units": "fmol_per_sec", "component": "SLC_template3_ss"}
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
    constants[0] = 810000.0
    constants[1] = 150.0
    constants[2] = 2.0
    constants[3] = 247140.0
    constants[4] = 2902500.0
    constants[5] = 2902500.0
    constants[6] = 3402000.0
    constants[7] = 21262500.0
    constants[8] = 1.1


def compute_computed_constants(constants, computed_constants, algebraic_variables):
    computed_constants[0] = constants[8]*(constants[7]*constants[1]-constants[6]*constants[2])/(constants[5]*constants[2]+constants[4]*constants[1]+constants[3]*constants[2]*constants[1]+constants[0])


def compute_variables(constants, computed_constants, algebraic_variables):
    pass
