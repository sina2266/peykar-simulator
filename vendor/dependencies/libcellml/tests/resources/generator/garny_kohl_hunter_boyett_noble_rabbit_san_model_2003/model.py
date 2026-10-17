# The content of this file was generated using the Python profile of libCellML 0.7.1.

from enum import Enum
from math import *


__version__ = "0.8.0"
LIBCELLML_VERSION = "0.7.1"

STATE_COUNT = 15
CONSTANT_COUNT = 110
COMPUTED_CONSTANT_COUNT = 23
ALGEBRAIC_VARIABLE_COUNT = 52

VOI_INFO = {"name": "time", "units": "second", "component": "environment"}

STATE_INFO = [
    {"name": "V", "units": "millivolt", "component": "membrane"},
    {"name": "m", "units": "dimensionless", "component": "sodium_current_m_gate"},
    {"name": "h2", "units": "dimensionless", "component": "sodium_current_h_gate"},
    {"name": "h1", "units": "dimensionless", "component": "sodium_current_h_gate"},
    {"name": "d_L", "units": "dimensionless", "component": "L_type_Ca_channel_d_gate"},
    {"name": "f_L", "units": "dimensionless", "component": "L_type_Ca_channel_f_gate"},
    {"name": "f_T", "units": "dimensionless", "component": "T_type_Ca_channel_f_gate"},
    {"name": "d_T", "units": "dimensionless", "component": "T_type_Ca_channel_d_gate"},
    {"name": "r", "units": "dimensionless", "component": "four_AP_sensitive_currents_r_gate"},
    {"name": "q", "units": "dimensionless", "component": "four_AP_sensitive_currents_q_gate"},
    {"name": "P_i", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current_P_i_gate"},
    {"name": "P_as", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current_P_as_gate"},
    {"name": "P_af", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current_P_af_gate"},
    {"name": "xs", "units": "dimensionless", "component": "slow_delayed_rectifying_potassium_current_xs_gate"},
    {"name": "y", "units": "dimensionless", "component": "hyperpolarisation_activated_current_y_gate"}
]

CONSTANT_INFO = [
    {"name": "dCell", "units": "dimensionless", "component": "membrane"},
    {"name": "Version", "units": "dimensionless", "component": "membrane"},
    {"name": "FCellConstant", "units": "dimensionless", "component": "membrane"},
    {"name": "CmCentre", "units": "microF", "component": "membrane"},
    {"name": "CmPeriphery", "units": "microF", "component": "membrane"},
    {"name": "R", "units": "millijoule_per_mole_kelvin", "component": "membrane"},
    {"name": "T", "units": "kelvin", "component": "membrane"},
    {"name": "F", "units": "coulomb_per_mole", "component": "membrane"},
    {"name": "g_b_Na_Centre_Published", "units": "microS", "component": "sodium_background_current"},
    {"name": "g_b_Na_Periphery_Published", "units": "microS", "component": "sodium_background_current"},
    {"name": "g_b_Na_Centre_1DCapable", "units": "microS", "component": "sodium_background_current"},
    {"name": "g_b_Na_Periphery_1DCapable", "units": "microS", "component": "sodium_background_current"},
    {"name": "g_b_Na_Centre_0DCapable", "units": "microS", "component": "sodium_background_current"},
    {"name": "g_b_Na_Periphery_0DCapable", "units": "microS", "component": "sodium_background_current"},
    {"name": "g_b_K_Centre_Published", "units": "microS", "component": "potassium_background_current"},
    {"name": "g_b_K_Periphery_Published", "units": "microS", "component": "potassium_background_current"},
    {"name": "g_b_K_Centre_1DCapable", "units": "microS", "component": "potassium_background_current"},
    {"name": "g_b_K_Periphery_1DCapable", "units": "microS", "component": "potassium_background_current"},
    {"name": "g_b_K_Centre_0DCapable", "units": "microS", "component": "potassium_background_current"},
    {"name": "g_b_K_Periphery_0DCapable", "units": "microS", "component": "potassium_background_current"},
    {"name": "g_b_Ca_Centre_Published", "units": "microS", "component": "calcium_background_current"},
    {"name": "g_b_Ca_Periphery_Published", "units": "microS", "component": "calcium_background_current"},
    {"name": "g_b_Ca_Centre_1DCapable", "units": "microS", "component": "calcium_background_current"},
    {"name": "g_b_Ca_Periphery_1DCapable", "units": "microS", "component": "calcium_background_current"},
    {"name": "g_b_Ca_Centre_0DCapable", "units": "microS", "component": "calcium_background_current"},
    {"name": "g_b_Ca_Periphery_0DCapable", "units": "microS", "component": "calcium_background_current"},
    {"name": "k_NaCa_Centre_Published", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "k_NaCa_Periphery_Published", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "k_NaCa_Centre_1DCapable", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "k_NaCa_Periphery_1DCapable", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "k_NaCa_Centre_0DCapable", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "k_NaCa_Periphery_0DCapable", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "Na_i", "units": "millimolar", "component": "ionic_concentrations"},
    {"name": "Ca_o", "units": "millimolar", "component": "ionic_concentrations"},
    {"name": "Na_o", "units": "millimolar", "component": "ionic_concentrations"},
    {"name": "Ca_i", "units": "millimolar", "component": "ionic_concentrations"},
    {"name": "d_NaCa", "units": "dimensionless", "component": "sodium_calcium_exchanger"},
    {"name": "gamma_NaCa", "units": "dimensionless", "component": "sodium_calcium_exchanger"},
    {"name": "i_p_max_Centre_Published", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_p_max_Periphery_Published", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_p_max_Centre_1DCapable", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_p_max_Periphery_1DCapable", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_p_max_Centre_0DCapable", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_p_max_Periphery_0DCapable", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "K_o", "units": "millimolar", "component": "ionic_concentrations"},
    {"name": "K_m_K", "units": "millimolar", "component": "sodium_potassium_pump"},
    {"name": "K_m_Na", "units": "millimolar", "component": "sodium_potassium_pump"},
    {"name": "i_Ca_p_max_Centre_Published", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "i_Ca_p_max_Periphery_Published", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "i_Ca_p_max_Centre_1DCapable", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "i_Ca_p_max_Periphery_1DCapable", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "i_Ca_p_max_Centre_0DCapable", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "i_Ca_p_max_Periphery_0DCapable", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "K_i", "units": "millimolar", "component": "ionic_concentrations"},
    {"name": "g_Na_Centre_Published", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Na_Periphery_Published", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Na_Centre_1DCapable", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Na_Periphery_1DCapable", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Na_Centre_0DCapable", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Na_Periphery_0DCapable", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Ca_L_Centre_Published", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_L_Periphery_Published", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_L_Centre_1DCapable", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_L_Periphery_1DCapable", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_L_Centre_0DCapable", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_L_Periphery_0DCapable", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "E_Ca_L", "units": "millivolt", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_T_Centre_Published", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "g_Ca_T_Periphery_Published", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "g_Ca_T_Centre_1DCapable", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "g_Ca_T_Periphery_1DCapable", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "g_Ca_T_Centre_0DCapable", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "g_Ca_T_Periphery_0DCapable", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "E_Ca_T", "units": "millivolt", "component": "T_type_Ca_channel"},
    {"name": "g_to_Centre_Published", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_to_Periphery_Published", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_to_Centre_1DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_to_Periphery_1DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_to_Centre_0DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_to_Periphery_0DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus_Centre_Published", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus_Periphery_Published", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus_Centre_1DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus_Periphery_1DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus_Centre_0DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus_Periphery_0DCapable", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_K_r_Centre_Published", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "g_K_r_Periphery_Published", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "g_K_r_Centre_1DCapable", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "g_K_r_Periphery_1DCapable", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "g_K_r_Centre_0DCapable", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "g_K_r_Periphery_0DCapable", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "g_K_s_Centre_Published", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_K_s_Periphery_Published", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_K_s_Centre_1DCapable", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_K_s_Periphery_1DCapable", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_K_s_Centre_0DCapable", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_K_s_Periphery_0DCapable", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_f_Na_Centre_Published", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_Na_Periphery_Published", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_Na_Centre_1DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_Na_Periphery_1DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_Na_Centre_0DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_Na_Periphery_0DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K_Centre_Published", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K_Periphery_Published", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K_Centre_1DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K_Periphery_1DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K_Centre_0DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K_Periphery_0DCapable", "units": "microS", "component": "hyperpolarisation_activated_current"}
]

COMPUTED_CONSTANT_INFO = [
    {"name": "FCell", "units": "dimensionless", "component": "membrane"},
    {"name": "Cm", "units": "microF", "component": "membrane"},
    {"name": "i_Ca_p", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "g_b_Na", "units": "microS", "component": "sodium_background_current"},
    {"name": "E_Na", "units": "millivolt", "component": "reversal_and_equilibrium_potentials"},
    {"name": "g_b_K", "units": "microS", "component": "potassium_background_current"},
    {"name": "E_K", "units": "millivolt", "component": "reversal_and_equilibrium_potentials"},
    {"name": "g_b_Ca", "units": "microS", "component": "calcium_background_current"},
    {"name": "E_Ca", "units": "millivolt", "component": "reversal_and_equilibrium_potentials"},
    {"name": "k_NaCa", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "i_p_max", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_Ca_p_max", "units": "nanoA", "component": "persistent_calcium_current"},
    {"name": "E_K_s", "units": "millivolt", "component": "reversal_and_equilibrium_potentials"},
    {"name": "g_Na", "units": "microlitre_per_second", "component": "sodium_current"},
    {"name": "g_Ca_L", "units": "microS", "component": "L_type_Ca_channel"},
    {"name": "g_Ca_T", "units": "microS", "component": "T_type_Ca_channel"},
    {"name": "g_to", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_sus", "units": "microS", "component": "four_AP_sensitive_currents"},
    {"name": "g_K_r", "units": "microS", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "tau_P_i", "units": "second", "component": "rapid_delayed_rectifying_potassium_current_P_i_gate"},
    {"name": "g_K_s", "units": "microS", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "g_f_Na", "units": "microS", "component": "hyperpolarisation_activated_current"},
    {"name": "g_f_K", "units": "microS", "component": "hyperpolarisation_activated_current"}
]

ALGEBRAIC_VARIABLE_INFO = [
    {"name": "i_p", "units": "nanoA", "component": "sodium_potassium_pump"},
    {"name": "i_NaCa", "units": "nanoA", "component": "sodium_calcium_exchanger"},
    {"name": "i_b_K", "units": "nanoA", "component": "potassium_background_current"},
    {"name": "i_b_Ca", "units": "nanoA", "component": "calcium_background_current"},
    {"name": "i_b_Na", "units": "nanoA", "component": "sodium_background_current"},
    {"name": "i_f_K", "units": "nanoA", "component": "hyperpolarisation_activated_current"},
    {"name": "i_f_Na", "units": "nanoA", "component": "hyperpolarisation_activated_current"},
    {"name": "i_K_s", "units": "nanoA", "component": "slow_delayed_rectifying_potassium_current"},
    {"name": "i_K_r", "units": "nanoA", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "i_sus", "units": "nanoA", "component": "four_AP_sensitive_currents"},
    {"name": "i_to", "units": "nanoA", "component": "four_AP_sensitive_currents"},
    {"name": "i_Ca_T", "units": "nanoA", "component": "T_type_Ca_channel"},
    {"name": "i_Ca_L", "units": "nanoA", "component": "L_type_Ca_channel"},
    {"name": "i_Na", "units": "nanoA", "component": "sodium_current"},
    {"name": "h", "units": "dimensionless", "component": "sodium_current_h_gate"},
    {"name": "tau_m", "units": "second", "component": "sodium_current_m_gate"},
    {"name": "m_infinity", "units": "dimensionless", "component": "sodium_current_m_gate"},
    {"name": "F_Na", "units": "dimensionless", "component": "sodium_current_h_gate"},
    {"name": "tau_h1", "units": "second", "component": "sodium_current_h_gate"},
    {"name": "h1_infinity", "units": "dimensionless", "component": "sodium_current_h_gate"},
    {"name": "tau_h2", "units": "second", "component": "sodium_current_h_gate"},
    {"name": "h2_infinity", "units": "dimensionless", "component": "sodium_current_h_gate"},
    {"name": "tau_d_L", "units": "second", "component": "L_type_Ca_channel_d_gate"},
    {"name": "d_L_infinity", "units": "dimensionless", "component": "L_type_Ca_channel_d_gate"},
    {"name": "alpha_d_L", "units": "per_second", "component": "L_type_Ca_channel_d_gate"},
    {"name": "beta_d_L", "units": "per_second", "component": "L_type_Ca_channel_d_gate"},
    {"name": "tau_f_L", "units": "second", "component": "L_type_Ca_channel_f_gate"},
    {"name": "f_L_infinity", "units": "dimensionless", "component": "L_type_Ca_channel_f_gate"},
    {"name": "alpha_f_L", "units": "per_second", "component": "L_type_Ca_channel_f_gate"},
    {"name": "beta_f_L", "units": "per_second", "component": "L_type_Ca_channel_f_gate"},
    {"name": "tau_d_T", "units": "second", "component": "T_type_Ca_channel_d_gate"},
    {"name": "d_T_infinity", "units": "dimensionless", "component": "T_type_Ca_channel_d_gate"},
    {"name": "alpha_d_T", "units": "per_second", "component": "T_type_Ca_channel_d_gate"},
    {"name": "beta_d_T", "units": "per_second", "component": "T_type_Ca_channel_d_gate"},
    {"name": "tau_f_T", "units": "second", "component": "T_type_Ca_channel_f_gate"},
    {"name": "f_T_infinity", "units": "dimensionless", "component": "T_type_Ca_channel_f_gate"},
    {"name": "alpha_f_T", "units": "per_second", "component": "T_type_Ca_channel_f_gate"},
    {"name": "beta_f_T", "units": "per_second", "component": "T_type_Ca_channel_f_gate"},
    {"name": "tau_q", "units": "second", "component": "four_AP_sensitive_currents_q_gate"},
    {"name": "q_infinity", "units": "dimensionless", "component": "four_AP_sensitive_currents_q_gate"},
    {"name": "tau_r", "units": "second", "component": "four_AP_sensitive_currents_r_gate"},
    {"name": "r_infinity", "units": "dimensionless", "component": "four_AP_sensitive_currents_r_gate"},
    {"name": "P_a", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current"},
    {"name": "tau_P_af", "units": "second", "component": "rapid_delayed_rectifying_potassium_current_P_af_gate"},
    {"name": "P_af_infinity", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current_P_af_gate"},
    {"name": "tau_P_as", "units": "second", "component": "rapid_delayed_rectifying_potassium_current_P_as_gate"},
    {"name": "P_as_infinity", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current_P_as_gate"},
    {"name": "P_i_infinity", "units": "dimensionless", "component": "rapid_delayed_rectifying_potassium_current_P_i_gate"},
    {"name": "beta_xs", "units": "per_second", "component": "slow_delayed_rectifying_potassium_current_xs_gate"},
    {"name": "alpha_xs", "units": "per_second", "component": "slow_delayed_rectifying_potassium_current_xs_gate"},
    {"name": "beta_y", "units": "per_second", "component": "hyperpolarisation_activated_current_y_gate"},
    {"name": "alpha_y", "units": "per_second", "component": "hyperpolarisation_activated_current_y_gate"}
]


def eq_func(x, y):
    return 1.0 if x == y else 0.0


def neq_func(x, y):
    return 1.0 if x != y else 0.0


def create_states_array():
    return [nan]*STATE_COUNT


def create_constants_array():
    return [nan]*CONSTANT_COUNT


def create_computed_constants_array():
    return [nan]*COMPUTED_CONSTANT_COUNT


def create_algebraic_variables_array():
    return [nan]*ALGEBRAIC_VARIABLE_COUNT


def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):
    states[0] = -39.013558536
    states[1] = 0.092361701692
    states[2] = 0.01445216109
    states[3] = 0.015905380261
    states[4] = 0.04804900895
    states[5] = 0.48779845203
    states[6] = 0.038968420558
    states[7] = 0.42074047435
    states[8] = 0.064402950262
    states[9] = 0.29760539675
    states[10] = 0.87993375273
    states[11] = 0.46960956028
    states[12] = 0.13034201158
    states[13] = 0.082293827208
    states[14] = 0.03889291759
    constants[0] = 0.0
    constants[1] = 1.0
    constants[2] = 1.0309347
    constants[3] = 2.0e-5
    constants[4] = 6.5e-5
    constants[5] = 8314.0
    constants[6] = 310.0
    constants[7] = 96845.0
    constants[8] = 5.8e-5
    constants[9] = 0.000189
    constants[10] = 5.8e-5
    constants[11] = 0.000189
    constants[12] = 5.81818e-5
    constants[13] = 0.0001888
    constants[14] = 2.52e-5
    constants[15] = 8.19e-5
    constants[16] = 2.52e-5
    constants[17] = 8.19e-5
    constants[18] = 2.523636e-5
    constants[19] = 8.1892e-5
    constants[20] = 1.32e-5
    constants[21] = 4.3e-5
    constants[22] = 1.323e-5
    constants[23] = 4.29e-5
    constants[24] = 1.3236e-5
    constants[25] = 4.2952e-5
    constants[26] = 2.7e-6
    constants[27] = 8.8e-6
    constants[28] = 2.8e-6
    constants[29] = 8.8e-6
    constants[30] = 2.7229e-6
    constants[31] = 8.83584e-6
    constants[32] = 8.0
    constants[33] = 2.0
    constants[34] = 140.0
    constants[35] = 0.0001
    constants[36] = 0.0001
    constants[37] = 0.5
    constants[38] = 0.0478
    constants[39] = 0.16
    constants[40] = 0.0478
    constants[41] = 0.16
    constants[42] = 0.04782545
    constants[43] = 0.1551936
    constants[44] = 5.4
    constants[45] = 0.621
    constants[46] = 5.64
    constants[47] = 0.0
    constants[48] = 0.0
    constants[49] = 0.0042
    constants[50] = 0.03339
    constants[51] = 0.0
    constants[52] = 0.0
    constants[53] = 140.0
    constants[54] = 0.0
    constants[55] = 1.2e-6
    constants[56] = 0.0
    constants[57] = 3.7e-7
    constants[58] = 0.0
    constants[59] = 1.204e-6
    constants[60] = 0.0058
    constants[61] = 0.0659
    constants[62] = 0.0082
    constants[63] = 0.0659
    constants[64] = 0.0057938
    constants[65] = 0.06588648
    constants[66] = 46.4
    constants[67] = 0.0043
    constants[68] = 0.0139
    constants[69] = 0.0021
    constants[70] = 0.00694
    constants[71] = 0.00427806
    constants[72] = 0.0138823
    constants[73] = 45.0
    constants[74] = 0.00491
    constants[75] = 0.03649
    constants[76] = 0.004905
    constants[77] = 0.0365
    constants[78] = 0.004905
    constants[79] = 0.036495
    constants[80] = 6.65e-5
    constants[81] = 0.0114
    constants[82] = 0.000266
    constants[83] = 0.0114
    constants[84] = 6.645504e-5
    constants[85] = 0.01138376
    constants[86] = 0.000797
    constants[87] = 0.016
    constants[88] = 0.000738
    constants[89] = 0.0208
    constants[90] = 0.00079704
    constants[91] = 0.016
    constants[92] = 0.000518
    constants[93] = 0.0104
    constants[94] = 0.000345
    constants[95] = 0.0104
    constants[96] = 0.0003445
    constants[97] = 0.0104
    constants[98] = 0.000548
    constants[99] = 0.0069
    constants[100] = 0.000437
    constants[101] = 0.0055
    constants[102] = 0.0005465
    constants[103] = 0.006875
    constants[104] = 0.000548
    constants[105] = 0.0069
    constants[106] = 0.000437
    constants[107] = 0.0055
    constants[108] = 0.0005465
    constants[109] = 0.006875


def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):
    computed_constants[0] = 1.07*(3.0*constants[0]-0.1)/(3.0*(1.0+0.7745*exp(-(3.0*constants[0]-2.05)/0.295))) if eq_func(constants[1], 0.0) else constants[2]*constants[0]/(1.0+0.7745*exp(-(3.0*constants[0]-2.05)/0.295)) if eq_func(constants[1], 1.0) else 1.07*29.0*constants[0]/(30.0*(1.0+0.7745*exp(-(29.0*constants[0]-24.5)/1.95)))
    computed_constants[1] = constants[3]+computed_constants[0]*(constants[4]-constants[3])
    computed_constants[3] = constants[8]+computed_constants[0]*(constants[9]-constants[8]) if eq_func(constants[1], 0.0) else constants[12]+computed_constants[0]*(constants[13]-constants[12]) if eq_func(constants[1], 1.0) else constants[10]+computed_constants[0]*(constants[11]-constants[10])
    computed_constants[5] = constants[14]+computed_constants[0]*(constants[15]-constants[14]) if eq_func(constants[1], 0.0) else constants[18]+computed_constants[0]*(constants[19]-constants[18]) if eq_func(constants[1], 1.0) else constants[16]+computed_constants[0]*(constants[17]-constants[16])
    computed_constants[7] = constants[20]+computed_constants[0]*(constants[21]-constants[20]) if eq_func(constants[1], 0.0) else constants[24]+computed_constants[0]*(constants[25]-constants[24]) if eq_func(constants[1], 1.0) else constants[22]+computed_constants[0]*(constants[23]-constants[22])
    computed_constants[9] = constants[26]+computed_constants[0]*(constants[27]-constants[26]) if eq_func(constants[1], 0.0) else constants[30]+computed_constants[0]*(constants[31]-constants[30]) if eq_func(constants[1], 1.0) else constants[28]+computed_constants[0]*(constants[29]-constants[28])
    computed_constants[10] = constants[38]+computed_constants[0]*(constants[39]-constants[38]) if eq_func(constants[1], 0.0) else constants[42]+computed_constants[0]*(constants[43]-constants[42]) if eq_func(constants[1], 1.0) else constants[40]+computed_constants[0]*(constants[41]-constants[40])
    computed_constants[11] = constants[47]+computed_constants[0]*(constants[48]-constants[47]) if eq_func(constants[1], 0.0) else constants[51]+computed_constants[0]*(constants[52]-constants[51]) if eq_func(constants[1], 1.0) else constants[49]+computed_constants[0]*(constants[50]-constants[49])
    computed_constants[2] = computed_constants[11]*constants[35]/(constants[35]+0.0004)
    computed_constants[4] = constants[5]*constants[6]/constants[7]*log(constants[34]/constants[32])
    computed_constants[6] = constants[5]*constants[6]/constants[7]*log(constants[44]/constants[53])
    computed_constants[8] = constants[5]*constants[6]/(2.0*constants[7])*log(constants[33]/constants[35])
    computed_constants[12] = constants[5]*constants[6]/constants[7]*log((constants[44]+0.12*constants[34])/(constants[53]+0.12*constants[32])) if eq_func(constants[1], 0.0) else constants[5]*constants[6]/constants[7]*log((constants[44]+0.03*constants[34])/(constants[53]+0.03*constants[32]))
    computed_constants[13] = constants[54]+computed_constants[0]*(constants[55]-constants[54]) if eq_func(constants[1], 0.0) else constants[58]+computed_constants[0]*(constants[59]-constants[58]) if eq_func(constants[1], 1.0) else constants[56]+computed_constants[0]*(constants[57]-constants[56])
    computed_constants[14] = constants[60]+computed_constants[0]*(constants[61]-constants[60]) if eq_func(constants[1], 0.0) else constants[64]+computed_constants[0]*(constants[65]-constants[64]) if eq_func(constants[1], 1.0) else constants[62]+computed_constants[0]*(constants[63]-constants[62])
    computed_constants[15] = constants[67]+computed_constants[0]*(constants[68]-constants[67]) if eq_func(constants[1], 0.0) else constants[71]+computed_constants[0]*(constants[72]-constants[71]) if eq_func(constants[1], 1.0) else constants[69]+computed_constants[0]*(constants[70]-constants[69])
    computed_constants[16] = constants[74]+computed_constants[0]*(constants[75]-constants[74]) if eq_func(constants[1], 0.0) else constants[78]+computed_constants[0]*(constants[79]-constants[78]) if eq_func(constants[1], 1.0) else constants[76]+computed_constants[0]*(constants[77]-constants[76])
    computed_constants[17] = constants[80]+computed_constants[0]*(constants[81]-constants[80]) if eq_func(constants[1], 0.0) else constants[84]+computed_constants[0]*(constants[85]-constants[84]) if eq_func(constants[1], 1.0) else constants[82]+computed_constants[0]*(constants[83]-constants[82])
    computed_constants[18] = constants[86]+computed_constants[0]*(constants[87]-constants[86]) if eq_func(constants[1], 0.0) else constants[90]+computed_constants[0]*(constants[91]-constants[90]) if eq_func(constants[1], 1.0) else constants[88]+computed_constants[0]*(constants[89]-constants[88])
    computed_constants[19] = 0.002 if eq_func(constants[1], 0.0) else 0.002 if eq_func(constants[1], 1.0) else 0.006
    computed_constants[20] = constants[92]+computed_constants[0]*(constants[93]-constants[92]) if eq_func(constants[1], 0.0) else constants[96]+computed_constants[0]*(constants[97]-constants[96]) if eq_func(constants[1], 1.0) else constants[94]+computed_constants[0]*(constants[95]-constants[94])
    computed_constants[21] = constants[98]+computed_constants[0]*(constants[99]-constants[98]) if eq_func(constants[1], 0.0) else constants[102]+computed_constants[0]*(constants[103]-constants[102]) if eq_func(constants[1], 1.0) else constants[100]+computed_constants[0]*(constants[101]-constants[100])
    computed_constants[22] = constants[104]+computed_constants[0]*(constants[105]-constants[104]) if eq_func(constants[1], 0.0) else constants[108]+computed_constants[0]*(constants[109]-constants[108]) if eq_func(constants[1], 1.0) else constants[106]+computed_constants[0]*(constants[107]-constants[106])


def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables):
    algebraic_variables[0] = computed_constants[10]*pow(constants[32]/(constants[46]+constants[32]), 3.0)*pow(constants[44]/(constants[45]+constants[44]), 2.0)*1.6/(1.5+exp(-(states[0]+60.0)/40.0))
    algebraic_variables[1] = computed_constants[9]*(pow(constants[32], 3.0)*constants[33]*exp(0.03743*states[0]*constants[37])-pow(constants[34], 3.0)*constants[35]*exp(0.0374*states[0]*(constants[37]-1.0)))/(1.0+constants[36]*(constants[35]*pow(constants[34], 3.0)+constants[33]*pow(constants[32], 3.0))) if eq_func(constants[1], 0.0) else computed_constants[9]*(pow(constants[32], 3.0)*constants[33]*exp(0.03743*states[0]*constants[37])-pow(constants[34], 3.0)*constants[35]*exp(0.03743*states[0]*(constants[37]-1.0)))/(1.0+constants[36]*(constants[35]*pow(constants[34], 3.0)+constants[33]*pow(constants[32], 3.0)))
    algebraic_variables[5] = computed_constants[22]*states[14]*(states[0]-computed_constants[6]) if neq_func(constants[1], 2.0) else computed_constants[22]*states[14]*(states[0]+102.0)
    algebraic_variables[6] = computed_constants[21]*states[14]*(states[0]-computed_constants[4]) if neq_func(constants[1], 2.0) else computed_constants[21]*states[14]*(states[0]-77.6)
    algebraic_variables[7] = computed_constants[20]*pow(states[13], 2.0)*(states[0]-computed_constants[12])
    algebraic_variables[9] = computed_constants[17]*states[8]*(states[0]-computed_constants[6])
    algebraic_variables[10] = computed_constants[16]*states[9]*states[8]*(states[0]-computed_constants[6])
    algebraic_variables[11] = computed_constants[15]*states[7]*states[6]*(states[0]-constants[73])
    algebraic_variables[12] = computed_constants[14]*(states[5]*states[4]+0.006/(1.0+exp(-(states[0]+14.1)/6.0)))*(states[0]-constants[66])
    algebraic_variables[2] = computed_constants[5]*(states[0]-computed_constants[6])
    algebraic_variables[3] = computed_constants[7]*(states[0]-computed_constants[8])
    algebraic_variables[4] = computed_constants[3]*(states[0]-computed_constants[4])
    algebraic_variables[42] = 0.6*states[12]+0.4*states[11]
    algebraic_variables[8] = computed_constants[18]*algebraic_variables[42]*states[10]*(states[0]-computed_constants[6])
    algebraic_variables[17] = 0.0952*exp(-0.063*(states[0]+34.4))/(1.0+1.66*exp(-0.225*(states[0]+63.7)))+0.0869 if eq_func(constants[1], 0.0) else 0.09518*exp(-0.06306*(states[0]+34.4))/(1.0+1.662*exp(-0.2251*(states[0]+63.7)))+0.08693
    algebraic_variables[14] = (1.0-algebraic_variables[17])*states[3]+algebraic_variables[17]*states[2]
    algebraic_variables[13] = computed_constants[13]*pow(states[1], 3.0)*algebraic_variables[14]*constants[34]*pow(constants[7], 2.0)/(constants[5]*constants[6])*(exp((states[0]-computed_constants[4])*constants[7]/(constants[5]*constants[6]))-1.0)/(exp(states[0]*constants[7]/(constants[5]*constants[6]))-1.0)*states[0]
    rates[0] = -1.0/computed_constants[1]*(algebraic_variables[13]+algebraic_variables[12]+algebraic_variables[11]+algebraic_variables[10]+algebraic_variables[9]+algebraic_variables[8]+algebraic_variables[7]+algebraic_variables[6]+algebraic_variables[5]+algebraic_variables[4]+algebraic_variables[3]+algebraic_variables[2]+algebraic_variables[1]+algebraic_variables[0]+computed_constants[2])
    algebraic_variables[15] = 0.0006247/(0.832*exp(-0.335*(states[0]+56.7))+0.627*exp(0.082*(states[0]+65.01)))+4.0e-5 if eq_func(constants[1], 0.0) else 0.0006247/(0.8322166*exp(-0.33566*(states[0]+56.7062))+0.6274*exp(0.0823*(states[0]+65.0131)))+4.569e-5
    algebraic_variables[16] = pow(1.0/(1.0+exp(-states[0]/5.46)), 1.0/3.0) if eq_func(constants[1], 0.0) else pow(1.0/(1.0+exp(-(states[0]+30.32)/5.46)), 1.0/3.0)
    rates[1] = (algebraic_variables[16]-states[1])/algebraic_variables[15]
    algebraic_variables[18] = 3.717e-6*exp(-0.2815*(states[0]+17.11))/(1.0+0.003732*exp(-0.3426*(states[0]+37.76)))+0.0005977
    algebraic_variables[19] = 1.0/(1.0+exp((states[0]+66.1)/6.4))
    rates[3] = (algebraic_variables[19]-states[3])/algebraic_variables[18]
    algebraic_variables[20] = 3.186e-8*exp(-0.6219*(states[0]+18.8))/(1.0+7.189e-5*exp(-0.6683*(states[0]+34.07)))+0.003556
    algebraic_variables[21] = algebraic_variables[19]
    rates[2] = (algebraic_variables[21]-states[2])/algebraic_variables[20]
    algebraic_variables[25] = 11.43*(states[0]-5.0)/(exp(0.4*(states[0]-5.0))-1.0) if eq_func(constants[1], 1.0) else 11.42*(states[0]-5.0)/(exp(0.4*(states[0]-5.0))-1.0)
    algebraic_variables[24] = -28.38*(states[0]+35.0)/(exp(-(states[0]+35.0)/2.5)-1.0)-84.9*states[0]/(exp(-0.208*states[0])-1.0) if eq_func(constants[1], 0.0) else -28.39*(states[0]+35.0)/(exp(-(states[0]+35.0)/2.5)-1.0)-84.9*states[0]/(exp(-0.208*states[0])-1.0) if eq_func(constants[1], 1.0) else -28.4*(states[0]+35.0)/(exp(-(states[0]+35.0)/2.5)-1.0)-84.9*states[0]/(exp(-0.208*states[0])-1.0)
    algebraic_variables[22] = 2.0/(algebraic_variables[24]+algebraic_variables[25])
    algebraic_variables[23] = 1.0/(1.0+exp(-(states[0]+23.1)/6.0)) if eq_func(constants[1], 0.0) else 1.0/(1.0+exp(-(states[0]+22.3+0.8*computed_constants[0])/6.0)) if eq_func(constants[1], 1.0) else 1.0/(1.0+exp(-(states[0]+22.2)/6.0))
    rates[4] = (algebraic_variables[23]-states[4])/algebraic_variables[22]
    algebraic_variables[29] = 30.0/(1.0+exp(-(states[0]+28.0)/4.0)) if eq_func(constants[1], 1.0) else 25.0/(1.0+exp(-(states[0]+28.0)/4.0))
    algebraic_variables[28] = 3.75*(states[0]+28.0)/(exp((states[0]+28.0)/4.0)-1.0) if eq_func(constants[1], 1.0) else 3.12*(states[0]+28.0)/(exp((states[0]+28.0)/4.0)-1.0)
    algebraic_variables[26] = (1.2-0.2*computed_constants[0])/(algebraic_variables[28]+algebraic_variables[29]) if eq_func(constants[1], 1.0) else 1.0/(algebraic_variables[28]+algebraic_variables[29])
    algebraic_variables[27] = 1.0/(1.0+exp((states[0]+45.0)/5.0))
    rates[5] = (algebraic_variables[27]-states[5])/algebraic_variables[26]
    algebraic_variables[33] = 1068.0*exp(-(states[0]+26.3)/30.0)
    algebraic_variables[32] = 1068.0*exp((states[0]+26.3)/30.0)
    algebraic_variables[30] = 1.0/(algebraic_variables[32]+algebraic_variables[33])
    algebraic_variables[31] = 1.0/(1.0+exp(-(states[0]+37.0)/6.8))
    rates[7] = (algebraic_variables[31]-states[7])/algebraic_variables[30]
    algebraic_variables[37] = 15.0*exp((states[0]+71.0)/15.38) if eq_func(constants[1], 1.0) else 15.0*exp((states[0]+71.7)/15.38)
    algebraic_variables[36] = 15.3*exp(-(states[0]+71.0+0.7*computed_constants[0])/83.3) if eq_func(constants[1], 1.0) else 15.3*exp(-(states[0]+71.7)/83.3)
    algebraic_variables[34] = 1.0/(algebraic_variables[36]+algebraic_variables[37])
    algebraic_variables[35] = 1.0/(1.0+exp((states[0]+71.0)/9.0))
    rates[6] = (algebraic_variables[35]-states[6])/algebraic_variables[34]
    algebraic_variables[38] = 0.0101+0.06517/(0.57*exp(-0.08*(states[0]+49.0)))+2.4e-5*exp(0.1*(states[0]+50.93)) if eq_func(constants[1], 0.0) else 0.001/3.0*(30.31+195.5/(0.5686*exp(-0.08161*(states[0]+39.0+10.0*computed_constants[0]))+0.7174*exp((0.2719-0.1719*computed_constants[0])*1.0*(states[0]+40.93+10.0*computed_constants[0])))) if eq_func(constants[1], 1.0) else 0.0101+0.06517/(0.5686*exp(-0.08161*(states[0]+39.0))+0.7174*exp(0.2719*(states[0]+40.93)))
    algebraic_variables[39] = 1.0/(1.0+exp((states[0]+59.37)/13.1))
    rates[9] = (algebraic_variables[39]-states[9])/algebraic_variables[38]
    algebraic_variables[40] = 0.001*(2.98+15.59/(1.037*exp(0.09*(states[0]+30.61))+0.369*exp(-0.12*(states[0]+23.84)))) if eq_func(constants[1], 0.0) else 0.0025*(1.191+7.838/(1.037*exp(0.09012*(states[0]+30.61))+0.369*exp(-0.119*(states[0]+23.84)))) if eq_func(constants[1], 1.0) else 0.001*(2.98+19.59/(1.037*exp(0.09012*(states[0]+30.61))+0.369*exp(-0.119*(states[0]+23.84))))
    algebraic_variables[41] = 1.0/(1.0+exp(-(states[0]-10.93)/19.7))
    rates[8] = (algebraic_variables[41]-states[8])/algebraic_variables[40]
    algebraic_variables[43] = 1.0/(37.2*exp((states[0]-9.0)/15.9)+0.96*exp(-(states[0]-9.0)/22.5)) if neq_func(constants[1], 2.0) else 1.0/(37.2*exp((states[0]-10.0)/15.9)+0.96*exp(-(states[0]-10.0)/22.5))
    algebraic_variables[44] = 1.0/(1.0+exp(-(states[0]+14.2)/10.6)) if neq_func(constants[1], 2.0) else 1.0/(1.0+exp(-(states[0]+13.2)/10.6))
    rates[12] = (algebraic_variables[44]-states[12])/algebraic_variables[43]
    algebraic_variables[45] = 1.0/(4.2*exp((states[0]-9.0)/17.0)+0.15*exp(-(states[0]-9.0)/21.6)) if neq_func(constants[1], 2.0) else 1.0/(4.2*exp((states[0]-10.0)/17.0)+0.15*exp(-(states[0]-10.0)/21.6))
    algebraic_variables[46] = algebraic_variables[44]
    rates[11] = (algebraic_variables[46]-states[11])/algebraic_variables[45]
    algebraic_variables[47] = 1.0/(1.0+exp((states[0]+18.6)/10.1))
    rates[10] = (algebraic_variables[47]-states[10])/computed_constants[19]
    algebraic_variables[48] = 1.0*exp(-states[0]/45.0)
    algebraic_variables[49] = 14.0/(1.0+exp(-(states[0]-40.0)/9.0))
    rates[13] = algebraic_variables[49]*(1.0-states[13])-algebraic_variables[48]*states[13]
    algebraic_variables[50] = 1.0*exp((states[0]+75.13)/21.25)
    algebraic_variables[51] = 1.0*exp(-(states[0]+78.91)/26.62) if eq_func(constants[1], 0.0) else 1.0*exp(-(states[0]+78.91)/26.63)
    rates[14] = algebraic_variables[51]*(1.0-states[14])-algebraic_variables[50]*states[14]


def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables):
    algebraic_variables[4] = computed_constants[3]*(states[0]-computed_constants[4])
    algebraic_variables[2] = computed_constants[5]*(states[0]-computed_constants[6])
    algebraic_variables[3] = computed_constants[7]*(states[0]-computed_constants[8])
    algebraic_variables[1] = computed_constants[9]*(pow(constants[32], 3.0)*constants[33]*exp(0.03743*states[0]*constants[37])-pow(constants[34], 3.0)*constants[35]*exp(0.0374*states[0]*(constants[37]-1.0)))/(1.0+constants[36]*(constants[35]*pow(constants[34], 3.0)+constants[33]*pow(constants[32], 3.0))) if eq_func(constants[1], 0.0) else computed_constants[9]*(pow(constants[32], 3.0)*constants[33]*exp(0.03743*states[0]*constants[37])-pow(constants[34], 3.0)*constants[35]*exp(0.03743*states[0]*(constants[37]-1.0)))/(1.0+constants[36]*(constants[35]*pow(constants[34], 3.0)+constants[33]*pow(constants[32], 3.0)))
    algebraic_variables[0] = computed_constants[10]*pow(constants[32]/(constants[46]+constants[32]), 3.0)*pow(constants[44]/(constants[45]+constants[44]), 2.0)*1.6/(1.5+exp(-(states[0]+60.0)/40.0))
    algebraic_variables[17] = 0.0952*exp(-0.063*(states[0]+34.4))/(1.0+1.66*exp(-0.225*(states[0]+63.7)))+0.0869 if eq_func(constants[1], 0.0) else 0.09518*exp(-0.06306*(states[0]+34.4))/(1.0+1.662*exp(-0.2251*(states[0]+63.7)))+0.08693
    algebraic_variables[14] = (1.0-algebraic_variables[17])*states[3]+algebraic_variables[17]*states[2]
    algebraic_variables[13] = computed_constants[13]*pow(states[1], 3.0)*algebraic_variables[14]*constants[34]*pow(constants[7], 2.0)/(constants[5]*constants[6])*(exp((states[0]-computed_constants[4])*constants[7]/(constants[5]*constants[6]))-1.0)/(exp(states[0]*constants[7]/(constants[5]*constants[6]))-1.0)*states[0]
    algebraic_variables[16] = pow(1.0/(1.0+exp(-states[0]/5.46)), 1.0/3.0) if eq_func(constants[1], 0.0) else pow(1.0/(1.0+exp(-(states[0]+30.32)/5.46)), 1.0/3.0)
    algebraic_variables[15] = 0.0006247/(0.832*exp(-0.335*(states[0]+56.7))+0.627*exp(0.082*(states[0]+65.01)))+4.0e-5 if eq_func(constants[1], 0.0) else 0.0006247/(0.8322166*exp(-0.33566*(states[0]+56.7062))+0.6274*exp(0.0823*(states[0]+65.0131)))+4.569e-5
    algebraic_variables[19] = 1.0/(1.0+exp((states[0]+66.1)/6.4))
    algebraic_variables[21] = algebraic_variables[19]
    algebraic_variables[18] = 3.717e-6*exp(-0.2815*(states[0]+17.11))/(1.0+0.003732*exp(-0.3426*(states[0]+37.76)))+0.0005977
    algebraic_variables[20] = 3.186e-8*exp(-0.6219*(states[0]+18.8))/(1.0+7.189e-5*exp(-0.6683*(states[0]+34.07)))+0.003556
    algebraic_variables[12] = computed_constants[14]*(states[5]*states[4]+0.006/(1.0+exp(-(states[0]+14.1)/6.0)))*(states[0]-constants[66])
    algebraic_variables[24] = -28.38*(states[0]+35.0)/(exp(-(states[0]+35.0)/2.5)-1.0)-84.9*states[0]/(exp(-0.208*states[0])-1.0) if eq_func(constants[1], 0.0) else -28.39*(states[0]+35.0)/(exp(-(states[0]+35.0)/2.5)-1.0)-84.9*states[0]/(exp(-0.208*states[0])-1.0) if eq_func(constants[1], 1.0) else -28.4*(states[0]+35.0)/(exp(-(states[0]+35.0)/2.5)-1.0)-84.9*states[0]/(exp(-0.208*states[0])-1.0)
    algebraic_variables[25] = 11.43*(states[0]-5.0)/(exp(0.4*(states[0]-5.0))-1.0) if eq_func(constants[1], 1.0) else 11.42*(states[0]-5.0)/(exp(0.4*(states[0]-5.0))-1.0)
    algebraic_variables[22] = 2.0/(algebraic_variables[24]+algebraic_variables[25])
    algebraic_variables[23] = 1.0/(1.0+exp(-(states[0]+23.1)/6.0)) if eq_func(constants[1], 0.0) else 1.0/(1.0+exp(-(states[0]+22.3+0.8*computed_constants[0])/6.0)) if eq_func(constants[1], 1.0) else 1.0/(1.0+exp(-(states[0]+22.2)/6.0))
    algebraic_variables[28] = 3.75*(states[0]+28.0)/(exp((states[0]+28.0)/4.0)-1.0) if eq_func(constants[1], 1.0) else 3.12*(states[0]+28.0)/(exp((states[0]+28.0)/4.0)-1.0)
    algebraic_variables[29] = 30.0/(1.0+exp(-(states[0]+28.0)/4.0)) if eq_func(constants[1], 1.0) else 25.0/(1.0+exp(-(states[0]+28.0)/4.0))
    algebraic_variables[26] = (1.2-0.2*computed_constants[0])/(algebraic_variables[28]+algebraic_variables[29]) if eq_func(constants[1], 1.0) else 1.0/(algebraic_variables[28]+algebraic_variables[29])
    algebraic_variables[27] = 1.0/(1.0+exp((states[0]+45.0)/5.0))
    algebraic_variables[11] = computed_constants[15]*states[7]*states[6]*(states[0]-constants[73])
    algebraic_variables[32] = 1068.0*exp((states[0]+26.3)/30.0)
    algebraic_variables[33] = 1068.0*exp(-(states[0]+26.3)/30.0)
    algebraic_variables[30] = 1.0/(algebraic_variables[32]+algebraic_variables[33])
    algebraic_variables[31] = 1.0/(1.0+exp(-(states[0]+37.0)/6.8))
    algebraic_variables[36] = 15.3*exp(-(states[0]+71.0+0.7*computed_constants[0])/83.3) if eq_func(constants[1], 1.0) else 15.3*exp(-(states[0]+71.7)/83.3)
    algebraic_variables[37] = 15.0*exp((states[0]+71.0)/15.38) if eq_func(constants[1], 1.0) else 15.0*exp((states[0]+71.7)/15.38)
    algebraic_variables[34] = 1.0/(algebraic_variables[36]+algebraic_variables[37])
    algebraic_variables[35] = 1.0/(1.0+exp((states[0]+71.0)/9.0))
    algebraic_variables[10] = computed_constants[16]*states[9]*states[8]*(states[0]-computed_constants[6])
    algebraic_variables[9] = computed_constants[17]*states[8]*(states[0]-computed_constants[6])
    algebraic_variables[39] = 1.0/(1.0+exp((states[0]+59.37)/13.1))
    algebraic_variables[38] = 0.0101+0.06517/(0.57*exp(-0.08*(states[0]+49.0)))+2.4e-5*exp(0.1*(states[0]+50.93)) if eq_func(constants[1], 0.0) else 0.001/3.0*(30.31+195.5/(0.5686*exp(-0.08161*(states[0]+39.0+10.0*computed_constants[0]))+0.7174*exp((0.2719-0.1719*computed_constants[0])*1.0*(states[0]+40.93+10.0*computed_constants[0])))) if eq_func(constants[1], 1.0) else 0.0101+0.06517/(0.5686*exp(-0.08161*(states[0]+39.0))+0.7174*exp(0.2719*(states[0]+40.93)))
    algebraic_variables[41] = 1.0/(1.0+exp(-(states[0]-10.93)/19.7))
    algebraic_variables[40] = 0.001*(2.98+15.59/(1.037*exp(0.09*(states[0]+30.61))+0.369*exp(-0.12*(states[0]+23.84)))) if eq_func(constants[1], 0.0) else 0.0025*(1.191+7.838/(1.037*exp(0.09012*(states[0]+30.61))+0.369*exp(-0.119*(states[0]+23.84)))) if eq_func(constants[1], 1.0) else 0.001*(2.98+19.59/(1.037*exp(0.09012*(states[0]+30.61))+0.369*exp(-0.119*(states[0]+23.84))))
    algebraic_variables[42] = 0.6*states[12]+0.4*states[11]
    algebraic_variables[8] = computed_constants[18]*algebraic_variables[42]*states[10]*(states[0]-computed_constants[6])
    algebraic_variables[44] = 1.0/(1.0+exp(-(states[0]+14.2)/10.6)) if neq_func(constants[1], 2.0) else 1.0/(1.0+exp(-(states[0]+13.2)/10.6))
    algebraic_variables[43] = 1.0/(37.2*exp((states[0]-9.0)/15.9)+0.96*exp(-(states[0]-9.0)/22.5)) if neq_func(constants[1], 2.0) else 1.0/(37.2*exp((states[0]-10.0)/15.9)+0.96*exp(-(states[0]-10.0)/22.5))
    algebraic_variables[46] = algebraic_variables[44]
    algebraic_variables[45] = 1.0/(4.2*exp((states[0]-9.0)/17.0)+0.15*exp(-(states[0]-9.0)/21.6)) if neq_func(constants[1], 2.0) else 1.0/(4.2*exp((states[0]-10.0)/17.0)+0.15*exp(-(states[0]-10.0)/21.6))
    algebraic_variables[47] = 1.0/(1.0+exp((states[0]+18.6)/10.1))
    algebraic_variables[7] = computed_constants[20]*pow(states[13], 2.0)*(states[0]-computed_constants[12])
    algebraic_variables[49] = 14.0/(1.0+exp(-(states[0]-40.0)/9.0))
    algebraic_variables[48] = 1.0*exp(-states[0]/45.0)
    algebraic_variables[6] = computed_constants[21]*states[14]*(states[0]-computed_constants[4]) if neq_func(constants[1], 2.0) else computed_constants[21]*states[14]*(states[0]-77.6)
    algebraic_variables[5] = computed_constants[22]*states[14]*(states[0]-computed_constants[6]) if neq_func(constants[1], 2.0) else computed_constants[22]*states[14]*(states[0]+102.0)
    algebraic_variables[51] = 1.0*exp(-(states[0]+78.91)/26.62) if eq_func(constants[1], 0.0) else 1.0*exp(-(states[0]+78.91)/26.63)
    algebraic_variables[50] = 1.0*exp((states[0]+75.13)/21.25)
