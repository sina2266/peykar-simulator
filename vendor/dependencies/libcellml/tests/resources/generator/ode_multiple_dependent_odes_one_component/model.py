# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

STATE_COUNT = 2
CONSTANT_COUNT = 1
COMPUTED_CONSTANT_COUNT = 0
ALGEBRAIC_VARIABLE_COUNT = 0

VOI_INFO = {"name": "t", "units": "second", "component": "my_component"}

STATE_INFO = [
    {"name": "y", "units": "dimensionless", "component": "my_component"},
    {"name": "x", "units": "dimensionless", "component": "my_component"}
]

CONSTANT_INFO = [
    {"name": "epsilon", "units": "dimensionless", "component": "my_component"}
]

COMPUTED_CONSTANT_INFO = [
]

ALGEBRAIC_VARIABLE_INFO = [
]


def create_states_array():
    return [nan]*STATE_COUNT


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):
    states[0] = 0.0
    states[1] = -2.0
    constants[0] = 1.0


def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):
    pass


def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables):
    rates[1] = states[0]*1.0
    rates[0] = (constants[0]*(1.0-pow(states[1], 2.0))*states[0]-states[1])*1.0


def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables):
    pass
