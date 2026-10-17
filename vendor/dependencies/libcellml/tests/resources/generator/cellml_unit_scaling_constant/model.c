/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t CONSTANT_COUNT = 9;
const size_t COMPUTED_CONSTANT_COUNT = 10;
const size_t ALGEBRAIC_VARIABLE_COUNT = 0;

const VariableInfo CONSTANT_INFO[] = {
    {"ind_c_mM", "mM", "constants"},
    {"c_mM", "mM", "constants"},
    {"c", "mM", "constants"},
    {"c_M", "M", "constants"},
    {"ind_c_M", "M", "constants"},
    {"ind_oc_mM", "mM", "other_constants"},
    {"oc_mM", "mM", "other_constants"},
    {"oc_M", "M", "other_constants"},
    {"ind_oc_M", "M", "other_constants"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"comp_c_mM_mM", "mM", "main"},
    {"comp_c_mM_M", "M", "main"},
    {"comp_c_M_mM", "mM", "main"},
    {"comp_c_M_M", "M", "main"},
    {"comp_k_mM", "mM", "main"},
    {"comp_k_M", "M", "main"},
    {"comp_oc_mM_mM", "mM", "main"},
    {"comp_oc_mM_M", "M", "main"},
    {"comp_oc_M_mM", "mM", "main"},
    {"comp_oc_M_M", "M", "main"}
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
    constants[2] = 123.0;
    constants[1] = constants[2];
    constants[0] = constants[1];
    constants[3] = constants[2];
    constants[4] = constants[3];
    constants[6] = 0.001*constants[2];
    constants[5] = constants[6];
    constants[7] = constants[2];
    constants[8] = constants[7];
}

void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = constants[1]+constants[0];
    computedConstants[1] = 0.001*constants[1]+0.001*constants[0];
    computedConstants[2] = 1000.0*constants[3]+1000.0*constants[4];
    computedConstants[3] = constants[3]+constants[4];
    computedConstants[4] = constants[2]+constants[2];
    computedConstants[5] = 0.001*constants[2]+0.001*constants[2];
    computedConstants[6] = constants[6]+constants[5];
    computedConstants[7] = 0.001*constants[6]+0.001*constants[5];
    computedConstants[8] = 1000.0*constants[7]+1000.0*constants[8];
    computedConstants[9] = constants[7]+constants[8];
}

void computeVariables(double *constants, double *computedConstants, double *algebraicVariables)
{
}
