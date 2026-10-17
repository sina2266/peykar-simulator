/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifdef _WIN32
#    define _USE_MATH_DEFINES
#endif

#include "libcellml/generatorprofile.h"

#include <cmath>

#include "generatorprofile_p.h"
#include "utilities.h"

namespace libcellml {

void GeneratorProfile::GeneratorProfileImpl::loadProfile(GeneratorProfile::Profile profile)
{
    mProfile = profile;

    if (profile == GeneratorProfile::Profile::C) {
        // Whether the profile requires an interface to be generated.

        mHasInterface = true;

        // Equality.

        mEqualityString = " = ";

        // Relational and logical operators.

        mEqString = " == ";
        mNeqString = " != ";
        mLtString = " < ";
        mLeqString = " <= ";
        mGtString = " > ";
        mGeqString = " >= ";
        mAndString = " && ";
        mOrString = " || ";
        mXorString = "xor";
        mNotString = "!";

        mHasEqOperator = true;
        mHasNeqOperator = true;
        mHasLtOperator = true;
        mHasLeqOperator = true;
        mHasGtOperator = true;
        mHasGeqOperator = true;
        mHasAndOperator = true;
        mHasOrOperator = true;
        mHasXorOperator = false;
        mHasNotOperator = true;

        // Arithmetic operators.

        mPlusString = "+";
        mMinusString = "-";
        mTimesString = "*";
        mDivideString = "/";
        mPowerString = "pow";
        mSquareRootString = "sqrt";
        mSquareString = "";
        mAbsoluteValueString = "fabs";
        mExponentialString = "exp";
        mNaturalLogarithmString = "log";
        mCommonLogarithmString = "log10";
        mCeilingString = "ceil";
        mFloorString = "floor";
        mMinString = "min";
        mMaxString = "max";
        mRemString = "fmod";

        mHasPowerOperator = false;

        // Trigonometric operators.

        mSinString = "sin";
        mCosString = "cos";
        mTanString = "tan";
        mSecString = "sec";
        mCscString = "csc";
        mCotString = "cot";
        mSinhString = "sinh";
        mCoshString = "cosh";
        mTanhString = "tanh";
        mSechString = "sech";
        mCschString = "csch";
        mCothString = "coth";
        mAsinString = "asin";
        mAcosString = "acos";
        mAtanString = "atan";
        mAsecString = "asec";
        mAcscString = "acsc";
        mAcotString = "acot";
        mAsinhString = "asinh";
        mAcoshString = "acosh";
        mAtanhString = "atanh";
        mAsechString = "asech";
        mAcschString = "acsch";
        mAcothString = "acoth";

        // Piecewise statement.

        mConditionalOperatorIfString = "([CONDITION])?[IF_STATEMENT]";
        mConditionalOperatorElseString = ":[ELSE_STATEMENT]";

        mHasConditionalOperator = true;

        // Constants.

        mTrueString = "1.0";
        mFalseString = "0.0";
        mEString = convertToString(exp(1.0));
        mPiString = convertToString(M_PI);
        mInfString = "INFINITY";
        mNanString = "NAN";

        // Arithmetic functions.

        mEqFunctionString = "";
        mNeqFunctionString = "";
        mLtFunctionString = "";
        mLeqFunctionString = "";
        mGtFunctionString = "";
        mGeqFunctionString = "";
        mAndFunctionString = "";
        mOrFunctionString = "";
        mXorFunctionString = "double xor(double x, double y)\n"
                             "{\n"
                             "    return (x != 0.0) ^ (y != 0.0);\n"
                             "}\n";
        mNotFunctionString = "";
        mMinFunctionString = "double min(double x, double y)\n"
                             "{\n"
                             "    return (x < y)?x:y;\n"
                             "}\n";
        mMaxFunctionString = "double max(double x, double y)\n"
                             "{\n"
                             "    return (x > y)?x:y;\n"
                             "}\n";

        // Trigonometric functions.

        mSecFunctionString = "double sec(double x)\n"
                             "{\n"
                             "    return 1.0/cos(x);\n"
                             "}\n";
        mCscFunctionString = "double csc(double x)\n"
                             "{\n"
                             "    return 1.0/sin(x);\n"
                             "}\n";
        mCotFunctionString = "double cot(double x)\n"
                             "{\n"
                             "    return 1.0/tan(x);\n"
                             "}\n";
        mSechFunctionString = "double sech(double x)\n"
                              "{\n"
                              "    return 1.0/cosh(x);\n"
                              "}\n";
        mCschFunctionString = "double csch(double x)\n"
                              "{\n"
                              "    return 1.0/sinh(x);\n"
                              "}\n";
        mCothFunctionString = "double coth(double x)\n"
                              "{\n"
                              "    return 1.0/tanh(x);\n"
                              "}\n";
        mAsecFunctionString = "double asec(double x)\n"
                              "{\n"
                              "    return acos(1.0/x);\n"
                              "}\n";
        mAcscFunctionString = "double acsc(double x)\n"
                              "{\n"
                              "    return asin(1.0/x);\n"
                              "}\n";
        mAcotFunctionString = "double acot(double x)\n"
                              "{\n"
                              "    return atan(1.0/x);\n"
                              "}\n";
        mAsechFunctionString = "double asech(double x)\n"
                               "{\n"
                               "    double oneOverX = 1.0/x;\n"
                               "\n"
                               "    return log(oneOverX+sqrt(oneOverX*oneOverX-1.0));\n"
                               "}\n";
        mAcschFunctionString = "double acsch(double x)\n"
                               "{\n"
                               "    double oneOverX = 1.0/x;\n"
                               "\n"
                               "    return log(oneOverX+sqrt(oneOverX*oneOverX+1.0));\n"
                               "}\n";
        mAcothFunctionString = "double acoth(double x)\n"
                               "{\n"
                               "    double oneOverX = 1.0/x;\n"
                               "\n"
                               "    return 0.5*log((1.0+oneOverX)/(1.0-oneOverX));\n"
                               "}\n";

        // Miscellaneous.

        mCommentString = "/* [CODE] */\n";
        mOriginCommentString = "The content of this file was generated using [PROFILE_INFORMATION] libCellML [LIBCELLML_VERSION].";

        mInterfaceFileNameString = "model.h";

        mInterfaceHeaderString = "#pragma once\n"
                                 "\n"
                                 "#include <stddef.h>\n";
        mImplementationHeaderString = "#include \"[INTERFACE_FILE_NAME]\"\n"
                                      "\n"
                                      "#include <math.h>\n"
                                      "#include <stdlib.h>\n";

        mInterfaceVersionString = "extern const char VERSION[];\n";
        mImplementationVersionString = "const char VERSION[] = \"0.8.0\";\n";

        mInterfaceLibcellmlVersionString = "extern const char LIBCELLML_VERSION[];\n";
        mImplementationLibcellmlVersionString = "const char LIBCELLML_VERSION[] = \"[LIBCELLML_VERSION]\";\n";

        mInterfaceStateCountString = "extern const size_t STATE_COUNT;\n";
        mImplementationStateCountString = "const size_t STATE_COUNT = [STATE_COUNT];\n";

        mInterfaceConstantCountString = "extern const size_t CONSTANT_COUNT;\n";
        mImplementationConstantCountString = "const size_t CONSTANT_COUNT = [CONSTANT_COUNT];\n";

        mInterfaceComputedConstantCountString = "extern const size_t COMPUTED_CONSTANT_COUNT;\n";
        mImplementationComputedConstantCountString = "const size_t COMPUTED_CONSTANT_COUNT = [COMPUTED_CONSTANT_COUNT];\n";

        mInterfaceAlgebraicVariableCountString = "extern const size_t ALGEBRAIC_VARIABLE_COUNT;\n";
        mImplementationAlgebraicVariableCountString = "const size_t ALGEBRAIC_VARIABLE_COUNT = [ALGEBRAIC_VARIABLE_COUNT];\n";

        mInterfaceExternalVariableCountString = "extern const size_t EXTERNAL_VARIABLE_COUNT;\n";
        mImplementationExternalVariableCountString = "const size_t EXTERNAL_VARIABLE_COUNT = [EXTERNAL_VARIABLE_COUNT];\n";

        mVariableInfoObjectString = "typedef struct {\n"
                                    "    char name[[NAME_SIZE]];\n"
                                    "    char units[[UNITS_SIZE]];\n"
                                    "    char component[[COMPONENT_SIZE]];\n"
                                    "} VariableInfo;\n";

        mInterfaceVoiInfoString = "extern const VariableInfo VOI_INFO;\n";
        mImplementationVoiInfoString = "const VariableInfo VOI_INFO = [CODE];\n";

        mInterfaceStateInfoString = "extern const VariableInfo STATE_INFO[];\n";
        mImplementationStateInfoString = "const VariableInfo STATE_INFO[] = {\n"
                                         "[CODE]"
                                         "};\n";

        mInterfaceConstantInfoString = "extern const VariableInfo CONSTANT_INFO[];\n";
        mImplementationConstantInfoString = "const VariableInfo CONSTANT_INFO[] = {\n"
                                            "[CODE]"
                                            "};\n";

        mInterfaceComputedConstantInfoString = "extern const VariableInfo COMPUTED_CONSTANT_INFO[];\n";
        mImplementationComputedConstantInfoString = "const VariableInfo COMPUTED_CONSTANT_INFO[] = {\n"
                                                    "[CODE]"
                                                    "};\n";

        mInterfaceAlgebraicVariableInfoString = "extern const VariableInfo ALGEBRAIC_VARIABLE_INFO[];\n";
        mImplementationAlgebraicVariableInfoString = "const VariableInfo ALGEBRAIC_VARIABLE_INFO[] = {\n"
                                                     "[CODE]"
                                                     "};\n";

        mInterfaceExternalVariableInfoString = "extern const VariableInfo EXTERNAL_VARIABLE_INFO[];\n";
        mImplementationExternalVariableInfoString = "const VariableInfo EXTERNAL_VARIABLE_INFO[] = {\n"
                                                    "[CODE]"
                                                    "};\n";

        mVariableInfoEntryString = "{\"[NAME]\", \"[UNITS]\", \"[COMPONENT]\"}";

        mVoiString = "voi";

        mStatesArrayString = "states";
        mRatesArrayString = "rates";
        mConstantsArrayString = "constants";
        mComputedConstantsArrayString = "computedConstants";
        mAlgebraicVariablesArrayString = "algebraicVariables";
        mExternalVariablesArrayString = "externalVariables";

        mExternalVariableMethodTypeDefinitionNonDiffString = "typedef double (* ExternalVariable)(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, size_t index);\n";
        mExternalVariableMethodTypeDefinitionDiffString = "typedef double (* ExternalVariable)(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, size_t index);\n";

        mExternalVariableMethodCallNonDiffString = "externalVariable(constants, computedConstants, algebraicVariables, externalVariables, [INDEX])";
        mExternalVariableMethodCallDiffString = "externalVariable(voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables, [INDEX])";

        mRootFindingInfoObjectFamWoevString = "typedef struct {\n"
                                              "    double *constants;\n"
                                              "    double *computedConstants;\n"
                                              "    double *algebraicVariables;\n"
                                              "} RootFindingInfo;\n";
        mRootFindingInfoObjectFamWevString = "typedef struct {\n"
                                             "    double *constants;\n"
                                             "    double *computedConstants;\n"
                                             "    double *algebraicVariables;\n"
                                             "    double *externalVariables;\n"
                                             "} RootFindingInfo;\n";
        mRootFindingInfoObjectFdmWoevString = "typedef struct {\n"
                                              "    double voi;\n"
                                              "    double *states;\n"
                                              "    double *rates;\n"
                                              "    double *constants;\n"
                                              "    double *computedConstants;\n"
                                              "    double *algebraicVariables;\n"
                                              "} RootFindingInfo;\n";
        mRootFindingInfoObjectFdmWevString = "typedef struct {\n"
                                             "    double voi;\n"
                                             "    double *states;\n"
                                             "    double *rates;\n"
                                             "    double *constants;\n"
                                             "    double *computedConstants;\n"
                                             "    double *algebraicVariables;\n"
                                             "    double *externalVariables;\n"
                                             "} RootFindingInfo;\n";

        mExternNlaSolveMethodString = "extern void nlaSolve(void (*objectiveFunction)(double *, double *, void *),\n"
                                      "                     double *u, size_t n, void *data);\n";

        mFindRootCallFamWoevString = "findRoot[INDEX](constants, computedConstants, algebraicVariables);\n";
        mFindRootCallFamWevString = "findRoot[INDEX](constants, computedConstants, algebraicVariables, externalVariables);\n";
        mFindRootCallFdmWoevString = "findRoot[INDEX](voi, states, rates, constants, computedConstants, algebraicVariables);\n";
        mFindRootCallFdmWevString = "findRoot[INDEX](voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables);\n";

        mFindRootMethodFamWoevString = "void findRoot[INDEX](double *constants, double *computedConstants, double *algebraicVariables)\n"
                                       "{\n"
                                       "    RootFindingInfo rfi = { constants, computedConstants, algebraicVariables };\n"
                                       "    double u[[SIZE]];\n"
                                       "\n"
                                       "[CODE]"
                                       "}\n";
        mFindRootMethodFamWevString = "void findRoot[INDEX](double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables)\n"
                                      "{\n"
                                      "    RootFindingInfo rfi = { constants, computedConstants, algebraicVariables, externalVariables };\n"
                                      "    double u[[SIZE]];\n"
                                      "\n"
                                      "[CODE]"
                                      "}\n";
        mFindRootMethodFdmWoevString = "void findRoot[INDEX](double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n"
                                       "{\n"
                                       "    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables };\n"
                                       "    double u[[SIZE]];\n"
                                       "\n"
                                       "[CODE]"
                                       "}\n";
        mFindRootMethodFdmWevString = "void findRoot[INDEX](double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables)\n"
                                      "{\n"
                                      "    RootFindingInfo rfi = { voi, states, rates, constants, computedConstants, algebraicVariables, externalVariables };\n"
                                      "    double u[[SIZE]];\n"
                                      "\n"
                                      "[CODE]"
                                      "}\n";

        mNlaSolveCallFamWoevString = "nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n";
        mNlaSolveCallFamWevString = "nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n";
        mNlaSolveCallFdmWoevString = "nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n";
        mNlaSolveCallFdmWevString = "nlaSolve(objectiveFunction[INDEX], u, [SIZE], &rfi);\n";

        mObjectiveFunctionMethodFamWoevString = "void objectiveFunction[INDEX](double *u, double *f, void *data)\n"
                                                "{\n"
                                                "    double *constants = ((RootFindingInfo *) data)->constants;\n"
                                                "    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n"
                                                "    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n"
                                                "\n"
                                                "[CODE]"
                                                "}\n";
        mObjectiveFunctionMethodFamWevString = "void objectiveFunction[INDEX](double *u, double *f, void *data)\n"
                                               "{\n"
                                               "    double *constants = ((RootFindingInfo *) data)->constants;\n"
                                               "    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n"
                                               "    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n"
                                               "    double *externalVariables = ((RootFindingInfo *) data)->externalVariables;\n"
                                               "\n"
                                               "[CODE]"
                                               "}\n";
        mObjectiveFunctionMethodFdmWoevString = "void objectiveFunction[INDEX](double *u, double *f, void *data)\n"
                                                "{\n"
                                                "    double voi = ((RootFindingInfo *) data)->voi;\n"
                                                "    double *states = ((RootFindingInfo *) data)->states;\n"
                                                "    double *rates = ((RootFindingInfo *) data)->rates;\n"
                                                "    double *constants = ((RootFindingInfo *) data)->constants;\n"
                                                "    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n"
                                                "    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n"
                                                "\n"
                                                "[CODE]"
                                                "}\n";
        mObjectiveFunctionMethodFdmWevString = "void objectiveFunction[INDEX](double *u, double *f, void *data)\n"
                                               "{\n"
                                               "    double voi = ((RootFindingInfo *) data)->voi;\n"
                                               "    double *states = ((RootFindingInfo *) data)->states;\n"
                                               "    double *rates = ((RootFindingInfo *) data)->rates;\n"
                                               "    double *constants = ((RootFindingInfo *) data)->constants;\n"
                                               "    double *computedConstants = ((RootFindingInfo *) data)->computedConstants;\n"
                                               "    double *algebraicVariables = ((RootFindingInfo *) data)->algebraicVariables;\n"
                                               "    double *externalVariables = ((RootFindingInfo *) data)->externalVariables;\n"
                                               "\n"
                                               "[CODE]"
                                               "}\n";

        mUArrayString = "u";
        mFArrayString = "f";

        mInterfaceCreateStatesArrayMethodString = "double * createStatesArray();\n";
        mImplementationCreateStatesArrayMethodString = "double * createStatesArray()\n"
                                                       "{\n"
                                                       "    double *res = (double *) malloc(STATE_COUNT*sizeof(double));\n"
                                                       "\n"
                                                       "    for (size_t i = 0; i < STATE_COUNT; ++i) {\n"
                                                       "        res[i] = NAN;\n"
                                                       "    }\n"
                                                       "\n"
                                                       "    return res;\n"
                                                       "}\n";

        mInterfaceCreateConstantsArrayMethodString = "double * createConstantsArray();\n";
        mImplementationCreateConstantsArrayMethodString = "double * createConstantsArray()\n"
                                                          "{\n"
                                                          "    double *res = (double *) malloc(CONSTANT_COUNT*sizeof(double));\n"
                                                          "\n"
                                                          "    for (size_t i = 0; i < CONSTANT_COUNT; ++i) {\n"
                                                          "        res[i] = NAN;\n"
                                                          "    }\n"
                                                          "\n"
                                                          "    return res;\n"
                                                          "}\n";

        mInterfaceCreateComputedConstantsArrayMethodString = "double * createComputedConstantsArray();\n";
        mImplementationCreateComputedConstantsArrayMethodString = "double * createComputedConstantsArray()\n"
                                                                  "{\n"
                                                                  "    double *res = (double *) malloc(COMPUTED_CONSTANT_COUNT*sizeof(double));\n"
                                                                  "\n"
                                                                  "    for (size_t i = 0; i < COMPUTED_CONSTANT_COUNT; ++i) {\n"
                                                                  "        res[i] = NAN;\n"
                                                                  "    }\n"
                                                                  "\n"
                                                                  "    return res;\n"
                                                                  "}\n";

        mInterfaceCreateAlgebraicVariablesArrayMethodString = "double * createAlgebraicVariablesArray();\n";
        mImplementationCreateAlgebraicVariablesArrayMethodString = "double * createAlgebraicVariablesArray()\n"
                                                                   "{\n"
                                                                   "    double *res = (double *) malloc(ALGEBRAIC_VARIABLE_COUNT*sizeof(double));\n"
                                                                   "\n"
                                                                   "    for (size_t i = 0; i < ALGEBRAIC_VARIABLE_COUNT; ++i) {\n"
                                                                   "        res[i] = NAN;\n"
                                                                   "    }\n"
                                                                   "\n"
                                                                   "    return res;\n"
                                                                   "}\n";

        mInterfaceCreateExternalVariablesArrayMethodString = "double * createExternalVariablesArray();\n";
        mImplementationCreateExternalVariablesArrayMethodString = "double * createExternalVariablesArray()\n"
                                                                  "{\n"
                                                                  "    double *res = (double *) malloc(EXTERNAL_VARIABLE_COUNT*sizeof(double));\n"
                                                                  "\n"
                                                                  "    for (size_t i = 0; i < EXTERNAL_VARIABLE_COUNT; ++i) {\n"
                                                                  "        res[i] = NAN;\n"
                                                                  "    }\n"
                                                                  "\n"
                                                                  "    return res;\n"
                                                                  "}\n";

        mInterfaceDeleteArrayMethodString = "void deleteArray(double *array);\n";
        mImplementationDeleteArrayMethodString = "void deleteArray(double *array)\n"
                                                 "{\n"
                                                 "    free(array);\n"
                                                 "}\n";

        mInterfaceInitialiseArraysMethodNonDiffString = "void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationInitialiseArraysMethodNonDiffString = "void initialiseArrays(double *constants, double *computedConstants, double *algebraicVariables)\n"
                                                             "{\n"
                                                             "[CODE]"
                                                             "}\n";

        mInterfaceInitialiseArraysMethodDiffString = "void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationInitialiseArraysMethodDiffString = "void initialiseArrays(double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n"
                                                          "{\n"
                                                          "[CODE]"
                                                          "}\n";

        mInterfaceComputeComputedConstantsMethodNonDiffString = "void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationComputeComputedConstantsMethodNonDiffString = "void computeComputedConstants(double *constants, double *computedConstants, double *algebraicVariables)\n"
                                                                     "{\n"
                                                                     "[CODE]"
                                                                     "}\n";

        mInterfaceComputeComputedConstantsMethodDiffString = "void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationComputeComputedConstantsMethodDiffString = "void computeComputedConstants(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n"
                                                                  "{\n"
                                                                  "[CODE]"
                                                                  "}\n";

        mInterfaceComputeRatesMethodWoevString = "void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationComputeRatesMethodWoevString = "void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n{\n"
                                                      "[CODE]"
                                                      "}\n";

        mInterfaceComputeRatesMethodWevString = "void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);\n";
        mImplementationComputeRatesMethodWevString = "void computeRates(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)\n{\n"
                                                     "[CODE]"
                                                     "}\n";

        mInterfaceComputeVariablesMethodFamWoevString = "void computeVariables(double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationComputeVariablesMethodFamWoevString = "void computeVariables(double *constants, double *computedConstants, double *algebraicVariables)\n"
                                                             "{\n"
                                                             "[CODE]"
                                                             "}\n";

        mInterfaceComputeVariablesMethodFamWevString = "void computeVariables(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);\n";
        mImplementationComputeVariablesMethodFamWevString = "void computeVariables(double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)\n"
                                                            "{\n"
                                                            "[CODE]"
                                                            "}\n";

        mInterfaceComputeVariablesMethodFdmWoevString = "void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables);\n";
        mImplementationComputeVariablesMethodFdmWoevString = "void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables)\n"
                                                             "{\n"
                                                             "[CODE]"
                                                             "}\n";

        mInterfaceComputeVariablesMethodFdmWevString = "void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable);\n";
        mImplementationComputeVariablesMethodFdmWevString = "void computeVariables(double voi, double *states, double *rates, double *constants, double *computedConstants, double *algebraicVariables, double *externalVariables, ExternalVariable externalVariable)\n"
                                                            "{\n"
                                                            "[CODE]"
                                                            "}\n";

        mEmptyMethodString = "";

        mIndentString = "    ";

        mVariableDeclarationString = "double [CODE]";

        mOpenArrayString = "[";
        mCloseArrayString = "]";

        mArrayElementSeparatorString = ",";

        mCommandSeparatorString = ";";
    } else { // GeneratorProfile::Profile::PYTHON.
        // Whether the profile requires an interface to be generated.

        mHasInterface = false;

        // Equality.

        mEqualityString = " = ";

        // Relational and logical operators.

        mEqString = "eq_func";
        mNeqString = "neq_func";
        mLtString = "lt_func";
        mLeqString = "leq_func";
        mGtString = "gt_func";
        mGeqString = "geq_func";
        mAndString = "and_func";
        mOrString = "or_func";
        mXorString = "xor_func";
        mNotString = "not_func";

        mHasEqOperator = false;
        mHasNeqOperator = false;
        mHasLtOperator = false;
        mHasLeqOperator = false;
        mHasGtOperator = false;
        mHasGeqOperator = false;
        mHasAndOperator = false;
        mHasOrOperator = false;
        mHasXorOperator = false;
        mHasNotOperator = false;

        // Arithmetic operators.

        mPlusString = "+";
        mMinusString = "-";
        mTimesString = "*";
        mDivideString = "/";
        mPowerString = "pow";
        mSquareRootString = "sqrt";
        mSquareString = "";
        mAbsoluteValueString = "fabs";
        mExponentialString = "exp";
        mNaturalLogarithmString = "log";
        mCommonLogarithmString = "log10";
        mCeilingString = "ceil";
        mFloorString = "floor";
        mMinString = "min";
        mMaxString = "max";
        mRemString = "fmod";

        mHasPowerOperator = false;

        // Trigonometric operators.

        mSinString = "sin";
        mCosString = "cos";
        mTanString = "tan";
        mSecString = "sec";
        mCscString = "csc";
        mCotString = "cot";
        mSinhString = "sinh";
        mCoshString = "cosh";
        mTanhString = "tanh";
        mSechString = "sech";
        mCschString = "csch";
        mCothString = "coth";
        mAsinString = "asin";
        mAcosString = "acos";
        mAtanString = "atan";
        mAsecString = "asec";
        mAcscString = "acsc";
        mAcotString = "acot";
        mAsinhString = "asinh";
        mAcoshString = "acosh";
        mAtanhString = "atanh";
        mAsechString = "asech";
        mAcschString = "acsch";
        mAcothString = "acoth";

        // Piecewise statement.

        mConditionalOperatorIfString = "[IF_STATEMENT] if [CONDITION]";
        mConditionalOperatorElseString = " else [ELSE_STATEMENT]";

        mHasConditionalOperator = true;

        // Constants.

        mTrueString = "1.0";
        mFalseString = "0.0";
        mEString = convertToString(exp(1.0));
        mPiString = convertToString(M_PI);
        mInfString = "inf";
        mNanString = "nan";

        // Arithmetic functions.

        mEqFunctionString = "\n"
                            "def eq_func(x, y):\n"
                            "    return 1.0 if x == y else 0.0\n";
        mNeqFunctionString = "\n"
                             "def neq_func(x, y):\n"
                             "    return 1.0 if x != y else 0.0\n";
        mLtFunctionString = "\n"
                            "def lt_func(x, y):\n"
                            "    return 1.0 if x < y else 0.0\n";
        mLeqFunctionString = "\n"
                             "def leq_func(x, y):\n"
                             "    return 1.0 if x <= y else 0.0\n";
        mGtFunctionString = "\n"
                            "def gt_func(x, y):\n"
                            "    return 1.0 if x > y else 0.0\n";
        mGeqFunctionString = "\n"
                             "def geq_func(x, y):\n"
                             "    return 1.0 if x >= y else 0.0\n";
        mAndFunctionString = "\n"
                             "def and_func(x, y):\n"
                             "    return 1.0 if bool(x) & bool(y) else 0.0\n";
        mOrFunctionString = "\n"
                            "def or_func(x, y):\n"
                            "    return 1.0 if bool(x) | bool(y) else 0.0\n";
        mXorFunctionString = "\n"
                             "def xor_func(x, y):\n"
                             "    return 1.0 if bool(x) ^ bool(y) else 0.0\n";
        mNotFunctionString = "\n"
                             "def not_func(x):\n"
                             "    return 1.0 if not bool(x) else 0.0\n";
        mMinFunctionString = "\n"
                             "def min(x, y):\n"
                             "    return x if x < y else y\n";
        mMaxFunctionString = "\n"
                             "def max(x, y):\n"
                             "    return x if x > y else y\n";

        // Trigonometric functions.

        mSecFunctionString = "\n"
                             "def sec(x):\n"
                             "    return 1.0/cos(x)\n";
        mCscFunctionString = "\n"
                             "def csc(x):\n"
                             "    return 1.0/sin(x)\n";
        mCotFunctionString = "\n"
                             "def cot(x):\n"
                             "    return 1.0/tan(x)\n";
        mSechFunctionString = "\n"
                              "def sech(x):\n"
                              "    return 1.0/cosh(x)\n";
        mCschFunctionString = "\n"
                              "def csch(x):\n"
                              "    return 1.0/sinh(x)\n";
        mCothFunctionString = "\n"
                              "def coth(x):\n"
                              "    return 1.0/tanh(x)\n";
        mAsecFunctionString = "\n"
                              "def asec(x):\n"
                              "    return acos(1.0/x)\n";
        mAcscFunctionString = "\n"
                              "def acsc(x):\n"
                              "    return asin(1.0/x)\n";
        mAcotFunctionString = "\n"
                              "def acot(x):\n"
                              "    return atan(1.0/x)\n";
        mAsechFunctionString = "\n"
                               "def asech(x):\n"
                               "    one_over_x = 1.0/x\n"
                               "\n"
                               "    return log(one_over_x+sqrt(one_over_x*one_over_x-1.0))\n";
        mAcschFunctionString = "\n"
                               "def acsch(x):\n"
                               "    one_over_x = 1.0/x\n"
                               "\n"
                               "    return log(one_over_x+sqrt(one_over_x*one_over_x+1.0))\n";
        mAcothFunctionString = "\n"
                               "def acoth(x):\n"
                               "    one_over_x = 1.0/x\n"
                               "\n"
                               "    return 0.5*log((1.0+one_over_x)/(1.0-one_over_x))\n";

        // Miscellaneous.

        mCommentString = "# [CODE]\n";
        mOriginCommentString = "The content of this file was generated using [PROFILE_INFORMATION] libCellML [LIBCELLML_VERSION].";

        mInterfaceFileNameString = "";

        mInterfaceHeaderString = "";
        mImplementationHeaderString = "from enum import Enum\n"
                                      "from math import *\n"
                                      "\n";

        mInterfaceVersionString = "";
        mImplementationVersionString = "__version__ = \"0.8.0\"\n";

        mInterfaceLibcellmlVersionString = "";
        mImplementationLibcellmlVersionString = "LIBCELLML_VERSION = \"[LIBCELLML_VERSION]\"\n";

        mInterfaceStateCountString = "";
        mImplementationStateCountString = "STATE_COUNT = [STATE_COUNT]\n";

        mInterfaceConstantCountString = "";
        mImplementationConstantCountString = "CONSTANT_COUNT = [CONSTANT_COUNT]\n";

        mInterfaceComputedConstantCountString = "";
        mImplementationComputedConstantCountString = "COMPUTED_CONSTANT_COUNT = [COMPUTED_CONSTANT_COUNT]\n";

        mInterfaceAlgebraicVariableCountString = "";
        mImplementationAlgebraicVariableCountString = "ALGEBRAIC_VARIABLE_COUNT = [ALGEBRAIC_VARIABLE_COUNT]\n";

        mInterfaceExternalVariableCountString = "";
        mImplementationExternalVariableCountString = "EXTERNAL_VARIABLE_COUNT = [EXTERNAL_VARIABLE_COUNT]\n";

        mVariableInfoObjectString = "";

        mInterfaceVoiInfoString = "";
        mImplementationVoiInfoString = "VOI_INFO = [CODE]\n";

        mInterfaceStateInfoString = "";
        mImplementationStateInfoString = "STATE_INFO = [\n"
                                         "[CODE]"
                                         "]\n";

        mInterfaceConstantInfoString = "";
        mImplementationConstantInfoString = "CONSTANT_INFO = [\n"
                                            "[CODE]"
                                            "]\n";

        mInterfaceComputedConstantInfoString = "";
        mImplementationComputedConstantInfoString = "COMPUTED_CONSTANT_INFO = [\n"
                                                    "[CODE]"
                                                    "]\n";

        mInterfaceAlgebraicVariableInfoString = "";
        mImplementationAlgebraicVariableInfoString = "ALGEBRAIC_VARIABLE_INFO = [\n"
                                                     "[CODE]"
                                                     "]\n";

        mInterfaceExternalVariableInfoString = "";
        mImplementationExternalVariableInfoString = "EXTERNAL_VARIABLE_INFO = [\n"
                                                    "[CODE]"
                                                    "]\n";

        mVariableInfoEntryString = "{\"name\": \"[NAME]\", \"units\": \"[UNITS]\", \"component\": \"[COMPONENT]\"}";

        mVoiString = "voi";

        mStatesArrayString = "states";
        mRatesArrayString = "rates";
        mConstantsArrayString = "constants";
        mComputedConstantsArrayString = "computed_constants";
        mAlgebraicVariablesArrayString = "algebraic_variables";
        mExternalVariablesArrayString = "external_variables";

        mExternalVariableMethodTypeDefinitionNonDiffString = "";
        mExternalVariableMethodTypeDefinitionDiffString = "";

        mExternalVariableMethodCallNonDiffString = "external_variable(constants, computed_constants, algebraic_variables, external_variables, [INDEX])";
        mExternalVariableMethodCallDiffString = "external_variable(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, [INDEX])";

        mRootFindingInfoObjectFamWoevString = "";
        mRootFindingInfoObjectFamWevString = "";
        mRootFindingInfoObjectFdmWoevString = "";
        mRootFindingInfoObjectFdmWevString = "";

        mExternNlaSolveMethodString = "\n"
                                      "from nlasolver import nla_solve"
                                      "\n";

        mFindRootCallFamWoevString = "find_root_[INDEX](constants, computed_constants, algebraic_variables)\n";
        mFindRootCallFamWevString = "find_root_[INDEX](constants, computed_constants, algebraic_variables, external_variables)\n";
        mFindRootCallFdmWoevString = "find_root_[INDEX](voi, states, rates, constants, computed_constants, algebraic_variables)\n";
        mFindRootCallFdmWevString = "find_root_[INDEX](voi, states, rates, constants, computed_constants, algebraic_variables, external_variables)\n";

        mFindRootMethodFamWoevString = "\n"
                                       "def find_root_[INDEX](constants, computed_constants, algebraic_variables):\n"
                                       "    u = [nan]*[SIZE]\n"
                                       "\n"
                                       "[CODE]";
        mFindRootMethodFamWevString = "\n"
                                      "def find_root_[INDEX](constants, computed_constants, algebraic_variables, external_variables):\n"
                                      "    u = [nan]*[SIZE]\n"
                                      "\n"
                                      "[CODE]";
        mFindRootMethodFdmWoevString = "\n"
                                       "def find_root_[INDEX](voi, states, rates, constants, computed_constants, algebraic_variables):\n"
                                       "    u = [nan]*[SIZE]\n"
                                       "\n"
                                       "[CODE]";
        mFindRootMethodFdmWevString = "\n"
                                      "def find_root_[INDEX](voi, states, rates, constants, computed_constants, algebraic_variables, external_variables):\n"
                                      "    u = [nan]*[SIZE]\n"
                                      "\n"
                                      "[CODE]";

        mNlaSolveCallFamWoevString = "u = nla_solve(objective_function_[INDEX], u, [SIZE], [constants, computed_constants, algebraic_variables])\n";
        mNlaSolveCallFamWevString = "u = nla_solve(objective_function_[INDEX], u, [SIZE], [constants, computed_constants, algebraic_variables, external_variables])\n";
        mNlaSolveCallFdmWoevString = "u = nla_solve(objective_function_[INDEX], u, [SIZE], [voi, states, rates, constants, computed_constants, algebraic_variables])\n";
        mNlaSolveCallFdmWevString = "u = nla_solve(objective_function_[INDEX], u, [SIZE], [voi, states, rates, constants, computed_constants, algebraic_variables, external_variables])\n";

        mObjectiveFunctionMethodFamWoevString = "\n"
                                                "def objective_function_[INDEX](u, f, data):\n"
                                                "    constants = data[0]\n"
                                                "    computed_constants = data[1]\n"
                                                "    algebraic_variables = data[2]\n"
                                                "\n"
                                                "[CODE]";
        mObjectiveFunctionMethodFamWevString = "\n"
                                               "def objective_function_[INDEX](u, f, data):\n"
                                               "    constants = data[0]\n"
                                               "    computed_constants = data[1]\n"
                                               "    algebraic_variables = data[2]\n"
                                               "    external_variables = data[3]\n"
                                               "\n"
                                               "[CODE]";
        mObjectiveFunctionMethodFdmWoevString = "\n"
                                                "def objective_function_[INDEX](u, f, data):\n"
                                                "    voi = data[0]\n"
                                                "    states = data[1]\n"
                                                "    rates = data[2]\n"
                                                "    constants = data[3]\n"
                                                "    computed_constants = data[4]\n"
                                                "    algebraic_variables = data[5]\n"
                                                "\n"
                                                "[CODE]";
        mObjectiveFunctionMethodFdmWevString = "\n"
                                               "def objective_function_[INDEX](u, f, data):\n"
                                               "    voi = data[0]\n"
                                               "    states = data[1]\n"
                                               "    rates = data[2]\n"
                                               "    constants = data[3]\n"
                                               "    computed_constants = data[4]\n"
                                               "    algebraic_variables = data[5]\n"
                                               "    external_variables = data[6]\n"
                                               "\n"
                                               "[CODE]";

        mUArrayString = "u";
        mFArrayString = "f";

        mInterfaceCreateStatesArrayMethodString = "";
        mImplementationCreateStatesArrayMethodString = "\n"
                                                       "def create_states_array():\n"
                                                       "    return [nan]*STATE_COUNT\n";

        mInterfaceCreateConstantsArrayMethodString = "";
        mImplementationCreateConstantsArrayMethodString = "\n"
                                                          "def create_constants_array():\n"
                                                          "    return [nan]*CONSTANT_COUNT\n";

        mInterfaceCreateComputedConstantsArrayMethodString = "";
        mImplementationCreateComputedConstantsArrayMethodString = "\n"
                                                                  "def create_computed_constants_array():\n"
                                                                  "    return [nan]*COMPUTED_CONSTANT_COUNT\n";

        mInterfaceCreateAlgebraicVariablesArrayMethodString = "";
        mImplementationCreateAlgebraicVariablesArrayMethodString = "\n"
                                                                   "def create_algebraic_variables_array():\n"
                                                                   "    return [nan]*ALGEBRAIC_VARIABLE_COUNT\n";

        mInterfaceCreateExternalVariablesArrayMethodString = "";
        mImplementationCreateExternalVariablesArrayMethodString = "\n"
                                                                  "def create_external_variables_array():\n"
                                                                  "    return [nan]*EXTERNAL_VARIABLE_COUNT\n";

        mInterfaceDeleteArrayMethodString = "";
        mImplementationDeleteArrayMethodString = "";

        mInterfaceInitialiseArraysMethodNonDiffString = "";
        mImplementationInitialiseArraysMethodNonDiffString = "\n"
                                                             "def initialise_arrays(constants, computed_constants, algebraic_variables):\n"
                                                             "[CODE]";

        mInterfaceInitialiseArraysMethodDiffString = "";
        mImplementationInitialiseArraysMethodDiffString = "\n"
                                                          "def initialise_arrays(states, rates, constants, computed_constants, algebraic_variables):\n"
                                                          "[CODE]";

        mInterfaceComputeComputedConstantsMethodNonDiffString = "";
        mImplementationComputeComputedConstantsMethodNonDiffString = "\n"
                                                                     "def compute_computed_constants(constants, computed_constants, algebraic_variables):\n"
                                                                     "[CODE]";

        mInterfaceComputeComputedConstantsMethodDiffString = "";
        mImplementationComputeComputedConstantsMethodDiffString = "\n"
                                                                  "def compute_computed_constants(voi, states, rates, constants, computed_constants, algebraic_variables):\n"
                                                                  "[CODE]";

        mInterfaceComputeRatesMethodWoevString = "";
        mImplementationComputeRatesMethodWoevString = "\n"
                                                      "def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables):\n"
                                                      "[CODE]";

        mInterfaceComputeRatesMethodWevString = "";
        mImplementationComputeRatesMethodWevString = "\n"
                                                     "def compute_rates(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, external_variable):\n"
                                                     "[CODE]";

        mInterfaceComputeVariablesMethodFamWoevString = "";
        mImplementationComputeVariablesMethodFamWoevString = "\n"
                                                             "def compute_variables(constants, computed_constants, algebraic_variables):\n"
                                                             "[CODE]";

        mInterfaceComputeVariablesMethodFamWevString = "";
        mImplementationComputeVariablesMethodFamWevString = "\n"
                                                            "def compute_variables(constants, computed_constants, algebraic_variables, external_variables, external_variable):\n"
                                                            "[CODE]";

        mInterfaceComputeVariablesMethodFdmWoevString = "";
        mImplementationComputeVariablesMethodFdmWoevString = "\n"
                                                             "def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables):\n"
                                                             "[CODE]";

        mInterfaceComputeVariablesMethodFdmWevString = "";
        mImplementationComputeVariablesMethodFdmWevString = "\n"
                                                            "def compute_variables(voi, states, rates, constants, computed_constants, algebraic_variables, external_variables, external_variable):\n"
                                                            "[CODE]";

        mEmptyMethodString = "pass\n";

        mIndentString = "    ";

        mVariableDeclarationString = "[CODE]";

        mOpenArrayString = "[";
        mCloseArrayString = "]";

        mArrayElementSeparatorString = ",";

        mCommandSeparatorString = "";
    }
}

GeneratorProfile::GeneratorProfile(Profile profile)
    : mPimpl(new GeneratorProfileImpl())
{
    setProfile(profile);
}

GeneratorProfile::~GeneratorProfile()
{
    delete mPimpl;
}

GeneratorProfilePtr GeneratorProfile::create(Profile profile) noexcept
{
    return std::shared_ptr<GeneratorProfile> {new GeneratorProfile {profile}};
}

GeneratorProfile::Profile GeneratorProfile::profile() const
{
    return mPimpl->mProfile;
}

static const std::map<GeneratorProfile::Profile, std::string> profileToString = {
    {GeneratorProfile::Profile::C, "c"},
    {GeneratorProfile::Profile::PYTHON, "python"}};

std::string GeneratorProfile::profileAsString(Profile profile)
{
    return profileToString.at(profile);
}

void GeneratorProfile::setProfile(Profile profile)
{
    mPimpl->loadProfile(profile);
}

bool GeneratorProfile::hasInterface() const
{
    return mPimpl->mHasInterface;
}

void GeneratorProfile::setHasInterface(bool hasInterface)
{
    mPimpl->mHasInterface = hasInterface;
}

std::string GeneratorProfile::equalityString() const
{
    return mPimpl->mEqualityString;
}

void GeneratorProfile::setEqualityString(const std::string &equalityString)
{
    mPimpl->mEqualityString = equalityString;
}

std::string GeneratorProfile::eqString() const
{
    return mPimpl->mEqString;
}

void GeneratorProfile::setEqString(const std::string &eqString)
{
    mPimpl->mEqString = eqString;
}

std::string GeneratorProfile::neqString() const
{
    return mPimpl->mNeqString;
}

void GeneratorProfile::setNeqString(const std::string &neqString)
{
    mPimpl->mNeqString = neqString;
}

std::string GeneratorProfile::ltString() const
{
    return mPimpl->mLtString;
}

void GeneratorProfile::setLtString(const std::string &ltString)
{
    mPimpl->mLtString = ltString;
}

std::string GeneratorProfile::leqString() const
{
    return mPimpl->mLeqString;
}

void GeneratorProfile::setLeqString(const std::string &leqString)
{
    mPimpl->mLeqString = leqString;
}

std::string GeneratorProfile::gtString() const
{
    return mPimpl->mGtString;
}

void GeneratorProfile::setGtString(const std::string &gtString)
{
    mPimpl->mGtString = gtString;
}

std::string GeneratorProfile::geqString() const
{
    return mPimpl->mGeqString;
}

void GeneratorProfile::setGeqString(const std::string &geqString)
{
    mPimpl->mGeqString = geqString;
}

std::string GeneratorProfile::andString() const
{
    return mPimpl->mAndString;
}

void GeneratorProfile::setAndString(const std::string &andString)
{
    mPimpl->mAndString = andString;
}

std::string GeneratorProfile::orString() const
{
    return mPimpl->mOrString;
}

void GeneratorProfile::setOrString(const std::string &orString)
{
    mPimpl->mOrString = orString;
}

std::string GeneratorProfile::xorString() const
{
    return mPimpl->mXorString;
}

void GeneratorProfile::setXorString(const std::string &xorString)
{
    mPimpl->mXorString = xorString;
}

std::string GeneratorProfile::notString() const
{
    return mPimpl->mNotString;
}

void GeneratorProfile::setNotString(const std::string &notString)
{
    mPimpl->mNotString = notString;
}

bool GeneratorProfile::hasEqOperator() const
{
    return mPimpl->mHasEqOperator;
}

void GeneratorProfile::setHasEqOperator(bool hasEqOperator)
{
    mPimpl->mHasEqOperator = hasEqOperator;
}

bool GeneratorProfile::hasNeqOperator() const
{
    return mPimpl->mHasNeqOperator;
}

void GeneratorProfile::setHasNeqOperator(bool hasNeqOperator)
{
    mPimpl->mHasNeqOperator = hasNeqOperator;
}

bool GeneratorProfile::hasLtOperator() const
{
    return mPimpl->mHasLtOperator;
}

void GeneratorProfile::setHasLtOperator(bool hasLtOperator)
{
    mPimpl->mHasLtOperator = hasLtOperator;
}

bool GeneratorProfile::hasLeqOperator() const
{
    return mPimpl->mHasLeqOperator;
}

void GeneratorProfile::setHasLeqOperator(bool hasLeqOperator)
{
    mPimpl->mHasLeqOperator = hasLeqOperator;
}

bool GeneratorProfile::hasGtOperator() const
{
    return mPimpl->mHasGtOperator;
}

void GeneratorProfile::setHasGtOperator(bool hasGtOperator)
{
    mPimpl->mHasGtOperator = hasGtOperator;
}

bool GeneratorProfile::hasGeqOperator() const
{
    return mPimpl->mHasGeqOperator;
}

void GeneratorProfile::setHasGeqOperator(bool hasGeqOperator)
{
    mPimpl->mHasGeqOperator = hasGeqOperator;
}

bool GeneratorProfile::hasAndOperator() const
{
    return mPimpl->mHasAndOperator;
}

void GeneratorProfile::setHasAndOperator(bool hasAndOperator)
{
    mPimpl->mHasAndOperator = hasAndOperator;
}

bool GeneratorProfile::hasOrOperator() const
{
    return mPimpl->mHasOrOperator;
}

void GeneratorProfile::setHasOrOperator(bool hasOrOperator)
{
    mPimpl->mHasOrOperator = hasOrOperator;
}

bool GeneratorProfile::hasXorOperator() const
{
    return mPimpl->mHasXorOperator;
}

void GeneratorProfile::setHasXorOperator(bool hasXorOperator)
{
    mPimpl->mHasXorOperator = hasXorOperator;
}

bool GeneratorProfile::hasNotOperator() const
{
    return mPimpl->mHasNotOperator;
}

void GeneratorProfile::setHasNotOperator(bool hasNotOperator)
{
    mPimpl->mHasNotOperator = hasNotOperator;
}

std::string GeneratorProfile::plusString() const
{
    return mPimpl->mPlusString;
}

void GeneratorProfile::setPlusString(const std::string &plusString)
{
    mPimpl->mPlusString = plusString;
}

std::string GeneratorProfile::minusString() const
{
    return mPimpl->mMinusString;
}

void GeneratorProfile::setMinusString(const std::string &minusString)
{
    mPimpl->mMinusString = minusString;
}

std::string GeneratorProfile::timesString() const
{
    return mPimpl->mTimesString;
}

void GeneratorProfile::setTimesString(const std::string &timesString)
{
    mPimpl->mTimesString = timesString;
}

std::string GeneratorProfile::divideString() const
{
    return mPimpl->mDivideString;
}

void GeneratorProfile::setDivideString(const std::string &divideString)
{
    mPimpl->mDivideString = divideString;
}

std::string GeneratorProfile::powerString() const
{
    return mPimpl->mPowerString;
}

void GeneratorProfile::setPowerString(const std::string &powerString)
{
    mPimpl->mPowerString = powerString;
}

std::string GeneratorProfile::squareRootString() const
{
    return mPimpl->mSquareRootString;
}

void GeneratorProfile::setSquareRootString(const std::string &squareRootString)
{
    mPimpl->mSquareRootString = squareRootString;
}

std::string GeneratorProfile::squareString() const
{
    return mPimpl->mSquareString;
}

void GeneratorProfile::setSquareString(const std::string &squareString)
{
    mPimpl->mSquareString = squareString;
}

std::string GeneratorProfile::absoluteValueString() const
{
    return mPimpl->mAbsoluteValueString;
}

void GeneratorProfile::setAbsoluteValueString(const std::string &absoluteValueString)
{
    mPimpl->mAbsoluteValueString = absoluteValueString;
}

std::string GeneratorProfile::exponentialString() const
{
    return mPimpl->mExponentialString;
}

void GeneratorProfile::setExponentialString(const std::string &exponentialString)
{
    mPimpl->mExponentialString = exponentialString;
}

std::string GeneratorProfile::naturalLogarithmString() const
{
    return mPimpl->mNaturalLogarithmString;
}

void GeneratorProfile::setNaturalLogarithmString(const std::string &naturalLogarithmString)
{
    mPimpl->mNaturalLogarithmString = naturalLogarithmString;
}

std::string GeneratorProfile::commonLogarithmString() const
{
    return mPimpl->mCommonLogarithmString;
}

void GeneratorProfile::setCommonLogarithmString(const std::string &commonLogarithmString)
{
    mPimpl->mCommonLogarithmString = commonLogarithmString;
}

std::string GeneratorProfile::ceilingString() const
{
    return mPimpl->mCeilingString;
}

void GeneratorProfile::setCeilingString(const std::string &ceilingString)
{
    mPimpl->mCeilingString = ceilingString;
}

std::string GeneratorProfile::floorString() const
{
    return mPimpl->mFloorString;
}

void GeneratorProfile::setFloorString(const std::string &floorString)
{
    mPimpl->mFloorString = floorString;
}

std::string GeneratorProfile::minString() const
{
    return mPimpl->mMinString;
}

void GeneratorProfile::setMinString(const std::string &minString)
{
    mPimpl->mMinString = minString;
}

std::string GeneratorProfile::maxString() const
{
    return mPimpl->mMaxString;
}

void GeneratorProfile::setMaxString(const std::string &maxString)
{
    mPimpl->mMaxString = maxString;
}

std::string GeneratorProfile::remString() const
{
    return mPimpl->mRemString;
}

void GeneratorProfile::setRemString(const std::string &remString)
{
    mPimpl->mRemString = remString;
}

bool GeneratorProfile::hasPowerOperator() const
{
    return mPimpl->mHasPowerOperator;
}

void GeneratorProfile::setHasPowerOperator(bool hasPowerOperator)
{
    mPimpl->mHasPowerOperator = hasPowerOperator;
}

std::string GeneratorProfile::sinString() const
{
    return mPimpl->mSinString;
}

void GeneratorProfile::setSinString(const std::string &sinString)
{
    mPimpl->mSinString = sinString;
}

std::string GeneratorProfile::cosString() const
{
    return mPimpl->mCosString;
}

void GeneratorProfile::setCosString(const std::string &cosString)
{
    mPimpl->mCosString = cosString;
}

std::string GeneratorProfile::tanString() const
{
    return mPimpl->mTanString;
}

void GeneratorProfile::setTanString(const std::string &tanString)
{
    mPimpl->mTanString = tanString;
}

std::string GeneratorProfile::secString() const
{
    return mPimpl->mSecString;
}

void GeneratorProfile::setSecString(const std::string &secString)
{
    mPimpl->mSecString = secString;
}

std::string GeneratorProfile::cscString() const
{
    return mPimpl->mCscString;
}

void GeneratorProfile::setCscString(const std::string &cscString)
{
    mPimpl->mCscString = cscString;
}

std::string GeneratorProfile::cotString() const
{
    return mPimpl->mCotString;
}

void GeneratorProfile::setCotString(const std::string &cotString)
{
    mPimpl->mCotString = cotString;
}

std::string GeneratorProfile::sinhString() const
{
    return mPimpl->mSinhString;
}

void GeneratorProfile::setSinhString(const std::string &sinhString)
{
    mPimpl->mSinhString = sinhString;
}

std::string GeneratorProfile::coshString() const
{
    return mPimpl->mCoshString;
}

void GeneratorProfile::setCoshString(const std::string &coshString)
{
    mPimpl->mCoshString = coshString;
}

std::string GeneratorProfile::tanhString() const
{
    return mPimpl->mTanhString;
}

void GeneratorProfile::setTanhString(const std::string &tanhString)
{
    mPimpl->mTanhString = tanhString;
}

std::string GeneratorProfile::sechString() const
{
    return mPimpl->mSechString;
}

void GeneratorProfile::setSechString(const std::string &sechString)
{
    mPimpl->mSechString = sechString;
}

std::string GeneratorProfile::cschString() const
{
    return mPimpl->mCschString;
}

void GeneratorProfile::setCschString(const std::string &cschString)
{
    mPimpl->mCschString = cschString;
}

std::string GeneratorProfile::cothString() const
{
    return mPimpl->mCothString;
}

void GeneratorProfile::setCothString(const std::string &cothString)
{
    mPimpl->mCothString = cothString;
}

std::string GeneratorProfile::asinString() const
{
    return mPimpl->mAsinString;
}

void GeneratorProfile::setAsinString(const std::string &asinString)
{
    mPimpl->mAsinString = asinString;
}

std::string GeneratorProfile::acosString() const
{
    return mPimpl->mAcosString;
}

void GeneratorProfile::setAcosString(const std::string &acosString)
{
    mPimpl->mAcosString = acosString;
}

std::string GeneratorProfile::atanString() const
{
    return mPimpl->mAtanString;
}

void GeneratorProfile::setAtanString(const std::string &atanString)
{
    mPimpl->mAtanString = atanString;
}

std::string GeneratorProfile::asecString() const
{
    return mPimpl->mAsecString;
}

void GeneratorProfile::setAsecString(const std::string &asecString)
{
    mPimpl->mAsecString = asecString;
}

std::string GeneratorProfile::acscString() const
{
    return mPimpl->mAcscString;
}

void GeneratorProfile::setAcscString(const std::string &acscString)
{
    mPimpl->mAcscString = acscString;
}

std::string GeneratorProfile::acotString() const
{
    return mPimpl->mAcotString;
}

void GeneratorProfile::setAcotString(const std::string &acotString)
{
    mPimpl->mAcotString = acotString;
}

std::string GeneratorProfile::asinhString() const
{
    return mPimpl->mAsinhString;
}

void GeneratorProfile::setAsinhString(const std::string &asinhString)
{
    mPimpl->mAsinhString = asinhString;
}

std::string GeneratorProfile::acoshString() const
{
    return mPimpl->mAcoshString;
}

void GeneratorProfile::setAcoshString(const std::string &acoshString)
{
    mPimpl->mAcoshString = acoshString;
}

std::string GeneratorProfile::atanhString() const
{
    return mPimpl->mAtanhString;
}

void GeneratorProfile::setAtanhString(const std::string &atanhString)
{
    mPimpl->mAtanhString = atanhString;
}

std::string GeneratorProfile::asechString() const
{
    return mPimpl->mAsechString;
}

void GeneratorProfile::setAsechString(const std::string &asechString)
{
    mPimpl->mAsechString = asechString;
}

std::string GeneratorProfile::acschString() const
{
    return mPimpl->mAcschString;
}

void GeneratorProfile::setAcschString(const std::string &acschString)
{
    mPimpl->mAcschString = acschString;
}

std::string GeneratorProfile::acothString() const
{
    return mPimpl->mAcothString;
}

void GeneratorProfile::setAcothString(const std::string &acothString)
{
    mPimpl->mAcothString = acothString;
}

std::string GeneratorProfile::conditionalOperatorIfString() const
{
    return mPimpl->mConditionalOperatorIfString;
}

void GeneratorProfile::setConditionalOperatorIfString(const std::string &conditionalOperatorIfString)
{
    mPimpl->mConditionalOperatorIfString = conditionalOperatorIfString;
}

std::string GeneratorProfile::conditionalOperatorElseString() const
{
    return mPimpl->mConditionalOperatorElseString;
}

void GeneratorProfile::setConditionalOperatorElseString(const std::string &conditionalOperatorElseString)
{
    mPimpl->mConditionalOperatorElseString = conditionalOperatorElseString;
}

std::string GeneratorProfile::piecewiseIfString() const
{
    return mPimpl->mPiecewiseIfString;
}

void GeneratorProfile::setPiecewiseIfString(const std::string &piecewiseIfString)
{
    mPimpl->mPiecewiseIfString = piecewiseIfString;
}

std::string GeneratorProfile::piecewiseElseString() const
{
    return mPimpl->mPiecewiseElseString;
}

void GeneratorProfile::setPiecewiseElseString(const std::string &piecewiseElseString)
{
    mPimpl->mPiecewiseElseString = piecewiseElseString;
}

bool GeneratorProfile::hasConditionalOperator() const
{
    return mPimpl->mHasConditionalOperator;
}

void GeneratorProfile::setHasConditionalOperator(bool hasConditionalOperator)
{
    mPimpl->mHasConditionalOperator = hasConditionalOperator;
}

std::string GeneratorProfile::trueString() const
{
    return mPimpl->mTrueString;
}

void GeneratorProfile::setTrueString(const std::string &trueString)
{
    mPimpl->mTrueString = trueString;
}

std::string GeneratorProfile::falseString() const
{
    return mPimpl->mFalseString;
}

void GeneratorProfile::setFalseString(const std::string &falseString)
{
    mPimpl->mFalseString = falseString;
}

std::string GeneratorProfile::eString() const
{
    return mPimpl->mEString;
}

void GeneratorProfile::setEString(const std::string &eString)
{
    mPimpl->mEString = eString;
}

std::string GeneratorProfile::piString() const
{
    return mPimpl->mPiString;
}

void GeneratorProfile::setPiString(const std::string &piString)
{
    mPimpl->mPiString = piString;
}

std::string GeneratorProfile::infString() const
{
    return mPimpl->mInfString;
}

void GeneratorProfile::setInfString(const std::string &infString)
{
    mPimpl->mInfString = infString;
}

std::string GeneratorProfile::nanString() const
{
    return mPimpl->mNanString;
}

void GeneratorProfile::setNanString(const std::string &nanString)
{
    mPimpl->mNanString = nanString;
}

std::string GeneratorProfile::eqFunctionString() const
{
    return mPimpl->mEqFunctionString;
}

void GeneratorProfile::setEqFunctionString(const std::string &eqFunctionString)
{
    mPimpl->mEqFunctionString = eqFunctionString;
}

std::string GeneratorProfile::neqFunctionString() const
{
    return mPimpl->mNeqFunctionString;
}

void GeneratorProfile::setNeqFunctionString(const std::string &neqFunctionString)
{
    mPimpl->mNeqFunctionString = neqFunctionString;
}

std::string GeneratorProfile::ltFunctionString() const
{
    return mPimpl->mLtFunctionString;
}

void GeneratorProfile::setLtFunctionString(const std::string &ltFunctionString)
{
    mPimpl->mLtFunctionString = ltFunctionString;
}

std::string GeneratorProfile::leqFunctionString() const
{
    return mPimpl->mLeqFunctionString;
}

void GeneratorProfile::setLeqFunctionString(const std::string &leqFunctionString)
{
    mPimpl->mLeqFunctionString = leqFunctionString;
}

std::string GeneratorProfile::gtFunctionString() const
{
    return mPimpl->mGtFunctionString;
}

void GeneratorProfile::setGtFunctionString(const std::string &gtFunctionString)
{
    mPimpl->mGtFunctionString = gtFunctionString;
}

std::string GeneratorProfile::geqFunctionString() const
{
    return mPimpl->mGeqFunctionString;
}

void GeneratorProfile::setGeqFunctionString(const std::string &geqFunctionString)
{
    mPimpl->mGeqFunctionString = geqFunctionString;
}

std::string GeneratorProfile::andFunctionString() const
{
    return mPimpl->mAndFunctionString;
}

void GeneratorProfile::setAndFunctionString(const std::string &andFunctionString)
{
    mPimpl->mAndFunctionString = andFunctionString;
}

std::string GeneratorProfile::orFunctionString() const
{
    return mPimpl->mOrFunctionString;
}

void GeneratorProfile::setOrFunctionString(const std::string &orFunctionString)
{
    mPimpl->mOrFunctionString = orFunctionString;
}

std::string GeneratorProfile::xorFunctionString() const
{
    return mPimpl->mXorFunctionString;
}

void GeneratorProfile::setXorFunctionString(const std::string &xorFunctionString)
{
    mPimpl->mXorFunctionString = xorFunctionString;
}

std::string GeneratorProfile::notFunctionString() const
{
    return mPimpl->mNotFunctionString;
}

void GeneratorProfile::setNotFunctionString(const std::string &notFunctionString)
{
    mPimpl->mNotFunctionString = notFunctionString;
}

std::string GeneratorProfile::minFunctionString() const
{
    return mPimpl->mMinFunctionString;
}

void GeneratorProfile::setMinFunctionString(const std::string &minFunctionString)
{
    mPimpl->mMinFunctionString = minFunctionString;
}

std::string GeneratorProfile::maxFunctionString() const
{
    return mPimpl->mMaxFunctionString;
}

void GeneratorProfile::setMaxFunctionString(const std::string &maxFunctionString)
{
    mPimpl->mMaxFunctionString = maxFunctionString;
}

std::string GeneratorProfile::secFunctionString() const
{
    return mPimpl->mSecFunctionString;
}

void GeneratorProfile::setSecFunctionString(const std::string &secFunctionString)
{
    mPimpl->mSecFunctionString = secFunctionString;
}

std::string GeneratorProfile::cscFunctionString() const
{
    return mPimpl->mCscFunctionString;
}

void GeneratorProfile::setCscFunctionString(const std::string &cscFunctionString)
{
    mPimpl->mCscFunctionString = cscFunctionString;
}

std::string GeneratorProfile::cotFunctionString() const
{
    return mPimpl->mCotFunctionString;
}

void GeneratorProfile::setCotFunctionString(const std::string &cotFunctionString)
{
    mPimpl->mCotFunctionString = cotFunctionString;
}

std::string GeneratorProfile::sechFunctionString() const
{
    return mPimpl->mSechFunctionString;
}

void GeneratorProfile::setSechFunctionString(const std::string &sechFunctionString)
{
    mPimpl->mSechFunctionString = sechFunctionString;
}

std::string GeneratorProfile::cschFunctionString() const
{
    return mPimpl->mCschFunctionString;
}

void GeneratorProfile::setCschFunctionString(const std::string &cschFunctionString)
{
    mPimpl->mCschFunctionString = cschFunctionString;
}

std::string GeneratorProfile::cothFunctionString() const
{
    return mPimpl->mCothFunctionString;
}

void GeneratorProfile::setCothFunctionString(const std::string &cothFunctionString)
{
    mPimpl->mCothFunctionString = cothFunctionString;
}

std::string GeneratorProfile::asecFunctionString() const
{
    return mPimpl->mAsecFunctionString;
}

void GeneratorProfile::setAsecFunctionString(const std::string &asecFunctionString)
{
    mPimpl->mAsecFunctionString = asecFunctionString;
}

std::string GeneratorProfile::acscFunctionString() const
{
    return mPimpl->mAcscFunctionString;
}

void GeneratorProfile::setAcscFunctionString(const std::string &acscFunctionString)
{
    mPimpl->mAcscFunctionString = acscFunctionString;
}

std::string GeneratorProfile::acotFunctionString() const
{
    return mPimpl->mAcotFunctionString;
}

void GeneratorProfile::setAcotFunctionString(const std::string &acotFunctionString)
{
    mPimpl->mAcotFunctionString = acotFunctionString;
}

std::string GeneratorProfile::asechFunctionString() const
{
    return mPimpl->mAsechFunctionString;
}

void GeneratorProfile::setAsechFunctionString(const std::string &asechFunctionString)
{
    mPimpl->mAsechFunctionString = asechFunctionString;
}

std::string GeneratorProfile::acschFunctionString() const
{
    return mPimpl->mAcschFunctionString;
}

void GeneratorProfile::setAcschFunctionString(const std::string &acschFunctionString)
{
    mPimpl->mAcschFunctionString = acschFunctionString;
}

std::string GeneratorProfile::acothFunctionString() const
{
    return mPimpl->mAcothFunctionString;
}

void GeneratorProfile::setAcothFunctionString(const std::string &acothFunctionString)
{
    mPimpl->mAcothFunctionString = acothFunctionString;
}

std::string GeneratorProfile::commentString() const
{
    return mPimpl->mCommentString;
}

void GeneratorProfile::setCommentString(const std::string &commentString)
{
    mPimpl->mCommentString = commentString;
}

std::string GeneratorProfile::originCommentString() const
{
    return mPimpl->mOriginCommentString;
}

void GeneratorProfile::setOriginCommentString(const std::string &originCommentString)
{
    mPimpl->mOriginCommentString = originCommentString;
}

std::string GeneratorProfile::interfaceFileNameString() const
{
    return mPimpl->mInterfaceFileNameString;
}

void GeneratorProfile::setInterfaceFileNameString(const std::string &interfaceFileNameString)
{
    mPimpl->mInterfaceFileNameString = interfaceFileNameString;
}

std::string GeneratorProfile::interfaceHeaderString() const
{
    return mPimpl->mInterfaceHeaderString;
}

void GeneratorProfile::setInterfaceHeaderString(const std::string &interfaceHeaderString)
{
    mPimpl->mInterfaceHeaderString = interfaceHeaderString;
}

std::string GeneratorProfile::implementationHeaderString() const
{
    return mPimpl->mImplementationHeaderString;
}

void GeneratorProfile::setImplementationHeaderString(const std::string &implementationHeaderString)
{
    mPimpl->mImplementationHeaderString = implementationHeaderString;
}

std::string GeneratorProfile::interfaceVersionString() const
{
    return mPimpl->mInterfaceVersionString;
}

void GeneratorProfile::setInterfaceVersionString(const std::string &interfaceVersionString)
{
    mPimpl->mInterfaceVersionString = interfaceVersionString;
}

std::string GeneratorProfile::implementationVersionString() const
{
    return mPimpl->mImplementationVersionString;
}

void GeneratorProfile::setImplementationVersionString(const std::string &implementationVersionString)
{
    mPimpl->mImplementationVersionString = implementationVersionString;
}

std::string GeneratorProfile::interfaceLibcellmlVersionString() const
{
    return mPimpl->mInterfaceLibcellmlVersionString;
}

void GeneratorProfile::setInterfaceLibcellmlVersionString(const std::string &interfaceLibcellmlVersionString)
{
    mPimpl->mInterfaceLibcellmlVersionString = interfaceLibcellmlVersionString;
}

std::string GeneratorProfile::implementationLibcellmlVersionString() const
{
    return mPimpl->mImplementationLibcellmlVersionString;
}

void GeneratorProfile::setImplementationLibcellmlVersionString(const std::string &implementationLibcellmlVersionString)
{
    mPimpl->mImplementationLibcellmlVersionString = implementationLibcellmlVersionString;
}

std::string GeneratorProfile::interfaceStateCountString() const
{
    return mPimpl->mInterfaceStateCountString;
}

void GeneratorProfile::setInterfaceStateCountString(const std::string &interfaceStateCountString)
{
    mPimpl->mInterfaceStateCountString = interfaceStateCountString;
}

std::string GeneratorProfile::implementationStateCountString() const
{
    return mPimpl->mImplementationStateCountString;
}

void GeneratorProfile::setImplementationStateCountString(const std::string &implementationStateCountString)
{
    mPimpl->mImplementationStateCountString = implementationStateCountString;
}

std::string GeneratorProfile::interfaceConstantCountString() const
{
    return mPimpl->mInterfaceConstantCountString;
}

void GeneratorProfile::setInterfaceConstantCountString(const std::string &interfaceConstantCountString)
{
    mPimpl->mInterfaceConstantCountString = interfaceConstantCountString;
}

std::string GeneratorProfile::implementationConstantCountString() const
{
    return mPimpl->mImplementationConstantCountString;
}

void GeneratorProfile::setImplementationConstantCountString(const std::string &implementationConstantCountString)
{
    mPimpl->mImplementationConstantCountString = implementationConstantCountString;
}

std::string GeneratorProfile::interfaceComputedConstantCountString() const
{
    return mPimpl->mInterfaceComputedConstantCountString;
}

void GeneratorProfile::setInterfaceComputedConstantCountString(const std::string &interfaceComputedConstantCountString)
{
    mPimpl->mInterfaceComputedConstantCountString = interfaceComputedConstantCountString;
}

std::string GeneratorProfile::implementationComputedConstantCountString() const
{
    return mPimpl->mImplementationComputedConstantCountString;
}

void GeneratorProfile::setImplementationComputedConstantCountString(const std::string &implementationComputedConstantCountString)
{
    mPimpl->mImplementationComputedConstantCountString = implementationComputedConstantCountString;
}

std::string GeneratorProfile::interfaceAlgebraicVariableCountString() const
{
    return mPimpl->mInterfaceAlgebraicVariableCountString;
}

void GeneratorProfile::setInterfaceAlgebraicVariableCountString(const std::string &interfaceAlgebraicVariableCountString)
{
    mPimpl->mInterfaceAlgebraicVariableCountString = interfaceAlgebraicVariableCountString;
}

std::string GeneratorProfile::implementationAlgebraicVariableCountString() const
{
    return mPimpl->mImplementationAlgebraicVariableCountString;
}

void GeneratorProfile::setImplementationAlgebraicVariableCountString(const std::string &implementationAlgebraicVariableCountString)
{
    mPimpl->mImplementationAlgebraicVariableCountString = implementationAlgebraicVariableCountString;
}

std::string GeneratorProfile::interfaceExternalVariableCountString() const
{
    return mPimpl->mInterfaceExternalVariableCountString;
}

void GeneratorProfile::setInterfaceExternalVariableCountString(const std::string &interfaceExternalVariableCountString)
{
    mPimpl->mInterfaceExternalVariableCountString = interfaceExternalVariableCountString;
}

std::string GeneratorProfile::implementationExternalVariableCountString() const
{
    return mPimpl->mImplementationExternalVariableCountString;
}

void GeneratorProfile::setImplementationExternalVariableCountString(const std::string &implementationExternalVariableCountString)
{
    mPimpl->mImplementationExternalVariableCountString = implementationExternalVariableCountString;
}

std::string GeneratorProfile::variableInfoObjectString() const
{
    return mPimpl->mVariableInfoObjectString;
}

void GeneratorProfile::setVariableInfoObjectString(const std::string &variableInfoObjectString)
{
    mPimpl->mVariableInfoObjectString = variableInfoObjectString;
}

std::string GeneratorProfile::interfaceVoiInfoString() const
{
    return mPimpl->mInterfaceVoiInfoString;
}

void GeneratorProfile::setInterfaceVoiInfoString(const std::string &interfaceVoiInfoString)
{
    mPimpl->mInterfaceVoiInfoString = interfaceVoiInfoString;
}

std::string GeneratorProfile::implementationVoiInfoString() const
{
    return mPimpl->mImplementationVoiInfoString;
}

void GeneratorProfile::setImplementationVoiInfoString(const std::string &implementationVoiInfoString)
{
    mPimpl->mImplementationVoiInfoString = implementationVoiInfoString;
}

std::string GeneratorProfile::interfaceStateInfoString() const
{
    return mPimpl->mInterfaceStateInfoString;
}

void GeneratorProfile::setInterfaceStateInfoString(const std::string &interfaceStateInfoString)
{
    mPimpl->mInterfaceStateInfoString = interfaceStateInfoString;
}

std::string GeneratorProfile::implementationStateInfoString() const
{
    return mPimpl->mImplementationStateInfoString;
}

void GeneratorProfile::setImplementationStateInfoString(const std::string &implementationStateInfoString)
{
    mPimpl->mImplementationStateInfoString = implementationStateInfoString;
}

std::string GeneratorProfile::interfaceConstantInfoString() const
{
    return mPimpl->mInterfaceConstantInfoString;
}

void GeneratorProfile::setInterfaceConstantInfoString(const std::string &interfaceConstantInfoString)
{
    mPimpl->mInterfaceConstantInfoString = interfaceConstantInfoString;
}

std::string GeneratorProfile::implementationConstantInfoString() const
{
    return mPimpl->mImplementationConstantInfoString;
}

void GeneratorProfile::setImplementationConstantInfoString(const std::string &implementationConstantInfoString)
{
    mPimpl->mImplementationConstantInfoString = implementationConstantInfoString;
}

std::string GeneratorProfile::interfaceComputedConstantInfoString() const
{
    return mPimpl->mInterfaceComputedConstantInfoString;
}

void GeneratorProfile::setInterfaceComputedConstantInfoString(const std::string &interfaceComputedConstantInfoString)
{
    mPimpl->mInterfaceComputedConstantInfoString = interfaceComputedConstantInfoString;
}

std::string GeneratorProfile::implementationComputedConstantInfoString() const
{
    return mPimpl->mImplementationComputedConstantInfoString;
}

void GeneratorProfile::setImplementationComputedConstantInfoString(const std::string &implementationComputedConstantInfoString)
{
    mPimpl->mImplementationComputedConstantInfoString = implementationComputedConstantInfoString;
}

std::string GeneratorProfile::interfaceAlgebraicVariableInfoString() const
{
    return mPimpl->mInterfaceAlgebraicVariableInfoString;
}

void GeneratorProfile::setInterfaceAlgebraicVariableInfoString(const std::string &interfaceAlgebraicVariableInfoString)
{
    mPimpl->mInterfaceAlgebraicVariableInfoString = interfaceAlgebraicVariableInfoString;
}

std::string GeneratorProfile::implementationAlgebraicVariableInfoString() const
{
    return mPimpl->mImplementationAlgebraicVariableInfoString;
}

void GeneratorProfile::setImplementationAlgebraicVariableInfoString(const std::string &implementationAlgebraicVariableInfoString)
{
    mPimpl->mImplementationAlgebraicVariableInfoString = implementationAlgebraicVariableInfoString;
}

std::string GeneratorProfile::interfaceExternalVariableInfoString() const
{
    return mPimpl->mInterfaceExternalVariableInfoString;
}

void GeneratorProfile::setInterfaceExternalVariableInfoString(const std::string &interfaceExternalVariableInfoString)
{
    mPimpl->mInterfaceExternalVariableInfoString = interfaceExternalVariableInfoString;
}

std::string GeneratorProfile::implementationExternalVariableInfoString() const
{
    return mPimpl->mImplementationExternalVariableInfoString;
}

void GeneratorProfile::setImplementationExternalVariableInfoString(const std::string &implementationExternalVariableInfoString)
{
    mPimpl->mImplementationExternalVariableInfoString = implementationExternalVariableInfoString;
}

std::string GeneratorProfile::variableInfoEntryString() const
{
    return mPimpl->mVariableInfoEntryString;
}

void GeneratorProfile::setVariableInfoEntryString(const std::string &variableInfoEntryString)
{
    mPimpl->mVariableInfoEntryString = variableInfoEntryString;
}

std::string GeneratorProfile::voiString() const
{
    return mPimpl->mVoiString;
}

void GeneratorProfile::setVoiString(const std::string &voiString)
{
    mPimpl->mVoiString = voiString;
}

std::string GeneratorProfile::statesArrayString() const
{
    return mPimpl->mStatesArrayString;
}

void GeneratorProfile::setStatesArrayString(const std::string &statesArrayString)
{
    mPimpl->mStatesArrayString = statesArrayString;
}

std::string GeneratorProfile::ratesArrayString() const
{
    return mPimpl->mRatesArrayString;
}

void GeneratorProfile::setRatesArrayString(const std::string &ratesArrayString)
{
    mPimpl->mRatesArrayString = ratesArrayString;
}

std::string GeneratorProfile::constantsArrayString() const
{
    return mPimpl->mConstantsArrayString;
}

void GeneratorProfile::setConstantsArrayString(const std::string &constantsArrayString)
{
    mPimpl->mConstantsArrayString = constantsArrayString;
}

std::string GeneratorProfile::computedConstantsArrayString() const
{
    return mPimpl->mComputedConstantsArrayString;
}

void GeneratorProfile::setComputedConstantsArrayString(const std::string &computedConstantsArrayString)
{
    mPimpl->mComputedConstantsArrayString = computedConstantsArrayString;
}

std::string GeneratorProfile::algebraicVariablesArrayString() const
{
    return mPimpl->mAlgebraicVariablesArrayString;
}

void GeneratorProfile::setAlgebraicVariablesArrayString(const std::string &algebraicVariablesArrayString)
{
    mPimpl->mAlgebraicVariablesArrayString = algebraicVariablesArrayString;
}

std::string GeneratorProfile::externalVariablesArrayString() const
{
    return mPimpl->mExternalVariablesArrayString;
}

void GeneratorProfile::setExternalVariablesArrayString(const std::string &externalVariablesArrayString)
{
    mPimpl->mExternalVariablesArrayString = externalVariablesArrayString;
}

std::string GeneratorProfile::externalVariableMethodTypeDefinitionString(bool forDifferentialModel) const
{
    if (forDifferentialModel) {
        return mPimpl->mExternalVariableMethodTypeDefinitionDiffString;
    }

    return mPimpl->mExternalVariableMethodTypeDefinitionNonDiffString;
}

void GeneratorProfile::setExternalVariableMethodTypeDefinitionString(bool forDifferentialModel,
                                                                     const std::string &externalVariableMethodTypeDefinitionString)
{
    if (forDifferentialModel) {
        mPimpl->mExternalVariableMethodTypeDefinitionDiffString = externalVariableMethodTypeDefinitionString;
    } else {
        mPimpl->mExternalVariableMethodTypeDefinitionNonDiffString = externalVariableMethodTypeDefinitionString;
    }
}

std::string GeneratorProfile::externalVariableMethodCallString(bool forDifferentialModel) const
{
    if (forDifferentialModel) {
        return mPimpl->mExternalVariableMethodCallDiffString;
    }

    return mPimpl->mExternalVariableMethodCallNonDiffString;
}

void GeneratorProfile::setExternalVariableMethodCallString(bool forDifferentialModel,
                                                           const std::string &externalVariableMethodCallString)
{
    if (forDifferentialModel) {
        mPimpl->mExternalVariableMethodCallDiffString = externalVariableMethodCallString;
    } else {
        mPimpl->mExternalVariableMethodCallNonDiffString = externalVariableMethodCallString;
    }
}

std::string GeneratorProfile::rootFindingInfoObjectString(bool forDifferentialModel,
                                                          bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mRootFindingInfoObjectFdmWevString;
        }

        return mPimpl->mRootFindingInfoObjectFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mRootFindingInfoObjectFamWevString;
    }

    return mPimpl->mRootFindingInfoObjectFamWoevString;
}

void GeneratorProfile::setRootFindingInfoObjectString(bool forDifferentialModel,
                                                      bool withExternalVariables,
                                                      const std::string &rootFindingInfoObjectString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mRootFindingInfoObjectFdmWevString = rootFindingInfoObjectString;
        } else {
            mPimpl->mRootFindingInfoObjectFdmWoevString = rootFindingInfoObjectString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mRootFindingInfoObjectFamWevString = rootFindingInfoObjectString;
        } else {
            mPimpl->mRootFindingInfoObjectFamWoevString = rootFindingInfoObjectString;
        }
    }
}

std::string GeneratorProfile::externNlaSolveMethodString() const
{
    return mPimpl->mExternNlaSolveMethodString;
}

void GeneratorProfile::setExternNlaSolveMethodString(const std::string &externNlaSolveMethodString)
{
    mPimpl->mExternNlaSolveMethodString = externNlaSolveMethodString;
}

std::string GeneratorProfile::findRootCallString(bool forDifferentialModel,
                                                 bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mFindRootCallFdmWevString;
        }

        return mPimpl->mFindRootCallFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mFindRootCallFamWevString;
    }

    return mPimpl->mFindRootCallFamWoevString;
}

void GeneratorProfile::setFindRootCallString(bool forDifferentialModel,
                                             bool withExternalVariables,
                                             const std::string &findRootCallString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mFindRootCallFdmWevString = findRootCallString;
        } else {
            mPimpl->mFindRootCallFdmWoevString = findRootCallString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mFindRootCallFamWevString = findRootCallString;
        } else {
            mPimpl->mFindRootCallFamWoevString = findRootCallString;
        }
    }
}

std::string GeneratorProfile::findRootMethodString(bool forDifferentialModel,
                                                   bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mFindRootMethodFdmWevString;
        }

        return mPimpl->mFindRootMethodFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mFindRootMethodFamWevString;
    }

    return mPimpl->mFindRootMethodFamWoevString;
}

void GeneratorProfile::setFindRootMethodString(bool forDifferentialModel,
                                               bool withExternalVariables,
                                               const std::string &findRootMethodString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mFindRootMethodFdmWevString = findRootMethodString;
        } else {
            mPimpl->mFindRootMethodFdmWoevString = findRootMethodString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mFindRootMethodFamWevString = findRootMethodString;
        } else {
            mPimpl->mFindRootMethodFamWoevString = findRootMethodString;
        }
    }
}

std::string GeneratorProfile::nlaSolveCallString(bool forDifferentialModel,
                                                 bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mNlaSolveCallFdmWevString;
        }

        return mPimpl->mNlaSolveCallFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mNlaSolveCallFamWevString;
    }

    return mPimpl->mNlaSolveCallFamWoevString;
}

void GeneratorProfile::setNlaSolveCallString(bool forDifferentialModel,
                                             bool withExternalVariables,
                                             const std::string &nlaSolveCallString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mNlaSolveCallFdmWevString = nlaSolveCallString;
        } else {
            mPimpl->mNlaSolveCallFdmWoevString = nlaSolveCallString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mNlaSolveCallFamWevString = nlaSolveCallString;
        } else {
            mPimpl->mNlaSolveCallFamWoevString = nlaSolveCallString;
        }
    }
}

std::string GeneratorProfile::objectiveFunctionMethodString(bool forDifferentialModel,
                                                            bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mObjectiveFunctionMethodFdmWevString;
        }

        return mPimpl->mObjectiveFunctionMethodFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mObjectiveFunctionMethodFamWevString;
    }

    return mPimpl->mObjectiveFunctionMethodFamWoevString;
}

void GeneratorProfile::setObjectiveFunctionMethodString(bool forDifferentialModel,
                                                        bool withExternalVariables,
                                                        const std::string &objectiveFunctionMethodString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mObjectiveFunctionMethodFdmWevString = objectiveFunctionMethodString;
        } else {
            mPimpl->mObjectiveFunctionMethodFdmWoevString = objectiveFunctionMethodString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mObjectiveFunctionMethodFamWevString = objectiveFunctionMethodString;
        } else {
            mPimpl->mObjectiveFunctionMethodFamWoevString = objectiveFunctionMethodString;
        }
    }
}

std::string GeneratorProfile::uArrayString() const
{
    return mPimpl->mUArrayString;
}

void GeneratorProfile::setUArrayString(const std::string &uArrayString)
{
    mPimpl->mUArrayString = uArrayString;
}

std::string GeneratorProfile::fArrayString() const
{
    return mPimpl->mFArrayString;
}

void GeneratorProfile::setFArrayString(const std::string &fArrayString)
{
    mPimpl->mFArrayString = fArrayString;
}

std::string GeneratorProfile::interfaceCreateStatesArrayMethodString() const
{
    return mPimpl->mInterfaceCreateStatesArrayMethodString;
}

void GeneratorProfile::setInterfaceCreateStatesArrayMethodString(const std::string &interfaceCreateStatesArrayMethodString)
{
    mPimpl->mInterfaceCreateStatesArrayMethodString = interfaceCreateStatesArrayMethodString;
}

std::string GeneratorProfile::implementationCreateStatesArrayMethodString() const
{
    return mPimpl->mImplementationCreateStatesArrayMethodString;
}

void GeneratorProfile::setImplementationCreateStatesArrayMethodString(const std::string &implementationCreateStatesArrayMethodString)
{
    mPimpl->mImplementationCreateStatesArrayMethodString = implementationCreateStatesArrayMethodString;
}

std::string GeneratorProfile::interfaceCreateConstantsArrayMethodString() const
{
    return mPimpl->mInterfaceCreateConstantsArrayMethodString;
}

void GeneratorProfile::setInterfaceCreateConstantsArrayMethodString(const std::string &interfaceCreateConstantsArrayMethodString)
{
    mPimpl->mInterfaceCreateConstantsArrayMethodString = interfaceCreateConstantsArrayMethodString;
}

std::string GeneratorProfile::implementationCreateConstantsArrayMethodString() const
{
    return mPimpl->mImplementationCreateConstantsArrayMethodString;
}

void GeneratorProfile::setImplementationCreateConstantsArrayMethodString(const std::string &implementationCreateConstantsArrayMethodString)
{
    mPimpl->mImplementationCreateConstantsArrayMethodString = implementationCreateConstantsArrayMethodString;
}

std::string GeneratorProfile::interfaceCreateComputedConstantsArrayMethodString() const
{
    return mPimpl->mInterfaceCreateComputedConstantsArrayMethodString;
}

void GeneratorProfile::setInterfaceCreateComputedConstantsArrayMethodString(const std::string &interfaceCreateComputedConstantsArrayMethodString)
{
    mPimpl->mInterfaceCreateComputedConstantsArrayMethodString = interfaceCreateComputedConstantsArrayMethodString;
}

std::string GeneratorProfile::implementationCreateComputedConstantsArrayMethodString() const
{
    return mPimpl->mImplementationCreateComputedConstantsArrayMethodString;
}

void GeneratorProfile::setImplementationCreateComputedConstantsArrayMethodString(const std::string &implementationCreateComputedConstantsArrayMethodString)
{
    mPimpl->mImplementationCreateComputedConstantsArrayMethodString = implementationCreateComputedConstantsArrayMethodString;
}

std::string GeneratorProfile::interfaceCreateAlgebraicVariablesArrayMethodString() const
{
    return mPimpl->mInterfaceCreateAlgebraicVariablesArrayMethodString;
}

void GeneratorProfile::setInterfaceCreateAlgebraicVariablesArrayMethodString(const std::string &interfaceCreateAlgebraicVariablesArrayMethodString)
{
    mPimpl->mInterfaceCreateAlgebraicVariablesArrayMethodString = interfaceCreateAlgebraicVariablesArrayMethodString;
}

std::string GeneratorProfile::implementationCreateAlgebraicVariablesArrayMethodString() const
{
    return mPimpl->mImplementationCreateAlgebraicVariablesArrayMethodString;
}

void GeneratorProfile::setImplementationCreateAlgebraicVariablesArrayMethodString(const std::string &implementationCreateAlgebraicVariablesArrayMethodString)
{
    mPimpl->mImplementationCreateAlgebraicVariablesArrayMethodString = implementationCreateAlgebraicVariablesArrayMethodString;
}

std::string GeneratorProfile::interfaceCreateExternalVariablesArrayMethodString() const
{
    return mPimpl->mInterfaceCreateExternalVariablesArrayMethodString;
}

void GeneratorProfile::setInterfaceCreateExternalVariablesArrayMethodString(const std::string &interfaceCreateExternalVariablesArrayMethodString)
{
    mPimpl->mInterfaceCreateExternalVariablesArrayMethodString = interfaceCreateExternalVariablesArrayMethodString;
}

std::string GeneratorProfile::implementationCreateExternalVariablesArrayMethodString() const
{
    return mPimpl->mImplementationCreateExternalVariablesArrayMethodString;
}

void GeneratorProfile::setImplementationCreateExternalVariablesArrayMethodString(const std::string &implementationCreateExternalVariablesArrayMethodString)
{
    mPimpl->mImplementationCreateExternalVariablesArrayMethodString = implementationCreateExternalVariablesArrayMethodString;
}

std::string GeneratorProfile::interfaceDeleteArrayMethodString() const
{
    return mPimpl->mInterfaceDeleteArrayMethodString;
}

void GeneratorProfile::setInterfaceDeleteArrayMethodString(const std::string &interfaceDeleteArrayMethodString)
{
    mPimpl->mInterfaceDeleteArrayMethodString = interfaceDeleteArrayMethodString;
}

std::string GeneratorProfile::implementationDeleteArrayMethodString() const
{
    return mPimpl->mImplementationDeleteArrayMethodString;
}

void GeneratorProfile::setImplementationDeleteArrayMethodString(const std::string &implementationDeleteArrayMethodString)
{
    mPimpl->mImplementationDeleteArrayMethodString = implementationDeleteArrayMethodString;
}

std::string GeneratorProfile::interfaceInitialiseArraysMethodString(bool forDifferentialModel) const
{
    if (forDifferentialModel) {
        return mPimpl->mInterfaceInitialiseArraysMethodDiffString;
    }

    return mPimpl->mInterfaceInitialiseArraysMethodNonDiffString;
}

void GeneratorProfile::setInterfaceInitialiseArraysMethodString(bool forDifferentialModel,
                                                                const std::string &interfaceInitialiseArraysMethodString)
{
    if (forDifferentialModel) {
        mPimpl->mInterfaceInitialiseArraysMethodDiffString = interfaceInitialiseArraysMethodString;
    } else {
        mPimpl->mInterfaceInitialiseArraysMethodNonDiffString = interfaceInitialiseArraysMethodString;
    }
}

std::string GeneratorProfile::implementationInitialiseArraysMethodString(bool forDifferentialModel) const
{
    if (forDifferentialModel) {
        return mPimpl->mImplementationInitialiseArraysMethodDiffString;
    }

    return mPimpl->mImplementationInitialiseArraysMethodNonDiffString;
}

void GeneratorProfile::setImplementationInitialiseArraysMethodString(bool forDifferentialModel,
                                                                     const std::string &implementationInitialiseArraysMethodString)
{
    if (forDifferentialModel) {
        mPimpl->mImplementationInitialiseArraysMethodDiffString = implementationInitialiseArraysMethodString;
    } else {
        mPimpl->mImplementationInitialiseArraysMethodNonDiffString = implementationInitialiseArraysMethodString;
    }
}

std::string GeneratorProfile::interfaceComputeComputedConstantsMethodString(bool forDifferentialModel) const
{
    if (forDifferentialModel) {
        return mPimpl->mInterfaceComputeComputedConstantsMethodDiffString;
    }

    return mPimpl->mInterfaceComputeComputedConstantsMethodNonDiffString;
}

void GeneratorProfile::setInterfaceComputeComputedConstantsMethodString(bool forDifferentialModel,
                                                                        const std::string &interfaceComputeComputedConstantsMethodString)
{
    if (forDifferentialModel) {
        mPimpl->mInterfaceComputeComputedConstantsMethodDiffString = interfaceComputeComputedConstantsMethodString;
    } else {
        mPimpl->mInterfaceComputeComputedConstantsMethodNonDiffString = interfaceComputeComputedConstantsMethodString;
    }
}

std::string GeneratorProfile::implementationComputeComputedConstantsMethodString(bool forDifferentialModel) const
{
    if (forDifferentialModel) {
        return mPimpl->mImplementationComputeComputedConstantsMethodDiffString;
    }

    return mPimpl->mImplementationComputeComputedConstantsMethodNonDiffString;
}

void GeneratorProfile::setImplementationComputeComputedConstantsMethodString(bool forDifferentialModel,
                                                                             const std::string &implementationComputeComputedConstantsMethodString)
{
    if (forDifferentialModel) {
        mPimpl->mImplementationComputeComputedConstantsMethodDiffString = implementationComputeComputedConstantsMethodString;
    } else {
        mPimpl->mImplementationComputeComputedConstantsMethodNonDiffString = implementationComputeComputedConstantsMethodString;
    }
}

std::string GeneratorProfile::interfaceComputeRatesMethodString(bool withExternalVariables) const
{
    if (withExternalVariables) {
        return mPimpl->mInterfaceComputeRatesMethodWevString;
    }

    return mPimpl->mInterfaceComputeRatesMethodWoevString;
}

void GeneratorProfile::setInterfaceComputeRatesMethodString(bool withExternalVariables,
                                                            const std::string &interfaceComputeRatesMethodString)
{
    if (withExternalVariables) {
        mPimpl->mInterfaceComputeRatesMethodWevString = interfaceComputeRatesMethodString;
    } else {
        mPimpl->mInterfaceComputeRatesMethodWoevString = interfaceComputeRatesMethodString;
    }
}

std::string GeneratorProfile::implementationComputeRatesMethodString(bool withExternalVariables) const
{
    if (withExternalVariables) {
        return mPimpl->mImplementationComputeRatesMethodWevString;
    }

    return mPimpl->mImplementationComputeRatesMethodWoevString;
}

void GeneratorProfile::setImplementationComputeRatesMethodString(bool withExternalVariables,
                                                                 const std::string &implementationComputeRatesMethodString)
{
    if (withExternalVariables) {
        mPimpl->mImplementationComputeRatesMethodWevString = implementationComputeRatesMethodString;
    } else {
        mPimpl->mImplementationComputeRatesMethodWoevString = implementationComputeRatesMethodString;
    }
}

std::string GeneratorProfile::interfaceComputeVariablesMethodString(bool forDifferentialModel,
                                                                    bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mInterfaceComputeVariablesMethodFdmWevString;
        }

        return mPimpl->mInterfaceComputeVariablesMethodFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mInterfaceComputeVariablesMethodFamWevString;
    }

    return mPimpl->mInterfaceComputeVariablesMethodFamWoevString;
}

void GeneratorProfile::setInterfaceComputeVariablesMethodString(bool forDifferentialModel,
                                                                bool withExternalVariables,
                                                                const std::string &interfaceComputeVariablesMethodString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mInterfaceComputeVariablesMethodFdmWevString = interfaceComputeVariablesMethodString;
        } else {
            mPimpl->mInterfaceComputeVariablesMethodFdmWoevString = interfaceComputeVariablesMethodString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mInterfaceComputeVariablesMethodFamWevString = interfaceComputeVariablesMethodString;
        } else {
            mPimpl->mInterfaceComputeVariablesMethodFamWoevString = interfaceComputeVariablesMethodString;
        }
    }
}

std::string GeneratorProfile::implementationComputeVariablesMethodString(bool forDifferentialModel,
                                                                         bool withExternalVariables) const
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            return mPimpl->mImplementationComputeVariablesMethodFdmWevString;
        }

        return mPimpl->mImplementationComputeVariablesMethodFdmWoevString;
    }

    if (withExternalVariables) {
        return mPimpl->mImplementationComputeVariablesMethodFamWevString;
    }

    return mPimpl->mImplementationComputeVariablesMethodFamWoevString;
}

void GeneratorProfile::setImplementationComputeVariablesMethodString(bool forDifferentialModel,
                                                                     bool withExternalVariables,
                                                                     const std::string &implementationComputeVariablesMethodString)
{
    if (forDifferentialModel) {
        if (withExternalVariables) {
            mPimpl->mImplementationComputeVariablesMethodFdmWevString = implementationComputeVariablesMethodString;
        } else {
            mPimpl->mImplementationComputeVariablesMethodFdmWoevString = implementationComputeVariablesMethodString;
        }
    } else {
        if (withExternalVariables) {
            mPimpl->mImplementationComputeVariablesMethodFamWevString = implementationComputeVariablesMethodString;
        } else {
            mPimpl->mImplementationComputeVariablesMethodFamWoevString = implementationComputeVariablesMethodString;
        }
    }
}

std::string GeneratorProfile::emptyMethodString() const
{
    return mPimpl->mEmptyMethodString;
}

void GeneratorProfile::setEmptyMethodString(const std::string &emptyMethodString)
{
    mPimpl->mEmptyMethodString = emptyMethodString;
}

std::string GeneratorProfile::indentString() const
{
    return mPimpl->mIndentString;
}

void GeneratorProfile::setIndentString(const std::string &indentString)
{
    mPimpl->mIndentString = indentString;
}

std::string GeneratorProfile::variableDeclarationString() const
{
    return mPimpl->mVariableDeclarationString;
}

void GeneratorProfile::setVariableDeclarationString(const std::string &variableDeclarationString)
{
    mPimpl->mVariableDeclarationString = variableDeclarationString;
}

std::string GeneratorProfile::openArrayString() const
{
    return mPimpl->mOpenArrayString;
}

void GeneratorProfile::setOpenArrayString(const std::string &openArrayString)
{
    mPimpl->mOpenArrayString = openArrayString;
}

std::string GeneratorProfile::closeArrayString() const
{
    return mPimpl->mCloseArrayString;
}

void GeneratorProfile::setCloseArrayString(const std::string &closeArrayString)
{
    mPimpl->mCloseArrayString = closeArrayString;
}

std::string GeneratorProfile::arrayElementSeparatorString() const
{
    return mPimpl->mArrayElementSeparatorString;
}

void GeneratorProfile::setArrayElementSeparatorString(const std::string &arrayElementSeparatorString)
{
    mPimpl->mArrayElementSeparatorString = arrayElementSeparatorString;
}

std::string GeneratorProfile::commandSeparatorString() const
{
    return mPimpl->mCommandSeparatorString;
}

void GeneratorProfile::setCommandSeparatorString(const std::string &commandSeparatorString)
{
    mPimpl->mCommandSeparatorString = commandSeparatorString;
}

} // namespace libcellml
