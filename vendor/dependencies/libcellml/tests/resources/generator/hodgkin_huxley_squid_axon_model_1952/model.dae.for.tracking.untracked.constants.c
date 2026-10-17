/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.dae.for.tracking.untracked.constants.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 4;
const size_t CONSTANT_COUNT = 0;
const size_t COMPUTED_CONSTANT_COUNT = 1;
const size_t ALGEBRAIC_VARIABLE_COUNT = 12;

const VariableInfo VOI_INFO = {"time", "millisecond", "environment"};

const VariableInfo STATE_INFO[] = {
    {"V", "millivolt", "membrane"},
    {"h", "dimensionless", "sodium_channel_h_gate"},
    {"m", "dimensionless", "sodium_channel_m_gate"},
    {"n", "dimensionless", "potassium_channel_n_gate"}
};

const VariableInfo CONSTANT_INFO[] = {
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"E_K", "millivolt", "potassium_channel"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"i_Stim", "microA_per_cm2", "membrane"},
    {"i_L", "microA_per_cm2", "leakage_current"},
    {"i_K", "microA_per_cm2", "potassium_channel"},
    {"i_Na", "microA_per_cm2", "sodium_channel"},
    {"E_L", "millivolt", "leakage_current"},
    {"E_Na", "millivolt", "sodium_channel"},
    {"alpha_m", "per_millisecond", "sodium_channel_m_gate"},
    {"beta_m", "per_millisecond", "sodium_channel_m_gate"},
    {"alpha_h", "per_millisecond", "sodium_channel_h_gate"},
    {"beta_h", "per_millisecond", "sodium_channel_h_gate"},
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

typedef struct {
    double voi;
    double *states;
    double *rates;
    double *constants;
    double *computedConstants;
    double *algebraicVariables;
} RootFindingInfo;

extern void nlaSolve(void (*objectiveFunction)(double *, double *, void *),
                     double *u, size_t n, void *data);

void objectiveFunction0(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[0] = u[0];

    f[0] = algebraicVariables[0]-(((voi >= 10.0) && (voi <= 10.5))?-20.0:0.0)-0.0;
}

void findRoot0(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[0];

    nlaSolve(objectiveFunction0, u, 1, &rfi);

    algebraicVariables[0] = u[0];
}

void objectiveFunction1(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    rates[0] = u[0];

    double membrane_Cm = 1.0;

    f[0] = rates[0]-(-(-algebraicVariables[0]+algebraicVariables[3]+algebraicVariables[2]+algebraicVariables[1])/membrane_Cm)-0.0;
}

void findRoot1(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = rates[0];

    nlaSolve(objectiveFunction1, u, 1, &rfi);

    rates[0] = u[0];
}

void objectiveFunction2(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[4] = u[0];

    double membrane_E_R = 0.0;

    f[0] = algebraicVariables[4]-(membrane_E_R-10.613)-0.0;
}

void findRoot2(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[4];

    nlaSolve(objectiveFunction2, u, 1, &rfi);

    algebraicVariables[4] = u[0];
}

void objectiveFunction3(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[1] = u[0];

    double leakage_current_g_L = 0.3;

    f[0] = algebraicVariables[1]-leakage_current_g_L*(states[0]-algebraicVariables[4])-0.0;
}

void findRoot3(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[1];

    nlaSolve(objectiveFunction3, u, 1, &rfi);

    algebraicVariables[1] = u[0];
}

void objectiveFunction4(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[5] = u[0];

    double membrane_E_R = 0.0;

    f[0] = algebraicVariables[5]-(membrane_E_R-115.0)-0.0;
}

void findRoot4(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[5];

    nlaSolve(objectiveFunction4, u, 1, &rfi);

    algebraicVariables[5] = u[0];
}

void objectiveFunction5(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[3] = u[0];

    double sodium_channel_g_Na = 120.0;

    f[0] = algebraicVariables[3]-sodium_channel_g_Na*pow(states[2], 3.0)*states[1]*(states[0]-algebraicVariables[5])-0.0;
}

void findRoot5(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[3];

    nlaSolve(objectiveFunction5, u, 1, &rfi);

    algebraicVariables[3] = u[0];
}

void objectiveFunction6(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[7] = u[0];

    f[0] = algebraicVariables[7]-4.0*exp(states[0]/18.0)-0.0;
}

void findRoot6(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[7];

    nlaSolve(objectiveFunction6, u, 1, &rfi);

    algebraicVariables[7] = u[0];
}

void objectiveFunction7(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    rates[2] = u[0];

    f[0] = rates[2]-(algebraicVariables[6]*(1.0-states[2])-algebraicVariables[7]*states[2])-0.0;
}

void findRoot7(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = rates[2];

    nlaSolve(objectiveFunction7, u, 1, &rfi);

    rates[2] = u[0];
}

void objectiveFunction8(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[8] = u[0];

    f[0] = algebraicVariables[8]-0.07*exp(states[0]/20.0)-0.0;
}

void findRoot8(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[8];

    nlaSolve(objectiveFunction8, u, 1, &rfi);

    algebraicVariables[8] = u[0];
}

void objectiveFunction9(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[9] = u[0];

    f[0] = algebraicVariables[9]-1.0/(exp((states[0]+30.0)/10.0)+1.0)-0.0;
}

void findRoot9(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[9];

    nlaSolve(objectiveFunction9, u, 1, &rfi);

    algebraicVariables[9] = u[0];
}

void objectiveFunction10(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    rates[1] = u[0];

    f[0] = rates[1]-(algebraicVariables[8]*(1.0-states[1])-algebraicVariables[9]*states[1])-0.0;
}

void findRoot10(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = rates[1];

    nlaSolve(objectiveFunction10, u, 1, &rfi);

    rates[1] = u[0];
}

void objectiveFunction11(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[2] = u[0];

    double potassium_channel_g_K = 36.0;

    f[0] = algebraicVariables[2]-potassium_channel_g_K*pow(states[3], 4.0)*(states[0]-computedConstants[0])-0.0;
}

void findRoot11(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[2];

    nlaSolve(objectiveFunction11, u, 1, &rfi);

    algebraicVariables[2] = u[0];
}

void objectiveFunction12(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[10] = u[0];

    f[0] = algebraicVariables[10]-0.01*(states[0]+10.0)/(exp((states[0]+10.0)/10.0)-1.0)-0.0;
}

void findRoot12(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[10];

    nlaSolve(objectiveFunction12, u, 1, &rfi);

    algebraicVariables[10] = u[0];
}

void objectiveFunction13(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[11] = u[0];

    f[0] = algebraicVariables[11]-0.125*exp(states[0]/80.0)-0.0;
}

void findRoot13(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[11];

    nlaSolve(objectiveFunction13, u, 1, &rfi);

    algebraicVariables[11] = u[0];
}

void objectiveFunction14(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    rates[3] = u[0];

    f[0] = rates[3]-(algebraicVariables[10]*(1.0-states[3])-algebraicVariables[11]*states[3])-0.0;
}

void findRoot14(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = rates[3];

    nlaSolve(objectiveFunction14, u, 1, &rfi);

    rates[3] = u[0];
}

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    states[0] = 0.0;
    states[1] = 0.6;
    states[2] = 0.05;
    states[3] = 0.325;
    rates[0] = 0.0;
    rates[1] = 0.0;
    rates[2] = 0.0;
    rates[3] = 0.0;
    algebraicVariables[0] = 0.0;
    algebraicVariables[1] = 0.0;
    algebraicVariables[2] = 0.0;
    algebraicVariables[3] = 0.0;
    algebraicVariables[4] = 0.0;
    algebraicVariables[5] = 0.0;
    algebraicVariables[7] = 0.0;
    algebraicVariables[8] = 0.0;
    algebraicVariables[9] = 0.0;
    algebraicVariables[10] = 0.0;
    algebraicVariables[11] = 0.0;
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    double membrane_E_R = 0.0;
    computedConstants[0] = membrane_E_R+12.0;
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    findRoot0(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot2(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot3(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot13(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot12(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot14(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot11(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot9(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot8(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot10(voi, states, rates, constants, computedConstants, algebraicVariables);
    algebraicVariables[6] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0);
    findRoot6(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot7(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot4(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot5(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot1(voi, states, rates, constants, computedConstants, algebraicVariables);
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    findRoot3(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot13(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot12(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot14(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot11(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot9(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot8(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot10(voi, states, rates, constants, computedConstants, algebraicVariables);
    algebraicVariables[6] = 0.1*(states[0]+25.0)/(exp((states[0]+25.0)/10.0)-1.0);
    findRoot6(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot7(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot5(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot1(voi, states, rates, constants, computedConstants, algebraicVariables);
}
