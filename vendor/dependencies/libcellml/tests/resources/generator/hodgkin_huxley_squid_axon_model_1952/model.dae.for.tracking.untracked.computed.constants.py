# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

STATE_COUNT = 4
CONSTANT_COUNT = 5
COMPUTED_CONSTANT_COUNT = 0
ALGEBRAIC_VARIABLE_COUNT = 12

VOI_INFO = {"name": "time", "units": "millisecond", "component": "environment"}

STATE_INFO = [
    {"name": "V", "units": "millivolt", "component": "membrane"},
    {"name": "h", "units": "dimensionless", "component": "sodium_channel_h_gate"},
    {"name": "m", "units": "dimensionless", "component": "sodium_channel_m_gate"},
    {"name": "n", "units": "dimensionless", "component": "potassium_channel_n_gate"}
]

CONSTANT_INFO = [
    {"name": "Cm", "units": "microF_per_cm2", "component": "membrane"},
    {"name": "E_R", "units": "millivolt", "component": "membrane"},
    {"name": "g_L", "units": "milliS_per_cm2", "component": "leakage_current"},
    {"name": "g_Na", "units": "milliS_per_cm2", "component": "sodium_channel"},
    {"name": "g_K", "units": "milliS_per_cm2", "component": "potassium_channel"}
]

COMPUTED_CONSTANT_INFO = [
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "i_Stim", "units": "microA_per_cm2", "component": "membrane"},
    {"name": "i_L", "units": "microA_per_cm2", "component": "leakage_current"},
    {"name": "i_K", "units": "microA_per_cm2", "component": "potassium_channel"},
    {"name": "i_Na", "units": "microA_per_cm2", "component": "sodium_channel"},
    {"name": "E_L", "units": "millivolt", "component": "leakage_current"},
    {"name": "E_Na", "units": "millivolt", "component": "sodium_channel"},
    {"name": "alpha_m", "units": "per_millisecond", "component": "sodium_channel_m_gate"},
    {"name": "beta_m", "units": "per_millisecond", "component": "sodium_channel_m_gate"},
    {"name": "alpha_h", "units": "per_millisecond", "component": "sodium_channel_h_gate"},
    {"name": "beta_h", "units": "per_millisecond", "component": "sodium_channel_h_gate"},
    {"name": "alpha_n", "units": "per_millisecond", "component": "potassium_channel_n_gate"},
    {"name": "beta_n", "units": "per_millisecond", "component": "potassium_channel_n_gate"}
]


def leq_func(x, y):
    return 1.0 if x <= y else 0.0


def geq_func(x, y):
    return 1.0 if x >= y else 0.0


def and_func(x, y):
    return 1.0 if bool(x) & bool(y) else 0.0


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

    f[0] = algebraic_variables[0]-(-20.0 if and_func(geq_func(voi, 10.0), leq_func(voi, 10.5)) else 0.0)-0.0


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

    rates[0] = u[0]

    f[0] = rates[0]-(-(-algebraic_variables[0]+algebraic_variables[3]+algebraic_variables[2]+algebraic_variables[1])/constants[0])-0.0


def find_root_1(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = rates[0]

    u = nla_solve(objective_function_1, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    rates[0] = u[0]


def objective_function_2(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[4] = u[0]

    f[0] = algebraic_variables[4]-(constants[1]-10.613)-0.0


def find_root_2(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[4]

    u = nla_solve(objective_function_2, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[4] = u[0]


def objective_function_3(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[1] = u[0]

    f[0] = algebraic_variables[1]-constants[2]*(states[0]-algebraic_variables[4])-0.0


def find_root_3(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[1]

    u = nla_solve(objective_function_3, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[1] = u[0]


def objective_function_4(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[5] = u[0]

    f[0] = algebraic_variables[5]-(constants[1]-115.0)-0.0


def find_root_4(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[5]

    u = nla_solve(objective_function_4, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[5] = u[0]


def objective_function_5(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[3] = u[0]

    f[0] = algebraic_variables[3]-constants[3]*pow(states[2], 3.0)*states[1]*(states[0]-algebraic_variables[5])-0.0


def find_root_5(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[3]

    u = nla_solve(objective_function_5, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[3] = u[0]


def objective_function_6(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[7] = u[0]

    f[0] = algebraic_variables[7]-4.0*exp(states[0]/18.0)-0.0


def find_root_6(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[7]

    u = nla_solve(objective_function_6, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[7] = u[0]


def objective_function_7(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    rates[2] = u[0]

    f[0] = rates[2]-(algebraic_variables[6]*(1.0-states[2])-algebraic_variables[7]*states[2])-0.0


def find_root_7(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = rates[2]

    u = nla_solve(objective_function_7, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    rates[2] = u[0]


def objective_function_8(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[8] = u[0]

    f[0] = algebraic_variables[8]-0.07*exp(states[0]/20.0)-0.0


def find_root_8(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[8]

    u = nla_solve(objective_function_8, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[8] = u[0]


def objective_function_9(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[9] = u[0]

    f[0] = algebraic_variables[9]-1.0/(exp((states[0]+30.0)/10.0)+1.0)-0.0


def find_root_9(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[9]

    u = nla_solve(objective_function_9, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[9] = u[0]


def objective_function_10(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    rates[1] = u[0]

    f[0] = rates[1]-(algebraic_variables[8]*(1.0-states[1])-algebraic_variables[9]*states[1])-0.0


def find_root_10(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = rates[1]

    u = nla_solve(objective_function_10, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    rates[1] = u[0]


def objective_function_11(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[2] = u[0]

    potassium_channel_E_K = constants[1]+12.0

    f[0] = algebraic_variables[2]-constants[4]*pow(states[3], 4.0)*(states[0]-potassium_channel_E_K)-0.0


def find_root_11(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[2]

    u = nla_solve(objective_function_11, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[2] = u[0]


def objective_function_12(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[10] = u[0]

    f[0] = algebraic_variables[10]-0.01*(states[0]+10.0)/(exp((states[0]+10.0)/10.0)-1.0)-0.0


def find_root_12(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[10]

    u = nla_solve(objective_function_12, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[10] = u[0]


def objective_function_13(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    algebraic_variables[11] = u[0]

    f[0] = algebraic_variables[11]-0.125*exp(states[0]/80.0)-0.0


def find_root_13(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = algebraic_variables[11]

    u = nla_solve(objective_function_13, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    algebraic_variables[11] = u[0]


def objective_function_14(u, f, data):
    voi = data[0]
    states = data[1]
    rates = data[2]
    constants = data[3]
    computed_constants = data[4]
    algebraic_variables = data[5]

    rates[3] = u[0]

    f[0] = rates[3]-(algebraic_variables[10]*(1.0-states[3])-algebraic_variables[11]*states[3])-0.0


def find_root_14(voi, states, rates, constants, computed_constants, algebraic_variables):
    u = [nan]*1

    u[0] = rates[3]

    u = nla_solve(objective_function_14, u, 1, [voi, states, rates, constants, computed_constants, algebraic_variables])

    rates[3] = u[0]


def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):
    states[0] = 0.0
    states[1] = 0.6
    states[2] = 0.05
    states[3] = 0.325
    rates[0] = 0.0
    rates[1] = 0.0
    rates[2] = 0.0
    rates[3] = 0.0
    constants[0] = 1.0
    constants[1] = 0.0
    constants[2] = 0.3
    constants[3] = 120.0
    constants[4] = 36.0
    algebraic_variables[0] = 0.0
    algebraic_variables[1] = 0.0
    algebraic_variables[2] = 0.0
    algebraic_variables[3] = 0.0
    algebraic_variables[4] = 0.0
    algebraic_variables[5] = 0.0
    algebraic_variables[7] = 0.0
    algebraic_variables[8] = 0.0
    algebraic_variables[9] = 0.0
    algebraic_variables[10] = 0.0
    algebraic_variables[11] = 0.0


def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):
    pass


def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables):
    find_root_0(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_2(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_3(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_13(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_12(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_14(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_11(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_9(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_8(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_10(voi, states, rates, constants, computed_constants, algebraic_variables)
    algebraic_variables[6] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0)
    find_root_6(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_7(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_4(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_5(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_1(voi, states, rates, constants, computed_constants, algebraic_variables)


def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables):
    find_root_3(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_13(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_12(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_14(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_11(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_9(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_8(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_10(voi, states, rates, constants, computed_constants, algebraic_variables)
    algebraic_variables[6] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0)
    find_root_6(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_7(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_5(voi, states, rates, constants, computed_constants, algebraic_variables)
    find_root_1(voi, states, rates, constants, computed_constants, algebraic_variables)
