/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 22;
const size_t CONSTANT_COUNT = 2;
const size_t COMPUTED_CONSTANT_COUNT = 2;
const size_t ALGEBRAIC_VARIABLE_COUNT = 22;

const VariableInfo VOI_INFO = {"t", "dimensionless", "main"};

const VariableInfo STATE_INFO[] = {
    {"xStateStateCst", "dimensionless", "main"},
    {"kStateStateCst", "dimensionless", "main"},
    {"xStateStateCompCst", "dimensionless", "main"},
    {"kStateStateCompCst", "dimensionless", "main"},
    {"xStateStateState", "dimensionless", "main"},
    {"kStateStateState", "dimensionless", "main"},
    {"xStateStateNla", "dimensionless", "main"},
    {"kStateStateNla", "dimensionless", "main"},
    {"kNlaStateCst", "dimensionless", "main"},
    {"kNlaStateCompCst", "dimensionless", "main"},
    {"kNlaStateState", "dimensionless", "main"},
    {"kNlaStateNla", "dimensionless", "main"},
    {"xStateNlaCst", "dimensionless", "main"},
    {"xStateNlaCompCst", "dimensionless", "main"},
    {"xStateNlaState", "dimensionless", "main"},
    {"xStateNlaNla", "dimensionless", "main"},
    {"xStateCst", "dimensionless", "main"},
    {"xStateCompCst", "dimensionless", "main"},
    {"xStateState", "dimensionless", "main"},
    {"kStateState", "dimensionless", "main"},
    {"xStateNla", "dimensionless", "main"},
    {"kNlaState", "dimensionless", "main"}
};

const VariableInfo CONSTANT_INFO[] = {
    {"kStateCst", "dimensionless", "main"},
    {"kNlaCst", "dimensionless", "main"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"kStateCompCst", "dimensionless", "main"},
    {"kNlaCompCst", "dimensionless", "main"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"xNlaStateCst", "dimensionless", "main"},
    {"xNlaStateCompCst", "dimensionless", "main"},
    {"xNlaStateState", "dimensionless", "main"},
    {"xNlaStateNla", "dimensionless", "main"},
    {"kStateNlaCst", "dimensionless", "main"},
    {"kStateNlaCompCst", "dimensionless", "main"},
    {"kStateNlaState", "dimensionless", "main"},
    {"kStateNlaNla", "dimensionless", "main"},
    {"xNlaNlaCst", "dimensionless", "main"},
    {"kNlaNlaCst", "dimensionless", "main"},
    {"xNlaNlaCompCst", "dimensionless", "main"},
    {"kNlaNlaCompCst", "dimensionless", "main"},
    {"xNlaNlaState", "dimensionless", "main"},
    {"kNlaNlaState", "dimensionless", "main"},
    {"xNlaNlaNla", "dimensionless", "main"},
    {"kNlaNlaNla", "dimensionless", "main"},
    {"kStateNla", "dimensionless", "main"},
    {"xNlaCst", "dimensionless", "main"},
    {"xNlaCompCst", "dimensionless", "main"},
    {"xNlaState", "dimensionless", "main"},
    {"xNlaNla", "dimensionless", "main"},
    {"kNlaNla", "dimensionless", "main"}
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

    f[0] = voi-algebraicVariables[0];
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

    algebraicVariables[1] = u[0];

    f[0] = voi-algebraicVariables[1];
}

void findRoot1(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[1];

    nlaSolve(objectiveFunction1, u, 1, &rfi);

    algebraicVariables[1] = u[0];
}

void objectiveFunction2(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[2] = u[0];

    f[0] = voi-algebraicVariables[2];
}

void findRoot2(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[2];

    nlaSolve(objectiveFunction2, u, 1, &rfi);

    algebraicVariables[2] = u[0];
}

void objectiveFunction3(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[3] = u[0];

    f[0] = voi-algebraicVariables[3];
}

void findRoot3(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[3];

    nlaSolve(objectiveFunction3, u, 1, &rfi);

    algebraicVariables[3] = u[0];
}

void objectiveFunction4(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[4] = u[0];

    f[0] = voi-algebraicVariables[4];
}

void findRoot4(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[4];

    nlaSolve(objectiveFunction4, u, 1, &rfi);

    algebraicVariables[4] = u[0];
}

void objectiveFunction5(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[5] = u[0];

    f[0] = voi-algebraicVariables[5];
}

void findRoot5(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[5];

    nlaSolve(objectiveFunction5, u, 1, &rfi);

    algebraicVariables[5] = u[0];
}

void objectiveFunction6(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[6] = u[0];

    f[0] = voi-algebraicVariables[6];
}

void findRoot6(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[6];

    nlaSolve(objectiveFunction6, u, 1, &rfi);

    algebraicVariables[6] = u[0];
}

void objectiveFunction7(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[7] = u[0];

    f[0] = voi-algebraicVariables[7];
}

void findRoot7(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[7];

    nlaSolve(objectiveFunction7, u, 1, &rfi);

    algebraicVariables[7] = u[0];
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

    f[0] = voi-algebraicVariables[8];
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

    f[0] = voi-algebraicVariables[9];
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

    algebraicVariables[10] = u[0];

    f[0] = voi-algebraicVariables[10];
}

void findRoot10(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[10];

    nlaSolve(objectiveFunction10, u, 1, &rfi);

    algebraicVariables[10] = u[0];
}

void objectiveFunction11(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[11] = u[0];

    f[0] = voi-algebraicVariables[11];
}

void findRoot11(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[11];

    nlaSolve(objectiveFunction11, u, 1, &rfi);

    algebraicVariables[11] = u[0];
}

void objectiveFunction12(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[12] = u[0];

    f[0] = voi-algebraicVariables[12];
}

void findRoot12(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[12];

    nlaSolve(objectiveFunction12, u, 1, &rfi);

    algebraicVariables[12] = u[0];
}

void objectiveFunction13(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[13] = u[0];

    f[0] = voi-algebraicVariables[13];
}

void findRoot13(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[13];

    nlaSolve(objectiveFunction13, u, 1, &rfi);

    algebraicVariables[13] = u[0];
}

void objectiveFunction14(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[14] = u[0];

    f[0] = voi-algebraicVariables[14];
}

void findRoot14(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[14];

    nlaSolve(objectiveFunction14, u, 1, &rfi);

    algebraicVariables[14] = u[0];
}

void objectiveFunction15(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[15] = u[0];

    f[0] = voi-algebraicVariables[15];
}

void findRoot15(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[15];

    nlaSolve(objectiveFunction15, u, 1, &rfi);

    algebraicVariables[15] = u[0];
}

void objectiveFunction16(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[16] = u[0];

    f[0] = voi-algebraicVariables[16];
}

void findRoot16(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[16];

    nlaSolve(objectiveFunction16, u, 1, &rfi);

    algebraicVariables[16] = u[0];
}

void objectiveFunction17(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[17] = u[0];

    f[0] = voi-algebraicVariables[17];
}

void findRoot17(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[17];

    nlaSolve(objectiveFunction17, u, 1, &rfi);

    algebraicVariables[17] = u[0];
}

void objectiveFunction18(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[18] = u[0];

    f[0] = voi-algebraicVariables[18];
}

void findRoot18(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[18];

    nlaSolve(objectiveFunction18, u, 1, &rfi);

    algebraicVariables[18] = u[0];
}

void objectiveFunction19(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[19] = u[0];

    f[0] = voi-algebraicVariables[19];
}

void findRoot19(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[19];

    nlaSolve(objectiveFunction19, u, 1, &rfi);

    algebraicVariables[19] = u[0];
}

void objectiveFunction20(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[20] = u[0];

    f[0] = voi-algebraicVariables[20];
}

void findRoot20(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[20];

    nlaSolve(objectiveFunction20, u, 1, &rfi);

    algebraicVariables[20] = u[0];
}

void objectiveFunction21(double *u, double *f, void *data)
{
    double voi = ((RootFindingInfo *) data)->voi;
    double *states = ((RootFindingInfo *) data)->states;
    double *rates = ((RootFindingInfo *) data)->rates;
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[21] = u[0];

    f[0] = voi-algebraicVariables[21];
}

void findRoot21(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[21];

    nlaSolve(objectiveFunction21, u, 1, &rfi);

    algebraicVariables[21] = u[0];
}

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    constants[0] = 3.0;
    states[1] = constants[0];
    states[0] = states[1];
    states[19] = 5.0;
    states[5] = states[19];
    states[4] = states[5];
    algebraicVariables[16] = 7.0;
    states[7] = algebraicVariables[16];
    states[6] = states[7];
    states[8] = constants[0];
    states[10] = states[19];
    states[11] = algebraicVariables[16];
    constants[1] = 3.0;
    algebraicVariables[4] = constants[1];
    states[12] = algebraicVariables[4];
    states[21] = 5.0;
    algebraicVariables[6] = states[21];
    states[14] = algebraicVariables[6];
    algebraicVariables[21] = 7.0;
    algebraicVariables[7] = algebraicVariables[21];
    states[15] = algebraicVariables[7];
    states[16] = constants[0];
    states[18] = states[19];
    states[20] = algebraicVariables[16];
    algebraicVariables[0] = states[8];
    algebraicVariables[2] = states[10];
    algebraicVariables[3] = states[11];
    algebraicVariables[9] = constants[1];
    algebraicVariables[8] = algebraicVariables[9];
    algebraicVariables[13] = states[21];
    algebraicVariables[12] = algebraicVariables[13];
    algebraicVariables[15] = algebraicVariables[21];
    algebraicVariables[14] = algebraicVariables[15];
    algebraicVariables[17] = constants[1];
    algebraicVariables[19] = states[21];
    algebraicVariables[20] = algebraicVariables[21];
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = 1.23*constants[0];
    states[3] = computedConstants[0];
    states[2] = states[3];
    states[9] = computedConstants[0];
    computedConstants[1] = 1.23*constants[1];
    algebraicVariables[5] = computedConstants[1];
    states[13] = algebraicVariables[5];
    states[17] = computedConstants[0];
    algebraicVariables[1] = states[9];
    algebraicVariables[11] = computedConstants[1];
    algebraicVariables[10] = algebraicVariables[11];
    algebraicVariables[18] = computedConstants[1];
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    rates[0] = 1.23;
    rates[1] = 1.23;
    rates[2] = 1.23;
    rates[3] = 1.23;
    rates[4] = 1.23;
    rates[5] = 1.23;
    rates[6] = 1.23;
    rates[7] = 1.23;
    rates[8] = 1.23;
    rates[9] = 1.23;
    rates[10] = 1.23;
    rates[11] = 1.23;
    rates[12] = 1.23;
    rates[13] = 1.23;
    rates[14] = 1.23;
    rates[15] = 1.23;
    rates[16] = 1.23;
    rates[17] = 1.23;
    rates[18] = 1.23;
    rates[19] = 1.23;
    rates[20] = 1.23;
    rates[21] = 1.23;
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    findRoot0(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot1(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot2(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot3(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot4(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot5(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot6(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot7(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot8(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot9(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot10(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot11(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot12(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot13(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot14(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot15(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot16(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot17(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot18(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot19(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot20(voi, states, rates, constants, computedConstants, algebraicVariables);
    findRoot21(voi, states, rates, constants, computedConstants, algebraicVariables);
}
