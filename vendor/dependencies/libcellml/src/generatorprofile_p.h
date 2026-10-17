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

#include "libcellml/generatorprofile.h"

namespace libcellml {

/**
 * @brief The GeneratorProfile::GeneratorProfileImpl struct.
 *
 * The private implementation for the GeneratorProfile class.
 */
struct GeneratorProfile::GeneratorProfileImpl
{
    // Whether the profile is official.

    GeneratorProfile::Profile mProfile = Profile::C;

    // Whether the profile requires an interface to be generated.

    bool mHasInterface = true;

    // Equality.

    std::string mEqualityString;

    // Relational and logical operators.

    std::string mEqString;
    std::string mNeqString;
    std::string mLtString;
    std::string mLeqString;
    std::string mGtString;
    std::string mGeqString;
    std::string mAndString;
    std::string mOrString;
    std::string mXorString;
    std::string mNotString;

    bool mHasEqOperator = true;
    bool mHasNeqOperator = true;
    bool mHasLtOperator = true;
    bool mHasLeqOperator = true;
    bool mHasGtOperator = true;
    bool mHasGeqOperator = true;
    bool mHasAndOperator = true;
    bool mHasOrOperator = true;
    bool mHasXorOperator = true;
    bool mHasNotOperator = true;

    // Arithmetic operators.

    std::string mPlusString;
    std::string mMinusString;
    std::string mTimesString;
    std::string mDivideString;
    std::string mPowerString;
    std::string mSquareRootString;
    std::string mSquareString;
    std::string mAbsoluteValueString;
    std::string mExponentialString;
    std::string mNaturalLogarithmString;
    std::string mCommonLogarithmString;
    std::string mCeilingString;
    std::string mFloorString;
    std::string mMinString;
    std::string mMaxString;
    std::string mRemString;

    bool mHasPowerOperator = false;

    // Trigonometric operators.

    std::string mSinString;
    std::string mCosString;
    std::string mTanString;
    std::string mSecString;
    std::string mCscString;
    std::string mCotString;
    std::string mSinhString;
    std::string mCoshString;
    std::string mTanhString;
    std::string mSechString;
    std::string mCschString;
    std::string mCothString;
    std::string mAsinString;
    std::string mAcosString;
    std::string mAtanString;
    std::string mAsecString;
    std::string mAcscString;
    std::string mAcotString;
    std::string mAsinhString;
    std::string mAcoshString;
    std::string mAtanhString;
    std::string mAsechString;
    std::string mAcschString;
    std::string mAcothString;

    // Piecewise statement.

    std::string mConditionalOperatorIfString;
    std::string mConditionalOperatorElseString;
    std::string mPiecewiseIfString;
    std::string mPiecewiseElseString;

    bool mHasConditionalOperator = true;

    // Constants.

    std::string mTrueString;
    std::string mFalseString;
    std::string mEString;
    std::string mPiString;
    std::string mInfString;
    std::string mNanString;

    // Arithmetic functions.

    std::string mEqFunctionString;
    std::string mNeqFunctionString;
    std::string mLtFunctionString;
    std::string mLeqFunctionString;
    std::string mGtFunctionString;
    std::string mGeqFunctionString;
    std::string mAndFunctionString;
    std::string mOrFunctionString;
    std::string mXorFunctionString;
    std::string mNotFunctionString;
    std::string mMinFunctionString;
    std::string mMaxFunctionString;

    // Trigonometric functions.

    std::string mSecFunctionString;
    std::string mCscFunctionString;
    std::string mCotFunctionString;
    std::string mSechFunctionString;
    std::string mCschFunctionString;
    std::string mCothFunctionString;
    std::string mAsecFunctionString;
    std::string mAcscFunctionString;
    std::string mAcotFunctionString;
    std::string mAsechFunctionString;
    std::string mAcschFunctionString;
    std::string mAcothFunctionString;

    // Miscellaneous.

    std::string mCommentString;
    std::string mOriginCommentString;

    std::string mInterfaceFileNameString;

    std::string mInterfaceHeaderString;
    std::string mImplementationHeaderString;

    std::string mInterfaceVersionString;
    std::string mImplementationVersionString;

    std::string mInterfaceLibcellmlVersionString;
    std::string mImplementationLibcellmlVersionString;

    std::string mInterfaceStateCountString;
    std::string mImplementationStateCountString;

    std::string mInterfaceConstantCountString;
    std::string mImplementationConstantCountString;

    std::string mInterfaceComputedConstantCountString;
    std::string mImplementationComputedConstantCountString;

    std::string mInterfaceAlgebraicVariableCountString;
    std::string mImplementationAlgebraicVariableCountString;

    std::string mInterfaceExternalVariableCountString;
    std::string mImplementationExternalVariableCountString;

    std::string mVariableInfoObjectString;

    std::string mInterfaceVoiInfoString;
    std::string mImplementationVoiInfoString;

    std::string mInterfaceStateInfoString;
    std::string mImplementationStateInfoString;

    std::string mInterfaceConstantInfoString;
    std::string mImplementationConstantInfoString;

    std::string mInterfaceComputedConstantInfoString;
    std::string mImplementationComputedConstantInfoString;

    std::string mInterfaceAlgebraicVariableInfoString;
    std::string mImplementationAlgebraicVariableInfoString;

    std::string mInterfaceExternalVariableInfoString;
    std::string mImplementationExternalVariableInfoString;

    std::string mVariableInfoEntryString;

    std::string mVoiString;

    std::string mStatesArrayString;
    std::string mRatesArrayString;
    std::string mConstantsArrayString;
    std::string mComputedConstantsArrayString;
    std::string mAlgebraicVariablesArrayString;
    std::string mExternalVariablesArrayString;

    std::string mExternalVariableMethodTypeDefinitionNonDiffString;
    std::string mExternalVariableMethodTypeDefinitionDiffString;

    std::string mExternalVariableMethodCallNonDiffString;
    std::string mExternalVariableMethodCallDiffString;

    std::string mRootFindingInfoObjectFamWoevString;
    std::string mRootFindingInfoObjectFamWevString;
    std::string mRootFindingInfoObjectFdmWoevString;
    std::string mRootFindingInfoObjectFdmWevString;

    std::string mExternNlaSolveMethodString;

    std::string mFindRootCallFamWoevString;
    std::string mFindRootCallFamWevString;
    std::string mFindRootCallFdmWoevString;
    std::string mFindRootCallFdmWevString;

    std::string mFindRootMethodFamWoevString;
    std::string mFindRootMethodFamWevString;
    std::string mFindRootMethodFdmWoevString;
    std::string mFindRootMethodFdmWevString;

    std::string mNlaSolveCallFamWoevString;
    std::string mNlaSolveCallFamWevString;
    std::string mNlaSolveCallFdmWoevString;
    std::string mNlaSolveCallFdmWevString;

    std::string mObjectiveFunctionMethodFamWoevString;
    std::string mObjectiveFunctionMethodFamWevString;
    std::string mObjectiveFunctionMethodFdmWoevString;
    std::string mObjectiveFunctionMethodFdmWevString;

    std::string mUArrayString;
    std::string mFArrayString;

    std::string mInterfaceCreateStatesArrayMethodString;
    std::string mImplementationCreateStatesArrayMethodString;

    std::string mInterfaceCreateConstantsArrayMethodString;
    std::string mImplementationCreateConstantsArrayMethodString;

    std::string mInterfaceCreateComputedConstantsArrayMethodString;
    std::string mImplementationCreateComputedConstantsArrayMethodString;

    std::string mInterfaceCreateAlgebraicVariablesArrayMethodString;
    std::string mImplementationCreateAlgebraicVariablesArrayMethodString;

    std::string mInterfaceCreateExternalVariablesArrayMethodString;
    std::string mImplementationCreateExternalVariablesArrayMethodString;

    std::string mInterfaceDeleteArrayMethodString;
    std::string mImplementationDeleteArrayMethodString;

    std::string mInterfaceInitialiseArraysMethodNonDiffString;
    std::string mImplementationInitialiseArraysMethodNonDiffString;

    std::string mInterfaceInitialiseArraysMethodDiffString;
    std::string mImplementationInitialiseArraysMethodDiffString;

    std::string mInterfaceComputeComputedConstantsMethodNonDiffString;
    std::string mImplementationComputeComputedConstantsMethodNonDiffString;

    std::string mInterfaceComputeComputedConstantsMethodDiffString;
    std::string mImplementationComputeComputedConstantsMethodDiffString;

    std::string mInterfaceComputeRatesMethodWoevString;
    std::string mImplementationComputeRatesMethodWoevString;

    std::string mInterfaceComputeRatesMethodWevString;
    std::string mImplementationComputeRatesMethodWevString;

    std::string mInterfaceComputeVariablesMethodFamWoevString;
    std::string mInterfaceComputeVariablesMethodFamWevString;
    std::string mInterfaceComputeVariablesMethodFdmWoevString;
    std::string mInterfaceComputeVariablesMethodFdmWevString;

    std::string mImplementationComputeVariablesMethodFamWoevString;
    std::string mImplementationComputeVariablesMethodFamWevString;
    std::string mImplementationComputeVariablesMethodFdmWoevString;
    std::string mImplementationComputeVariablesMethodFdmWevString;

    std::string mEmptyMethodString;

    std::string mIndentString;

    std::string mVariableDeclarationString;

    std::string mOpenArrayString;
    std::string mCloseArrayString;

    std::string mArrayElementSeparatorString;

    std::string mCommandSeparatorString;

    void loadProfile(GeneratorProfile::Profile profile);
};

} // namespace libcellml
