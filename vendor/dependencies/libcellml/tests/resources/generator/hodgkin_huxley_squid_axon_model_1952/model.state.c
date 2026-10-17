/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.state.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 3;
const size_t CONSTANT_COUNT = 5;
const size_t COMPUTED_CONSTANT_COUNT = 3;
const size_t ALGEBRAIC_VARIABLE_COUNT = 10;
const size_t EXTERNAL_VARIABLE_COUNT = 1;

const VariableInfo VOI_INFO = {"time", "millisecond", "environment"};

const VariableInfo STATE_INFO[] = {
    {"V", "millivolt", "membrane"},
    {"h", "dimensionless", "sodium_channel_h_gate"},
    {"n", "dimensionless", "potassium_channel_n_gate"}
};

const VariableInfo CONSTANT_INFO[] = {
    {"Cm", "microF_per_cm2", "membrane"},
    {"E_R", "millivolt", "membrane"},
    {"g_L", "milliS_per_cm2", "leakage_current"},
    {"g_Na", "milliS_per_cm2", "sodium_channel"},
    {"g_K", "milliS_per_cm2", "potassium_channel"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"E_L", "millivolt", "leakage_current"},
    {"E_Na", "millivolt", "sodium_channel"},
    {"E_K", "millivolt", "potassium_channel"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"i_Stim", "microA_per_cm2", "membrane"},
    {"i_L", "microA_per_cm2", "leakage_current"},
    {"i_K", "microA_per_cm2", "potassium_channel"},
    {"i_Na", "microA_per_cm2", "sodium_channel"},
    {"alpha_m", "per_millisecond", "sodium_channel_m_gate"},
    {"beta_m", "per_millisecond", "sodium_channel_m_gate"},
    {"alpha_h", "per_millisecond", "sodium_channel_h_gate"},
    {"beta_h", "per_millisecond", "sodium_channel_h_gate"},
    {"alpha_n", "per_millisecond", "potassium_channel_n_gate"},
    {"beta_n", "per_millisecond", "potassium_channel_n_gate"}
};

const VariableInfo EXTERNAL_VARIABLE_INFO[] = {
    {"m", "dimensionless", "sodium_channel_m_gate"}
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
    states[2] = 0.325;
    constants[0] = 1.0;
    constants[1] = 0.0;
    constants[2] = 0.3;
    constants[3] = 120.0;
    constants[4] = 36.0;
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = constants[1]-10.613;
    computedConstants[1] = constants[1]-115.0;
    computedConstants[2] = constants[1]+12.0;
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    algebraicVariables[0] = ((voi >= 10.0) && (voi <= 10.5))?-20.0:0.0;
    algebraicVariables[1] = constants[2]*(states[0]-computedConstants[0]);
    algebraicVariables[2] = constants[4]*pow(states[2], 4.0)*(states[0]-computedConstants[2]);
    externalVariables[0] = externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, 0);
    algebraicVariables[3] = constants[3]*pow(externalVariables[0], 3.0)*states[1]*(states[0]-computedConstants[1]);
    rates[0] = -(-algebraicVariables[0]+algebraicVariables[3]+algebraicVariables[2]+algebraicVariables[1])/constants[0];
    algebraicVariables[7] = 1.0/(exp((states[0]+30.0)/10.0)+1.0);
    algebraicVariables[6] = 0.07*exp(states[0]/20.0);
    rates[1] = algebraicVariables[6]*(1.0-states[1])-algebraicVariables[7]*states[1];
    algebraicVariables[9] = 0.125*exp(states[0]/80.0);
    algebraicVariables[8] = 0.01*(states[0]+10.0)/(exp((states[0]+10.0)/10.0)-1.0);
    rates[2] = algebraicVariables[8]*(1.0-states[2])-algebraicVariables[9]*states[2];
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    algebraicVariables[1] = constants[2]*(states[0]-computedConstants[0]);
    externalVariables[0] = externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, 0);
    algebraicVariables[3] = constants[3]*pow(externalVariables[0], 3.0)*states[1]*(states[0]-computedConstants[1]);
    algebraicVariables[4] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0);
    algebraicVariables[5] = 4.0*exp(states[0]/18.0);
    algebraicVariables[6] = 0.07*exp(states[0]/20.0);
    algebraicVariables[7] = 1.0/(exp((states[0]+30.0)/10.0)+1.0);
    algebraicVariables[2] = constants[4]*pow(states[2], 4.0)*(states[0]-computedConstants[2]);
    algebraicVariables[8] = 0.01*(states[0]+10.0)/(exp((states[0]+10.0)/10.0)-1.0);
    algebraicVariables[9] = 0.125*exp(states[0]/80.0);
}
