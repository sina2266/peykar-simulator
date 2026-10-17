/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 4;
const size_t CONSTANT_COUNT = 5;
const size_t COMPUTED_CONSTANT_COUNT = 0;
const size_t ALGEBRAIC_VARIABLE_COUNT = 12;

const VariableInfo VOI_INFO = {"time", "millisecond", "environment"};

const VariableInfo STATE_INFO[] = {
    {"V", "millivolt", "membrane"},
    {"h", "dimensionless", "sodium_channel_h_gate"},
    {"m", "dimensionless", "sodium_channel_m_gate"},
    {"n", "dimensionless", "potassium_channel_n_gate"}
};

const VariableInfo CONSTANT_INFO[] = {
    {"Cm", "microF_per_cm2", "membrane"},
    {"E_L", "millivolt", "leakage_current"},
    {"g_L", "milliS_per_cm2", "leakage_current"},
    {"g_Na_max", "milliS_per_cm2", "sodium_channel"},
    {"E_Na", "millivolt", "sodium_channel"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"i_Leak", "microA_per_cm2", "leakage_current"},
    {"i_K", "microA_per_cm2", "potassium_channel"},
    {"i_Na", "microA_per_cm2", "sodium_channel"},
    {"g_Na", "milliS_per_cm2", "sodium_channel"},
    {"alpha_m", "per_millisecond", "sodium_channel_m_gate"},
    {"beta_m", "per_millisecond", "sodium_channel_m_gate"},
    {"alpha_h", "per_millisecond", "sodium_channel_h_gate"},
    {"beta_h", "per_millisecond", "sodium_channel_h_gate"},
    {"g_K2", "milliS_per_cm2", "potassium_channel"},
    {"g_K1", "milliS_per_cm2", "potassium_channel"},
    {"alpha_n", "per_millisecond", "potassium_channel_n_gate"},
    {"beta_n", "per_millisecond", "potassium_channel_n_gate"}
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

void deleteArray(double *array)
{
    free(array);
}

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    states[0] = -87.0;
    states[1] = 0.8;
    states[2] = 0.01;
    states[3] = 0.01;
    constants[0] = 12.0;
    constants[1] = -60.0;
    constants[2] = 0.075;
    constants[3] = 400.0;
    constants[4] = 40.0;
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    algebraicVariables[0] = constants[2]*(states[0]-constants[1]);
    algebraicVariables[3] = pow(states[2], 3.0)*states[1]*constants[3];
    algebraicVariables[2] = (algebraicVariables[3]+0.14)*(states[0]-constants[4]);
    algebraicVariables[8] = 1.2*pow(states[3], 4.0);
    algebraicVariables[9] = 1.2*exp((-states[0]-90.0)/50.0)+0.015*exp((states[0]+90.0)/60.0);
    algebraicVariables[1] = (algebraicVariables[9]+algebraicVariables[8])*(states[0]+100.0);
    rates[0] = -(algebraicVariables[2]+algebraicVariables[1]+algebraicVariables[0])/constants[0];
    algebraicVariables[5] = 0.12*(states[0]+8.0)/(exp((states[0]+8.0)/5.0)-1.0);
    algebraicVariables[4] = 0.1*(-states[0]-48.0)/(exp((-states[0]-48.0)/15.0)-1.0);
    rates[2] = algebraicVariables[4]*(1.0-states[2])-algebraicVariables[5]*states[2];
    algebraicVariables[7] = 1.0/(1.0+exp((-states[0]-42.0)/10.0));
    algebraicVariables[6] = 0.17*exp((-states[0]-90.0)/20.0);
    rates[1] = algebraicVariables[6]*(1.0-states[1])-algebraicVariables[7]*states[1];
    algebraicVariables[11] = 0.002*exp((-states[0]-90.0)/80.0);
    algebraicVariables[10] = 0.0001*(-states[0]-50.0)/(exp((-states[0]-50.0)/10.0)-1.0);
    rates[3] = algebraicVariables[10]*(1.0-states[3])-algebraicVariables[11]*states[3];
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    algebraicVariables[0] = constants[2]*(states[0]-constants[1]);
    algebraicVariables[3] = pow(states[2], 3.0)*states[1]*constants[3];
    algebraicVariables[2] = (algebraicVariables[3]+0.14)*(states[0]-constants[4]);
    algebraicVariables[4] = 0.1*(-states[0]-48.0)/(exp((-states[0]-48.0)/15.0)-1.0);
    algebraicVariables[5] = 0.12*(states[0]+8.0)/(exp((states[0]+8.0)/5.0)-1.0);
    algebraicVariables[6] = 0.17*exp((-states[0]-90.0)/20.0);
    algebraicVariables[7] = 1.0/(1.0+exp((-states[0]-42.0)/10.0));
    algebraicVariables[8] = 1.2*pow(states[3], 4.0);
    algebraicVariables[9] = 1.2*exp((-states[0]-90.0)/50.0)+0.015*exp((states[0]+90.0)/60.0);
    algebraicVariables[1] = (algebraicVariables[9]+algebraicVariables[8])*(states[0]+100.0);
    algebraicVariables[10] = 0.0001*(-states[0]-50.0)/(exp((-states[0]-50.0)/10.0)-1.0);
    algebraicVariables[11] = 0.002*exp((-states[0]-90.0)/80.0);
}
