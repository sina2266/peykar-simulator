/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.external.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t CONSTANT_COUNT = 0;
const size_t COMPUTED_CONSTANT_COUNT = 0;
const size_t ALGEBRAIC_VARIABLE_COUNT = 1;
const size_t EXTERNAL_VARIABLE_COUNT = 1;

const VariableInfo CONSTANT_INFO[] = {
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"x", "dimensionless", "my_algebraic_eqn"}
};

const VariableInfo EXTERNAL_VARIABLE_INFO[] = {
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

void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables)
{
}

void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)
{
}

void computeVariables(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    externalVariables[0] = externalVariable(constants, computedConstants, algebraicVariables, externalVariables, 0);
    algebraicVariables[0] = externalVariables[0];
}
