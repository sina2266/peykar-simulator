# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

STATE_COUNT = 1
CONSTANT_COUNT = 2
COMPUTED_CONSTANT_COUNT = 5
ALGEBRAIC_VARIABLE_COUNT = 3

VOI_INFO = {"name": "x", "units": "dimensionless", "component": "main"}

STATE_INFO = [
    {"name": "sin", "units": "dimensionless", "component": "deriv_approx_sin"}
]

CONSTANT_INFO = [
    {"name": "deriv_approx_initial_value", "units": "dimensionless", "component": "main"},
    {"name": "C", "units": "dimensionless", "component": "main"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "k2_oPi", "units": "dimensionless", "component": "parabolic_approx_sin"},
    {"name": "k2Pi", "units": "dimensionless", "component": "parabolic_approx_sin"},
    {"name": "kPi_2", "units": "dimensionless", "component": "parabolic_approx_sin"},
    {"name": "kPi", "units": "dimensionless", "component": "parabolic_approx_sin"},
    {"name": "kPi_32", "units": "dimensionless", "component": "parabolic_approx_sin"}
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "sin", "units": "dimensionless", "component": "actual_sin"},
    {"name": "sin", "units": "dimensionless", "component": "parabolic_approx_sin"},
    {"name": "z", "units": "dimensionless", "component": "parabolic_approx_sin"}
]


def lt_func(x, y):
    return 1.0 if x < y else 0.0


def create_states_array():
    return [nan]*STATE_COUNT


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):
    constants[0] = 0.0
    states[0] = constants[0]
    constants[1] = 0.75
    computed_constants[0] = 2.0/3.14159265358979
    computed_constants[1] = 2.0*3.14159265358979
    computed_constants[2] = 3.14159265358979/2.0
    computed_constants[3] = 3.14159265358979
    computed_constants[4] = 3.0*3.14159265358979/2.0


def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):
    pass


def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables):
    rates[0] = cos(voi)


def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables):
    algebraic_variables[0] = sin(voi)
    algebraic_variables[2] = voi*computed_constants[0]-0.5 if lt_func(voi, computed_constants[2]) else (3.14159265358979-voi)*computed_constants[0]-0.5 if lt_func(voi, computed_constants[3]) else (voi-3.14159265358979)*computed_constants[0]-0.5 if lt_func(voi, computed_constants[4]) else (computed_constants[1]-voi)*computed_constants[0]-0.5
    algebraic_variables[1] = -algebraic_variables[2]*algebraic_variables[2]+constants[1]+algebraic_variables[2] if lt_func(voi, computed_constants[2]) else -algebraic_variables[2]*algebraic_variables[2]+constants[1]+algebraic_variables[2] if lt_func(voi, computed_constants[3]) else algebraic_variables[2]*algebraic_variables[2]-constants[1]-algebraic_variables[2] if lt_func(voi, computed_constants[4]) else algebraic_variables[2]*algebraic_variables[2]-constants[1]-algebraic_variables[2]
