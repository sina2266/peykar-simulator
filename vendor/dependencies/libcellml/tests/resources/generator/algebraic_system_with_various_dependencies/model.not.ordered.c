/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.not.ordered.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t CONSTANT_COUNT = 2;
const size_t COMPUTED_CONSTANT_COUNT = 1;
const size_t ALGEBRAIC_VARIABLE_COUNT = 3;

const VariableInfo CONSTANT_INFO[] = {
    {"y", "dimensionless", "my_algebraic_system"},
    {"x", "dimensionless", "my_algebraic_system"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"a", "dimensionless", "my_algebraic_system"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"c", "dimensionless", "my_algebraic_system"},
    {"b", "dimensionless", "my_algebraic_system"},
    {"d", "dimensionless", "my_algebraic_system"}
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
    algebraicVariables[1] = u[1];

    f[0] = 3.0*computedConstants[0]+2.0*algebraicVariables[1]+algebraicVariables[0]-57.0;
    f[1] = computedConstants[0]+3.0*algebraicVariables[1]-algebraicVariables[0]-19.0;
}

void findRoot0(double *constants, double *computedConstants, double *algebraicVariables)
{
    RootFindingInfo rfi = { constants, computedConstants, algebraicVariables };
    double u[2];

    u[0] = algebraicVariables[0];
    u[1] = algebraicVariables[1];

    nlaSolve(objectiveFunction0, u, 2, &rfi);

    algebraicVariables[0] = u[0];
    algebraicVariables[1] = u[1];
}

void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables)
{
    constants[0] = 5.0;
    constants[1] = 3.0;
    algebraicVariables[0] = 1.0;
    algebraicVariables[1] = 1.0;
}

void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = 3.0*constants[1]+constants[0];
}

void computeVariables(double *constants, double *computedConstants, double *algebraicVariables)
{
    findRoot0(constants, computedConstants, algebraicVariables);
    algebraicVariables[2] = algebraicVariables[1]+algebraicVariables[0];
}
