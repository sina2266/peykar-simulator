# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

STATE_COUNT = 4
CONSTANT_COUNT = 1
COMPUTED_CONSTANT_COUNT = 1
ALGEBRAIC_VARIABLE_COUNT = 1
EXTERNAL_VARIABLE_COUNT = 1

VOI_INFO = {"name": "time", "units": "millisecond", "component": "environment"}

STATE_INFO = [
    {"name": "V", "units": "millivolt", "component": "membrane"},
    {"name": "h", "units": "dimensionless", "component": "sodium_channel_h_gate"},
    {"name": "m", "units": "dimensionless", "component": "sodium_channel_m_gate"},
    {"name": "n", "units": "dimensionless", "component": "potassium_channel_n_gate"}
]

CONSTANT_INFO = [
    {"name": "Cm", "units": "microF_per_cm2", "component": "membrane"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "E_K", "units": "millivolt", "component": "potassium_channel"}
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "alpha_m", "units": "per_millisecond", "component": "sodium_channel_m_gate"}
]

EXTERNAL_VARIABLE_INFO = [
    {"name": "i_Na", "units": "microA_per_cm2", "component": "sodium_channel"}
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


def create_external_variables_array():
    return [nan]*EXTERNAL_VARIABLE_COUNT


def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):
    states[0] = 0.0
    states[1] = 0.6
    states[2] = 0.05
    states[3] = 0.325
    constants[0] = 1.0


def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):
    membrane_E_R = 0.0
    computed_constants[0] = membrane_E_R+12.0


def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, external_variable):
    membrane_i_Stim = -20.0 if and_func(geq_func(voi, 10.0), leq_func(voi, 10.5)) else 0.0
    leakage_current_g_L = 0.3
    membrane_E_R = 0.0
    leakage_current_E_L = membrane_E_R-10.613
    leakage_current_i_L = leakage_current_g_L*(states[0]-leakage_current_E_L)
    potassium_channel_g_K = 36.0
    potassium_channel_i_K = potassium_channel_g_K*pow(states[3], 4.0)*(states[0]-computed_constants[0])
    algebraic_variables[0] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0)
    external_variables[0] = external_variable(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, 0)
    rates[0] = -(-membrane_i_Stim+external_variables[0]+potassium_channel_i_K+leakage_current_i_L)/constants[0]
    sodium_channel_m_gate_beta_m = 4.0*exp(states[0]/18.0)
    rates[2] = algebraic_variables[0]*(1.0-states[2])-sodium_channel_m_gate_beta_m*states[2]
    sodium_channel_h_gate_beta_h = 1.0/(exp((states[0]+30.0)/10.0)+1.0)
    sodium_channel_h_gate_alpha_h = 0.07*exp(states[0]/20.0)
    rates[1] = sodium_channel_h_gate_alpha_h*(1.0-states[1])-sodium_channel_h_gate_beta_h*states[1]
    potassium_channel_n_gate_beta_n = 0.125*exp(states[0]/80.0)
    potassium_channel_n_gate_alpha_n = 0.01*(states[0]+10.0)/(exp((states[0]+10.0)/10.0)-1.0)
    rates[3] = potassium_channel_n_gate_alpha_n*(1.0-states[3])-potassium_channel_n_gate_beta_n*states[3]


def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, external_variable):
    algebraic_variables[0] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0)
    external_variables[0] = external_variable(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, 0)
