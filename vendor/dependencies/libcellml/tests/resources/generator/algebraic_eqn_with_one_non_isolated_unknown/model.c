/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t CONSTANT_COUNT = 0;
const size_t COMPUTED_CONSTANT_COUNT = 3;
const size_t ALGEBRAIC_VARIABLE_COUNT = 1;

const VariableInfo CONSTANT_INFO[] = {
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"b", "dimensionless", "my_algebraic_eqn"},
    {"c", "dimensionless", "my_algebraic_eqn"},
    {"d", "dimensionless", "my_algebraic_eqn"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"a", "dimensionless", "my_algebraic_eqn"}
};

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
    double *constants;
    double *computedConstants;
    double *algebraicVariables;
} RootFindingInfo;

extern void nlaSolve(void (*objectiveFunction)(double *, double *, void *),
                     double *u, size_t n, void *data);

void objectiveFunction0(double *u, double *f, void *data)
{
    double *constants = ((RootFindingInfo *) data)->constants;
    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;
    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;

    algebraicVariables[0] = u[0];

    f[0] = algebraicVariables[0]+computedConstants[0]-(computedConstants[1]+computedConstants[2]);
}

void findRoot0(double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { constants, computedConstants, algebraicVariables };
    double u[1];

    u[0] = algebraicVariables[0];

    nlaSolve(objectiveFunction0, u, 1, &rfi);

    algebraicVariables[0] = u[0];
}

void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = 3.0;
    computedConstants[1] = 5.0;
    computedConstants[2] = 7.0;
    algebraicVariables[0] = 1.0;
}

void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)
{
}

void computeVariables(double *constants, double *computedConstants, double *algebraicVariables)
{
    findRoot0(constants, computedConstants, algebraicVariables);
}
