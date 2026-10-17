%module(package="libcellml") generatorprofile

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "createconstructor.i"
%import "types.i"

%feature("docstring") libcellml::GeneratorProfile
"Holds the information for the :class:`Generator` to generate code according to this profile.";

%feature("docstring") libcellml::GeneratorProfile::profile
"Returns the :enum:`GeneratorProfile::Profile` for this :class:`GeneratorProfile`.";

%feature("docstring") libcellml::GeneratorProfile::profileAsString
"Returns the :enum:`GeneratorProfile::Profile` as a string for this :class:`GeneratorProfile`.";

%feature("docstring") libcellml::GeneratorProfile::setProfile
"Sets the :enum:`GeneratorProfile::Profile` for this :class:`GeneratorProfile`.";

%feature("docstring") libcellml::GeneratorProfile::hasInterface
"Tests if this :class:`GeneratorProfile` requires an interface.";

%feature("docstring") libcellml::GeneratorProfile::setHasInterface
"Sets whether this :class:`GeneratorProfile` requires an interface.";

%feature("docstring") libcellml::GeneratorProfile::equalityString
"Returns the string representing the MathML \"equality\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setEqualityString
"Sets the string representing the MathML \"equality\" operator.";

%feature("docstring") libcellml::GeneratorProfile::eqString
"Returns the string representing the MathML \"equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setEqString
"Sets the string representing the MathML \"equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::neqString
"Returns the the string representing the MathML \"not equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setNeqString
"Sets the string representing the MathML \"not equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::ltString
"Returns the string representing the MathML \"less than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setLtString
"Sets the string representing the MathML \"less than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::leqString
"Returns the string representing the MathML \"less than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setLeqString
"Sets the string representing the MathML \"less than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::gtString
"Returns the string representing the MathML \"greater than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setGtString
"Sets the string representing the MathML \"greater than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::geqString
"Returns the string representing the MathML \"greater than or equal to\ operator.";

%feature("docstring") libcellml::GeneratorProfile::setGeqString
"Sets the string representing the MathML \"greater than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::andString
"Returns the string representing the MathML \"and\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setAndString
"Sets the string representing the MathML \"and\" operator.";

%feature("docstring") libcellml::GeneratorProfile::orString
"Returns the string representing the MathML \"or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setOrString
"Sets the string representing the MathML \"or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::xorString
"Returns the string representing the MathML \"exclusive or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setXorString
"Sets the string representing the MathML \"exclusive or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::notString
"Returns the string representing the MathML \"not\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setNotString
"Sets the string representing the MathML \"not\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasEqOperator
"Tests if this :class:`GeneratorProfile` has an \"equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasEqOperator
"Sets whether this :class:`GeneratorProfile` has an \"equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasNeqOperator
"Tests if this :class:`GeneratorProfile` has a \"not equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasNeqOperator
"Sets whether this :class:`GeneratorProfile` has a \"not equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasLtOperator
"Tests if this :class:`GeneratorProfile` has a \"less than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasLtOperator
"Sets whether this :class:`GeneratorProfile` has a \"less than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasLeqOperator
"Tests if this :class:`GeneratorProfile` has a \"less than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasLeqOperator
"Sets whether this :class:`GeneratorProfile` has a \"less than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasGtOperator
"Tests if this :class:`GeneratorProfile` has a \"greater than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasGtOperator
"Sets whether this :class:`GeneratorProfile` has a \"greater than\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasGeqOperator
"Tests if this :class:`GeneratorProfile` has a \"greater than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasGeqOperator
"Sets whether this :class:`GeneratorProfile` has a \"greater than or equal to\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasAndOperator
"Tests if this :class:`GeneratorProfile` has an \"and\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasAndOperator
"Sets whether this :class:`GeneratorProfile` has an \"and\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasOrOperator
"Tests if this :class:`GeneratorProfile` has an \"or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasOrOperator
"Sets whether this :class:`GeneratorProfile` has an \"or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasXorOperator
"Tests if this :class:`GeneratorProfile` has an \"exclusive or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasXorOperator
"Sets whether this :class:`GeneratorProfile` has an \"exclusive or\" operator.";

%feature("docstring") libcellml::GeneratorProfile::hasNotOperator
"Tests if this :class:`GeneratorProfile` has a \"not\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasNotOperator
"Sets whether this :class:`GeneratorProfile` has a \"not\" operator.";

%feature("docstring") libcellml::GeneratorProfile::plusString
"Returns the string representing the MathML \"plus\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setPlusString
"Sets the string representing the MathML \"plus\" operator.";

%feature("docstring") libcellml::GeneratorProfile::minusString
"Returns the string representing the MathML \"minus\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setMinusString
"Sets the string representing the MathML \"minus\" operator.";

%feature("docstring") libcellml::GeneratorProfile::timesString
"Returns the string representing the MathML \"times\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setTimesString
"Sets the string representing the MathML \"times\" operator.";

%feature("docstring") libcellml::GeneratorProfile::divideString
"Returns the string representing the MathML \"divide\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setDivideString
"Sets the string representing the MathML \"divide\" operator.";

%feature("docstring") libcellml::GeneratorProfile::powerString
"Returns the string representing the MathML \"power\" operator or function.";

%feature("docstring") libcellml::GeneratorProfile::setPowerString
"Sets the string representing the MathML \"power\" operator or function.";

%feature("docstring") libcellml::GeneratorProfile::squareRootString
"Returns the string representing the MathML \"square root\" function.";

%feature("docstring") libcellml::GeneratorProfile::setSquareRootString
"Sets the string representing the MathML \"square root\" function.";

%feature("docstring") libcellml::GeneratorProfile::squareString
"Returns the string representing the MathML \"square\" function.";

%feature("docstring") libcellml::GeneratorProfile::setSquareString
"Sets the string representing the MathML \"square\" function.";

%feature("docstring") libcellml::GeneratorProfile::absoluteValueString
"Returns the string representing the MathML \"absolute value\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAbsoluteValueString
"Sets the string representing the MathML \"absolute value\" function.";

%feature("docstring") libcellml::GeneratorProfile::exponentialString
"Returns the string representing the MathML \"exponential\" function.";

%feature("docstring") libcellml::GeneratorProfile::setExponentialString
"Sets the string representing the MathML \"exponential\" function.";

%feature("docstring") libcellml::GeneratorProfile::naturalLogarithmString
"Returns the string representing the MathML \"natural logarithm\" function.";

%feature("docstring") libcellml::GeneratorProfile::setNaturalLogarithmString
"Sets the string representing the MathML \"natural logarithm\" function.";

%feature("docstring") libcellml::GeneratorProfile::commonLogarithmString
"Returns the string representing the MathML \"common logarithm\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCommonLogarithmString
"Sets the string representing the MathML \"common logarithm\" function.";

%feature("docstring") libcellml::GeneratorProfile::ceilingString
"Returns the string representing the MathML \"ceiling\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCeilingString
"Sets the string representing the MathML \"ceiling\" function.";

%feature("docstring") libcellml::GeneratorProfile::floorString
"Returns the string representing the MathML \"floor\" function.";

%feature("docstring") libcellml::GeneratorProfile::setFloorString
"Sets the string representing the MathML \"floor\" function.";

%feature("docstring") libcellml::GeneratorProfile::minString
"Returns the string representing the MathML \"minimum\" function.";

%feature("docstring") libcellml::GeneratorProfile::setMinString
"Sets the string representing the MathML \"minimum\" function.";

%feature("docstring") libcellml::GeneratorProfile::maxString
"Returns the string representing the MathML \"maximum\" function.";

%feature("docstring") libcellml::GeneratorProfile::setMaxString
"Sets the string representing the MathML \"maximum\" function.";

%feature("docstring") libcellml::GeneratorProfile::remString
"Returns the string representing the MathML \"remainder\" function.";

%feature("docstring") libcellml::GeneratorProfile::setRemString
"Sets the string representing the MathML \"remainder\" function.";

%feature("docstring") libcellml::GeneratorProfile::hasPowerOperator
"Tests if this :class:`GeneratorProfile` has a \"power\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasPowerOperator
"Sets whether this :class:`GeneratorProfile` has a \"power\" operator.";

%feature("docstring") libcellml::GeneratorProfile::sinString
"Returns the string representing the MathML \"sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setSinString
"Sets the string representing the MathML \"sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::cosString
"Returns the string representing the MathML \"cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCosString
"Sets the string representing the MathML \"cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::tanString
"Returns the string representing the MathML \"tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setTanString
"Sets the string representing the MathML \"tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::secString
"Returns the string representing the MathML \"secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setSecString
"Sets the string representing the MathML \"secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::cscString
"Returns the string representing the MathML \"cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCscString
"Sets the string representing the MathML \"cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::cotString
"Returns the string representing the MathML \"cotangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCotString
"Sets the string representing the MathML \"cotangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::sinhString
"Returns the string representing the MathML \"hyperbolic sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setSinhString
"Sets the string representing the MathML \"hyperbolic sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::coshString
"Returns the string representing the MathML \"hyperbolic cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCoshString
"Sets the string representing the MathML \"hyperbolic cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::tanhString
"Returns the string representing the MathML \"hyperbolic tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setTanhString
"Sets the string representing the MathML \"hyperbolic tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::sechString
"Returns the string representing the MathML \"hyperbolic secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setSechString
"Sets the string representing the MathML \"hyperbolic secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::cschString
"Returns the string representing the MathML \"hyperbolic cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCschString
"Sets the string representing the MathML \"hyperbolic cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::cothString
"Returns the string representing the MathML \"hyperbolic cotangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setCothString
"Sets the string representing the MathML \"hyperbolic cotangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::asinString
"Returns the string representing the MathML \"arc sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAsinString
"Sets the string representing the MathML \"arc sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::acosString
"Returns the string representing the MathML \"arc cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAcosString
"Sets the string representing the MathML \"arc cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::atanString
"Returns the string representing the MathML \"arc tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAtanString
"Sets the string representing the MathML \"arc tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::asecString
"Returns the string representing the MathML \"arc secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAsecString
"Sets the string representing the MathML \"arc secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::acscString
"Returns the string representing the MathML \"arc cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAcscString
"Sets the string representing the MathML \"arc cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::acotString
"Returns the string representing the MathML \"arc cotangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAcotString
"Sets the string representing the MathML \"arc cotangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::asinhString
"Returns the string representing the MathML \"arc hyperbolic sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAsinhString
"Sets the string representing the MathML \"arc hyperbolic sine\" function.";

%feature("docstring") libcellml::GeneratorProfile::acoshString
"Returns the string representing the MathML \"arc hyperbolic cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAcoshString
"Sets the string representing the MathML \"arc hyperbolic cosine\" function.";

%feature("docstring") libcellml::GeneratorProfile::atanhString
"Returns the string representing the MathML \"arc hyperbolic tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAtanhString
"Sets the string representing the MathML \"arc hyperbolic tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::asechString
"Returns the string representing the MathML \"arc hyperbolic secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAsechString
"Sets the string representing the MathML \"arc hyperbolic secant\" function.";

%feature("docstring") libcellml::GeneratorProfile::acschString
"Returns the string representing the MathML \"arc hyperbolic cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAcschString
"Sets the string representing the MathML \"arc hyperbolic cosecant\" function.";

%feature("docstring") libcellml::GeneratorProfile::acothString
"Returns the string representing the MathML \"arc hyperbolic tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::setAcothString
"Sets the string representing the MathML \"arc hyperbolic tangent\" function.";

%feature("docstring") libcellml::GeneratorProfile::conditionalOperatorIfString
"Returns the string representing the MathML \"if\" part of a \"conditional\" statement or operator.";

%feature("docstring") libcellml::GeneratorProfile::setConditionalOperatorIfString
"Sets the string representing the MathML \"if\" part of a \"conditional\" statement or operator.";

%feature("docstring") libcellml::GeneratorProfile::conditionalOperatorElseString
"Returns the string representing the MathML \"else\" part of a \"conditional\" statement or operator.";

%feature("docstring") libcellml::GeneratorProfile::setConditionalOperatorElseString
"Sets the string representing the MathML \"else\" part of a \"conditional\" statement or operator.";

%feature("docstring") libcellml::GeneratorProfile::piecewiseIfString
"Returns the string representing the MathML \"if\" part of a \"piecewise\" statement.";

%feature("docstring") libcellml::GeneratorProfile::setPiecewiseIfString
"Sets the string representing the MathML \"if\" part of a \"piecewise\" statement.";

%feature("docstring") libcellml::GeneratorProfile::piecewiseElseString
"Returns the string representing the MathML \"else\" part of a \"piecewise\" statement.";

%feature("docstring") libcellml::GeneratorProfile::setPiecewiseElseString
"Sets the string representing the MathML \"else\" part of a \"piecewise\" statement.";

%feature("docstring") libcellml::GeneratorProfile::hasConditionalOperator
"Tests if this :class:`GeneratorProfile` has a \"conditional\" operator.";

%feature("docstring") libcellml::GeneratorProfile::setHasConditionalOperator
"Sets whether this :class:`GeneratorProfile` has a \"conditional\" operator.";

%feature("docstring") libcellml::GeneratorProfile::trueString
"Returns the string representing the MathML \"true\" boolean.";

%feature("docstring") libcellml::GeneratorProfile::setTrueString
"Sets the string representing the MathML \"true\" boolean.";

%feature("docstring") libcellml::GeneratorProfile::falseString
"Returns the string representing the MathML \"false\" boolean.";

%feature("docstring") libcellml::GeneratorProfile::setFalseString
"Sets the string representing the MathML \"false\" boolean.";

%feature("docstring") libcellml::GeneratorProfile::eString
"Returns the string representing the MathML \"Euler's number\".";

%feature("docstring") libcellml::GeneratorProfile::setEString
"Sets the string representing the MathML \"Euler's number\".";

%feature("docstring") libcellml::GeneratorProfile::piString
"Returns the string representing the MathML \"π\" constant.";

%feature("docstring") libcellml::GeneratorProfile::setPiString
"Sets the string representing the MathML \"π\" constant.";

%feature("docstring") libcellml::GeneratorProfile::infString
"Returns the string representing the MathML \"infinity\" value.";

%feature("docstring") libcellml::GeneratorProfile::setInfString
"Sets the string representing the MathML \"infinity\" value.";

%feature("docstring") libcellml::GeneratorProfile::nanString
"Returns the string representing the MathML \"not-a-number\" value.";

%feature("docstring") libcellml::GeneratorProfile::setNanString
"Sets the string representing the MathML \"not-a-number\" value.";

%feature("docstring") libcellml::GeneratorProfile::eqFunctionString
"Returns the string for the \"equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setEqFunctionString
"Sets the string for the \"equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::neqFunctionString
"Returns the string for the \"not equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setNeqFunctionString
"Sets the string for the \"not equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::ltFunctionString
"Returns the string for the \"less than\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setLtFunctionString
"Sets the string for the \"less than\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::leqFunctionString
"Returns the string for the \"less than or equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setLeqFunctionString
"Sets the string for the \"less than or equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::gtFunctionString
"Returns the string for the \"greater than\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setGtFunctionString
"Sets the string for the \"greater than\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::geqFunctionString
"Returns the string for the \"greater than or equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setGeqFunctionString
"Sets the string for the \"greater than or equal to\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::andFunctionString
"Returns the string for the \"and\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAndFunctionString
"Sets the string for the \"and\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::orFunctionString
"Returns the string for the \"or\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setOrFunctionString
"Sets the string for the \"or\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::xorFunctionString
"Returns the string for the \"exclusive or\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setXorFunctionString
"Sets the string for the \"exclusive or\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::notFunctionString
"Returns the string for the \"not\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setNotFunctionString
"Sets the string for the \"not\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::minFunctionString
"Returns the string for the \"minimum\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setMinFunctionString
"Sets the string for the \"minimum\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::maxFunctionString
"Returns the string for the \"maximum\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setMaxFunctionString
"Sets the string for the \"maximum\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::secFunctionString
"Returns the string for the \"secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setSecFunctionString
"Sets the string for the \"secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::cscFunctionString
"Returns the string for the \"cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setCscFunctionString
"Sets the string for the \"cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::cotFunctionString
"Returns the string for the \"cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setCotFunctionString
"Sets the string for the \"cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::sechFunctionString
"Returns the string for the \"hyperbolic secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setSechFunctionString
"Sets the string for the \"hyperbolic secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::cschFunctionString
"Returns the string for the \"hyperbolic cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setCschFunctionString
"Sets the string for the \"hyperbolic cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::cothFunctionString
"Returns the string for the \"hyperbolic cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setCothFunctionString
"Sets the string for the \"hyperbolic cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::asecFunctionString
"Returns the string for the \"arc secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAsecFunctionString
"Sets the string for the \"arc secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::acscFunctionString
"Returns the string for the \"arc cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAcscFunctionString
"Sets the string for the \"arc cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::acotFunctionString
"Returns the string for the \"arc cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAcotFunctionString
"Sets the string for the \"arc cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::asechFunctionString
"Returns the string for the \"arc hyperbolic secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAsechFunctionString
"Sets the string for the \"arc hyperbolic secant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::acschFunctionString
"Returns the string for the \"arc hyperbolic cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAcschFunctionString
"Sets the string for the \"arc hyperbolic cosecant\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::acothFunctionString
"Returns the string for the \"arc hyperbolic cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::setAcothFunctionString
"Sets the string for the \"arc hyperbolic cotangent\" function implementation.";

%feature("docstring") libcellml::GeneratorProfile::commentString
"Returns the string for a comment.";

%feature("docstring") libcellml::GeneratorProfile::setCommentString
"Sets the string for a comment. To be useful, the string should contain the [CODE] tag, which will be replaced
with a (proper) comment.";

%feature("docstring") libcellml::GeneratorProfile::originCommentString
"Returns the string for an origin comment.";

%feature("docstring") libcellml::GeneratorProfile::setOriginCommentString
"Sets the string for an origin comment. To be useful, the string should contain the [PROFILE_INFORMATION] and
[LIBCELLML_VERSION] tags, which will be replaced with a statement about the profile and the version of libCellML
used respectively.";

%feature("docstring") libcellml::GeneratorProfile::interfaceFileNameString
"Returns the string for the interface file name.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceFileNameString
"Sets the string for the interface file name.";

%feature("docstring") libcellml::GeneratorProfile::interfaceHeaderString
"Returns the string for the interface of a header.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceHeaderString
"Sets the string for the interface of a header.";

%feature("docstring") libcellml::GeneratorProfile::implementationHeaderString
"Returns the string for an implementation header.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationHeaderString
"Sets the string for an implementation header.";

%feature("docstring") libcellml::GeneratorProfile::interfaceVersionString
"Returns the string for the interface of the version constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceVersionString
"Sets the string for the interface of the version constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationVersionString
"Returns the string for the implementation of the version constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationVersionString
"Sets the string for the implementation of the version constant.";

%feature("docstring") libcellml::GeneratorProfile::interfaceLibcellmlVersionString
"Returns the string for the interface of the libCellML version constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceLibcellmlVersionString
"Sets the string for the interface of the libCellML version constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationLibcellmlVersionString
"Returns the string for the implementation of the libCellML version constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationLibcellmlVersionString
"Sets the string for the implementation of the libCellML version constant.";

%feature("docstring") libcellml::GeneratorProfile::interfaceStateCountString
"Returns the string for the interface of the state count constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceStateCountString
"Sets the string for the interface of the state count constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationStateCountString
"Returns the string for the implementation of the state count constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationStateCountString
"Sets the string for the implementation of the state count constant. To be useful, the string should contain the
[STATE_COUNT] tag, which will be replaced with the number of states in the model.";

%feature("docstring") libcellml::GeneratorProfile::interfaceConstantCountString
"Returns the string for the interface of the constant count constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceConstantCountString
"Sets the string for the interface of the constant count constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationConstantCountString
"Returns the string for the implementation of the constant count constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationConstantCountString
"Sets the string for the implementation of the constant count constant. To be useful, the string should contain
the [CONSTANT_COUNT] tag, which will be replaced with the number of states in the model.";

%feature("docstring") libcellml::GeneratorProfile::interfaceComputedConstantCountString
"Returns the string for the interface of the computed constant count constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceComputedConstantCountString
"Sets the string for the interface of the computed constant count constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationComputedConstantCountString
"Returns the string for the implementation of the computed constant count constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationComputedConstantCountString
"Sets the string for the implementation of the computed constant count constant. To be useful, the string should contain
the [COMPUTED_CONSTANT_COUNT] tag, which will be replaced with the number of states in the model.";

%feature("docstring") libcellml::GeneratorProfile::interfaceAlgebraicVariableCountString
"Returns the string for the interface of the algebraic variable count constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceAlgebraicVariableCountString
"Sets the string for the interface of the algebraic variable count constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationAlgebraicVariableCountString
"Returns the string for the implementation of the algebraic variable count constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationAlgebraicVariableCountString
"Sets the string for the implementation of the algebraic variable count constant. To be useful, the string should contain
the [ALGEBRAIC_VARIABLE_COUNT] tag, which will be replaced with the number of states in the model.";

%feature("docstring") libcellml::GeneratorProfile::interfaceExternalVariableCountString
"Returns the string for the interface of the external variable count constant.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceExternalVariableCountString
"Sets the string for the interface of the external variable count constant.";

%feature("docstring") libcellml::GeneratorProfile::implementationExternalVariableCountString
"Returns the string for the implementation of the external variable count constant.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationExternalVariableCountString
"Sets the string for the implementation of the external variable count constant. To be useful, the string should contain
the [EXTERNAL_VARIABLE_COUNT] tag, which will be replaced with the number of states in the model.";

%feature("docstring") libcellml::GeneratorProfile::variableInfoObjectString
"Returns the string for the data structure for the variable information object.";

%feature("docstring") libcellml::GeneratorProfile::setVariableInfoObjectString
"Sets the string for the data structure for the variable information object. To be useful, the string should
contain the [NAME_SIZE], [UNITS_SIZE] and [COMPONENT_SIZE] tags, which will be replaced with the maximum size of
a string for holding the name of a component, variable and units, respectively.";

%feature("docstring") libcellml::GeneratorProfile::interfaceVoiInfoString
"Returns the string for the interface of some information about the variable of integration.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceVoiInfoString
"Sets the string for the interface of some information about the variable of integration.";

%feature("docstring") libcellml::GeneratorProfile::implementationVoiInfoString
"Returns the string for the implementation of some information about the variable of integration.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationVoiInfoString
"Sets the string for the implementation of some information about the variable of integration. To be useful, the
string should contain the [CODE] tag, which will be replaced with some information about the variable of
integration.";

%feature("docstring") libcellml::GeneratorProfile::interfaceStateInfoString
"Returns the string for the interface of some information about the different states.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceStateInfoString
"Sets the string for the interface of some information about the different states.";

%feature("docstring") libcellml::GeneratorProfile::implementationStateInfoString
"Returns the string for the implementation of some information about the different states.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationStateInfoString
"Sets the string for the implementation of some information about the different states. To be useful, the string
should contain the [CODE] tag, which will be replaced with some information about the different states.";

%feature("docstring") libcellml::GeneratorProfile::interfaceConstantInfoString
"Returns the string for the interface of some information about the different constants.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceConstantInfoString
"Sets the string for the interface of some information about the different constants.";

%feature("docstring") libcellml::GeneratorProfile::implementationConstantInfoString
"Returns the string for the implementation of some information about the different constants.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationConstantInfoString
"Sets the string for the implementation of some information about the different constants. To be useful, the string
should contain the [CODE] tag, which will be replaced with some information about the different constants.";

%feature("docstring") libcellml::GeneratorProfile::interfaceComputedConstantInfoString
"Returns the string for the interface of some information about the different computed constants.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceComputedConstantInfoString
"Sets the string for the interface of some information about the different computed constants.";

%feature("docstring") libcellml::GeneratorProfile::implementationComputedConstantInfoString
"Returns the string for the implementation of some information about the different computed constants.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationComputedConstantInfoString
"Sets the string for the implementation of some information about the different computed constants. To be useful, the string
should contain the [CODE] tag, which will be replaced with some information about the different computed constants.";

%feature("docstring") libcellml::GeneratorProfile::interfaceAlgebraicVariableInfoString
"Returns the string for the interface of some information about the different algebraic variables.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceAlgebraicVariableInfoString
"Sets the string for the interface of some information about the different algebraic variables.";

%feature("docstring") libcellml::GeneratorProfile::implementationAlgebraicVariableInfoString
"Returns the string for the implementation of some information about the different algebraic variables.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationAlgebraicVariableInfoString
"Sets the string for the implementation of some information about the different algebraic variables. To be useful, the string
should contain the [CODE] tag, which will be replaced with some information about the different algebraic variables.";

%feature("docstring") libcellml::GeneratorProfile::interfaceExternalVariableInfoString
"Returns the string for the interface of some information about the different external variables.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceExternalVariableInfoString
"Sets the string for the interface of some information about the different external variables.";

%feature("docstring") libcellml::GeneratorProfile::implementationExternalVariableInfoString
"Returns the string for the implementation of some information about the different external variables.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationExternalVariableInfoString
"Sets the string for the implementation of some information about the different external variables. To be useful, the string
should contain the [CODE] tag, which will be replaced with some information about the different external variables.";

%feature("docstring") libcellml::GeneratorProfile::variableInfoEntryString
"Returns the string for an entry in an array for some information about a variable.";

%feature("docstring") libcellml::GeneratorProfile::setVariableInfoEntryString
"Sets the string for an entry in an array for some information about a variable. To be useful, the string should
contain the [COMPONENT], [NAME] and [UNITS] tags, which will be replaced with the name of the component, name
and units of a variable respectively.";

%feature("docstring") libcellml::GeneratorProfile::voiString
"Returns the string for the name of the variable of integration.";

%feature("docstring") libcellml::GeneratorProfile::setVoiString
"Sets the string for the name of the variable of integration.";

%feature("docstring") libcellml::GeneratorProfile::statesArrayString
"Returns the string for the name of the states array.";

%feature("docstring") libcellml::GeneratorProfile::setStatesArrayString
"Sets the string for the name of the states array.";

%feature("docstring") libcellml::GeneratorProfile::ratesArrayString
"Returns the string for the name of the rates array.";

%feature("docstring") libcellml::GeneratorProfile::setRatesArrayString
"Sets the string for the name of the rates array.";

%feature("docstring") libcellml::GeneratorProfile::constantsArrayString
"Returns the string for the name of the constants array.";

%feature("docstring") libcellml::GeneratorProfile::setConstantsArrayString
"Sets the string for the name of the constants array.";

%feature("docstring") libcellml::GeneratorProfile::computedConstantsArrayString
"Returns the string for the name of the computed constants array.";

%feature("docstring") libcellml::GeneratorProfile::setComputedConstantsArrayString
"Sets the string for the name of the computed constants array.";

%feature("docstring") libcellml::GeneratorProfile::algebraicVariablesArrayString
"Returns the string for the name of the algebraic variables array.";

%feature("docstring") libcellml::GeneratorProfile::setAlgebraicVariablesArrayString
"Sets the string for the name of the algebraic variables array.";

%feature("docstring") libcellml::GeneratorProfile::externalVariablesArrayString
"Returns the string for the name of the external variables array.";

%feature("docstring") libcellml::GeneratorProfile::setExternalVariablesArrayString
"Sets the string for the name of the external variables array.";

%feature("docstring") libcellml::GeneratorProfile::externalVariableMethodTypeDefinitionString
"Returns the string for the type definition of an external variable method.";

%feature("docstring") libcellml::GeneratorProfile::setExternalVariableMethodTypeDefinitionString
"Sets the string for the type definition of an external variable method.";

%feature("docstring") libcellml::GeneratorProfile::externalVariableMethodCallString
"Returns the string for the call to the external variable method.";

%feature("docstring") libcellml::GeneratorProfile::setExternalVariableMethodCallString
"Sets the string for the call to the external variable method.";

%feature("docstring") libcellml::GeneratorProfile::rootFindingInfoObjectString
"Returns the string for the data structure for the root finding information object.";

%feature("docstring") libcellml::GeneratorProfile::setRootFindingInfoObjectString
"Sets the string for the data structure for the root finding information object.";

%feature("docstring") libcellml::GeneratorProfile::externNlaSolveMethodString
"Returns the string for the external NLA solve method.";

%feature("docstring") libcellml::GeneratorProfile::setExternNlaSolveMethodString
"Sets the string for the external NLA solve method.";

%feature("docstring") libcellml::GeneratorProfile::findRootCallString
"Returns the string for the call to the find root method.";

%feature("docstring") libcellml::GeneratorProfile::setFindRootCallString
"Sets the string for the call to the find root method.";

%feature("docstring") libcellml::GeneratorProfile::findRootMethodString
"Returns the string for the find root method.";

%feature("docstring") libcellml::GeneratorProfile::setFindRootMethodString
"Sets the string for the find root method.";

%feature("docstring") libcellml::GeneratorProfile::nlaSolveCallString
"Returns the string for the call to the NLA solve method.";

%feature("docstring") libcellml::GeneratorProfile::setNlaSolveCallString
"Sets the string for the call to the NLA solve method.";

%feature("docstring") libcellml::GeneratorProfile::objectiveFunctionMethodString
"Returns the string for the objective function method.";

%feature("docstring") libcellml::GeneratorProfile::setObjectiveFunctionMethodString
"Sets the string for the objective function method.";

%feature("docstring") libcellml::GeneratorProfile::uArrayString
"Returns the string for the u array used in the objective function and find root methods.";

%feature("docstring") libcellml::GeneratorProfile::setUArrayString
"Sets the string for the u array used in the objective function and find root methods.";

%feature("docstring") libcellml::GeneratorProfile::fArrayString
"Returns the string for the f array used in the objective function and find root methods.";

%feature("docstring") libcellml::GeneratorProfile::setFArrayString
"Sets the string for the f array used in the objective function and find root methods.";

%feature("docstring") libcellml::GeneratorProfile::setReturnCreatedArrayString
"Sets the string for returning a created array. To be useful, the string should contain the [ARRAY_SIZE] tag,
which will be replaced with the size of the array to be created.";

%feature("docstring") libcellml::GeneratorProfile::interfaceCreateStatesArrayMethodString
"Returns the string for the interface to create the states array.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceCreateStatesArrayMethodString
"Sets the string for the interface to create the states array. To be useful, the string should contain the [CODE]
tag, which will be replaced with some code to create the states array.";

%feature("docstring") libcellml::GeneratorProfile::implementationCreateStatesArrayMethodString
"Returns the string for the implementation to create the states array.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationCreateStatesArrayMethodString
"Sets the string for the implementation to create the states array. To be useful, the string should contain the
[CODE] tag, which will be replaced with some code to create the states array.";

%feature("docstring") libcellml::GeneratorProfile::interfaceCreateConstantsArrayMethodString
"Returns the string for the interface to create the constants array.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceCreateConstantsArrayMethodString
"Sets the string for the interface to create the constants array. To be useful, the string should contain the [CODE]
tag, which will be replaced with some code to create the constants array.";

%feature("docstring") libcellml::GeneratorProfile::implementationCreateConstantsArrayMethodString
"Returns the string for the implementation to create the constants array.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationCreateConstantsArrayMethodString
"Sets the string for the implementation to create the constants array. To be useful, the string should contain the
[CODE] tag, which will be replaced with some code to create the constants array.";

%feature("docstring") libcellml::GeneratorProfile::interfaceCreateComputedConstantsArrayMethodString
"Returns the string for the interface to create the computed constants array.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceCreateComputedConstantsArrayMethodString
"Sets the string for the interface to create the computed constants array. To be useful, the string should contain the [CODE]
tag, which will be replaced with some code to create the computed constants array.";

%feature("docstring") libcellml::GeneratorProfile::implementationCreateComputedConstantsArrayMethodString
"Returns the string for the implementation to create the computed constants array.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationCreateComputedConstantsArrayMethodString
"Sets the string for the implementation to create the computed constants array. To be useful, the string should contain the
[CODE] tag, which will be replaced with some code to create the computed constants array.";

%feature("docstring") libcellml::GeneratorProfile::interfaceCreateAlgebraicVariablesArrayMethodString
"Returns the string for the interface to create the algebraic variables array.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceCreateAlgebraicVariablesArrayMethodString
"Sets the string for the interface to create the algebraic variables array. To be useful, the string should contain the [CODE]
tag, which will be replaced with some code to create the algebraic variables array.";

%feature("docstring") libcellml::GeneratorProfile::implementationCreateAlgebraicVariablesArrayMethodString
"Returns the string for the implementation to create the algebraic variables array.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationCreateAlgebraicVariablesArrayMethodString
"Sets the string for the implementation to create the algebraic variables array. To be useful, the string should contain the
[CODE] tag, which will be replaced with some code to create the algebraic variables array.";

%feature("docstring") libcellml::GeneratorProfile::interfaceCreateExternalVariablesArrayMethodString
"Returns the string for the interface to create the external variables array.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceCreateExternalVariablesArrayMethodString
"Sets the string for the interface to create the external variables array. To be useful, the string should contain the [CODE]
tag, which will be replaced with some code to create the external variables array.";

%feature("docstring") libcellml::GeneratorProfile::implementationCreateExternalVariablesArrayMethodString
"Returns the string for the implementation to create the external variables array.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationCreateExternalVariablesArrayMethodString
"Sets the string for the implementation to create the external variables array. To be useful, the string should contain the
[CODE] tag, which will be replaced with some code to create the external variables array.";

%feature("docstring") libcellml::GeneratorProfile::interfaceDeleteArrayMethodString
"Returns the string for the interface to delete an array.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceDeleteArrayMethodString
"Sets the string for the interface to delete an array.";

%feature("docstring") libcellml::GeneratorProfile::implementationDeleteArrayMethodString
"Returns the string for the implementation to delete an array.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationDeleteArrayMethodString
"Sets the string for the implementation to delete an array.";

%feature("docstring") libcellml::GeneratorProfile::interfaceInitialiseArraysMethodString
"Returns the string for the interface to initialise variables.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceInitialiseArraysMethodString
"Sets the string for the interface to initialise variables.";

%feature("docstring") libcellml::GeneratorProfile::implementationInitialiseArraysMethodString
"Returns the string for the implementation to initialise variables.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationInitialiseArraysMethodString
"Sets the string for the implementation to initialise variables.";

%feature("docstring") libcellml::GeneratorProfile::interfaceComputeComputedConstantsMethodString
"Returns the string for the interface to compute computed constants.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceComputeComputedConstantsMethodString
"Sets the string for the interface to compute computed constants.";

%feature("docstring") libcellml::GeneratorProfile::implementationComputeComputedConstantsMethodString
"Returns the string for the implementation to compute computed constants.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationComputeComputedConstantsMethodString
"Sets the string for the implementation to compute computed constants.";

%feature("docstring") libcellml::GeneratorProfile::interfaceComputeRatesMethodString
"Returns the string for the interface to compute rates.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceComputeRatesMethodString
"Sets the string for the interface to compute rates.";

%feature("docstring") libcellml::GeneratorProfile::implementationComputeRatesMethodString
"Returns the string for the implementation to compute rates.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationComputeRatesMethodString
"Sets the string for the implementation to compute rates.";

%feature("docstring") libcellml::GeneratorProfile::interfaceComputeVariablesMethodString
"Returns the string for the interface to compute variables.";

%feature("docstring") libcellml::GeneratorProfile::setInterfaceComputeVariablesMethodString
"Sets the string for the interface to compute variables.";

%feature("docstring") libcellml::GeneratorProfile::implementationComputeVariablesMethodString
"Returns the string for the implementation to compute variables.";

%feature("docstring") libcellml::GeneratorProfile::setImplementationComputeVariablesMethodString
"Sets the string for the implementation to compute variables.";

%feature("docstring") libcellml::GeneratorProfile::emptyMethodString
"Returns the string for an empty method.";

%feature("docstring") libcellml::GeneratorProfile::setEmptyMethodString
"Sets the string for an empty method.";

%feature("docstring") libcellml::GeneratorProfile::indentString
"Returns the string for an indent.";

%feature("docstring") libcellml::GeneratorProfile::setIndentString
"Sets the string for an indent.";

%feature("docstring") libcellml::GeneratorProfile::variableDeclarationString
"Returns the string for declaring a variable.";

%feature("docstring") libcellml::GeneratorProfile::setVariableDeclarationString
"Sets the string for declaring a variable.";

%feature("docstring") libcellml::GeneratorProfile::openArrayString
"Returns the string for opening an array.";

%feature("docstring") libcellml::GeneratorProfile::setOpenArrayString
"Sets the string for opening an array.";

%feature("docstring") libcellml::GeneratorProfile::closeArrayString
"Returns the string for closing an array.";

%feature("docstring") libcellml::GeneratorProfile::setCloseArrayString
"Sets the string for closing an array.";

%feature("docstring") libcellml::GeneratorProfile::arrayElementSeparatorString
"Returns the string for separating elements in an array.";

%feature("docstring") libcellml::GeneratorProfile::setArrayElementSeparatorString
"Sets the string for separating elements in an array.";

%feature("docstring") libcellml::GeneratorProfile::commandSeparatorString
"Returns the string for a command separator.";

%feature("docstring") libcellml::GeneratorProfile::setCommandSeparatorString
"Sets the string for a command separator.";

%{
#include "libcellml/generatorprofile.h"

#include <memory>
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(GeneratorProfile)
%extend libcellml::GeneratorProfile {
    GeneratorProfile(libcellml::GeneratorProfile::Profile profileType) {
        auto ptr = new std::shared_ptr<  libcellml::GeneratorProfile >(libcellml::GeneratorProfile::create(profileType));
        return reinterpret_cast<libcellml::GeneratorProfile *>(ptr);
    }
}

%include "libcellml/generatorprofile.h"
