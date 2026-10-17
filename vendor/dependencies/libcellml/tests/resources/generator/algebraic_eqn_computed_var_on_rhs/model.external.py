# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

CONSTANT_COUNT = 0
COMPUTED_CONSTANT_COUNT = 0
ALGEBRAIC_VARIABLE_COUNT = 1
EXTERNAL_VARIABLE_COUNT = 1

CONSTANT_INFO = [
]

COMPUTED_CONSTANT_INFO = [
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "x", "units": "dimensionless", "component": "my_algebraic_eqn"}
]

EXTERNAL_VARIABLE_INFO = [
    {"name": "a", "units": "dimensionless", "component": "my_algebraic_eqn"}
]


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


def create_external_variables_array():
    return [nan]*EXTERNAL_VARIABLE_COUNT


def initialise_arrays(constants, computed_constants, algebraic_variables):
    pass


def compute_computed_constants(constants, computed_constants, algebraic_variables):
    pass


def compute_variables(constants, computed_constants, algebraic_variables, external_variables, external_variable):
    external_variables[0] = external_variable(constants, computed_constants, algebraic_variables, external_variables, 0)
    algebraic_variables[0] = external_variables[0]
