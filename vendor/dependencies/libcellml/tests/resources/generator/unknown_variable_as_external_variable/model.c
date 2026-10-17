/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t CONSTANT_COUNT = 8;
const size_t COMPUTED_CONSTANT_COUNT = 0;
const size_t ALGEBRAIC_VARIABLE_COUNT = 1;
const size_t EXTERNAL_VARIABLE_COUNT = 1;

const VariableInfo CONSTANT_INFO[] = {
    {"P_5", "per_sec3", "SLC_template3_ss"},
    {"q_Ao", "fmol", "SLC_template3_ss"},
    {"q_Ai", "fmol", "SLC_template3_ss"},
    {"P_4", "per_fmol2_sec3", "SLC_template3_ss"},
    {"P_2", "per_fmol_sec3", "SLC_template3_ss"},
    {"P_1", "per_fmol_sec4", "SLC_template3_ss"},
    {"P_0", "per_fmol_sec4", "SLC_template3_ss"},
    {"E", "fmol", "SLC_template3_ss"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"v", "fmol_per_sec", "SLC_template3_ss"}
};

const VariableInfo EXTERNAL_VARIABLE_INFO[] = {
    {"P_3", "per_fmol_sec3", "SLC_template3_ss"}
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
    constants[0] = 810000.0;
    constants[1] = 150.0;
    constants[2] = 2.0;
    constants[3] = 247140.0;
    constants[4] = 2902500.0;
    constants[5] = 3402000.0;
    constants[6] = 21262500.0;
    constants[7] = 1.1;
}

void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)
{
}

void computeVariables(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    externalVariables[0] = externalVariable(constants, computedConstants, algebraicVariables, externalVariables, 0);
    algebraicVariables[0] = constants[7]*(constants[6]*constants[1]-constants[5]*constants[2])/(constants[4]*constants[2]+externalVariables[0]*constants[1]+constants[3]*constants[2]*constants[1]+constants[0]);
}
