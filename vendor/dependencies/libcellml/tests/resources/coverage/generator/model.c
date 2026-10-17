/* The content of this file was generated using the C profile of libCellML 0.7.1. */

#include "model.h"

#include <math.h>
#include <stdlib.h>

const char VERSION[] = "0.8.0";
const char LIBCELLML_VERSION[] = "0.7.1";

const size_t STATE_COUNT = 1;
const size_t CONSTANT_COUNT = 7;
const size_t COMPUTED_CONSTANT_COUNT = 199;
const size_t ALGEBRAIC_VARIABLE_COUNT = 2;
const size_t EXTERNAL_VARIABLE_COUNT = 1;

const VariableInfo VOI_INFO = {"t", "second", "my_component"};

const VariableInfo STATE_INFO[] = {
    {"x", "dimensionless", "my_component"}
};

const VariableInfo CONSTANT_INFO[] = {
    {"n", "dimensionless", "my_component"},
    {"m", "dimensionless", "my_component"},
    {"o", "dimensionless", "my_component"},
    {"p", "dimensionless", "my_component"},
    {"q", "dimensionless", "my_component"},
    {"r", "dimensionless", "my_component"},
    {"s", "dimensionless", "my_component"}
};

const VariableInfo COMPUTED_CONSTANT_INFO[] = {
    {"eqnEq", "dimensionless", "my_component"},
    {"eqnEqCoverageParentheses", "dimensionless", "my_component"},
    {"eqnNeq", "dimensionless", "my_component"},
    {"eqnNeqCoverageParentheses", "dimensionless", "my_component"},
    {"eqnLt", "dimensionless", "my_component"},
    {"eqnLtCoverageParentheses", "dimensionless", "my_component"},
    {"eqnLeq", "dimensionless", "my_component"},
    {"eqnLeqCoverageParentheses", "dimensionless", "my_component"},
    {"eqnGt", "dimensionless", "my_component"},
    {"eqnGtCoverageParentheses", "dimensionless", "my_component"},
    {"eqnGeq", "dimensionless", "my_component"},
    {"eqnGeqCoverageParentheses", "dimensionless", "my_component"},
    {"eqnAnd", "dimensionless", "my_component"},
    {"eqnAndMultiple", "dimensionless", "my_component"},
    {"eqnAndParentheses", "dimensionless", "my_component"},
    {"eqnAndParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnAndParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnAndParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnAndParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnAndParenthesesLeftPower", "dimensionless", "my_component"},
    {"eqnAndParenthesesLeftRoot", "dimensionless", "my_component"},
    {"eqnAndParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnAndParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnAndParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnAndParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnAndParenthesesRightPower", "dimensionless", "my_component"},
    {"eqnAndParenthesesRightRoot", "dimensionless", "my_component"},
    {"eqnAndCoverageParentheses", "dimensionless", "my_component"},
    {"eqnOr", "dimensionless", "my_component"},
    {"eqnOrMultiple", "dimensionless", "my_component"},
    {"eqnOrParentheses", "dimensionless", "my_component"},
    {"eqnOrParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnOrParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnOrParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnOrParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnOrParenthesesLeftPower", "dimensionless", "my_component"},
    {"eqnOrParenthesesLeftRoot", "dimensionless", "my_component"},
    {"eqnOrParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnOrParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnOrParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnOrParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnOrParenthesesRightPower", "dimensionless", "my_component"},
    {"eqnOrParenthesesRightRoot", "dimensionless", "my_component"},
    {"eqnOrCoverageParentheses", "dimensionless", "my_component"},
    {"eqnXor", "dimensionless", "my_component"},
    {"eqnXorMultiple", "dimensionless", "my_component"},
    {"eqnXorParentheses", "dimensionless", "my_component"},
    {"eqnXorParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnXorParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnXorParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnXorParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnXorParenthesesLeftPower", "dimensionless", "my_component"},
    {"eqnXorParenthesesLeftRoot", "dimensionless", "my_component"},
    {"eqnXorParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnXorParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnXorParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnXorParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnXorParenthesesRightPower", "dimensionless", "my_component"},
    {"eqnXorParenthesesRightRoot", "dimensionless", "my_component"},
    {"eqnXorCoverageParentheses", "dimensionless", "my_component"},
    {"eqnNot", "dimensionless", "my_component"},
    {"eqnPlusMultiple", "dimensionless", "my_component"},
    {"eqnPlusParentheses", "dimensionless", "my_component"},
    {"eqnPlusUnary", "dimensionless", "my_component"},
    {"eqnMinus", "dimensionless", "my_component"},
    {"eqnMinusParentheses", "dimensionless", "my_component"},
    {"eqnMinusParenthesesPlusWith", "dimensionless", "my_component"},
    {"eqnMinusParenthesesPlusWithout", "dimensionless", "my_component"},
    {"eqnMinusParenthesesDirectUnaryMinus", "dimensionless", "my_component"},
    {"eqnMinusParenthesesIndirectUnaryMinus", "dimensionless", "my_component"},
    {"eqnMinusUnary", "dimensionless", "my_component"},
    {"eqnMinusUnaryParentheses", "dimensionless", "my_component"},
    {"eqnTimes", "dimensionless", "my_component"},
    {"eqnTimesMultiple", "dimensionless", "my_component"},
    {"eqnTimesParentheses", "dimensionless", "my_component"},
    {"eqnTimesParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnTimesParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnTimesParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnTimesParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnTimesParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnTimesParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnTimesParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnTimesParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnDivide", "dimensionless", "my_component"},
    {"eqnDivideParentheses", "dimensionless", "my_component"},
    {"eqnDivideParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnDivideParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnDivideParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnDivideParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnDivideParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnDivideParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnDivideParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnDivideParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnDivideParenthesesRightTimes", "dimensionless", "my_component"},
    {"eqnDivideParenthesesRightDivide", "dimensionless", "my_component"},
    {"eqnPowerSqrt", "dimensionless", "my_component"},
    {"eqnPowerSqr", "dimensionless", "my_component"},
    {"eqnPowerCube", "dimensionless", "my_component"},
    {"eqnPowerCi", "dimensionless", "my_component"},
    {"eqnPowerParentheses", "dimensionless", "my_component"},
    {"eqnPowerParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnPowerParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnPowerParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnPowerParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnPowerParenthesesLeftTimes", "dimensionless", "my_component"},
    {"eqnPowerParenthesesLeftDivide", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightTimes", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightDivide", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightPower", "dimensionless", "my_component"},
    {"eqnPowerParenthesesRightRoot", "dimensionless", "my_component"},
    {"eqnRootSqrt", "dimensionless", "my_component"},
    {"eqnRootSqrtOther", "dimensionless", "my_component"},
    {"eqnRootCube", "dimensionless", "my_component"},
    {"eqnRootCi", "dimensionless", "my_component"},
    {"eqnRootParentheses", "dimensionless", "my_component"},
    {"eqnRootParenthesesLeftPlusWith", "dimensionless", "my_component"},
    {"eqnRootParenthesesLeftPlusWithout", "dimensionless", "my_component"},
    {"eqnRootParenthesesLeftMinusWith", "dimensionless", "my_component"},
    {"eqnRootParenthesesLeftMinusWithout", "dimensionless", "my_component"},
    {"eqnRootParenthesesLeftTimes", "dimensionless", "my_component"},
    {"eqnRootParenthesesLeftDivide", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightPlusWith", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightPlusWithout", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightMinusWith", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightMinusWithout", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightTimes", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightDivide", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightPower", "dimensionless", "my_component"},
    {"eqnRootParenthesesRightRoot", "dimensionless", "my_component"},
    {"eqnAbs", "dimensionless", "my_component"},
    {"eqnExp", "dimensionless", "my_component"},
    {"eqnLn", "dimensionless", "my_component"},
    {"eqnLog", "dimensionless", "my_component"},
    {"eqnLog2", "dimensionless", "my_component"},
    {"eqnLog10", "dimensionless", "my_component"},
    {"eqnLogCi", "dimensionless", "my_component"},
    {"eqnCeiling", "dimensionless", "my_component"},
    {"eqnFloor", "dimensionless", "my_component"},
    {"eqnMin", "dimensionless", "my_component"},
    {"eqnMinMultiple", "dimensionless", "my_component"},
    {"eqnMax", "dimensionless", "my_component"},
    {"eqnMaxMultiple", "dimensionless", "my_component"},
    {"eqnRem", "dimensionless", "my_component"},
    {"eqnSin", "dimensionless", "my_component"},
    {"eqnCos", "dimensionless", "my_component"},
    {"eqnTan", "dimensionless", "my_component"},
    {"eqnSec", "dimensionless", "my_component"},
    {"eqnCsc", "dimensionless", "my_component"},
    {"eqnCot", "dimensionless", "my_component"},
    {"eqnSinh", "dimensionless", "my_component"},
    {"eqnCosh", "dimensionless", "my_component"},
    {"eqnTanh", "dimensionless", "my_component"},
    {"eqnSech", "dimensionless", "my_component"},
    {"eqnCsch", "dimensionless", "my_component"},
    {"eqnCoth", "dimensionless", "my_component"},
    {"eqnArcsin", "dimensionless", "my_component"},
    {"eqnArccos", "dimensionless", "my_component"},
    {"eqnArctan", "dimensionless", "my_component"},
    {"eqnArcsec", "dimensionless", "my_component"},
    {"eqnArccsc", "dimensionless", "my_component"},
    {"eqnArccot", "dimensionless", "my_component"},
    {"eqnArcsinh", "dimensionless", "my_component"},
    {"eqnArccosh", "dimensionless", "my_component"},
    {"eqnArctanh", "dimensionless", "my_component"},
    {"eqnArcsech", "dimensionless", "my_component"},
    {"eqnArccsch", "dimensionless", "my_component"},
    {"eqnArccoth", "dimensionless", "my_component"},
    {"eqnPiecewisePiece", "dimensionless", "my_component"},
    {"eqnPiecewisePieceOtherwise", "dimensionless", "my_component"},
    {"eqnPiecewisePiecePiecePiece", "dimensionless", "my_component"},
    {"eqnPiecewisePiecePiecePieceOtherwise", "dimensionless", "my_component"},
    {"eqnWithPiecewise", "dimensionless", "my_component"},
    {"eqnCnInteger", "dimensionless", "my_component"},
    {"eqnCnDouble", "dimensionless", "my_component"},
    {"eqnCnIntegerWithExponent", "dimensionless", "my_component"},
    {"eqnCnDoubleWithExponent", "dimensionless", "my_component"},
    {"eqnCi", "dimensionless", "my_component"},
    {"eqnTrue", "dimensionless", "my_component"},
    {"eqnFalse", "dimensionless", "my_component"},
    {"eqnExponentiale", "dimensionless", "my_component"},
    {"eqnPi", "dimensionless", "my_component"},
    {"eqnInfinity", "dimensionless", "my_component"},
    {"eqnNotanumber", "dimensionless", "my_component"},
    {"eqnCoverageForPlusOperator", "dimensionless", "my_component"},
    {"eqnCoverageForMinusOperator", "dimensionless", "my_component"},
    {"eqnCoverageForTimesOperator", "dimensionless", "my_component"},
    {"eqnCoverageForDivideOperator", "dimensionless", "my_component"},
    {"eqnCoverageForAndOperator", "dimensionless", "my_component"},
    {"eqnCoverageForOrOperator", "dimensionless", "my_component"},
    {"eqnCoverageForXorOperator", "dimensionless", "my_component"},
    {"eqnCoverageForPowerOperator", "dimensionless", "my_component"},
    {"eqnCoverageForRootOperator", "dimensionless", "my_component"},
    {"eqnCoverageForMinusUnary", "dimensionless", "my_component"},
    {"eqnComputedConstant2", "dimensionless", "my_component"},
    {"eqnComputedConstant1", "dimensionless", "my_component"}
};

const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {
    {"eqnNlaVariable2", "dimensionless", "my_component"},
    {"eqnNlaVariable1", "dimensionless", "my_component"}
};

const VariableInfo EXTERNAL_VARIABLE_INFO[] = {
    {"eqnPlus", "dimensionless", "my_component"}
};

double xor(double x, double y)
{
    return (x != 0.0) ^ (y != 0.0);
}

double min(double x, double y)
{
    return (x < y)?x:y;
}

double max(double x, double y)
{
    return (x > y)?x:y;
}

double sec(double x)
{
    return 1.0/cos(x);
}

double csc(double x)
{
    return 1.0/sin(x);
}

double cot(double x)
{
    return 1.0/tan(x);
}

double sech(double x)
{
    return 1.0/cosh(x);
}

double csch(double x)
{
    return 1.0/sinh(x);
}

double coth(double x)
{
    return 1.0/tanh(x);
}

double asec(double x)
{
    return acos(1.0/x);
}

double acsc(double x)
{
    return asin(1.0/x);
}

double acot(double x)
{
    return atan(1.0/x);
}

double asech(double x)
{
    double oneOverX = 1.0/x;

    return log(oneOverX+sqrt(oneOverX*oneOverX-1.0));
}

double acsch(double x)
{
    double oneOverX = 1.0/x;

    return log(oneOverX+sqrt(oneOverX*oneOverX+1.0));
}

double acoth(double x)
{
    double oneOverX = 1.0/x;

    return 0.5*log((1.0+oneOverX)/(1.0-oneOverX));
}

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

typedef struct {
    double voi;
    double *states;
    double *rates;
    double *constants;
    double *computedConstants;
    double *algebraicVariables;
    double *externalVariables;
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
    double *externalVariables = ((RootFindingInfo *) data)->externalVariables;

    algebraicVariables[0] = u[0];
    algebraicVariables[1] = u[1];

    f[0] = algebraicVariables[1]+algebraicVariables[0]+states[0]-0.0;
    f[1] = algebraicVariables[1]-algebraicVariables[0]-(computedConstants[198]+computedConstants[197]);
}

void findRoot0(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables)
{
    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables };
    double u[2];

    u[0] = algebraicVariables[0];
    u[1] = algebraicVariables[1];

    nlaSolve(objectiveFunction0, u, 2, &rfi);

    algebraicVariables[0] = u[0];
    algebraicVariables[1] = u[1];
}

void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    states[0] = 0.0;
    constants[0] = 1.23;
    constants[1] = 123.0;
    constants[2] = 1.0e1;
    constants[3] = 1.23e1;
    constants[4] = 1.0E1;
    constants[5] = 1.23E1;
    constants[6] = 7.0;
    computedConstants[176] = 123.0;
    computedConstants[177] = 123.456789;
    computedConstants[178] = 123.0e99;
    computedConstants[179] = 123.456789e99;
    computedConstants[181] = 1.0;
    computedConstants[182] = 0.0;
    computedConstants[183] = 2.71828182845905;
    computedConstants[184] = 3.14159265358979;
    computedConstants[185] = INFINITY;
    computedConstants[186] = NAN;
    computedConstants[198] = 1.0;
    computedConstants[197] = 3.0;
    algebraicVariables[0] = 2.0;
    algebraicVariables[1] = 1.0;
}

void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)
{
    computedConstants[0] = constants[1] == constants[0];
    computedConstants[1] = constants[1]/(constants[0] == constants[0]);
    computedConstants[2] = constants[1] != constants[0];
    computedConstants[3] = constants[1]/(constants[0] != constants[2]);
    computedConstants[4] = constants[1] < constants[0];
    computedConstants[5] = constants[1]/(constants[0] < constants[2]);
    computedConstants[6] = constants[1] <= constants[0];
    computedConstants[7] = constants[1]/(constants[0] <= constants[2]);
    computedConstants[8] = constants[1] > constants[0];
    computedConstants[9] = constants[1]/(constants[0] > constants[2]);
    computedConstants[10] = constants[1] >= constants[0];
    computedConstants[11] = constants[1]/(constants[0] >= constants[2]);
    computedConstants[12] = constants[1] && constants[0];
    computedConstants[13] = constants[1] && constants[0] && constants[2];
    computedConstants[14] = (constants[1] < constants[0]) && (constants[2] > constants[3]);
    computedConstants[15] = (constants[1]+constants[0]) && (constants[2] > constants[3]);
    computedConstants[16] = constants[1] && (constants[0] > constants[2]);
    computedConstants[17] = (constants[1]-constants[0]) && (constants[2] > constants[3]);
    computedConstants[18] = -constants[1] && (constants[0] > constants[2]);
    computedConstants[19] = pow(constants[1], constants[0]) && (constants[2] > constants[3]);
    computedConstants[20] = pow(constants[1], 1.0/constants[0]) && (constants[2] > constants[3]);
    computedConstants[21] = (constants[1] < constants[0]) && (constants[2]+constants[3]);
    computedConstants[22] = (constants[1] < constants[0]) && constants[2];
    computedConstants[23] = (constants[1] < constants[0]) && (constants[2]-constants[3]);
    computedConstants[24] = (constants[1] < constants[0]) && -constants[2];
    computedConstants[25] = (constants[1] < constants[0]) && pow(constants[2], constants[3]);
    computedConstants[26] = (constants[1] < constants[0]) && pow(constants[2], 1.0/constants[3]);
    computedConstants[27] = constants[1]/(constants[0] && constants[2]);
    computedConstants[28] = constants[1] || constants[0];
    computedConstants[29] = constants[1] || constants[0] || constants[2];
    computedConstants[30] = (constants[1] < constants[0]) || (constants[2] > constants[3]);
    computedConstants[31] = (constants[1]+constants[0]) || (constants[2] > constants[3]);
    computedConstants[32] = constants[1] || (constants[0] > constants[2]);
    computedConstants[33] = (constants[1]-constants[0]) || (constants[2] > constants[3]);
    computedConstants[34] = -constants[1] || (constants[0] > constants[2]);
    computedConstants[35] = pow(constants[1], constants[0]) || (constants[2] > constants[3]);
    computedConstants[36] = pow(constants[1], 1.0/constants[0]) || (constants[2] > constants[3]);
    computedConstants[37] = (constants[1] < constants[0]) || (constants[2]+constants[3]);
    computedConstants[38] = (constants[1] < constants[0]) || constants[2];
    computedConstants[39] = (constants[1] < constants[0]) || (constants[2]-constants[3]);
    computedConstants[40] = (constants[1] < constants[0]) || -constants[2];
    computedConstants[41] = (constants[1] < constants[0]) || pow(constants[2], constants[3]);
    computedConstants[42] = (constants[1] < constants[0]) || pow(constants[2], 1.0/constants[3]);
    computedConstants[43] = constants[1]/(constants[0] || constants[2]);
    computedConstants[44] = xor(constants[1], constants[0]);
    computedConstants[45] = xor(constants[1], xor(constants[0], constants[2]));
    computedConstants[46] = xor(constants[1] < constants[0], constants[2] > constants[3]);
    computedConstants[47] = xor(constants[1]+constants[0], constants[2] > constants[3]);
    computedConstants[48] = xor(constants[1], constants[0] > constants[2]);
    computedConstants[49] = xor(constants[1]-constants[0], constants[2] > constants[3]);
    computedConstants[50] = xor(-constants[1], constants[0] > constants[2]);
    computedConstants[51] = xor(pow(constants[1], constants[0]), constants[2] > constants[3]);
    computedConstants[52] = xor(pow(constants[1], 1.0/constants[0]), constants[2] > constants[3]);
    computedConstants[53] = xor(constants[1] < constants[0], constants[2]+constants[3]);
    computedConstants[54] = xor(constants[1] < constants[0], constants[2]);
    computedConstants[55] = xor(constants[1] < constants[0], constants[2]-constants[3]);
    computedConstants[56] = xor(constants[1] < constants[0], -constants[2]);
    computedConstants[57] = xor(constants[1] < constants[0], pow(constants[2], constants[3]));
    computedConstants[58] = xor(constants[1] < constants[0], pow(constants[2], 1.0/constants[3]));
    computedConstants[59] = constants[1]/xor(constants[0], constants[2]);
    computedConstants[60] = !constants[1];
    computedConstants[61] = constants[1]+constants[0]+constants[2];
    computedConstants[62] = (constants[1] < constants[0])+(constants[2] > constants[3]);
    computedConstants[63] = constants[1];
    computedConstants[64] = constants[1]-constants[0];
    computedConstants[65] = (constants[1] < constants[0])-(constants[2] > constants[3]);
    computedConstants[66] = (constants[1] < constants[0])-(constants[2]+constants[3]);
    computedConstants[67] = (constants[1] < constants[0])-constants[2];
    computedConstants[68] = constants[1]-(-constants[0]);
    computedConstants[69] = constants[1]-(-constants[0]*constants[2]);
    computedConstants[70] = -constants[1];
    computedConstants[71] = -(constants[1] < constants[0]);
    computedConstants[72] = constants[1]*constants[0];
    computedConstants[73] = constants[1]*constants[0]*constants[2];
    computedConstants[74] = (constants[1] < constants[0])*(constants[2] > constants[3]);
    computedConstants[75] = (constants[1]+constants[0])*(constants[2] > constants[3]);
    computedConstants[76] = constants[1]*(constants[0] > constants[2]);
    computedConstants[77] = (constants[1]-constants[0])*(constants[2] > constants[3]);
    computedConstants[78] = -constants[1]*(constants[0] > constants[2]);
    computedConstants[79] = (constants[1] < constants[0])*(constants[2]+constants[3]);
    computedConstants[80] = (constants[1] < constants[0])*constants[2];
    computedConstants[81] = (constants[1] < constants[0])*(constants[2]-constants[3]);
    computedConstants[82] = (constants[1] < constants[0])*-constants[2];
    computedConstants[83] = constants[1]/constants[0];
    computedConstants[84] = (constants[1] < constants[0])/(constants[3] > constants[2]);
    computedConstants[85] = (constants[1]+constants[0])/(constants[3] > constants[2]);
    computedConstants[86] = constants[1]/(constants[2] > constants[0]);
    computedConstants[87] = (constants[1]-constants[0])/(constants[3] > constants[2]);
    computedConstants[88] = -constants[1]/(constants[2] > constants[0]);
    computedConstants[89] = (constants[1] < constants[0])/(constants[2]+constants[3]);
    computedConstants[90] = (constants[1] < constants[0])/constants[2];
    computedConstants[91] = (constants[1] < constants[0])/(constants[2]-constants[3]);
    computedConstants[92] = (constants[1] < constants[0])/-constants[2];
    computedConstants[93] = (constants[1] < constants[0])/(constants[2]*constants[3]);
    computedConstants[94] = (constants[1] < constants[0])/(constants[2]/constants[3]);
    computedConstants[95] = sqrt(constants[1]);
    computedConstants[96] = pow(constants[1], 2.0);
    computedConstants[97] = pow(constants[1], 3.0);
    computedConstants[98] = pow(constants[1], constants[0]);
    computedConstants[99] = pow(constants[1] <= constants[0], constants[2] >= constants[3]);
    computedConstants[100] = pow(constants[1]+constants[0], constants[2] >= constants[3]);
    computedConstants[101] = pow(constants[1], constants[0] >= constants[2]);
    computedConstants[102] = pow(constants[1]-constants[0], constants[2] >= constants[3]);
    computedConstants[103] = pow(-constants[1], constants[0] >= constants[2]);
    computedConstants[104] = pow(constants[1]*constants[0], constants[2] >= constants[3]);
    computedConstants[105] = pow(constants[1]/constants[0], constants[2] >= constants[3]);
    computedConstants[106] = pow(constants[1] <= constants[0], constants[2]+constants[3]);
    computedConstants[107] = pow(constants[1] <= constants[0], constants[2]);
    computedConstants[108] = pow(constants[1] <= constants[0], constants[2]-constants[3]);
    computedConstants[109] = pow(constants[1] <= constants[0], -constants[2]);
    computedConstants[110] = pow(constants[1] <= constants[0], constants[2]*constants[3]);
    computedConstants[111] = pow(constants[1] <= constants[0], constants[2]/constants[3]);
    computedConstants[112] = pow(constants[1] <= constants[0], pow(constants[2], constants[3]));
    computedConstants[113] = pow(constants[1] <= constants[0], pow(constants[2], 1.0/constants[3]));
    computedConstants[114] = sqrt(constants[1]);
    computedConstants[115] = sqrt(constants[1]);
    computedConstants[116] = pow(constants[1], 1.0/3.0);
    computedConstants[117] = pow(constants[1], 1.0/constants[0]);
    computedConstants[118] = pow(constants[1] < constants[0], 1.0/(constants[3] > constants[2]));
    computedConstants[119] = pow(constants[1]+constants[0], 1.0/(constants[3] > constants[2]));
    computedConstants[120] = pow(constants[1], 1.0/(constants[2] > constants[0]));
    computedConstants[121] = pow(constants[1]-constants[0], 1.0/(constants[3] > constants[2]));
    computedConstants[122] = pow(-constants[1], 1.0/(constants[2] > constants[0]));
    computedConstants[123] = pow(constants[1]*constants[0], 1.0/(constants[3] > constants[2]));
    computedConstants[124] = pow(constants[1]/constants[0], 1.0/(constants[3] > constants[2]));
    computedConstants[125] = pow(constants[1] < constants[0], 1.0/(constants[2]+constants[3]));
    computedConstants[126] = pow(constants[1] < constants[0], 1.0/constants[2]);
    computedConstants[127] = pow(constants[1] < constants[0], 1.0/(constants[2]-constants[3]));
    computedConstants[128] = pow(constants[1] < constants[0], 1.0/-constants[2]);
    computedConstants[129] = pow(constants[1] < constants[0], 1.0/(constants[2]*constants[3]));
    computedConstants[130] = pow(constants[1] < constants[0], 1.0/(constants[2]/constants[3]));
    computedConstants[131] = pow(constants[1] < constants[0], 1.0/pow(constants[2], constants[3]));
    computedConstants[132] = pow(constants[1] < constants[0], 1.0/pow(constants[2], 1.0/constants[3]));
    computedConstants[133] = fabs(constants[1]);
    computedConstants[134] = exp(constants[1]);
    computedConstants[135] = log(constants[1]);
    computedConstants[136] = log10(constants[1]);
    computedConstants[137] = log(constants[1])/log(2.0);
    computedConstants[138] = log10(constants[1]);
    computedConstants[139] = log(constants[1])/log(constants[0]);
    computedConstants[140] = ceil(constants[1]);
    computedConstants[141] = floor(constants[1]);
    computedConstants[142] = min(constants[1], constants[0]);
    computedConstants[143] = min(constants[1], min(constants[0], constants[2]));
    computedConstants[144] = max(constants[1], constants[0]);
    computedConstants[145] = max(constants[1], max(constants[0], constants[2]));
    computedConstants[146] = fmod(constants[1], constants[0]);
    computedConstants[147] = sin(constants[1]);
    computedConstants[148] = cos(constants[1]);
    computedConstants[149] = tan(constants[1]);
    computedConstants[150] = sec(constants[1]);
    computedConstants[151] = csc(constants[1]);
    computedConstants[152] = cot(constants[1]);
    computedConstants[153] = sinh(constants[1]);
    computedConstants[154] = cosh(constants[1]);
    computedConstants[155] = tanh(constants[1]);
    computedConstants[156] = sech(constants[1]);
    computedConstants[157] = csch(constants[1]);
    computedConstants[158] = coth(constants[1]);
    computedConstants[159] = asin(constants[1]);
    computedConstants[160] = acos(constants[1]);
    computedConstants[161] = atan(constants[1]);
    computedConstants[162] = asec(constants[1]);
    computedConstants[163] = acsc(constants[1]);
    computedConstants[164] = acot(constants[1]);
    computedConstants[165] = asinh(constants[1]);
    computedConstants[166] = acosh(constants[1]);
    computedConstants[167] = atanh(constants[1]/2.0);
    computedConstants[168] = asech(constants[1]);
    computedConstants[169] = acsch(constants[1]);
    computedConstants[170] = acoth(2.0*constants[1]);
    computedConstants[171] = (constants[1] > constants[0])?constants[1]:NAN;
    computedConstants[172] = (constants[1] > constants[0])?constants[1]:constants[2];
    computedConstants[173] = (constants[1] > constants[0])?constants[1]:(constants[2] > constants[3])?constants[2]:(constants[4] > constants[5])?constants[4]:NAN;
    computedConstants[174] = (constants[1] > constants[0])?constants[1]:(constants[2] > constants[3])?constants[2]:(constants[4] > constants[5])?constants[4]:constants[6];
    computedConstants[175] = 123.0+((constants[1] > constants[0])?constants[1]:NAN);
    computedConstants[180] = constants[1];
    computedConstants[187] = (constants[1] && constants[0])+((constants[2] > constants[3])?constants[0]:NAN)+constants[4]+(constants[5] && constants[6]);
    computedConstants[188] = (constants[1] && constants[0])-(((constants[2] > constants[3])?constants[0]:NAN)-(constants[4]-((constants[2] > constants[3])?constants[0]:NAN)))-(constants[5] && constants[6]);
    computedConstants[189] = (constants[1] && constants[0])*((constants[2] > constants[3])?constants[0]:NAN)*constants[4]*((constants[2] > constants[3])?constants[0]:NAN)*(constants[5] && constants[6]);
    computedConstants[190] = (constants[1] && constants[0])/(((constants[2] > constants[3])?constants[0]:NAN)/(constants[4]/((constants[2] > constants[3])?constants[0]:NAN)));
    computedConstants[191] = (constants[1] || constants[0]) && xor(constants[1], constants[0]) && ((constants[2] > constants[3])?constants[0]:NAN) && constants[4] && ((constants[2] > constants[3])?constants[0]:NAN) && xor(constants[1], constants[0]) && (constants[1] || constants[0]);
    computedConstants[192] = (constants[1] && constants[0]) || xor(constants[1], constants[0]) || ((constants[2] > constants[3])?constants[0]:NAN) || constants[4] || ((constants[2] > constants[3])?constants[0]:NAN) || xor(constants[1], constants[0]) || (constants[1] && constants[0]);
    computedConstants[193] = xor(constants[1] && constants[0], xor(constants[1] || constants[0], xor((constants[2] > constants[3])?constants[0]:NAN, xor(xor(xor(constants[4], (constants[2] > constants[3])?constants[0]:NAN), constants[1] || constants[0]), constants[1] && constants[0]))));
    computedConstants[194] = pow(constants[1] && constants[0], pow((constants[2] > constants[3])?constants[0]:NAN, pow(pow(constants[4], (constants[2] > constants[3])?constants[0]:NAN), constants[1] && constants[0])));
    computedConstants[195] = pow(pow(pow(constants[1] && constants[0], 1.0/pow((constants[2] > constants[3])?constants[0]:NAN, 1.0/constants[4])), 1.0/((constants[2] > constants[3])?constants[0]:NAN)), 1.0/(constants[1] && constants[0]));
    computedConstants[196] = -(constants[1] && constants[0])+-((constants[2] > constants[3])?constants[0]:NAN);
}

void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    rates[0] = 1.0;
}

void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)
{
    externalVariables[0] = externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, 0);
    findRoot0(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables);
}
