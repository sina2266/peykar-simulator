# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

STATE_COUNT = 2
CONSTANT_COUNT = 5
COMPUTED_CONSTANT_COUNT = 0
ALGEBRAIC_VARIABLE_COUNT = 5

VOI_INFO = {"name": "t", "units": "second", "component": "main"}

STATE_INFO = [
    {"name": "q_1", "units": "coulomb", "component": "main"},
    {"name": "v_3", "units": "C_per_s", "component": "main"}
]

CONSTANT_INFO = [
    {"name": "v_in", "units": "C_per_s", "component": "main"},
    {"name": "v_out", "units": "C_per_s", "component": "main"},
    {"name": "C", "units": "C2_per_J", "component": "main"},
    {"name": "R", "units": "Js_per_C2", "component": "main"},
    {"name": "L", "units": "Js2_per_C2", "component": "main"}
]

COMPUTED_CONSTANT_INFO = [
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "v_1", "units": "C_per_s", "component": "main"},
    {"name": "v_2", "units": "C_per_s", "component": "main"},
    {"name": "u_3", "units": "J_per_C", "component": "main"},
    {"name": "u_2", "units": "J_per_C", "component": "main"},
    {"name": "u_1", "units": "J_per_C", "component": "main"}
]


def create_states_array():
    return [nan]*STATE_COUNT


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


from nlasolver import nla_solve


def objective_function_0(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[0] = u[0]

    f[0] = constants[0]-(algebraic_variables[0]+algebraic_variables[1])


def find_root_0(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[0]

    u = nla_solve(objective_function_0, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[0] = u[0]


def objective_function_1(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[2] = u[0]

    f[0] = algebraic_variables[4]-(algebraic_variables[3]+algebraic_variables[2])


def find_root_1(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[2]

    u = nla_solve(objective_function_1, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[2] = u[0]


def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):
    states[0] = 1.0
    states[1] = 0.0
    constants[0] = 1.0
    constants[1] = 1.0
    constants[2] = 20.0
    constants[3] = 2.0
    constants[4] = 10.0
    algebraic_variables[0] = 0.0
    algebraic_variables[2] = 0.0


def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):
    pass


def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables):
    algebraic_variables[1] = states[1]+constants[1]
    find_root_0(voi, states, rates, constants, computed_constants, algebraic_variables)
    rates[0] = algebraic_variables[0]
    algebraic_variables[3] = constants[3]*algebraic_variables[1]
    algebraic_variables[4] = states[0]/constants[2]
    find_root_1(voi, states, rates, constants, computed_constants, algebraic_variables)
    rates[1] = algebraic_variables[2]/constants[4]


def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables):
    algebraic_variables[1] = states[1]+constants[1]
    find_root_0(voi, states, rates, constants, computed_constants, algebraic_variables)
    algebraic_variables[3] = constants[3]*algebraic_variables[1]
    algebraic_variables[4] = states[0]/constants[2]
    find_root_1(voi, states, rates, constants, computed_constants, algebraic_variables)
