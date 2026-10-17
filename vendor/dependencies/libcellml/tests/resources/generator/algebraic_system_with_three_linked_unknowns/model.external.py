# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

CONSTANT_COUNT = 0
COMPUTED_CONSTANT_COUNT = 0
ALGEBRAIC_VARIABLE_COUNT = 0
EXTERNAL_VARIABLE_COUNT = 3

CONSTANT_INFO = [
]

COMPUTED_CONSTANT_INFO = [
]

ALGEBRAIC_VARIABLE_INFO = [
]

EXTERNAL_VARIABLE_INFO = [
    {"name": "z", "units": "dimensionless", "component": "my_algebraic_system"},
    {"name": "y", "units": "dimensionless", "component": "my_algebraic_system"},
    {"name": "x", "units": "dimensionless", "component": "my_algebraic_system"}
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
    external_variables[1] = external_variable(constants, computed_constants, algebraic_variables, external_variables, 1)
    external_variables[2] = external_variable(constants, computed_constants, algebraic_variables, external_variables, 2)
