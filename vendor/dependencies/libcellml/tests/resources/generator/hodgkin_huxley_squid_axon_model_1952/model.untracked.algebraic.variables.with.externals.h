/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#pragma once

#include <stddef.h>

extern const char VERSION[];
extern const char LIBCELLML_VERSION[];

extern const size_t STATE_COUNT;
extern const size_t CONSTANT_COUNT;
extern const size_t COMPUTED_CONSTANT_COUNT;
extern const size_t ALGEBRAIC_VARIABLE_COUNT;
extern const size_t EXTERNAL_VARIABLE_COUNT;

typedef struct {
    char name[8];
    char units[16];
    char component[25];
} VariableInfo;

extern const VariableInfo VOI_INFO;
extern const VariableInfo STATE_INFO[];
extern const VariableInfo CONSTANT_INFO[];
extern const VariableInfo COMPUTED_CONSTANT_INFO[];
extern const VariableInfo ALGEBRAIC_VARIABLE_INFO[];
extern const VariableInfo EXTERNAL_VARIABLE_INFO[];

double * createStatesArray();
double * createConstantsArray();
double * createComputedConstantsArray();
double * createAlgebraicVariablesArray();
double * createExternalVariablesArray();

void deleteArray(double *array);

typedef double (* ExternalVariable)(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, size_t index);

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);
void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);
void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);
void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);
