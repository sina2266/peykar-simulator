# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

CONSTANT_COUNT = 1
COMPUTED_CONSTANT_COUNT = 1
ALGEBRAIC_VARIABLE_COUNT = 0

CONSTANT_INFO = [
    {"name": "a", "units": "dimensionless", "component": "my_algebraic_eqn"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "x", "units": "dimensionless", "component": "my_algebraic_eqn"}
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
    constants[0] = 1.0


def compute_computed_constants(constants, computed_constants, algebraic_variables):
    computed_constants[0] = constants[0]


def compute_variables(constants, computed_constants, algebraic_variables):
    pass
