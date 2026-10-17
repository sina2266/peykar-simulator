/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 2;
const size_t CONSTANT_COUNT = 5;
const size_t COMPUTED_CONSTANT_COUNT = 0;
const size_t ALGEBRAIC_VARIABLE_COUNT = 5;

const VariableInfo VOI_INFO = {"t", "second", "main"};

const VariableInfo STATE_INFO[] = {
    {"q_1", "coulomb", "main"},
    {"v_3", "C_per_s", "main"}
};

const VariableInfo CONSTANT_INFO[] = {
    {"v_in", "C_per_s", "main"},
    {"v_out", "C_per_s", "main"},
    {"C", "C2_per_J", "main"},
    {"R", "Js_per_C2", "main"},
    {"L", "Js2_per_C2", "main"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"v_1", "C_per_s", "main"},
    {"v_2", "C_per_s", "main"},
    {"u_3", "J_per_C", "main"},
    {"u_2", "J_per_C", "main"},
    {"u_1", "J_per_C", "main"}
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

    f[0] = constants[0]-(algebraicVariables[0]+algebraicVariables[1]);
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

    algebraicVariables[2] = u[0];

    f[0] = algebraicVariables[4]-(algebraicVariables[3]+algebraicVariables[2]);
}

void findRoot1(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[2];

    nlaSolve(objectiveFunction1, u, 1, &rfi);

    algebraicVariables[2] = u[0];
}

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    states[0] = 1.0;
    states[1] = 0.0;
    constants[0] = 1.0;
    constants[1] = 1.0;
    constants[2] = 20.0;
    constants[3] = 2.0;
    constants[4] = 10.0;
    algebraicVariables[0] = 0.0;
    algebraicVariables[2] = 0.0;
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    algebraicVariables[1] = states[1]+constants[1];
    findRoot0(voi, states, rates, constants, computedConstants, algebraicVariables);
    rates[0] = algebraicVariables[0];
    algebraicVariables[3] = constants[3]*algebraicVariables[1];
    algebraicVariables[4] = states[0]/constants[2];
    findRoot1(voi, states, rates, constants, computedConstants, algebraicVariables);
    rates[1] = algebraicVariables[2]/constants[4];
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    algebraicVariables[1] = states[1]+constants[1];
    findRoot0(voi, states, rates, constants, computedConstants, algebraicVariables);
    algebraicVariables[3] = constants[3]*algebraicVariables[1];
    algebraicVariables[4] = states[0]/constants[2];
    findRoot1(voi, states, rates, constants, computedConstants, algebraicVariables);
}
