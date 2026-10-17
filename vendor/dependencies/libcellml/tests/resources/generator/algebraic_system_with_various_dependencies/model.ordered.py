# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

CONSTANT_COUNT = 2
COMPUTED_CONSTANT_COUNT = 1
ALGEBRAIC_VARIABLE_COUNT = 3

CONSTANT_INFO = [
    {"name": "y", "units": "dimensionless", "component": "my_algebraic_system"},
    {"name": "x", "units": "dimensionless", "component": "my_algebraic_system"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "a", "units": "dimensionless", "component": "my_algebraic_system"}
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "c", "units": "dimensionless", "component": "my_algebraic_system"},
    {"name": "b", "units": "dimensionless", "component": "my_algebraic_system"},
    {"name": "d", "units": "dimensionless", "component": "my_algebraic_system"}
]


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


from nlasolver import nla_solve


def objective_function_0(u, f, data):
    constants = data[0]
    computed_constants = data[1]
    algebraic_variables = data[2]

    algebraic_variables[0] = u[0]
    algebraic_variables[1] = u[1]

    f[0] = 3.0*computed_constants[0]+2.0*algebraic_variables[1]+algebraic_variables[0]-57.0
    f[1] = computed_constants[0]+3.0*algebraic_variables[1]-algebraic_variables[0]-19.0


def find_root_0(constants, computed_constants, algebraic_variables):
    u = [nan]*2

    u[0] = algebraic_variables[0]
    u[1] = algebraic_variables[1]

    u = nla_solve(objective_function_0, u, 2, [constants, computed_constants, algebraic_variables])

    algebraic_variables[0] = u[0]
    algebraic_variables[1] = u[1]


def initialise_arrays(constants, computed_constants, algebraic_variables):
    constants[0] = 5.0
    constants[1] = 3.0
    algebraic_variables[0] = 1.0
    algebraic_variables[1] = 1.0


def compute_computed_constants(constants, computed_constants, algebraic_variables):
    computed_constants[0] = 3.0*constants[1]+constants[0]


def compute_variables(constants, computed_constants, algebraic_variables):
    find_root_0(constants, computed_constants, algebraic_variables)
    algebraic_variables[2] = algebraic_variables[1]+algebraic_variables[0]
