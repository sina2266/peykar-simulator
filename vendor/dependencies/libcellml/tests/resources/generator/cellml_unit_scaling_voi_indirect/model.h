/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#pragma once

#include <stddef.h>

extern const char VERSION[];
extern const char LIBCELLML_VERSION[];

extern const size_t STATE_COUNT;
extern const size_t CONSTANT_COUNT;
extern const size_t COMPUTED_CONSTANT_COUNT;
extern const size_t ALGEBRAIC_VARIABLE_COUNT;

typedef struct {
    char name[2];
    char units[7];
    char component[12];
} VariableInfo;

extern const VariableInfo VOI_INFO;
extern const VariableInfo STATE_INFO[];
extern const VariableInfo CONSTANT_INFO[];
extern const VariableInfo COMPUTED_CONSTANT_INFO[];
extern const VariableInfo ALGEBRAIC_VARIABLE_INFO[];

double * createStatesArray();
double * createConstantsArray();
double * createComputedConstantsArray();
double * createAlgebraicVariablesArray();

void deleteArray(double *array);

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);
void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);
void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);
void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);
