/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.untracked.variables.with.externals.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 4;
const size_t CONSTANT_COUNT = 1;
const size_t COMPUTED_CONSTANT_COUNT = 1;
const size_t ALGEBRAIC_VARIABLE_COUNT = 1;
const size_t EXTERNAL_VARIABLE_COUNT = 1;

const VariableInfo VOI_INFO = {"time", "millisecond", "environment"};

const VariableInfo STATE_INFO[] = {
    {"V", "millivolt", "membrane"},
    {"h", "dimensionless", "sodium_channel_h_gate"},
    {"m", "dimensionless", "sodium_channel_m_gate"},
    {"n", "dimensionless", "potassium_channel_n_gate"}
};

const VariableInfo CONSTANT_INFO[] = {
    {"Cm", "microF_per_cm2", "membrane"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"E_K", "millivolt", "potassium_channel"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"alpha_m", "per_millisecond", "sodium_channel_m_gate"}
};

const VariableInfo EXTERNAL_VARIABLE_INFO[] = {
    {"i_Na", "microA_per_cm2", "sodium_channel"}
};

double * createStatesArray()
{
    double *res = (double *) malloc(STATE_COUNT*sizeof(double));

    for (size_t i = 0; i < STATE_COUNT; ++i) {
        res[i] = NAN;
    }

    return res;
}

double * createConstantsArray()
{
    double *res = (double *) malloc(CONSTANT_COUNT*sizeof(double));

    for (size_t i = 0; i < CONSTANT_COUNT; ++i) {
        res[i] = NAN;
    }

    return res;
}

double * createComputedConstantsArray()
{
    double *res = (double *) malloc(COMPUTED_CONSTANT_COUNT*sizeof(double));

    for (size_t i = 0; i < COMPUTED_CONSTANT_COUNT; ++i) {
        res[i] = NAN;
    }

    return res;
}

double * createAlgebraicVariablesArray()
{
    double *res = (double *) malloc(ALGEBRAIC_VARIABLE_COUNT*sizeof(double));

    for (size_t i = 0; i < ALGEBRAIC_VARIABLE_COUNT; ++i) {
        res[i] = NAN;
    }

    return res;
}

double * createExternalVariablesArray()
{
    double *res = (double *) malloc(EXTERNAL_VARIABLE_COUNT*sizeof(double));

    for (size_t i = 0; i < EXTERNAL_VARIABLE_COUNT; ++i) {
        res[i] = NAN;
    }

    return res;
}

void deleteArray(double *array)
{
    free(array);
}

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    states[0] = 0.0;
    states[1] = 0.6;
    states[2] = 0.05;
    states[3] = 0.325;
    constants[0] = 1.0;
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    double membrane_E_R = 0.0;
    computedConstants[0] = membrane_E_R+12.0;
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    double membrane_i_Stim = ((voi >= 10.0) && (voi <= 10.5))?-20.0:0.0;
    double leakage_current_g_L = 0.3;
    double membrane_E_R = 0.0;
    double leakage_current_E_L = membrane_E_R-10.613;
    double leakage_current_i_L = leakage_current_g_L*(states[0]-leakage_current_E_L);
    double potassium_channel_g_K = 36.0;
    double potassium_channel_i_K = potassium_channel_g_K*pow(states[3], 4.0)*(states[0]-computedConstants[0]);
    algebraicVariables[0] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0);
    externalVariables[0] = externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, 0);
    rates[0] = -(-membrane_i_Stim+externalVariables[0]+potassium_channel_i_K+leakage_current_i_L)/constants[0];
    double sodium_channel_m_gate_beta_m = 4.0*exp(states[0]/18.0);
    rates[2] = algebraicVariables[0]*(1.0-states[2])-sodium_channel_m_gate_beta_m*states[2];
    double sodium_channel_h_gate_beta_h = 1.0/(exp((states[0]+30.0)/10.0)+1.0);
    double sodium_channel_h_gate_alpha_h = 0.07*exp(states[0]/20.0);
    rates[1] = sodium_channel_h_gate_alpha_h*(1.0-states[1])-sodium_channel_h_gate_beta_h*states[1];
    double potassium_channel_n_gate_beta_n = 0.125*exp(states[0]/80.0);
    double potassium_channel_n_gate_alpha_n = 0.01*(states[0]+10.0)/(exp((states[0]+10.0)/10.0)-1.0);
    rates[3] = potassium_channel_n_gate_alpha_n*(1.0-states[3])-potassium_channel_n_gate_beta_n*states[3];
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    algebraicVariables[0] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0);
    externalVariables[0] = externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, 0);
}
