/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t CONSTANT_COUNT = 1;
const size_t COMPUTED_CONSTANT_COUNT = 1;
const size_t ALGEBRAIC_VARIABLE_COUNT = 0;

const VariableInfo CONSTANT_INFO[] = {
    {"a", "dimensionless", "my_algebraic_eqn"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"x", "dimensionless", "my_algebraic_eqn"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
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

void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables)
{
    constants[0] = 1.0;
}

void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = constants[0];
}

void computeVariables(double *constants, double *computedConstants, double *algebraicVariables)
{
}
