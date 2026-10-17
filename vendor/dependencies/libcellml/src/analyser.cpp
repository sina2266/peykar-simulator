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

#include "libcellml/analyser.h"

#include <cmath>
#include <iterator>

#include "libcellml/analyserequation.h"
#include "libcellml/analyserexternalvariable.h"
#include "libcellml/analysermodel.h"
#include "libcellml/validator.h"

#include "analyser_p.h"
#include "analyserequation_p.h"
#include "analyserequationast_p.h"
#include "analysermodel_p.h"
#include "analyservariable_p.h"
#include "commonutils.h"
#include "generator_p.h"
#include "xmlutils.h"

namespace libcellml {

AnalyserInternalVariablePtr AnalyserInternalVariable::create(const VariablePtr &variable)
{
    auto res = AnalyserInternalVariablePtr {new AnalyserInternalVariable {}};

    res->setVariable(variable);

    return res;
}

void AnalyserInternalVariable::setVariable(const VariablePtr &variable,
                                           bool checkInitialValue)
{
    if (checkInitialValue && !variable->initialValue().empty()) {
        // The variable has an initial value, so it can either be a constant, an
        // algebraic variable (that needs to be computed using an NLA solver),
        // or a state. For now, all we know is that it is initialised.

        mType = Type::INITIALISED;

        mInitialisingVariable = variable;
    }

    mVariable = variable;
}

void AnalyserInternalVariable::makeVoi()
{
    mType = Type::VARIABLE_OF_INTEGRATION;
}

void AnalyserInternalVariable::makeState()
{
    switch (mType) {
    case Type::UNKNOWN:
        mType = Type::SHOULD_BE_STATE;

        break;
    case Type::INITIALISED:
        mType = Type::STATE;

        break;
    default: // Other types we don't care about.
        break;
    }
}

void AnalyserInternalVariable::makeConstant()
{
    mType = Type::CONSTANT;
}

AnalyserInternalEquationPtr AnalyserInternalEquation::create(const ComponentPtr &component)
{
    auto res = AnalyserInternalEquationPtr {new AnalyserInternalEquation {}};

    res->mAst = AnalyserEquationAst::create();
    res->mComponent = component;

    return res;
}

AnalyserInternalEquationPtr AnalyserInternalEquation::create(const AnalyserInternalVariablePtr &variable)
{
    auto res = AnalyserInternalEquationPtr {new AnalyserInternalEquation {}};

    res->mComponent = owningComponent(variable->mVariable);

    res->mUnknownVariables.push_back(variable);

    return res;
}

void AnalyserInternalEquation::addVariable(const AnalyserInternalVariablePtr &variable)
{
    if (std::find(mVariables.begin(), mVariables.end(), variable) == mVariables.end()) {
        mVariables.push_back(variable);
        mAllVariables.push_back(variable);
    }
}

void AnalyserInternalEquation::addStateVariable(const AnalyserInternalVariablePtr &stateVariable)
{
    if (std::find(mStateVariables.begin(), mStateVariables.end(), stateVariable) == mStateVariables.end()) {
        mStateVariables.push_back(stateVariable);
        mAllVariables.push_back(stateVariable);
    }
}

bool AnalyserInternalEquation::isKnownVariable(const AnalyserInternalVariablePtr &variable)
{
    return variable->mType != AnalyserInternalVariable::Type::UNKNOWN;
}

bool AnalyserInternalEquation::isKnownStateVariable(const AnalyserInternalVariablePtr &stateVariable)
{
    return stateVariable->mIsKnownStateVariable;
}

bool AnalyserInternalEquation::hasKnownVariables(const AnalyserInternalVariablePtrs &variables)
{
    return std::any_of(variables.begin(), variables.end(), [](const auto &v) {
        return isKnownVariable(v);
    });
}

bool AnalyserInternalEquation::hasKnownVariables()
{
    return hasKnownVariables(mVariables) || hasKnownVariables(mStateVariables);
}

bool AnalyserInternalEquation::isNonConstantVariable(const AnalyserInternalVariablePtr &variable)
{
    // Note: we don't check for AnalyserInternalVariable::Type::CONSTANT because
    //       a variable's type becomes constant at the very end, i.e. once we
    //       know for sure that it's neither a state variable nor a variable
    //       that is computed using an NLA system.

    return variable->mIsExternalVariable
           || ((variable->mType != AnalyserInternalVariable::Type::UNKNOWN)
               && (variable->mType != AnalyserInternalVariable::Type::INITIALISED)
               && (variable->mType != AnalyserInternalVariable::Type::COMPUTED_TRUE_CONSTANT)
               && (variable->mType != AnalyserInternalVariable::Type::COMPUTED_VARIABLE_BASED_CONSTANT));
}

bool AnalyserInternalEquation::hasNonConstantVariables(const AnalyserInternalVariablePtrs &variables)
{
    return std::any_of(variables.begin(), variables.end(), [](const auto &v) {
        return isNonConstantVariable(v);
    });
}

bool AnalyserInternalEquation::hasNonConstantVariables()
{
    return hasNonConstantVariables(mVariables) || hasNonConstantVariables(mStateVariables);
}

bool AnalyserInternalEquation::variableOnLhsRhs(const AnalyserInternalVariablePtr &variable,
                                                const AnalyserEquationAstPtr &astChild)
{
    switch (astChild->type()) {
    case AnalyserEquationAst::Type::CI:
        return astChild->variable()->name() == variable->mVariable->name();
    case AnalyserEquationAst::Type::DIFF:
        return astChild->rightChild()->variable()->name() == variable->mVariable->name();
    default:
        return false;
    }
}

bool AnalyserInternalEquation::variableOnRhs(const AnalyserInternalVariablePtr &variable)
{
    return variableOnLhsRhs(variable, mAst->rightChild());
}

bool AnalyserInternalEquation::variableOnLhsOrRhs(const AnalyserInternalVariablePtr &variable)
{
    return variableOnLhsRhs(variable, mAst->leftChild())
           || variableOnRhs(variable);
}

bool AnalyserInternalEquation::check(const AnalyserModelPtr &analyserModel, bool checkNlaSystems)
{
    // Nothing to check if the equation has a known type.

    if (mType != Type::UNKNOWN) {
        return false;
    }

    // Determine, from the (new) known (state) variables, whether the equation is
    // used to compute a true constant or a variable-based constant.

    mComputedTrueConstant = mComputedTrueConstant && !hasKnownVariables();
    mComputedVariableBasedConstant = mComputedVariableBasedConstant && !hasNonConstantVariables();

    // Add, as a dependency, the variables used to compute the (new) known (state)
    // variables.

    for (const auto &variable : mVariables) {
        if (isKnownVariable(variable)) {
            mDependencies.push_back(variable->mVariable);
        }
    }

    // Stop tracking (new) known (state) variables.

    mVariables.erase(std::remove_if(mVariables.begin(), mVariables.end(), isKnownVariable), mVariables.end());
    mStateVariables.erase(std::remove_if(mStateVariables.begin(), mStateVariables.end(), isKnownStateVariable), mStateVariables.end());

    // If there is no (state) variable left then it means that the variables in
    // the equation are overconstrained unless one of them was initialised in
    // which case it will now be considered as an algebraic variable and this
    // equation as an NLA equation.

    auto unknownVariablesOrStateVariablesLeft = mVariables.size() + mStateVariables.size();
    AnalyserInternalVariablePtrs initialisedVariables;

    if (checkNlaSystems && (unknownVariablesOrStateVariablesLeft == 0)) {
        for (const auto &variable : mAllVariables) {
            switch (variable->mType) {
            case AnalyserInternalVariable::Type::INITIALISED:
            case AnalyserInternalVariable::Type::INITIALISED_ALGEBRAIC_VARIABLE:
                // The equation contains an initialised variable, so track it
                // and consider it as an algebraic variable.

                initialisedVariables.push_back(variable);

                variable->mType = AnalyserInternalVariable::Type::INITIALISED_ALGEBRAIC_VARIABLE;

                break;
            default:
                break;
            }
        }

        if (initialisedVariables.empty()) {
            // The equation doesn't contain any initialised variables, which
            // means that it is overconstrained.

            for (const auto &variable : mAllVariables) {
                variable->mType = AnalyserInternalVariable::Type::OVERCONSTRAINED;
            }

            return false;
        }
    }

    // If there is one (state) variable left (on its own on the LHS/RHS of the
    // equation or in case we check for NLA systems) or some initialised
    // variables then update its variable (to be the corresponding one in the
    // component in which the equation is), as well as set its type (if it is
    // currently unknown) and index (if its type is one of the expected ones).
    // Finally, set the type and order of the equation, should everything have
    // gone as planned.

    auto unknownVariableLeft = (unknownVariablesOrStateVariablesLeft == 1) ?
                                   mVariables.empty() ?
                                   mStateVariables.front() :
                                   mVariables.front() :
                                   nullptr;

    if (((unknownVariableLeft != nullptr)
         && (checkNlaSystems || variableOnLhsOrRhs(unknownVariableLeft)))
        || !initialisedVariables.empty()) {
        auto variables = mVariables.empty() ?
                             mStateVariables.empty() ?
                             initialisedVariables :
                             mStateVariables :
                             mVariables;

        for (const auto &variable : variables) {
            auto i = MAX_SIZE_T;
            VariablePtr localVariable;

            do {
                localVariable = mComponent->variable(++i);
            } while (!analyserModel->areEquivalentVariables(variable->mVariable, localVariable));

            variable->setVariable(localVariable, false);

            if (variable->mType == AnalyserInternalVariable::Type::UNKNOWN) {
                variable->mType = mComputedTrueConstant ?
                                      AnalyserInternalVariable::Type::COMPUTED_TRUE_CONSTANT :
                                  mComputedVariableBasedConstant ?
                                      AnalyserInternalVariable::Type::COMPUTED_VARIABLE_BASED_CONSTANT :
                                      AnalyserInternalVariable::Type::ALGEBRAIC_VARIABLE;
            }

            switch (variable->mType) {
            case AnalyserInternalVariable::Type::STATE:
            case AnalyserInternalVariable::Type::COMPUTED_TRUE_CONSTANT:
            case AnalyserInternalVariable::Type::COMPUTED_VARIABLE_BASED_CONSTANT:
            case AnalyserInternalVariable::Type::INITIALISED_ALGEBRAIC_VARIABLE:
            case AnalyserInternalVariable::Type::ALGEBRAIC_VARIABLE:
                variable->mIsKnownStateVariable = variable->mType == AnalyserInternalVariable::Type::STATE;

                mUnknownVariables.push_back(variable);

                break;
            default:
                return false;
            }
        }

        // Set the equation's order and type.
        // Note: an equation may be used to compute one variable, but if it is
        //       not on its own on the LHS/RHS of the equation then it needs to
        //       be solved as an NLA equation.

        if ((unknownVariableLeft == nullptr)
            || !variableOnLhsOrRhs(unknownVariableLeft)) {
            mType = Type::NLA;
        } else {
            switch (unknownVariableLeft->mType) {
            case AnalyserInternalVariable::Type::STATE:
                mType = Type::ODE;

                break;
            case AnalyserInternalVariable::Type::COMPUTED_TRUE_CONSTANT:
                mType = Type::CONSTANT;

                break;
            case AnalyserInternalVariable::Type::COMPUTED_VARIABLE_BASED_CONSTANT:
                mType = Type::COMPUTED_CONSTANT;

                break;
            default:
                mType = Type::ALGEBRAIC;

                break;
            }
        }

        // An ODE equation may have a dependency on the state of that ODE (e.g.,
        // dx/dt = x+3). Similarly, an NLA equation will have a "dependency" on
        // its unknown variables. Either way, we must remove our "dependencies"
        // on our unknown variables or we will end up in a circular dependency.

        for (const auto &unknownVariable : mUnknownVariables) {
            auto it = std::find(mDependencies.begin(), mDependencies.end(), unknownVariable->mVariable);

            if (it != mDependencies.end()) {
                mDependencies.erase(it);
            }
        }

        return true;
    }

    return false;
}

Analyser::AnalyserImpl::AnalyserImpl()
{
    // Customise our generator's profile.

    mGeneratorProfile->setAbsoluteValueString("abs");
    mGeneratorProfile->setNaturalLogarithmString("ln");
    mGeneratorProfile->setCommonLogarithmString("log");
    mGeneratorProfile->setRemString("rem");
    mGeneratorProfile->setAsinString("arcsin");
    mGeneratorProfile->setAcosString("arccos");
    mGeneratorProfile->setAtanString("arctan");
    mGeneratorProfile->setAsecString("arcsec");
    mGeneratorProfile->setAcscString("arccsc");
    mGeneratorProfile->setAcotString("arccot");
    mGeneratorProfile->setAsinhString("arcsinh");
    mGeneratorProfile->setAcoshString("arccosh");
    mGeneratorProfile->setAtanhString("arctanh");
    mGeneratorProfile->setAsechString("arcsech");
    mGeneratorProfile->setAcschString("arccsch");
    mGeneratorProfile->setAcothString("arccoth");
    mGeneratorProfile->setTrueString("true");
    mGeneratorProfile->setFalseString("false");
    mGeneratorProfile->setEString("exponentiale");
    mGeneratorProfile->setPiString("pi");
    mGeneratorProfile->setInfString("infinity");
    mGeneratorProfile->setNanString("notanumber");
}

AnalyserInternalVariablePtr Analyser::AnalyserImpl::internalVariable(const VariablePtr &variable)
{
    // Find and return, if there is one, the internal variable associated with
    // the given variable.

    auto rawPtr = reinterpret_cast<uintptr_t>(variable.get());
    auto rawPtrIt = mInternalVariableMap.find(rawPtr);

    if (rawPtrIt != mInternalVariableMap.end()) {
        return rawPtrIt->second;
    }

    for (const auto &internalVariable : mInternalVariables) {
        if (mAnalyserModel->areEquivalentVariables(variable, internalVariable->mVariable)) {
            mInternalVariableMap[rawPtr] = internalVariable;

            return internalVariable;
        }
    }

    // No internal variable exists for the given variable, so create one, track
    // it and return it.

    auto res = AnalyserInternalVariable::create(variable);

    mInternalVariables.push_back(res);

    mInternalVariableMap[rawPtr] = res;

    return res;
}

VariablePtr Analyser::AnalyserImpl::voiFirstOccurrence(const VariablePtr &variable,
                                                       const ComponentPtr &component)
{
    // Recursively look for the first occurrence of the given variable in the
    // given component.

    for (size_t i = 0; i < component->variableCount(); ++i) {
        auto componentVariable = component->variable(i);

        if (mAnalyserModel->areEquivalentVariables(variable, componentVariable)) {
            return componentVariable;
        }
    }

    VariablePtr res;

    for (size_t i = 0; (res == nullptr) && (i < component->componentCount()); ++i) {
        res = voiFirstOccurrence(variable, component->component(i));
    }

    return res;
}

void Analyser::AnalyserImpl::analyseNode(const XmlNodePtr &node,
                                         AnalyserEquationAstPtr &ast,
                                         const AnalyserEquationAstPtr &astParent,
                                         const ComponentPtr &component,
                                         const AnalyserInternalEquationPtr &equation)
{
    // Create the AST, if needed.

    if (ast == nullptr) {
        ast.reset(new AnalyserEquationAst {});
    }

    // Basic content elements.

    if (node->isMathmlElement("apply")) {
        // We may have 1, 2, 3 or more child nodes, e.g.
        //
        //                 +--------+
        //                 |   +    |
        //        "+a" ==> |  / \   |
        //                 | a  nil |
        //                 +--------+
        //
        //                 +-------+
        //                 |   +   |
        //       "a+b" ==> |  / \  |
        //                 | a   b |
        //                 +-------+
        //
        //                 +-------------+
        //                 |   +         |
        //                 |  / \        |
        //                 | a   +       |
        //                 |    / \      |
        // "a+b+c+d+e" ==> |   b   +     |
        //                 |      / \    |
        //                 |     c   +   |
        //                 |        / \  |
        //                 |       d   e |
        //                 +-------------+

        auto childCount = mathmlChildCount(node);
        AnalyserEquationAstPtr astRightChild;
        AnalyserEquationAstPtr tempAst;

        for (size_t i = childCount - 1; i > 0; --i) {
            astRightChild = tempAst;
            tempAst = AnalyserEquationAst::create();

            if (astRightChild != nullptr) {
                if (i == childCount - 2) {
                    astRightChild->swapLeftAndRightChildren();

                    tempAst = astRightChild;
                } else {
                    astRightChild->mPimpl->mParent = tempAst;
                    tempAst->mPimpl->mOwnedRightChild = astRightChild;
                }
            }

            if (i != childCount - 2) {
                analyseNode(mathmlChildNode(node, 0), tempAst, nullptr, component, equation);
            }

            analyseNode(mathmlChildNode(node, i), tempAst->mPimpl->mOwnedLeftChild, tempAst, component, equation);
        }

        analyseNode(mathmlChildNode(node, 0), tempAst, astParent, component, equation);

        ast = tempAst;

        // Relational and logical operators.

    } else if (node->isMathmlElement("eq")) {
        // This element is used both to describe "a = b" and "a == b". We can
        // distinguish between the two by checking its grandparent. If it's a
        // "math" element then it means that it is used to describe "a = b"
        // otherwise it is used to describe "a == b". In the former case, there
        // is nothing more we need to do since `ast` is already of
        // AnalyserEquationAst::Type::EQUALITY type.

        if (!node->parent()->parent()->isMathmlElement("math")) {
            ast->mPimpl->populate(AnalyserEquationAst::Type::EQ, astParent);

            mAnalyserModel->mPimpl->mNeedEqFunction = true;
        }
    } else if (node->isMathmlElement("neq")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::NEQ, astParent);

        mAnalyserModel->mPimpl->mNeedNeqFunction = true;
    } else if (node->isMathmlElement("lt")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::LT, astParent);

        mAnalyserModel->mPimpl->mNeedLtFunction = true;
    } else if (node->isMathmlElement("leq")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::LEQ, astParent);

        mAnalyserModel->mPimpl->mNeedLeqFunction = true;
    } else if (node->isMathmlElement("gt")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::GT, astParent);

        mAnalyserModel->mPimpl->mNeedGtFunction = true;
    } else if (node->isMathmlElement("geq")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::GEQ, astParent);

        mAnalyserModel->mPimpl->mNeedGeqFunction = true;
    } else if (node->isMathmlElement("and")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::AND, astParent);

        mAnalyserModel->mPimpl->mNeedAndFunction = true;
    } else if (node->isMathmlElement("or")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::OR, astParent);

        mAnalyserModel->mPimpl->mNeedOrFunction = true;
    } else if (node->isMathmlElement("xor")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::XOR, astParent);

        mAnalyserModel->mPimpl->mNeedXorFunction = true;
    } else if (node->isMathmlElement("not")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::NOT, astParent);

        mAnalyserModel->mPimpl->mNeedNotFunction = true;

        // Arithmetic operators.

    } else if (node->isMathmlElement("plus")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::PLUS, astParent);
    } else if (node->isMathmlElement("minus")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::MINUS, astParent);
    } else if (node->isMathmlElement("times")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::TIMES, astParent);
    } else if (node->isMathmlElement("divide")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::DIVIDE, astParent);
    } else if (node->isMathmlElement("power")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::POWER, astParent);
    } else if (node->isMathmlElement("root")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ROOT, astParent);
    } else if (node->isMathmlElement("abs")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ABS, astParent);
    } else if (node->isMathmlElement("exp")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::EXP, astParent);
    } else if (node->isMathmlElement("ln")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::LN, astParent);
    } else if (node->isMathmlElement("log")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::LOG, astParent);
    } else if (node->isMathmlElement("ceiling")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::CEILING, astParent);
    } else if (node->isMathmlElement("floor")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::FLOOR, astParent);
    } else if (node->isMathmlElement("min")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::MIN, astParent);

        mAnalyserModel->mPimpl->mNeedMinFunction = true;
    } else if (node->isMathmlElement("max")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::MAX, astParent);

        mAnalyserModel->mPimpl->mNeedMaxFunction = true;
    } else if (node->isMathmlElement("rem")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::REM, astParent);

        // Calculus elements.

    } else if (node->isMathmlElement("diff")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::DIFF, astParent);

        // Trigonometric operators.

    } else if (node->isMathmlElement("sin")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::SIN, astParent);
    } else if (node->isMathmlElement("cos")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::COS, astParent);
    } else if (node->isMathmlElement("tan")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::TAN, astParent);
    } else if (node->isMathmlElement("sec")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::SEC, astParent);

        mAnalyserModel->mPimpl->mNeedSecFunction = true;
    } else if (node->isMathmlElement("csc")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::CSC, astParent);

        mAnalyserModel->mPimpl->mNeedCscFunction = true;
    } else if (node->isMathmlElement("cot")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::COT, astParent);

        mAnalyserModel->mPimpl->mNeedCotFunction = true;
    } else if (node->isMathmlElement("sinh")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::SINH, astParent);
    } else if (node->isMathmlElement("cosh")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::COSH, astParent);
    } else if (node->isMathmlElement("tanh")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::TANH, astParent);
    } else if (node->isMathmlElement("sech")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::SECH, astParent);

        mAnalyserModel->mPimpl->mNeedSechFunction = true;
    } else if (node->isMathmlElement("csch")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::CSCH, astParent);

        mAnalyserModel->mPimpl->mNeedCschFunction = true;
    } else if (node->isMathmlElement("coth")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::COTH, astParent);

        mAnalyserModel->mPimpl->mNeedCothFunction = true;
    } else if (node->isMathmlElement("arcsin")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ASIN, astParent);
    } else if (node->isMathmlElement("arccos")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ACOS, astParent);
    } else if (node->isMathmlElement("arctan")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ATAN, astParent);
    } else if (node->isMathmlElement("arcsec")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ASEC, astParent);

        mAnalyserModel->mPimpl->mNeedAsecFunction = true;
    } else if (node->isMathmlElement("arccsc")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ACSC, astParent);

        mAnalyserModel->mPimpl->mNeedAcscFunction = true;
    } else if (node->isMathmlElement("arccot")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ACOT, astParent);

        mAnalyserModel->mPimpl->mNeedAcotFunction = true;
    } else if (node->isMathmlElement("arcsinh")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ASINH, astParent);
    } else if (node->isMathmlElement("arccosh")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ACOSH, astParent);
    } else if (node->isMathmlElement("arctanh")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ATANH, astParent);
    } else if (node->isMathmlElement("arcsech")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ASECH, astParent);

        mAnalyserModel->mPimpl->mNeedAsechFunction = true;
    } else if (node->isMathmlElement("arccsch")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ACSCH, astParent);

        mAnalyserModel->mPimpl->mNeedAcschFunction = true;
    } else if (node->isMathmlElement("arccoth")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::ACOTH, astParent);

        mAnalyserModel->mPimpl->mNeedAcothFunction = true;

        // Piecewise statement.

    } else if (node->isMathmlElement("piecewise")) {
        auto childCount = mathmlChildCount(node);

        ast->mPimpl->populate(AnalyserEquationAst::Type::PIECEWISE, astParent);

        analyseNode(mathmlChildNode(node, 0), ast->mPimpl->mOwnedLeftChild, ast, component, equation);

        if (childCount >= 2) {
            AnalyserEquationAstPtr astRight;
            AnalyserEquationAstPtr tempAst;

            analyseNode(mathmlChildNode(node, childCount - 1), astRight, nullptr, component, equation);

            for (auto i = childCount - 2; i > 0; --i) {
                tempAst = AnalyserEquationAst::create();

                tempAst->mPimpl->populate(AnalyserEquationAst::Type::PIECEWISE, astParent);

                analyseNode(mathmlChildNode(node, i), tempAst->mPimpl->mOwnedLeftChild, tempAst, component, equation);

                astRight->mPimpl->mParent = tempAst;

                tempAst->mPimpl->mOwnedRightChild = astRight;
                astRight = tempAst;
            }

            astRight->mPimpl->mParent = ast;

            ast->mPimpl->mOwnedRightChild = astRight;
        }
    } else if (node->isMathmlElement("piece")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::PIECE, astParent);

        analyseNode(mathmlChildNode(node, 0), ast->mPimpl->mOwnedLeftChild, ast, component, equation);
        analyseNode(mathmlChildNode(node, 1), ast->mPimpl->mOwnedRightChild, ast, component, equation);
    } else if (node->isMathmlElement("otherwise")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::OTHERWISE, astParent);

        analyseNode(mathmlChildNode(node, 0), ast->mPimpl->mOwnedLeftChild, ast, component, equation);

        // Token elements.

    } else if (node->isMathmlElement("ci")) {
        auto variableName = node->firstChild()->convertToStrippedString();
        auto variable = component->variable(variableName);
        // Note: we always have a variable. Indeed, if we were not to have one,
        //       it would mean that `variableName` is the name of a variable
        //       that is referenced in an equation, but not defined anywhere,
        //       something that is not allowed in CellML and will therefore be
        //       reported when we validate the model.

        // Have our equation track the (state) variable (by state variable, we mean
        // a variable that is used in a "diff" element).

        if (node->parent()->firstChild()->isMathmlElement("diff")) {
            equation->addStateVariable(internalVariable(variable));
        } else if (!node->parent()->isMathmlElement("bvar")) {
            equation->addVariable(internalVariable(variable));
        }

        // Add the variable to our AST and keep track of its unit.

        ast->mPimpl->populate(AnalyserEquationAst::Type::CI, variable, astParent);

        mCiCnUnits.emplace(ast, variable->units());
    } else if (node->isMathmlElement("cn")) {
        // Add the number to our AST and keep track of its unit. Note that in
        // the case of a standard unit, we need to create a units since it's
        // not declared in the model.

        if (mathmlChildCount(node) == 1) {
            // We are dealing with an e-notation based CN value.

            ast->mPimpl->populate(AnalyserEquationAst::Type::CN, node->firstChild()->convertToStrippedString() + "e" + node->firstChild()->next()->next()->convertToStrippedString(), astParent);
        } else {
            ast->mPimpl->populate(AnalyserEquationAst::Type::CN, node->firstChild()->convertToStrippedString(), astParent);
        }

        std::string unitsName = node->attribute("units");

        if (isStandardUnitName(unitsName)) {
            auto iter = mStandardUnits.find(unitsName);

            if (iter == mStandardUnits.end()) {
                auto units = Units::create(unitsName);

                mCiCnUnits.emplace(ast, units);
                mStandardUnits.emplace(unitsName, units);
            } else {
                mCiCnUnits.emplace(ast, iter->second);
            }
        } else {
            mCiCnUnits.emplace(ast, owningModel(component)->units(unitsName));
        }

        // Qualifier elements.

    } else if (node->isMathmlElement("degree")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::DEGREE, astParent);

        analyseNode(mathmlChildNode(node, 0), ast->mPimpl->mOwnedLeftChild, ast, component, equation);
    } else if (node->isMathmlElement("logbase")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::LOGBASE, astParent);

        analyseNode(mathmlChildNode(node, 0), ast->mPimpl->mOwnedLeftChild, ast, component, equation);
    } else if (node->isMathmlElement("bvar")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::BVAR, astParent);

        analyseNode(mathmlChildNode(node, 0), ast->mPimpl->mOwnedLeftChild, ast, component, equation);

        auto rightNode = mathmlChildNode(node, 1);

        if (rightNode != nullptr) {
            analyseNode(rightNode, ast->mPimpl->mOwnedRightChild, ast, component, equation);
        }

        // Constants.

    } else if (node->isMathmlElement("true")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::TRUE, astParent);
    } else if (node->isMathmlElement("false")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::FALSE, astParent);
    } else if (node->isMathmlElement("exponentiale")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::E, astParent);
    } else if (node->isMathmlElement("pi")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::PI, astParent);
    } else if (node->isMathmlElement("infinity")) {
        ast->mPimpl->populate(AnalyserEquationAst::Type::INF, astParent);
    } else {
        // We have checked for everything, so if we reach this point it means
        // that we have a NaN.

        ast->mPimpl->populate(AnalyserEquationAst::Type::NAN, astParent);
    }
}

void Analyser::AnalyserImpl::analyseComponent(const ComponentPtr &component)
{
    // Retrieve the math string associated with the given component and analyse
    // it, one equation at a time, keeping in mind that it may consist of
    // several <math> elements, hence our use of multiRootXml().

    if (!component->math().empty()) {
        for (const auto &doc : multiRootXml(component->math())) {
            for (auto node = doc->rootNode()->firstChild(); node != nullptr; node = node->next()) {
                if (node->isMathmlElement()) {
                    // Create and keep track of the equation associated with the
                    // given node.

                    auto internalEquation = AnalyserInternalEquation::create(component);

                    mInternalEquations.push_back(internalEquation);

                    // Actually analyse the node.
                    // Note: we must not test internalEquation->mAst->parent()
                    //       since if it is equal to nullptr then a parent will
                    //       be created by analyseNode().

                    analyseNode(node, internalEquation->mAst, internalEquation->mAst->parent(), component, internalEquation);

                    // Make sure that our internal equation is an equality
                    // statement.

                    if (internalEquation->mAst->mPimpl->mType != AnalyserEquationAst::Type::EQUALITY) {
                        auto issue = Issue::IssueImpl::create();

                        issue->mPimpl->setDescription("Equation " + expression(internalEquation->mAst)
                                                      + " is not an equality statement (i.e. LHS = RHS).");
                        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT);
                        issue->mPimpl->mItem->mPimpl->setComponent(component);

                        addIssue(issue);
                    }
                }
            }
        }
    }

    // Go through the given component's variables and internally keep track of
    // the ones that have an initial value.

    for (size_t i = 0; i < component->variableCount(); ++i) {
        // If `variable` has an initial value and the variable held by
        // `internalVariable` doesn't, then replace the variable held by
        // `internalVariable`.

        auto variable = component->variable(i);
        auto internalVariable = Analyser::AnalyserImpl::internalVariable(variable);

        if (!variable->initialValue().empty()
            && internalVariable->mVariable->initialValue().empty()) {
            internalVariable->setVariable(variable);
        }
    }

    // Do the same for the components encapsulated by the given component.

    for (size_t i = 0; i < component->componentCount(); ++i) {
        analyseComponent(component->component(i));
    }
}

void Analyser::AnalyserImpl::analyseComponentVariables(const ComponentPtr &component)
{
    // Go through the given component's variables and make sure that everything
    // makes sense.

    for (size_t i = 0; i < component->variableCount(); ++i) {
        // If `variable` and the variable held by `internalVariable` are
        // different then make sure that they don't both have an initial value.
        // Alternatively, if the variable held by `internalVariable` has an
        // initial value which is the name of another variable then make sure
        // that it has the same units.
        // Note: we always have an initialising variable in the second case.
        //       Indeed, if we were not to have one, it would mean that the
        //       variable is initialised using a reference to a variable that is
        //       not defined anywhere, something that is not allowed in CellML
        //       and will therefore be reported when we validate the model.

        auto variable = component->variable(i);
        auto internalVariable = Analyser::AnalyserImpl::internalVariable(variable);

        if ((variable != internalVariable->mVariable)
            && !variable->initialValue().empty()) {
            auto issue = Issue::IssueImpl::create();

            issue->mPimpl->setDescription("Variable '" + variable->name()
                                          + "' in component '" + component->name()
                                          + "' and variable '" + internalVariable->mVariable->name()
                                          + "' in component '" + owningComponent(internalVariable->mVariable)->name()
                                          + "' are equivalent and cannot therefore both be initialised.");
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE);
            issue->mPimpl->mItem->mPimpl->setVariable(variable);

            addIssue(issue);
        } else if (!variable->initialValue().empty()
                   && !isCellMLReal(variable->initialValue())) {
            auto initialisingVariable = owningComponent(variable)->variable(variable->initialValue());
            auto initialisingInternalVariable = Analyser::AnalyserImpl::internalVariable(initialisingVariable);

            if (initialisingInternalVariable->mType == AnalyserInternalVariable::Type::INITIALISED) {
                auto scalingFactor = Units::scalingFactor(variable->units(), initialisingVariable->units());

                if (!areNearlyEqual(scalingFactor, 1.0)) {
                    auto issue = Issue::IssueImpl::create();

                    issue->mPimpl->setDescription("Variable '" + variable->name()
                                                  + "' in component '" + component->name()
                                                  + "' is initialised using variable '" + variable->initialValue()
                                                  + "' which has different units.");
                    issue->mPimpl->setLevel(Issue::Level::WARNING);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_VARIABLE_WITH_DIFFERENT_UNITS);
                    issue->mPimpl->mItem->mPimpl->setVariable(variable);

                    addIssue(issue);
                }
            }
        }
    }

    // Do the same for the components encapsulated by the given component.

    for (size_t i = 0; i < component->componentCount(); ++i) {
        analyseComponentVariables(component->component(i));
    }
}

void Analyser::AnalyserImpl::equivalentVariables(const VariablePtr &variable,
                                                 VariablePtrs &equivVariables) const
{
    for (size_t i = 0; i < variable->equivalentVariableCount(); ++i) {
        auto equivVariable = variable->equivalentVariable(i);

        if (std::find(equivVariables.begin(), equivVariables.end(), equivVariable) == equivVariables.end()) {
            equivVariables.push_back(equivVariable);

            equivalentVariables(equivVariable, equivVariables);
        }
    }
}

VariablePtrs Analyser::AnalyserImpl::equivalentVariables(const VariablePtr &variable) const
{
    VariablePtrs res = {variable};

    equivalentVariables(variable, res);

    return res;
}

void Analyser::AnalyserImpl::analyseEquationAst(const AnalyserEquationAstPtr &ast)
{
    // Make sure that we have an AST to analyse.

    if (ast == nullptr) {
        return;
    }

    // Look for the definition of a variable of integration and make sure that
    // we don't have more than one of it and that it's not initialised.

    auto astParent = ast->parent();
    auto astGrandparent = (astParent != nullptr) ? astParent->parent() : nullptr;
    auto astGreatGrandparent = (astGrandparent != nullptr) ? astGrandparent->parent() : nullptr;

    if ((ast->mPimpl->mType == AnalyserEquationAst::Type::CI)
        && (astParent->mPimpl->mType == AnalyserEquationAst::Type::BVAR)) {
        auto astVariable = ast->variable();

        internalVariable(astVariable)->makeVoi();
        // Note: we must make the variable a variable of integration in all
        //       cases (i.e. even if there is, for example, already another
        //       variable of integration) otherwise unnecessary issue messages
        //       may be reported (since the type of the variable would be
        //       unknown).

        if (mAnalyserModel->mPimpl->mVoi == nullptr) {
            // We have found our variable of integration, but this may not be
            // the one defined in our first component (i.e. the component under
            // which we are likely to expect to see the variable of integration
            // to be defined), so go through our components and look for the
            // first occurrence of our variable of integration.

            auto model = owningModel(astVariable);
            auto i = MAX_SIZE_T;
            VariablePtr voi;

            do {
                voi = voiFirstOccurrence(astVariable, model->component(++i));

                if (voi != nullptr) {
                    // We have found the first occurrence of our variable of
                    // integration, but now we must ensure that it (or any of
                    // its equivalent variables) is not initialised.

                    auto isVoiInitialised = false;

                    for (const auto &voiEquivalentVariable : equivalentVariables(voi)) {
                        if (!voiEquivalentVariable->initialValue().empty()) {
                            auto issue = Issue::IssueImpl::create();

                            issue->mPimpl->setDescription("Variable '" + voiEquivalentVariable->name()
                                                          + "' in component '" + owningComponent(voiEquivalentVariable)->name()
                                                          + "' cannot be both a variable of integration and initialised.");
                            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_VOI_INITIALISED);
                            issue->mPimpl->mItem->mPimpl->setVariable(voiEquivalentVariable);

                            addIssue(issue);

                            isVoiInitialised = true;
                        }
                    }

                    if (!isVoiInitialised) {
                        mAnalyserModel->mPimpl->mVoi = AnalyserVariable::AnalyserVariableImpl::create();

                        mAnalyserModel->mPimpl->mVoi->mPimpl->populate(AnalyserVariable::Type::VARIABLE_OF_INTEGRATION,
                                                                       0, nullptr, voi, mAnalyserModel, {});
                    }
                }
            } while (voi == nullptr);
        } else {
            auto voiVariable = mAnalyserModel->mPimpl->mVoi->variable();

            if (!mAnalyserModel->areEquivalentVariables(astVariable, voiVariable)) {
                auto issue = Issue::IssueImpl::create();

                issue->mPimpl->setDescription("Variable '" + voiVariable->name()
                                              + "' in component '" + owningComponent(voiVariable)->name()
                                              + "' and variable '" + astVariable->name()
                                              + "' in component '" + owningComponent(astVariable)->name()
                                              + "' cannot both be the variable of integration.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_VOI_SEVERAL);
                issue->mPimpl->mItem->mPimpl->setVariable(astVariable);

                addIssue(issue);
            }
        }
    }

    // Make sure that we only use first-order ODEs.

    if ((ast->mPimpl->mType == AnalyserEquationAst::Type::CN)
        && (astParent->mPimpl->mType == AnalyserEquationAst::Type::DEGREE)
        && (astGrandparent->mPimpl->mType == AnalyserEquationAst::Type::BVAR)) {
        double value;

        convertToDouble(ast->mPimpl->mValue, value);

        if (!areEqual(value, 1.0)) {
            auto variable = astGreatGrandparent->mPimpl->mOwnedRightChild->variable();
            auto issue = Issue::IssueImpl::create();

            issue->mPimpl->setDescription("The differential equation for variable '" + variable->name()
                                          + "' in component '" + owningComponent(variable)->name()
                                          + "' must be of the first order.");
            issue->mPimpl->mItem->mPimpl->setMath(owningComponent(variable));
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_ODE_NOT_FIRST_ORDER);

            addIssue(issue);
        }
    }

    // Make a variable a state if it is used in an ODE.

    if ((ast->mPimpl->mType == AnalyserEquationAst::Type::CI)
        && (astParent->mPimpl->mType == AnalyserEquationAst::Type::DIFF)) {
        internalVariable(ast->variable())->makeState();
    }

    // Recursively check the given AST's children.

    analyseEquationAst(ast->mPimpl->mOwnedLeftChild);
    analyseEquationAst(ast->mPimpl->mOwnedRightChild);
}

void Analyser::AnalyserImpl::updateUnitsMapWithStandardUnit(const std::string &unitsName,
                                                            UnitsMap &unitsMap,
                                                            double unitsExponent)
{
    // Update the given units map using the given standard unit.

    for (const auto &iter : standardUnitsList.at(unitsName)) {
        if (unitsMap.find(iter.first) == unitsMap.end()) {
            unitsMap.emplace(iter.first, 0.0);
        }

        unitsMap[iter.first] += iter.second * unitsExponent;
    }
}

void Analyser::AnalyserImpl::updateUnitsMap(const ModelPtr &model,
                                            const std::string &unitsName,
                                            UnitsMap &unitsMap,
                                            bool userUnitsMap,
                                            double unitsExponent,
                                            double unitsMultiplier)
{
    // Update the given units map using the given information.

    if (userUnitsMap) {
        if (unitsName != "dimensionless") {
            unitsMap.emplace(unitsName, unitsExponent);
        }
    } else {
        if (isStandardUnitName(unitsName)) {
            updateUnitsMapWithStandardUnit(unitsName, unitsMap, unitsExponent);
        } else {
            UnitsPtr units = model->units(unitsName);

            if (units->isBaseUnit()) {
                auto iter = unitsMap.find(unitsName);

                if (iter == unitsMap.end()) {
                    unitsMap.emplace(unitsName, unitsExponent);
                } else {
                    unitsMap[iter->first] += unitsExponent;
                }
            } else {
                std::string reference;
                std::string prefix;
                double exponent;
                double multiplier;
                std::string id;

                for (size_t i = 0; i < units->unitCount(); ++i) {
                    units->unitAttributes(i, reference, prefix, exponent, multiplier, id);

                    if (isStandardUnitName(reference)) {
                        updateUnitsMapWithStandardUnit(reference, unitsMap, exponent * unitsExponent);
                    } else {
                        updateUnitsMap(model, reference, unitsMap, userUnitsMap,
                                       exponent * unitsExponent,
                                       unitsMultiplier + (std::log10(multiplier) + convertPrefixToInt(prefix)) * unitsExponent);
                    }
                }
            }
        }
    }
}

UnitsMap Analyser::AnalyserImpl::multiplyDivideUnitsMaps(const UnitsMap &firstUnitsMap,
                                                         const UnitsMap &secondUnitsMap,
                                                         bool multiply)
{
    // Multiply/divide the given units maps together, following a multiplication
    // (multiply = true) or a division (multiply = false).

    auto res = firstUnitsMap;
    auto sign = multiply ? 1.0 : -1.0;

    for (const auto &units : secondUnitsMap) {
        auto it = res.find(units.first);

        if (it == res.end()) {
            res.emplace(units.first, sign * units.second);
        } else {
            it->second += sign * units.second;

            if (areNearlyEqual(it->second, 0.0)) {
                // The units has now an exponent value of zero, so no need to
                // track it anymore.

                res.erase(it);
            }
        }
    }

    return res;
}

UnitsMaps Analyser::AnalyserImpl::multiplyDivideUnitsMaps(const UnitsMaps &firstUnitsMaps,
                                                          const UnitsMaps &secondUnitsMaps,
                                                          bool multiply)
{
    // Multiply/divide the given units maps together, following a multiplication
    // (multiply = true) or a division (multiply = false).

    UnitsMaps res;

    for (const auto &firstUnitsMap : firstUnitsMaps) {
        for (const auto &secondUnitsMap : secondUnitsMaps) {
            res.push_back(multiplyDivideUnitsMaps(firstUnitsMap, secondUnitsMap, multiply));
        }
    }

    return res;
}

UnitsMaps Analyser::AnalyserImpl::multiplyDivideUnitsMaps(const UnitsMaps &unitsMaps,
                                                          double factor,
                                                          bool multiply)
{
    // Multiply/divide the given units maps by the given factor, following a
    // multiplication (multiply = true) or a division (multiply = false).

    auto res = unitsMaps;
    auto realFactor = multiply ? factor : 1.0 / factor;

    for (auto &unitsMap : res) {
        for (auto &unitsItem : unitsMap) {
            unitsItem.second *= realFactor;
        }
    }

    return res;
}

double Analyser::AnalyserImpl::multiplyDivideUnitsMultipliers(double firstUnitsMultiplier,
                                                              double secondUnitsMultiplier,
                                                              bool multiply)
{
    // Multiply/divide the given units multipliers together, following a
    // multiplication (multiply = true) or a division (multiply = false).

    return firstUnitsMultiplier + (multiply ? 1.0 : -1.0) * secondUnitsMultiplier;
}

UnitsMultipliers Analyser::AnalyserImpl::multiplyDivideUnitsMultipliers(const UnitsMultipliers &firstUnitsMultipliers,
                                                                        const UnitsMultipliers &secondUnitsMultipliers,
                                                                        bool multiply)
{
    // Multiply/divide the given units multipliers together, following a
    // multiplication (multiply = true) or a division (multiply = false).

    UnitsMultipliers res;

    for (const auto &firstUnitsMultiplier : firstUnitsMultipliers) {
        for (const auto &secondUnitsMultiplier : secondUnitsMultipliers) {
            res.push_back(multiplyDivideUnitsMultipliers(firstUnitsMultiplier,
                                                         secondUnitsMultiplier,
                                                         multiply));
        }
    }

    return res;
}

UnitsMultipliers Analyser::AnalyserImpl::multiplyDivideUnitsMultipliers(double firstUnitsMultiplier,
                                                                        const UnitsMultipliers &secondUnitsMultipliers,
                                                                        bool multiply)
{
    // Multiply/divide the given units multipliers together, following a
    // multiplication (multiply = true) or a division (multiply = false).

    UnitsMultipliers res;

    for (const auto &secondUnitsMultiplier : secondUnitsMultipliers) {
        res.push_back(multiplyDivideUnitsMultipliers(firstUnitsMultiplier,
                                                     secondUnitsMultiplier,
                                                     multiply));
    }

    return res;
}

UnitsMultipliers Analyser::AnalyserImpl::powerRootUnitsMultipliers(const UnitsMultipliers &unitsMultipliers,
                                                                   double factor,
                                                                   bool power)
{
    // Power/root the given units multipliers to the given factor, following a
    // power (power = true) or a root (power = false) operation.

    UnitsMultipliers res;
    auto realFactor = power ? factor : 1.0 / factor;

    for (const auto &unitsMultiplier : unitsMultipliers) {
        res.push_back(realFactor * unitsMultiplier);
    }

    return res;
}

bool Analyser::AnalyserImpl::areSameUnitsMaps(const UnitsMaps &firstUnitsMaps,
                                              const UnitsMaps &secondUnitsMaps)
{
    // Check whether the given units maps are the same by checking their
    // exponents.

    for (const auto &firstUnitsMap : firstUnitsMaps) {
        for (const auto &secondUnitsMap : secondUnitsMaps) {
            UnitsMap unitsMap;

            for (const auto &units : firstUnitsMap) {
                if (units.first != "dimensionless") {
                    unitsMap[units.first] += units.second;
                }
            }

            for (const auto &units : secondUnitsMap) {
                if (units.first != "dimensionless") {
                    unitsMap[units.first] -= units.second;
                }
            }

            for (const auto &unitsItem : unitsMap) {
                if (!areNearlyEqual(unitsItem.second, 0.0)) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool Analyser::AnalyserImpl::isDimensionlessUnitsMaps(const UnitsMaps &unitsMaps)
{
    // Check whether the given units maps is dimensionless.

    for (const auto &unitsMap : unitsMaps) {
        for (const auto &unitsItem : unitsMap) {
            if (unitsItem.first != "dimensionless") {
                return false;
            }
        }
    }

    return true;
}

bool Analyser::AnalyserImpl::areSameUnitsMultipliers(const UnitsMultipliers &firstUnitsMultipliers,
                                                     const UnitsMultipliers &secondUnitsMultipliers)
{
    // Return whether the units multipliers are equals.

    for (const auto &firstUnitsMultiplier : firstUnitsMultipliers) {
        for (const auto &secondUnitsMultiplier : secondUnitsMultipliers) {
            if (!areNearlyEqual(firstUnitsMultiplier, secondUnitsMultiplier)) {
                return false;
            }
        }
    }

    return true;
}

void Analyser::AnalyserImpl::updateUnitsMultiplier(const ModelPtr &model,
                                                   const std::string &unitsName,
                                                   double &newUnitsMultiplier,
                                                   double unitsExponent,
                                                   double unitsMultiplier)
{
    // Update the given units multiplier using the given information.

    if (isStandardUnitName(unitsName)) {
        newUnitsMultiplier += unitsMultiplier + standardMultiplierList.at(unitsName);
    } else {
        auto units = model->units(unitsName);

        if (units->isBaseUnit()) {
            newUnitsMultiplier += unitsMultiplier;
        } else {
            std::string reference;
            std::string prefix;
            double exponent;
            double multiplier;
            std::string id;

            for (size_t i = 0; i < units->unitCount(); ++i) {
                units->unitAttributes(i, reference, prefix, exponent, multiplier, id);

                if (isStandardUnitName(reference)) {
                    newUnitsMultiplier += unitsMultiplier + (standardMultiplierList.at(reference) + std::log10(multiplier) + convertPrefixToInt(prefix)) * exponent * unitsExponent;
                } else {
                    updateUnitsMultiplier(model, reference, newUnitsMultiplier,
                                          exponent * unitsExponent,
                                          unitsMultiplier + (std::log10(multiplier) + convertPrefixToInt(prefix)) * unitsExponent);
                }
            }
        }
    }
}

std::string Analyser::AnalyserImpl::componentName(const AnalyserEquationAstPtr &ast)
{
    // Return the name of the component in which the given AST is, by going
    // through the AST, if needed, and returning the component of the first
    // variable we find on the LHS/RHS.

    auto astVariable = ast->variable();

    if (astVariable != nullptr) {
        return std::dynamic_pointer_cast<Component>(astVariable->parent())->name();
    }

    auto res = (ast->mPimpl->mOwnedLeftChild != nullptr) ?
                   componentName(ast->mPimpl->mOwnedLeftChild) :
                   "";

    if (res.empty()) {
        res = (ast->mPimpl->mOwnedRightChild != nullptr) ?
                  componentName(ast->mPimpl->mOwnedRightChild) :
                  "";
    }

    return res;
}

double Analyser::AnalyserImpl::powerValue(const AnalyserEquationAstPtr &ast,
                                          PowerData &powerData)
{
    // Make sure that we have an AST to process.

    static const double NAN = std::numeric_limits<double>::quiet_NaN();

    if (ast == nullptr) {
        return NAN;
    }

    // Retrieve the power value of the LHS and RHS of the given AST.

    auto lhs = powerValue(ast->mPimpl->mOwnedLeftChild, powerData);

    if (!powerData.mExponentValueAvailable) {
        return lhs;
    }

    auto rhs = powerValue(ast->mPimpl->mOwnedRightChild, powerData);

    if (!powerData.mExponentValueAvailable) {
        return rhs;
    }

    // Return the power value for the given AST.

    switch (ast->mPimpl->mType) {
        // Relational and logical operators.

    case AnalyserEquationAst::Type::EQ:
        return lhs == rhs;
    case AnalyserEquationAst::Type::NEQ:
        return lhs != rhs;
    case AnalyserEquationAst::Type::LT:
        return lhs < rhs;
    case AnalyserEquationAst::Type::LEQ:
        return lhs <= rhs;
    case AnalyserEquationAst::Type::GT:
        return lhs > rhs;
    case AnalyserEquationAst::Type::GEQ:
        return lhs >= rhs;
    case AnalyserEquationAst::Type::AND:
        return lhs && rhs;
    case AnalyserEquationAst::Type::OR:
        return lhs || rhs;
    case AnalyserEquationAst::Type::XOR:
        return (lhs != 0.0) ^ (rhs != 0.0);
    case AnalyserEquationAst::Type::NOT:
        return !lhs;

        // Arithmetic operators.

    case AnalyserEquationAst::Type::PLUS:
        if (ast->mPimpl->mOwnedRightChild != nullptr) {
            return lhs + rhs;
        }

        return lhs;
    case AnalyserEquationAst::Type::MINUS:
        if (ast->mPimpl->mOwnedRightChild != nullptr) {
            return lhs - rhs;
        }

        return -lhs;
    case AnalyserEquationAst::Type::TIMES:
        return lhs * rhs;
    case AnalyserEquationAst::Type::DIVIDE:
        return lhs / rhs;
    case AnalyserEquationAst::Type::POWER:
        return std::pow(lhs, rhs);
    case AnalyserEquationAst::Type::ROOT:
        if (ast->mPimpl->mOwnedLeftChild->type() == AnalyserEquationAst::Type::DEGREE) {
            return std::pow(rhs, 1.0 / lhs);
        }

        return std::pow(lhs, 1.0 / 2.0);
    case AnalyserEquationAst::Type::ABS:
        return std::abs(lhs);
    case AnalyserEquationAst::Type::EXP:
        return std::exp(lhs);
    case AnalyserEquationAst::Type::LN:
        return std::log(lhs);
    case AnalyserEquationAst::Type::LOG:
        if (ast->mPimpl->mOwnedLeftChild->type() == AnalyserEquationAst::Type::LOGBASE) {
            auto logBase = lhs;

            if (areNearlyEqual(logBase, 10.0)) {
                return std::log10(rhs);
            }

            return std::log(rhs) / std::log(logBase);
        }

        return std::log10(lhs);
    case AnalyserEquationAst::Type::CEILING:
        return std::ceil(lhs);
    case AnalyserEquationAst::Type::FLOOR:
        return std::floor(lhs);
    case AnalyserEquationAst::Type::MIN:
        return (lhs < rhs) ? lhs : rhs;
    case AnalyserEquationAst::Type::MAX:
        return (lhs > rhs) ? lhs : rhs;
    case AnalyserEquationAst::Type::REM:
        return std::fmod(lhs, rhs);

        // Trigonometric operators.

    case AnalyserEquationAst::Type::SIN:
        return std::sin(lhs);
    case AnalyserEquationAst::Type::COS:
        return std::cos(lhs);
    case AnalyserEquationAst::Type::TAN:
        return std::tan(lhs);
    case AnalyserEquationAst::Type::SEC:
        return 1.0 / std::cos(lhs);
    case AnalyserEquationAst::Type::CSC:
        return 1.0 / std::sin(lhs);
    case AnalyserEquationAst::Type::COT:
        return 1.0 / std::tan(lhs);
    case AnalyserEquationAst::Type::SINH:
        return std::sinh(lhs);
    case AnalyserEquationAst::Type::COSH:
        return std::cosh(lhs);
    case AnalyserEquationAst::Type::TANH:
        return std::tanh(lhs);
    case AnalyserEquationAst::Type::SECH:
        return 1.0 / std::cosh(lhs);
    case AnalyserEquationAst::Type::CSCH:
        return 1.0 / std::sinh(lhs);
    case AnalyserEquationAst::Type::COTH:
        return 1.0 / std::tanh(lhs);
    case AnalyserEquationAst::Type::ASIN:
        return std::asin(lhs);
    case AnalyserEquationAst::Type::ACOS:
        return std::acos(lhs);
    case AnalyserEquationAst::Type::ATAN:
        return std::atan(lhs);
    case AnalyserEquationAst::Type::ASEC:
        return std::acos(1.0 / lhs);
    case AnalyserEquationAst::Type::ACSC:
        return std::asin(1.0 / lhs);
    case AnalyserEquationAst::Type::ACOT:
        return std::atan(1.0 / lhs);
    case AnalyserEquationAst::Type::ASINH:
        return std::asinh(lhs);
    case AnalyserEquationAst::Type::ACOSH:
        return std::acosh(lhs);
    case AnalyserEquationAst::Type::ATANH:
        return std::atanh(lhs);
    case AnalyserEquationAst::Type::ASECH: {
        auto xInv = 1.0 / lhs;

        return std::log(xInv + std::sqrt(xInv * xInv - 1.0));
    }
    case AnalyserEquationAst::Type::ACSCH: {
        auto xInv = 1.0 / lhs;

        return std::log(xInv + std::sqrt(xInv * xInv + 1.0));
    }
    case AnalyserEquationAst::Type::ACOTH: {
        auto xInv = 1.0 / lhs;

        return 0.5 * std::log((1.0 + xInv) / (1.0 - xInv));
    }

        // Token elements.

    case AnalyserEquationAst::Type::CI: {
        auto initialValue = ast->variable()->initialValue();

        if (initialValue.empty()) {
            powerData.mExponentValueAvailable = false;

            return NAN;
        }

        powerData.mExponentValueChangeable = true;

        return std::stod(initialValue);
    }
    case AnalyserEquationAst::Type::CN:
        return std::stod(ast->value());

        // Qualifier elements.

    case AnalyserEquationAst::Type::DEGREE:
    case AnalyserEquationAst::Type::LOGBASE:
        return lhs;

        // Constants.

    case AnalyserEquationAst::Type::TRUE:
        return 1.0;
    case AnalyserEquationAst::Type::FALSE:
        return 0.0;
    case AnalyserEquationAst::Type::E: {
        static const double E = exp(1.0);

        return E;
    }
    case AnalyserEquationAst::Type::PI:
        return M_PI;
    case AnalyserEquationAst::Type::INF: {
        static const double INF = std::numeric_limits<double>::infinity();

        return INF;
    }
    case AnalyserEquationAst::Type::NAN:
        return NAN;
    default:
        // This corresponds to one of the following cases:
        //  - AnalyserEquationAst::Type::EQUALITY (we should never come across this case);
        //  - AnalyserEquationAst::Type::DIFF,
        //    AnalyserEquationAst::Type::BVAR; and
        //  - AnalyserEquationAst::Type::PIECEWISE,
        //    AnalyserEquationAst::Type::PIECE,
        //    AnalyserEquationAst::Type::OTHERWISE.
        // In all these cases, we may not have a constant (power) value.

        powerData.mExponentValueAvailable = false;

        return NAN;
    }
}

std::string Analyser::AnalyserImpl::expression(const AnalyserEquationAstPtr &ast,
                                               bool includeHierarchy)
{
    // Return the generated code for the given AST, specifying the equation and
    // component in which it is, if needed and requested.

    std::string res = "'" + Generator::equationCode(ast, mGeneratorProfile) + "'";

    if (includeHierarchy) {
        auto equationAst = ast;
        auto equationAstParent = ast->parent();
        auto equationAstGrandparent = (equationAstParent != nullptr) ? equationAstParent->parent() : nullptr;

        while (equationAstParent != nullptr) {
            equationAst = equationAstParent;
            equationAstParent = equationAstGrandparent;
            equationAstGrandparent = (equationAstParent != nullptr) ? equationAstParent->parent() : nullptr;

            res += std::string(" in")
                   + ((equationAstParent == nullptr) ? " equation" : "")
                   + " '" + Generator::equationCode(equationAst, mGeneratorProfile) + "'";
        }

        res += " in component '" + componentName(equationAst) + "'";
    }

    return res;
}

std::string Analyser::AnalyserImpl::expressionUnits(const UnitsMaps &unitsMaps,
                                                    const UnitsMultipliers &unitsMultipliers)
{
    // Return a string version of the given units maps and units multipliers.

    Strings units;

    for (size_t i = 0; i < unitsMaps.size(); ++i) {
        auto unitsMap = unitsMaps[i];
        std::string unit;

        if (!unitsMultipliers.empty()) {
            auto intExponent = int(unitsMultipliers[i]);
            auto exponent = areNearlyEqual(unitsMultipliers[i], intExponent) ?
                                convertToString(intExponent) :
                                convertToString(unitsMultipliers[i], false);

            if (exponent != "0") {
                unit += "10^" + exponent;
            }
        }

        for (const auto &unitsItem : unitsMap) {
            if ((unitsItem.first != "dimensionless")
                && !areNearlyEqual(unitsItem.second, 0.0)) {
                auto intExponent = int(unitsItem.second);
                auto exponent = areNearlyEqual(unitsItem.second, intExponent) ?
                                    convertToString(intExponent) :
                                    convertToString(unitsItem.second, false);

                if (!unit.empty()) {
                    unit += " x ";
                }

                unit += unitsItem.first;

                if (exponent != "1") {
                    unit += "^" + exponent;
                }
            }
        }

        if (!unit.empty()) {
            units.push_back(unit);
        }
    }

    std::string unitsString;

    for (size_t i = 0; i < units.size(); ++i) {
        if (i > 0) {
            unitsString += (i == units.size() - 1) ? " and " : ", ";
        }

        unitsString += "'" + units[i] + "'";
    }

    return unitsString;
}

std::string Analyser::AnalyserImpl::expressionUnits(const AnalyserEquationAstPtr &ast,
                                                    const UnitsMaps &unitsMaps,
                                                    const UnitsMaps &userUnitsMaps,
                                                    const UnitsMultipliers &unitsMultipliers)
{
    // Return a string version of the given AST and (user) units maps and units
    // multipliers.

    auto res = expression(ast, false) + " is ";
    auto unitsString = expressionUnits(unitsMaps, unitsMultipliers);
    auto userUnitsString = expressionUnits(userUnitsMaps);

    if (userUnitsString.empty()) {
        res += "'dimensionless'";
    } else {
        res += "in " + userUnitsString;

        if (!unitsString.empty() && (unitsString != userUnitsString)) {
            res += " (i.e. " + unitsString + ")";
        }
    }

    return res;
}

void Analyser::AnalyserImpl::defaultUnitsMapsAndMultipliers(UnitsMaps &unitsMaps,
                                                            UnitsMaps &userUnitsMaps,
                                                            UnitsMultipliers &unitsMultipliers)
{
    // Default units maps and multipliers.

    unitsMaps = {UnitsMap()};
    userUnitsMaps = {UnitsMap()};
    unitsMultipliers = {0.0};
}

void Analyser::AnalyserImpl::analyseEquationUnits(const AnalyserEquationAstPtr &ast,
                                                  UnitsMaps &unitsMaps,
                                                  UnitsMaps &userUnitsMaps,
                                                  UnitsMultipliers &unitsMultipliers,
                                                  std::string &issueDescription,
                                                  PowerData &powerData)
{
    // Analyse the units used with different MathML elements (table 2.1 of the
    // CellML 2.0 normative specification; see https://bit.ly/3vBbyO5):
    //  - Simple operands ('ci' and 'cn'; note: 'sep' is not relevant here): the
    //    operand can have any unit.
    //  - Basic structural (note: 'apply' is not relevant here):
    //     - 'piecewise': the returned value of the different 'piece' and
    //       'otherwise' statements should have equivalent units.
    //     - 'piece': the returned value can have any unit while the condition
    //       should be dimensionless.
    //     - 'otherwise': the returned value can have any unit.
    //  - Relational operators ('eq', 'neq', 'gt', 'lt', 'geq' and 'leq'): the
    //    two operands should have equivalent units. (The result of the
    //    comparison is dimensionless.)
    //  - Logical operators:
    //     - 'and', 'or', 'xor': the two operands should be dimensionless.
    //     - 'not': the operand should be dimensionless.
    //  - Arithmetic operators:
    //     - 'plus': the two operands should have equivalent units.
    //     - 'minus': if there is one operand, then it can have any unit. If
    //       there are two operands, then they should have equivalent units.
    //     - 'times' and 'divide': the two operands can have any units.
    //     - 'power': the base can have any unit while the exponent should be
    //       dimensionless.
    //     - 'root': the base can have any unit while the exponent, if present,
    //       should be dimensionless.
    //     - 'abs': the argument can have any unit.
    //     - 'exp' and 'ln': the argument should be dimensionless.
    //     - 'log': the argument and the base, if present, should be
    //       dimensionless.
    //     - 'floor' and 'ceiling': the argument can have any unit.
    //     - 'min' and 'max': all the arguments should have equivalent units.
    //     - 'rem': the two arguments should have equivalent units.
    //  - Calculus elements ('diff'): the differentiated variable can have any
    //    unit. (See 'bvar' below for the bounding variable.)
    //  - Qualifier elements:
    //     - 'bvar': a bounding variable can have any unit.
    //     - 'degree': a degree should be dimensionless.
    //     - 'logbase': a base should be dimensionless.
    //  - Trigonometric operators ('sin', 'cos', 'tan', etc.): the argument
    //    should be dimensionless.
    //  - Mathematical and logical constants ('pi', 'exponentiale',
    //    'notanumber','infinity', 'true' and 'false'): those constants are
    //    dimensionless.

    // Make sure that we have an AST to analyse.

    if (ast == nullptr) {
        unitsMaps = {};
        userUnitsMaps = {};
        unitsMultipliers = {};

        return;
    }

    // Check whether we are dealing with a CI/CN element and, if so, retrieve
    // both its units maps and multipliers.

    switch (ast->mPimpl->mType) {
    case AnalyserEquationAst::Type::CI:
    case AnalyserEquationAst::Type::CN: {
        auto units = mCiCnUnits[ast];
        auto model = owningModel(units);

        defaultUnitsMapsAndMultipliers(unitsMaps, userUnitsMaps, unitsMultipliers);

        for (auto &unitsMap : unitsMaps) {
            updateUnitsMap(model, units->name(), unitsMap);
        }

        for (auto &userUnitsMap : userUnitsMaps) {
            updateUnitsMap(model, units->name(), userUnitsMap, true);
        }

        for (auto &unitsMultiplier : unitsMultipliers) {
            updateUnitsMultiplier(model, units->name(), unitsMultiplier);
        }

        return;
    }
    default:
        break;
    }

    // Check the left and right children.

    analyseEquationUnits(ast->mPimpl->mOwnedLeftChild, unitsMaps, userUnitsMaps, unitsMultipliers, issueDescription, powerData);

    if (!issueDescription.empty()) {
        return;
    }

    UnitsMaps rightUnitsMaps;
    UnitsMaps rightUserUnitsMaps;
    UnitsMultipliers rightUnitsMultipliers;

    analyseEquationUnits(ast->mPimpl->mOwnedRightChild, rightUnitsMaps, rightUserUnitsMaps, rightUnitsMultipliers, issueDescription, powerData);

    if (!issueDescription.empty()) {
        return;
    }

    switch (ast->mPimpl->mType) {
    case AnalyserEquationAst::Type::EQUALITY:
    case AnalyserEquationAst::Type::EQ:
    case AnalyserEquationAst::Type::NEQ:
    case AnalyserEquationAst::Type::LT:
    case AnalyserEquationAst::Type::LEQ:
    case AnalyserEquationAst::Type::GT:
    case AnalyserEquationAst::Type::GEQ:
    case AnalyserEquationAst::Type::PLUS:
    case AnalyserEquationAst::Type::MINUS:
    case AnalyserEquationAst::Type::MIN:
    case AnalyserEquationAst::Type::MAX:
    case AnalyserEquationAst::Type::REM: {
        auto sameUnitsMaps = rightUnitsMaps.empty()
                             || areSameUnitsMaps(unitsMaps, rightUnitsMaps);
        auto sameUnitsMultipliers = rightUnitsMultipliers.empty()
                                    || areSameUnitsMultipliers(unitsMultipliers, rightUnitsMultipliers);
        auto sameUnits = sameUnitsMaps && sameUnitsMultipliers;

        if (sameUnits && powerData.mExponentValueAvailable && !powerData.mExponentValueChangeable) {
            // Relational operators result in a dimensionless unit.

            switch (ast->mPimpl->mType) {
            case AnalyserEquationAst::Type::EQ:
            case AnalyserEquationAst::Type::NEQ:
            case AnalyserEquationAst::Type::LT:
            case AnalyserEquationAst::Type::LEQ:
            case AnalyserEquationAst::Type::GT:
            case AnalyserEquationAst::Type::GEQ:
                defaultUnitsMapsAndMultipliers(unitsMaps, userUnitsMaps, unitsMultipliers);

                break;
            default:
                break;
            }
        } else if (powerData.mExponentValueAvailable) {
            if (sameUnits) {
                if (!powerData.mDimensionlessBase) {
                    issueDescription = "The units in " + expression(ast) + " are equivalent as long as the value of "
                                       + expression(powerData.mExponentAst, false) + " is equal to '" + generateDoubleCode(convertToString(powerData.mExponentValue)) + "'.";
                }
            } else {
                issueDescription = "The units in " + expression(ast) + " are not equivalent. "
                                   + expressionUnits(ast->mPimpl->mOwnedLeftChild, unitsMaps, userUnitsMaps, unitsMultipliers) + " while "
                                   + expressionUnits(ast->mPimpl->mOwnedRightChild, rightUnitsMaps, rightUserUnitsMaps, rightUnitsMultipliers) + ".";
            }
        } else if (!isDimensionlessUnitsMaps(unitsMaps)) {
            auto leftChild = ast->mPimpl->mOwnedLeftChild;
            auto rightChild = ast->mPimpl->mOwnedRightChild;

            if (leftChild->type() == AnalyserEquationAst::Type::POWER) {
                if (rightChild != nullptr) {
                    if (rightChild->type() == AnalyserEquationAst::Type::POWER) {
                        issueDescription = "The units in " + expression(ast) + " may not be equivalent. "
                                           + expression(leftChild->mPimpl->mOwnedRightChild, false) + " may result in " + expression(leftChild, false) + " having different units while "
                                           + expression(rightChild->mPimpl->mOwnedRightChild, false) + " may result in " + expression(rightChild, false) + " having different units.";
                    } else {
                        issueDescription = "The units in " + expression(ast) + " may not be equivalent. "
                                           + expression(leftChild->mPimpl->mOwnedRightChild, false) + " may result in " + expression(leftChild, false) + " having different units while "
                                           + expressionUnits(rightChild, unitsMaps, userUnitsMaps, unitsMultipliers) + ".";
                    }
                }
            } else if (rightChild->type() == AnalyserEquationAst::Type::POWER) {
                issueDescription = "The units in " + expression(ast) + " may not be equivalent. "
                                   + expressionUnits(leftChild, unitsMaps, userUnitsMaps, unitsMultipliers) + " while "
                                   + expression(rightChild->mPimpl->mOwnedRightChild, false) + " may result in " + expression(rightChild, false) + " having different units.";
            }
        }
    } break;
    case AnalyserEquationAst::Type::PIECEWISE:
        unitsMaps.insert(std::end(unitsMaps),
                         std::begin(rightUnitsMaps),
                         std::end(rightUnitsMaps));
        userUnitsMaps.insert(std::end(userUnitsMaps),
                             std::begin(rightUserUnitsMaps),
                             std::end(rightUserUnitsMaps));
        unitsMultipliers.insert(std::end(unitsMultipliers),
                                std::begin(rightUnitsMultipliers),
                                std::end(rightUnitsMultipliers));

        break;
    case AnalyserEquationAst::Type::PIECE:
        if (!Analyser::AnalyserImpl::isDimensionlessUnitsMaps(rightUnitsMaps)) {
            issueDescription = "The unit of " + expression(ast->mPimpl->mOwnedRightChild) + " is not dimensionless. "
                               + expressionUnits(ast->mPimpl->mOwnedRightChild, rightUnitsMaps, rightUserUnitsMaps, rightUnitsMultipliers) + ".";
        }

        break;
    case AnalyserEquationAst::Type::AND:
    case AnalyserEquationAst::Type::OR:
    case AnalyserEquationAst::Type::XOR:
    case AnalyserEquationAst::Type::NOT:
    case AnalyserEquationAst::Type::EXP:
    case AnalyserEquationAst::Type::LN:
    case AnalyserEquationAst::Type::LOG: {
        auto isDimensionlessUnitsMaps = Analyser::AnalyserImpl::isDimensionlessUnitsMaps(unitsMaps);

        if (!isDimensionlessUnitsMaps) {
            auto isDimensionlessRightUnitsMaps = Analyser::AnalyserImpl::isDimensionlessUnitsMaps(rightUnitsMaps);
            issueDescription = "The unit";

            if (!isDimensionlessRightUnitsMaps) {
                issueDescription += "s";
            }

            issueDescription += " of " + expression(ast->mPimpl->mOwnedLeftChild, false);

            if (!isDimensionlessRightUnitsMaps) {
                issueDescription += " and " + expression(ast->mPimpl->mOwnedRightChild, false);
            }

            issueDescription += " in " + expression(ast);

            if (!isDimensionlessRightUnitsMaps) {
                issueDescription += " are ";
            } else {
                issueDescription += " is ";
            }

            issueDescription += "not dimensionless. " + expressionUnits(ast->mPimpl->mOwnedLeftChild, unitsMaps, userUnitsMaps, unitsMultipliers);

            if (!isDimensionlessRightUnitsMaps) {
                issueDescription += " while " + expressionUnits(ast->mPimpl->mOwnedRightChild, rightUnitsMaps, rightUserUnitsMaps, rightUnitsMultipliers);
            }

            issueDescription += ".";
        }
    } break;
    case AnalyserEquationAst::Type::TIMES:
    case AnalyserEquationAst::Type::DIVIDE: {
        auto isTimes = ast->mPimpl->mType == AnalyserEquationAst::Type::TIMES;

        unitsMaps = multiplyDivideUnitsMaps(unitsMaps, rightUnitsMaps, isTimes);
        userUnitsMaps = multiplyDivideUnitsMaps(userUnitsMaps, rightUserUnitsMaps, isTimes);
        unitsMultipliers = multiplyDivideUnitsMultipliers(unitsMultipliers, rightUnitsMultipliers, isTimes);
    } break;
    case AnalyserEquationAst::Type::POWER:
    case AnalyserEquationAst::Type::ROOT: {
        auto isPower = ast->mPimpl->mType == AnalyserEquationAst::Type::POWER;

        // Determine whether we are dealing with a dimensionless base.

        if (isPower) {
            powerData.mDimensionlessBase = Analyser::AnalyserImpl::isDimensionlessUnitsMaps(unitsMaps);
        } else {
            if (ast->mPimpl->mOwnedLeftChild->type() == AnalyserEquationAst::Type::DEGREE) {
                powerData.mDimensionlessBase = Analyser::AnalyserImpl::isDimensionlessUnitsMaps(rightUnitsMaps);
            } else {
                powerData.mDimensionlessBase = Analyser::AnalyserImpl::isDimensionlessUnitsMaps(unitsMaps);
            }
        }

        // Determine whether we are dealing with a dimensionless exponent and report it if not.

        auto isDimensionlessExponent = true;

        if (isPower
            || (ast->mPimpl->mOwnedLeftChild->type() == AnalyserEquationAst::Type::DEGREE)) {
            powerData.mExponentAst = isPower ?
                                         ast->mPimpl->mOwnedRightChild :
                                         ast->mPimpl->mOwnedLeftChild;
            isDimensionlessExponent = Analyser::AnalyserImpl::isDimensionlessUnitsMaps(isPower ?
                                                                                           rightUnitsMaps :
                                                                                           unitsMaps);

            if (!isDimensionlessExponent) {
                auto exponentUnitsMaps = isPower ? rightUnitsMaps : unitsMaps;
                auto exponentUserUnitsMaps = isPower ? rightUserUnitsMaps : userUnitsMaps;
                auto exponentUnitsMultipliers = isPower ? rightUnitsMultipliers : unitsMultipliers;

                issueDescription = "The unit of " + expression(powerData.mExponentAst) + " is not dimensionless. "
                                   + expressionUnits(powerData.mExponentAst, exponentUnitsMaps, exponentUserUnitsMaps, exponentUnitsMultipliers) + ".";
            }
        }

        // Retrieve the exponent and apply it to our units maps and multipliers.

        if (isDimensionlessExponent) {
            if (isPower) {
                powerData.mExponentValue = powerValue(ast->mPimpl->mOwnedRightChild, powerData);
            } else { // AnalyserEquationAst::Type::ROOT.
                if (ast->mPimpl->mOwnedLeftChild->type() == AnalyserEquationAst::Type::DEGREE) {
                    unitsMaps = rightUnitsMaps;
                    userUnitsMaps = rightUserUnitsMaps;
                    unitsMultipliers = rightUnitsMultipliers;

                    powerData.mExponentValue = powerValue(ast->mPimpl->mOwnedLeftChild, powerData);
                } else {
                    // No DEGREE element, which means that we are dealing with a
                    // square root.

                    powerData.mExponentValue = 2.0;
                }
            }

            if (powerData.mExponentValueAvailable) {
                unitsMaps = multiplyDivideUnitsMaps(unitsMaps, powerData.mExponentValue, isPower);
                userUnitsMaps = multiplyDivideUnitsMaps(userUnitsMaps, powerData.mExponentValue, isPower);
                unitsMultipliers = powerRootUnitsMultipliers(unitsMultipliers, powerData.mExponentValue, isPower);
            }
        }
    } break;
    case AnalyserEquationAst::Type::SIN:
    case AnalyserEquationAst::Type::COS:
    case AnalyserEquationAst::Type::TAN:
    case AnalyserEquationAst::Type::SEC:
    case AnalyserEquationAst::Type::CSC:
    case AnalyserEquationAst::Type::COT:
    case AnalyserEquationAst::Type::SINH:
    case AnalyserEquationAst::Type::COSH:
    case AnalyserEquationAst::Type::TANH:
    case AnalyserEquationAst::Type::SECH:
    case AnalyserEquationAst::Type::CSCH:
    case AnalyserEquationAst::Type::COTH:
    case AnalyserEquationAst::Type::ASIN:
    case AnalyserEquationAst::Type::ACOS:
    case AnalyserEquationAst::Type::ATAN:
    case AnalyserEquationAst::Type::ASEC:
    case AnalyserEquationAst::Type::ACSC:
    case AnalyserEquationAst::Type::ACOT:
    case AnalyserEquationAst::Type::ASINH:
    case AnalyserEquationAst::Type::ACOSH:
    case AnalyserEquationAst::Type::ATANH:
    case AnalyserEquationAst::Type::ASECH:
    case AnalyserEquationAst::Type::ACSCH:
    case AnalyserEquationAst::Type::ACOTH:
        if (!Analyser::AnalyserImpl::isDimensionlessUnitsMaps(unitsMaps)) {
            issueDescription = "The unit of " + expression(ast->mPimpl->mOwnedLeftChild) + " is not dimensionless. "
                               + expressionUnits(ast->mPimpl->mOwnedLeftChild, unitsMaps, userUnitsMaps, unitsMultipliers) + ".";
        }

        break;
    case AnalyserEquationAst::Type::DIFF:
        unitsMaps = multiplyDivideUnitsMaps(unitsMaps, rightUnitsMaps);
        userUnitsMaps = multiplyDivideUnitsMaps(userUnitsMaps, rightUserUnitsMaps);
        unitsMultipliers = multiplyDivideUnitsMultipliers(unitsMultipliers, rightUnitsMultipliers);

        break;
    case AnalyserEquationAst::Type::BVAR:
        for (auto &unitsMap : unitsMaps) {
            for (auto &unitsItem : unitsMap) {
                unitsItem.second *= -1.0;
            }
        }

        for (auto &userUnitsMap : userUnitsMaps) {
            for (auto &userUnits : userUnitsMap) {
                userUnits.second *= -1.0;
            }
        }

        unitsMultipliers = multiplyDivideUnitsMultipliers(0.0, unitsMultipliers, false);

        break;
    case AnalyserEquationAst::Type::TRUE:
    case AnalyserEquationAst::Type::FALSE:
    case AnalyserEquationAst::Type::E:
    case AnalyserEquationAst::Type::PI:
    case AnalyserEquationAst::Type::INF:
    case AnalyserEquationAst::Type::NAN:
        defaultUnitsMapsAndMultipliers(unitsMaps, userUnitsMaps, unitsMultipliers);

        break;
    default: // Other types we don't care about.
        break;
    }
}

double Analyser::AnalyserImpl::scalingFactor(const VariablePtr &variable)
{
    return Units::scalingFactor(variable->units(), internalVariable(variable)->mVariable->units());
}

void Analyser::AnalyserImpl::scaleAst(const AnalyserEquationAstPtr &ast,
                                      const AnalyserEquationAstPtr &astParent,
                                      double scalingFactor)
{
    // Scale the given AST using the given scaling factor.

    auto scaledAst = AnalyserEquationAst::create();

    scaledAst->mPimpl->populate(AnalyserEquationAst::Type::TIMES, astParent);

    scaledAst->mPimpl->mOwnedLeftChild = AnalyserEquationAst::create();
    scaledAst->mPimpl->mOwnedRightChild = ast;

    scaledAst->mPimpl->mOwnedLeftChild->mPimpl->populate(AnalyserEquationAst::Type::CN, convertToString(scalingFactor), scaledAst);

    ast->mPimpl->mParent = scaledAst;

    if (astParent->mPimpl->mOwnedLeftChild == ast) {
        astParent->mPimpl->mOwnedLeftChild = scaledAst;
    } else {
        astParent->mPimpl->mOwnedRightChild = scaledAst;
    }
}

void Analyser::AnalyserImpl::scaleEquationAst(const AnalyserEquationAstPtr &ast)
{
    // Make sure that we have an AST to scale.

    if (ast == nullptr) {
        return;
    }

    // Recursively scale the given AST's children.

    scaleEquationAst(ast->mPimpl->mOwnedLeftChild);
    scaleEquationAst(ast->mPimpl->mOwnedRightChild);

    // If the given AST node is a variable (i.e. a CI node) then we may need to
    // do some scaling.

    if (ast->mPimpl->mType == AnalyserEquationAst::Type::CI) {
        // The kind of scaling we may end up doing depends on whether we are
        // dealing with a rate or some other variable, i.e. whether or not it
        // has a DIFF node as a parent.

        auto astParent = ast->parent();

        if (astParent->mPimpl->mType == AnalyserEquationAst::Type::DIFF) {
            // We are dealing with a rate, so retrieve the scaling factor for
            // its corresponding variable of integration and apply it, if
            // needed.

            auto scalingFactor = Analyser::AnalyserImpl::scalingFactor(astParent->mPimpl->mOwnedLeftChild->mPimpl->mOwnedLeftChild->variable());

            if (!areNearlyEqual(scalingFactor, 1.0)) {
                // We need to scale using the inverse of the scaling factor, but
                // how we do it depends on whether the rate is to be computed or
                // used.

                auto astGrandparent = astParent->parent();

                if (astGrandparent->mPimpl->mType == AnalyserEquationAst::Type::EQUALITY) {
                    scaleAst(astGrandparent->mPimpl->mOwnedRightChild, astGrandparent, scalingFactor);
                } else {
                    scaleAst(astParent, astGrandparent, 1.0 / scalingFactor);
                }
            }
        }

        if (((astParent->mPimpl->mType != AnalyserEquationAst::Type::EQUALITY)
             || (astParent->mPimpl->mOwnedLeftChild != ast))
            && (astParent->mPimpl->mType != AnalyserEquationAst::Type::BVAR)) {
            // We are dealing with a variable which is neither a computed
            // variable nor our variable of integration, so retrieve its scaling
            // factor and apply it, if needed, distinguishing between a rate
            // variable and an algebraic variable.

            auto scalingFactor = Analyser::AnalyserImpl::scalingFactor(ast->variable());

            if (!areNearlyEqual(scalingFactor, 1.0)) {
                if (astParent->mPimpl->mType == AnalyserEquationAst::Type::DIFF) {
                    scaleAst(astParent, astParent->parent(), scalingFactor);
                } else {
                    scaleAst(ast, astParent, scalingFactor);
                }
            }
        }
    }
}

bool Analyser::AnalyserImpl::isExternalVariable(const AnalyserInternalVariablePtr &variable)
{
    return variable->mIsExternalVariable;
}

bool Analyser::AnalyserImpl::isStateRateBased(const AnalyserEquationPtr &analyserEquation,
                                              AnalyserEquationPtrs &checkedEquations)
{
    if (std::find(checkedEquations.begin(), checkedEquations.end(), analyserEquation) != checkedEquations.end()) {
        return false;
    }

    checkedEquations.push_back(analyserEquation);

    for (const auto &dependency : analyserEquation->dependencies()) {
        // A rate is computed either through an ODE equation or through an NLA
        // equation in case the rate is not on its own on either the LHS or RHS
        // of the equation.

        if ((dependency->type() == AnalyserEquation::Type::ODE)
            || ((dependency->type() == AnalyserEquation::Type::NLA)
                && (dependency->stateCount() == 1))
            || isStateRateBased(dependency, checkedEquations)) {
            return true;
        }
    }

    return false;
}

void Analyser::AnalyserImpl::addInvalidVariableIssue(const AnalyserInternalVariablePtr &variable,
                                                     Issue::ReferenceRule referenceRule)
{
    std::string descriptionStart = "Variable";
    std::string descriptionEnd;

    switch (variable->mType) {
    case AnalyserInternalVariable::Type::UNKNOWN:
        descriptionStart = "The type of variable";
        descriptionEnd = "is unknown";

        break;
    case AnalyserInternalVariable::Type::SHOULD_BE_STATE:
        descriptionEnd = "is used in an ODE, but it is not initialised";

        break;
    case AnalyserInternalVariable::Type::UNDERCONSTRAINED:
        descriptionEnd = "is underconstrained";

        break;
    default: // AnalyserInternalVariable::Type::OVERCONSTRAINED.
        descriptionEnd = "is overconstrained";

        break;
    }

    auto issue = Issue::IssueImpl::create();
    auto realVariable = variable->mVariable;

    issue->mPimpl->setDescription(descriptionStart + " '" + realVariable->name()
                                  + "' in component '" + owningComponent(realVariable)->name()
                                  + "' " + descriptionEnd + ".");
    issue->mPimpl->setReferenceRule(referenceRule);
    issue->mPimpl->mItem->mPimpl->setVariable(realVariable);

    addIssue(issue);
}

void Analyser::AnalyserImpl::analyseModel(const ModelPtr &model)
{
    // Reset a few things in case this analyser was to be used to analyse more
    // than one model.

    mAnalyserModel = AnalyserModel::AnalyserModelImpl::create(model);

    mInternalVariables.clear();
    mInternalVariableMap.clear();
    mInternalEquations.clear();

    mCiCnUnits.clear();

    // Recursively analyse the model's components, so that we end up with an AST
    // for each of the model's equations.

    for (size_t i = 0; i < model->componentCount(); ++i) {
        analyseComponent(model->component(i));
    }

    // Recursively analyse the model's components' variables.
    // Note #1: we can't do this as part of analyseComponent() since we don't
    //          necessarily know the state of all the variables.
    // Note #2: when it comes to variables initialised using another variable,
    //          we can only do this after all the equations have been analysed,
    //          i.e. once we know the type of all the variables since a variable
    //          can be initialised using another variable, but only if it is not
    //          an algebraic variable.

    for (size_t i = 0; i < model->componentCount(); ++i) {
        analyseComponentVariables(model->component(i));
    }

    if (mAnalyser->errorCount() != 0) {
        mAnalyserModel->mPimpl->mType = AnalyserModel::Type::INVALID;

        return;
    }

    // Mark some variables as external variables, should there be some and
    // should they belong to the model being analysed.

    std::map<VariablePtr, VariablePtrs> primaryExternalVariables;

    if (!mExternalVariables.empty()) {
        for (const auto &externalVariable : mExternalVariables) {
            auto variable = externalVariable->variable();

            if (owningModel(variable) != model) {
                auto issue = Issue::IssueImpl::create();

                issue->mPimpl->setDescription("Variable '" + variable->name()
                                              + "' in component '" + owningComponent(variable)->name()
                                              + "' is marked as an external variable, but it belongs to a different model and will therefore be ignored.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL);
                issue->mPimpl->mItem->mPimpl->setVariable(variable);

                addIssue(issue);
            } else {
                auto internalVariable = Analyser::AnalyserImpl::internalVariable(variable);

                primaryExternalVariables[internalVariable->mVariable].push_back(variable);

                if (!internalVariable->mIsExternalVariable) {
                    internalVariable->mIsExternalVariable = true;

                    for (const auto &dependency : externalVariable->dependencies()) {
                        internalVariable->mDependencies.push_back(Analyser::AnalyserImpl::internalVariable(dependency)->mVariable);
                    }
                }
            }
        }
    }

    // Analyse our different equations' AST to determine the type of our
    // variables.

    for (const auto &internalEquation : mInternalEquations) {
        analyseEquationAst(internalEquation->mAst);
    }

    if (mAnalyser->errorCount() != 0) {
        mAnalyserModel->mPimpl->mType = AnalyserModel::Type::INVALID;

        return;
    }

    // Check that the variables that were marked as external were rightly so.

    for (const auto &primaryExternalVariable : primaryExternalVariables) {
        std::string description;
        auto isVoi = (mAnalyserModel->mPimpl->mVoi != nullptr)
                     && (primaryExternalVariable.first == mAnalyserModel->mPimpl->mVoi->variable());
        auto equivalentVariableCount = primaryExternalVariable.second.size();
        auto hasPrimaryVariable = std::find(primaryExternalVariable.second.begin(),
                                            primaryExternalVariable.second.end(),
                                            primaryExternalVariable.first)
                                  != primaryExternalVariable.second.end();

        if (isVoi || (equivalentVariableCount > 1) || !hasPrimaryVariable) {
            description += (equivalentVariableCount == 2) ? "Both " : "";

            for (size_t i = 0; i < equivalentVariableCount; ++i) {
                if (i != 0) {
                    description += (i != equivalentVariableCount - 1) ? ", " : " and ";
                }

                auto variableString = ((i == 0) && (equivalentVariableCount != 2)) ?
                                          std::string("Variable") :
                                          std::string("variable");

                description += variableString + " '" + primaryExternalVariable.second[i]->name()
                               + "' in component '" + owningComponent(primaryExternalVariable.second[i])->name()
                               + "'";
            }

            Issue::ReferenceRule referenceRule;

            if (isVoi) {
                description += (equivalentVariableCount == 1) ?
                                   " is marked as an external variable, but it is" :
                                   " are marked as external variables, but they are";

                if ((equivalentVariableCount == 1) && hasPrimaryVariable) {
                    description += " the";
                } else {
                    description += " equivalent to variable '" + primaryExternalVariable.first->name()
                                   + "' in component '" + owningComponent(primaryExternalVariable.first)->name()
                                   + "', the primary";
                }

                description += " variable of integration which cannot be used as an external variable.";

                referenceRule = Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI;
            } else {
                description += (equivalentVariableCount == 1) ?
                                   " is marked as an external variable, but it is not a primary variable." :
                                   " are marked as external variables, but they are";
                description += (equivalentVariableCount > 2) ? " all" : "";
                description += (equivalentVariableCount == 1) ? "" : " equivalent.";
                description += " Variable '" + primaryExternalVariable.first->name()
                               + "' in component '" + owningComponent(primaryExternalVariable.first)->name()
                               + "' is";
                description += hasPrimaryVariable ?
                                   " the" :
                               (equivalentVariableCount == 1) ?
                                   " its corresponding" :
                                   " their corresponding";
                description += " primary variable and will therefore be the one used as an external variable.";

                referenceRule = Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE;
            }

            auto issue = Issue::IssueImpl::create();

            issue->mPimpl->setDescription(description);
            issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            issue->mPimpl->setReferenceRule(referenceRule);
            issue->mPimpl->mItem->mPimpl->setVariable(primaryExternalVariable.first);

            addIssue(issue);
        }
    }

    // Analyse our different equations' units to make sure that everything is
    // consistent.

    for (const auto &internalEquation : mInternalEquations) {
        UnitsMaps unitsMaps;
        UnitsMaps userUnitsMaps;
        UnitsMultipliers unitsMultipliers;
        std::string issueDescription;
        PowerData powerData;

        analyseEquationUnits(internalEquation->mAst, unitsMaps, userUnitsMaps, unitsMultipliers,
                             issueDescription, powerData);

        if (!issueDescription.empty()) {
            auto issue = Issue::IssueImpl::create();

            issue->mPimpl->setDescription(issueDescription);
            issue->mPimpl->setLevel(Issue::Level::WARNING);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_UNITS);

            addIssue(issue);
        }
    }

    // Determine whether some variables have been marked as external.

    auto hasExternalVariables = std::any_of(mInternalVariables.begin(), mInternalVariables.end(), [](const auto &iv) {
        return iv->mIsExternalVariable;
    });

    // Loop over our equations, checking which variables, if any, can be
    // determined using a given equation.
    // Note: we loop twice by checking the model with the view of:
    //        1) getting an ODE system WITHOUT any NLA systems; and then
    //        2) getting an ODE system WITH one or several NLA systems.
    //       After those two loops, if we still have some unknown variables and
    //       they have been marked as external, then we consider them as
    //       initialised and we go through the two loops one more time. This is
    //       to account for models that have unknown variables (rendering the
    //       model invalid) that have been marked as external (rendering the
    //       model valid).

    auto loopNumber = 1;
    bool relevantCheck;
    auto checkNlaSystems = false;

    do {
        relevantCheck = false;

        for (const auto &internalEquation : mInternalEquations) {
            relevantCheck = internalEquation->check(mAnalyserModel, checkNlaSystems)
                            || relevantCheck;
        }

        if (((loopNumber == 1) || (loopNumber == 3)) && !relevantCheck) {
            ++loopNumber;

            relevantCheck = true;
            checkNlaSystems = true;
        } else if ((loopNumber == 2) && !relevantCheck) {
            // We have gone through the two loops and we still have some unknown
            // variables, so we consider as initialised those that have been
            // marked as external and we go through the two loops one more time.

            for (const auto &internalVariable : mInternalVariables) {
                if (internalVariable->mIsExternalVariable
                    && (internalVariable->mType == AnalyserInternalVariable::Type::UNKNOWN)) {
                    internalVariable->mType = AnalyserInternalVariable::Type::INITIALISED;
                }
            }

            if (hasExternalVariables) {
                ++loopNumber;

                relevantCheck = true;
                checkNlaSystems = false;
            }
        }
    } while (relevantCheck);

    // Make sure that our variables are valid.

    for (const auto &internalVariable : mInternalVariables) {
        switch (internalVariable->mType) {
        case AnalyserInternalVariable::Type::UNKNOWN:
            addInvalidVariableIssue(internalVariable, Issue::ReferenceRule::ANALYSER_VARIABLE_UNUSED);

            break;
        case AnalyserInternalVariable::Type::SHOULD_BE_STATE:
            addInvalidVariableIssue(internalVariable, Issue::ReferenceRule::ANALYSER_STATE_NOT_INITIALISED);

            break;
        case AnalyserInternalVariable::Type::INITIALISED:
            // The variable is (still) initialised so it has to be a constant.

            internalVariable->makeConstant();

            break;
        case AnalyserInternalVariable::Type::OVERCONSTRAINED:
            addInvalidVariableIssue(internalVariable, Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED);

            break;
        default: // Other types we don't care about.
            break;
        }
    }

    if (mAnalyser->errorCount() != 0) {
        auto hasUnderconstrainedVariables = std::any_of(mInternalVariables.begin(), mInternalVariables.end(), [](const auto &iv) {
            switch (iv->mType) {
            case AnalyserInternalVariable::Type::UNKNOWN:
            case AnalyserInternalVariable::Type::SHOULD_BE_STATE:
                return true;
            default:
                return false;
            }
        });
        auto hasOverconstrainedVariables = std::any_of(mInternalVariables.begin(), mInternalVariables.end(), [](const auto &iv) {
            return iv->mType == AnalyserInternalVariable::Type::OVERCONSTRAINED;
        });

        if (hasUnderconstrainedVariables) {
            if (hasOverconstrainedVariables) {
                mAnalyserModel->mPimpl->mType = AnalyserModel::Type::UNSUITABLY_CONSTRAINED;
            } else {
                mAnalyserModel->mPimpl->mType = AnalyserModel::Type::UNDERCONSTRAINED;
            }
        } else {
            mAnalyserModel->mPimpl->mType = AnalyserModel::Type::OVERCONSTRAINED;
        }

        return;
    }

    // Make sure that variables that are initialised using another variable are
    // not initialised using an algebraic variable.

    for (const auto &internalVariable : mInternalVariables) {
        if ((internalVariable->mInitialisingVariable != nullptr)
            && !isCellMLReal(internalVariable->mInitialisingVariable->initialValue())) {
            auto initialisingInternalVariable = Analyser::AnalyserImpl::internalVariable(owningComponent(internalVariable->mInitialisingVariable)->variable(internalVariable->mInitialisingVariable->initialValue()));

            if (initialisingInternalVariable->mType == AnalyserInternalVariable::Type::ALGEBRAIC_VARIABLE) {
                auto issue = Issue::IssueImpl::create();

                issue->mPimpl->setDescription("Variable '" + internalVariable->mVariable->name()
                                              + "' in component '" + owningComponent(internalVariable->mVariable)->name()
                                              + "' is initialised using variable '" + initialisingInternalVariable->mVariable->name()
                                              + "', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE);
                issue->mPimpl->mItem->mPimpl->setVariable(internalVariable->mVariable);

                addIssue(issue);
            }
        }
    }

    if (mAnalyser->errorCount() != 0) {
        mAnalyserModel->mPimpl->mType = AnalyserModel::Type::INVALID;

        return;
    }

    // Make sure that our equations are valid.

    AnalyserInternalVariablePtrs addedExternalVariables;
    AnalyserInternalEquationPtrs addedInternalEquations;
    AnalyserInternalEquationPtrs removedInternalEquations;
    auto nlaSystemIndex = MAX_SIZE_T;

    for (const auto &internalEquation : mInternalEquations) {
        // Account for the unknown variables, in an NLA equation, that have been
        // marked as an external variable. This means removing them from
        // mUnknownVariables and adding a new equation for them so that the NLA
        // equation can have a dependency on them.

        if (internalEquation->mType == AnalyserInternalEquation::Type::NLA) {
            for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
                if (unknownVariable->mIsExternalVariable
                    && (std::find(addedExternalVariables.begin(), addedExternalVariables.end(), unknownVariable) == addedExternalVariables.end())) {
                    addedExternalVariables.push_back(unknownVariable);
                    addedInternalEquations.push_back(AnalyserInternalEquation::create(unknownVariable));
                }
            }

            internalEquation->mUnknownVariables.erase(std::remove_if(internalEquation->mUnknownVariables.begin(), internalEquation->mUnknownVariables.end(), isExternalVariable), internalEquation->mUnknownVariables.end());
        }

        // Discard the equation if we have no unknown variables left.

        if (internalEquation->mUnknownVariables.empty()) {
            removedInternalEquations.push_back(internalEquation);
        }

        // Make the NLA equations that compute the same variables aware of one
        // another and assign them an index for the NLA system in which they are
        // used.

        if (internalEquation->mType == AnalyserInternalEquation::Type::NLA) {
            if (internalEquation->mNlaSystemIndex == MAX_SIZE_T) {
                internalEquation->mNlaSystemIndex = ++nlaSystemIndex;
            }

            for (const auto &otherInternalEquation : mInternalEquations) {
                if ((otherInternalEquation != internalEquation)
                    && (otherInternalEquation->mType == AnalyserInternalEquation::Type::NLA)) {
                    // Check what common unknown variables there are between
                    // internalEquation and otherInternalEquation, if any.
                    // Note: we would normally use std::set_intersection() for
                    //       this, but this would require
                    //       internalEquation->mUnknownVariables and
                    //       otherInternalEquation->mUnknownVariables to be
                    //       sorted which neither of them is and it's not worth
                    //       sorting them for such a trivial case, hence we do
                    //       the intersection ourselves.

                    AnalyserInternalVariablePtrs commonUnknownVariables;

                    for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
                        if (std::find(otherInternalEquation->mUnknownVariables.begin(), otherInternalEquation->mUnknownVariables.end(), unknownVariable) != otherInternalEquation->mUnknownVariables.end()) {
                            commonUnknownVariables.push_back(unknownVariable);
                        }
                    }

                    // Consider otherInternalEquation as an NLA sibling of
                    // internalEquation, if there are some common unknown
                    // variables, and make sure that it has the same NLA system
                    // index as internalEquation.

                    if (!commonUnknownVariables.empty()) {
                        internalEquation->mNlaSiblings.push_back(otherInternalEquation);

                        otherInternalEquation->mNlaSystemIndex = internalEquation->mNlaSystemIndex;
                    }
                }
            }
        }
    }

    // Add/remove some internal equations.

    for (const auto &addedInternalEquation : addedInternalEquations) {
        mInternalEquations.push_back(addedInternalEquation);
    }

    for (const auto &removedInternalEquation : removedInternalEquations) {
        mInternalEquations.erase(std::find(mInternalEquations.begin(), mInternalEquations.end(), removedInternalEquation));
    }

    // Confirm that equations that compute a variable-based constant are still
    // of that type.
    // Note: indeed, when originally qualifying such an equation, all we know
    //       about the variables on which the equation depends is that they have
    //       an initial value. However, those variables may then have been
    //       proven to be computed using an NLA system, in which case the
    //       equation should now be considered as an algebraic equation and the
    //       variable it computes an algebraic variable.

    // Confirm that the variables in an NLA system are not overconstrained.
    // Note: this may happen if an NLA system contains too many NLA equations
    //       to compute its unknown variables and/or if some internal equations
    //       were removed (as a result of some variables in an NLA equation
    //       having been marked as external).

    AnalyserInternalVariablePtrs underconstrainedVariables;
    AnalyserInternalVariablePtrs overconstrainedVariables;

    for (const auto &internalEquation : mInternalEquations) {
        switch (internalEquation->mType) {
        case AnalyserInternalEquation::Type::COMPUTED_CONSTANT: {
            auto unknownVariable = internalEquation->mUnknownVariables.front();

            for (const auto &variable : internalEquation->mAllVariables) {
                if ((variable != unknownVariable)
                    && (variable->mType != AnalyserInternalVariable::Type::CONSTANT)
                    && (variable->mType != AnalyserInternalVariable::Type::COMPUTED_TRUE_CONSTANT)
                    && (variable->mType != AnalyserInternalVariable::Type::COMPUTED_VARIABLE_BASED_CONSTANT)) {
                    // We are supposed to compute a variable-based constant, yet
                    // we have come across a variable which is not some kind of
                    // a constant. In fact, it was an algebraic variable (with
                    // an initial guess) that needs to be computed using an NLA
                    // system. So, requalify the unknown variable and equation.

                    unknownVariable->mType = AnalyserInternalVariable::Type::ALGEBRAIC_VARIABLE;
                    internalEquation->mType = AnalyserInternalEquation::Type::ALGEBRAIC;

                    break;
                }
            }
        } break;
        case AnalyserInternalEquation::Type::NLA:
            if (internalEquation->mNlaSiblings.size() + 1 < internalEquation->mUnknownVariables.size()) {
                // There are fewer NLA equations than unknown variables, so all the unknown variables involved in the
                // NLA system should be considered as underconstrained.

                for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
                    if (std::find(underconstrainedVariables.begin(), underconstrainedVariables.end(), unknownVariable) == underconstrainedVariables.end()) {
                        unknownVariable->mType = AnalyserInternalVariable::Type::UNDERCONSTRAINED;

                        addInvalidVariableIssue(unknownVariable, Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED);

                        underconstrainedVariables.push_back(unknownVariable);
                    }
                }
            } else if (internalEquation->mNlaSiblings.size() + 1 > internalEquation->mUnknownVariables.size()) {
                // There are more NLA equations than unknown variables, so all the unknown variables involved in the NLA
                // system should be considered as overconstrained.

                for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
                    if (std::find(overconstrainedVariables.begin(), overconstrainedVariables.end(), unknownVariable) == overconstrainedVariables.end()) {
                        unknownVariable->mType = AnalyserInternalVariable::Type::OVERCONSTRAINED;

                        addInvalidVariableIssue(unknownVariable, Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED);

                        overconstrainedVariables.push_back(unknownVariable);
                    }
                }
            }

            break;
        default: // Other types we don't care about.
            break;
        }
    }

    if (mAnalyser->errorCount() != 0) {
        if (!underconstrainedVariables.empty()) {
            if (!overconstrainedVariables.empty()) {
                mAnalyserModel->mPimpl->mType = AnalyserModel::Type::UNSUITABLY_CONSTRAINED;
            } else {
                mAnalyserModel->mPimpl->mType = AnalyserModel::Type::UNDERCONSTRAINED;
            }
        } else {
            mAnalyserModel->mPimpl->mType = AnalyserModel::Type::OVERCONSTRAINED;
        }

        return;
    }

    // Determine the type of our model.

    auto hasNlaEquations = std::any_of(mInternalEquations.begin(), mInternalEquations.end(), [=](const auto &ie) {
        if (ie->mType == AnalyserInternalEquation::Type::NLA) {
            // Make sure that not all the variables involved in the NLA system
            // have been marked as external

            return std::any_of(ie->mUnknownVariables.begin(), ie->mUnknownVariables.end(), [](const auto &uv) {
                return !uv->mIsExternalVariable;
            });
        }

        return false;
    });

    if (mAnalyserModel->mPimpl->mVoi != nullptr) {
        mAnalyserModel->mPimpl->mType = hasNlaEquations ?
                                            AnalyserModel::Type::DAE :
                                            AnalyserModel::Type::ODE;
    } else if (!mInternalVariables.empty()) {
        mAnalyserModel->mPimpl->mType = hasNlaEquations ?
                                            AnalyserModel::Type::NLA :
                                            AnalyserModel::Type::ALGEBRAIC;
    }

    if (!mAnalyserModel->isValid()) {
        return;
    }

    // Add a dummy equation for each of our true constants.
    // Note: this is so that a constant can be marked as an external variable.

    for (const auto &internalVariable : mInternalVariables) {
        if (internalVariable->mType == AnalyserInternalVariable::Type::CONSTANT) {
            mInternalEquations.push_back(AnalyserInternalEquation::create(internalVariable));
        }
    }

    // Make it known through our API whether the model has some external
    // variables.

    mAnalyserModel->mPimpl->mHasExternalVariables = hasExternalVariables;

    // Create a mapping between our internal equations and our future equations
    // in the API.

    std::map<AnalyserInternalEquationPtr, AnalyserEquationPtr> aie2aeMappings;

    for (const auto &internalEquation : mInternalEquations) {
        aie2aeMappings.emplace(internalEquation, AnalyserEquation::AnalyserEquationImpl::create());
    }

    // Start making our internal equations available through our API.
    // Note: start because we need to determine the type of our equations before we can make our internal variables
    //       available through our API.

    std::map<AnalyserInternalEquationPtr, AnalyserEquation::Type> aie2aetMappings;

    for (const auto &internalEquation : mInternalEquations) {
        // Determine whether the equation is an external one.

        auto externalEquation = true;

        for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
            if (!unknownVariable->mIsExternalVariable) {
                externalEquation = false;

                break;
            }
        }

        // Determine and keep track of the type of the equation.

        AnalyserEquation::Type type;

        if (externalEquation) {
            type = AnalyserEquation::Type::EXTERNAL;
        } else {
            switch (internalEquation->mType) {
            case AnalyserInternalEquation::Type::CONSTANT:
                type = AnalyserEquation::Type::CONSTANT;

                break;
            case AnalyserInternalEquation::Type::COMPUTED_CONSTANT:
                type = AnalyserEquation::Type::COMPUTED_CONSTANT;

                break;
            case AnalyserInternalEquation::Type::ODE:
                type = AnalyserEquation::Type::ODE;

                break;
            case AnalyserInternalEquation::Type::NLA:
                type = AnalyserEquation::Type::NLA;

                break;
            case AnalyserInternalEquation::Type::ALGEBRAIC:
                type = AnalyserEquation::Type::ALGEBRAIC;

                break;
            default: // AnalyserEquation::Type::UNKNOWN.
                continue;
            }
        }

        aie2aetMappings.emplace(internalEquation, type);
    }

    // Make our internal variables available through our API.

    std::map<AnalyserInternalVariablePtr, AnalyserVariablePtr> aiv2avMappings;
    std::map<VariablePtr, AnalyserVariablePtr> v2avMappings;
    auto stateIndex = MAX_SIZE_T;
    auto constantIndex = MAX_SIZE_T;
    auto computedConstantIndex = MAX_SIZE_T;
    auto algebraicVariableIndex = MAX_SIZE_T;
    auto externalVariableIndex = MAX_SIZE_T;

    for (const auto &internalVariable : mInternalVariables) {
        // Determine the type of the variable.

        AnalyserVariable::Type variableType;

        if (internalVariable->mIsExternalVariable) {
            variableType = AnalyserVariable::Type::EXTERNAL_VARIABLE;
        } else {
            switch (internalVariable->mType) {
            case AnalyserInternalVariable::Type::STATE:
                variableType = AnalyserVariable::Type::STATE;

                break;
            case AnalyserInternalVariable::Type::CONSTANT:
                variableType = AnalyserVariable::Type::CONSTANT;

                break;
            case AnalyserInternalVariable::Type::COMPUTED_TRUE_CONSTANT:
            case AnalyserInternalVariable::Type::COMPUTED_VARIABLE_BASED_CONSTANT:
                variableType = AnalyserVariable::Type::COMPUTED_CONSTANT;

                break;
            case AnalyserInternalVariable::Type::ALGEBRAIC_VARIABLE:
            case AnalyserInternalVariable::Type::INITIALISED_ALGEBRAIC_VARIABLE:
                variableType = AnalyserVariable::Type::ALGEBRAIC_VARIABLE;

                break;
            default: // AnalyserInternalVariable::Type::VARIABLE_OF_INTEGRATION.
                // This is the variable of integration, so skip it.

                continue;
            }
        }

        // Retrieve the equations used to compute the variable.

        AnalyserEquationPtrs equations;
        auto isNlaEquation = false;

        for (const auto &internalEquation : mInternalEquations) {
            if (std::find(internalEquation->mUnknownVariables.begin(), internalEquation->mUnknownVariables.end(), internalVariable) != internalEquation->mUnknownVariables.end()) {
                equations.push_back(aie2aeMappings[internalEquation]);

                if ((aie2aetMappings.find(internalEquation) != aie2aetMappings.end())
                    && (aie2aetMappings[internalEquation] == AnalyserEquation::Type::NLA)) {
                    isNlaEquation = true;
                }
            }
        }

        // Correct the type of the variable if it is a computed constant that is computed using an NLA equation.

        if ((variableType == AnalyserVariable::Type::COMPUTED_CONSTANT) && isNlaEquation) {
            variableType = AnalyserVariable::Type::ALGEBRAIC_VARIABLE;
        }

        // Populate and keep track of the state/variable.

        auto variable = AnalyserVariable::AnalyserVariableImpl::create();

        variable->mPimpl->populate(variableType,
                                   (variableType == AnalyserVariable::Type::STATE) ?
                                       ++stateIndex :
                                   (variableType == AnalyserVariable::Type::CONSTANT) ?
                                       ++constantIndex :
                                   (variableType == AnalyserVariable::Type::COMPUTED_CONSTANT) ?
                                       ++computedConstantIndex :
                                   (variableType == AnalyserVariable::Type::ALGEBRAIC_VARIABLE) ?
                                       ++algebraicVariableIndex :
                                       ++externalVariableIndex,
                                   (variableType == AnalyserVariable::Type::EXTERNAL_VARIABLE) ?
                                       nullptr :
                                       internalVariable->mInitialisingVariable,
                                   internalVariable->mVariable, mAnalyserModel, equations);

        aiv2avMappings.emplace(internalVariable, variable);
        v2avMappings.emplace(internalVariable->mVariable, variable);

        if (variableType == AnalyserVariable::Type::STATE) {
            mAnalyserModel->mPimpl->mStates.push_back(variable);
        } else if (variableType == AnalyserVariable::Type::CONSTANT) {
            mAnalyserModel->mPimpl->mConstants.push_back(variable);
        } else if (variableType == AnalyserVariable::Type::COMPUTED_CONSTANT) {
            mAnalyserModel->mPimpl->mComputedConstants.push_back(variable);
        } else if (variableType == AnalyserVariable::Type::ALGEBRAIC_VARIABLE) {
            mAnalyserModel->mPimpl->mAlgebraicVariables.push_back(variable);
        } else { // AnalyserVariable::Type::EXTERNAL_VARIABLE.
            mAnalyserModel->mPimpl->mExternalVariables.push_back(variable);
        }
    }

    // Finish making our internal equations available through our API.

    for (const auto &internalEquation : mInternalEquations) {
        // Make sure that the type of the equation is known.

        if (aie2aetMappings.find(internalEquation) == aie2aetMappings.end()) {
            continue;
        }

        // Scale our internal equation's AST to take into account the fact that
        // we may have mapped variables that use compatible units rather than
        // equivalent ones.

        scaleEquationAst(internalEquation->mAst);

        // Manipulate the equation, if needed.

        auto equationType = aie2aetMappings[internalEquation];

        switch (equationType) {
        case AnalyserEquation::Type::NLA:
            // The equation is currently of the form LHS = RHS, but we want it
            // in the form LHS-RHS, so replace the equality element with a minus
            // one.

            internalEquation->mAst->setType(AnalyserEquationAst::Type::MINUS);

            break;
        case AnalyserEquation::Type::EXTERNAL:
            // Do nothing.

            break;
        default:
            // Swap the LHS and RHS of the equation if its unknown variable is
            // on its RHS.

            if (internalEquation->variableOnRhs(internalEquation->mUnknownVariables.front())) {
                internalEquation->mAst->swapLeftAndRightChildren();
            }

            break;
        }

        // Determine the equation's dependencies, i.e. the equations for the
        // variables on which this equation depends.

        VariablePtrs variableDependencies;

        if (equationType == AnalyserEquation::Type::EXTERNAL) {
            for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
                for (const auto &dependency : unknownVariable->mDependencies) {
                    variableDependencies.push_back(dependency);
                }
            }
        } else {
            variableDependencies = internalEquation->mDependencies;
        }

        AnalyserEquationPtrs equationDependencies;

        for (const auto &variableDependency : variableDependencies) {
            auto analyserVariable = v2avMappings[variableDependency];

            if (analyserVariable != nullptr) {
                for (const auto &analyserEquation : analyserVariable->analyserEquations()) {
                    if (std::find(equationDependencies.begin(), equationDependencies.end(), analyserEquation) == equationDependencies.end()) {
                        if (analyserVariable->type() == AnalyserVariable::Type::CONSTANT) {
                            // This is a constant, so keep track of it in case it is untracked and in case we need to
                            // generate some code for it.

                            analyserEquation->mPimpl->mConstant = analyserVariable;
                        }

                        equationDependencies.push_back(analyserEquation);
                    }
                }
            }
        }

        // Determine the equation's NLA siblings, i.e. the equations that should
        // be computed as part of an NLA system, should this equation be an NLA
        // one.

        AnalyserEquationPtrs equationNlaSiblings;

        for (const auto &nlaSibling : internalEquation->mNlaSiblings) {
            equationNlaSiblings.push_back(aie2aeMappings[nlaSibling.lock()]);
        }

        // Populate and keep track of the equation.

        auto equation = aie2aeMappings[internalEquation];

        equation->mPimpl->mType = equationType;
        equation->mPimpl->mAst = (equationType == AnalyserEquation::Type::EXTERNAL) ?
                                     nullptr :
                                     internalEquation->mAst;
        equation->mPimpl->mNlaSystemIndex = internalEquation->mNlaSystemIndex;

        for (const auto &unknownVariable : internalEquation->mUnknownVariables) {
            // Keep track of the variable that the equation computes.
            // Note: an unknown variable cannot be the variable of integration (since it cannot be computed) or a
            //       constant (snce it is not computed but set), so no need to check for those.

            auto variable = aiv2avMappings[unknownVariable];
            auto variableType = variable->type();

            if (variableType == AnalyserVariable::Type::STATE) {
                equation->mPimpl->mStates.push_back(variable);
            } else if (variableType == AnalyserVariable::Type::COMPUTED_CONSTANT) {
                equation->mPimpl->mComputedConstants.push_back(variable);
            } else if (variableType == AnalyserVariable::Type::ALGEBRAIC_VARIABLE) {
                equation->mPimpl->mAlgebraicVariables.push_back(variable);
            } else { // AnalyserVariable::Type::EXTERNAL_VARIABLE.
                equation->mPimpl->mExternalVariables.push_back(variable);
            }
        }

        std::copy(equationDependencies.begin(), equationDependencies.end(), back_inserter(equation->mPimpl->mDependencies));
        std::copy(equationNlaSiblings.begin(), equationNlaSiblings.end(), back_inserter(equation->mPimpl->mNlaSiblings));

        mAnalyserModel->mPimpl->mAnalyserEquations.push_back(equation);
    }

    // Remove the staging equations for our constants.
    // Note: indeed, some equations may have a dependency on one or several constants (for which there are no proper
    //       equations). So, we need to remove those dependencies, and obviously this can only be done once all our
    //       equations are ready.

    for (const auto &analyserEquation : mAnalyserModel->mPimpl->mAnalyserEquations) {
        analyserEquation->mPimpl->removeStagingDependencies();
    }

    // Determine whether our equations are state/rate based.
    // Note: obviously, this can only be done once all our equations are ready.

    for (const auto &analyserEquation : mAnalyserModel->mPimpl->mAnalyserEquations) {
        AnalyserEquationPtrs checkedEquations;

        analyserEquation->mPimpl->mIsStateRateBased = isStateRateBased(analyserEquation, checkedEquations);
    }
}

AnalyserExternalVariablePtrs::const_iterator Analyser::AnalyserImpl::findExternalVariable(const VariablePtr &variable) const
{
    return std::find_if(mExternalVariables.begin(), mExternalVariables.end(), [=](const auto &ev) {
        return ev->variable() == variable;
    });
}

AnalyserExternalVariablePtrs::const_iterator Analyser::AnalyserImpl::findExternalVariable(const AnalyserExternalVariablePtr &externalVariable) const
{
    return std::find_if(mExternalVariables.begin(), mExternalVariables.end(), [=](const auto &ev) {
        return ev == externalVariable;
    });
}

Analyser::AnalyserImpl *Analyser::pFunc()
{
    return reinterpret_cast<Analyser::AnalyserImpl *>(Logger::pFunc());
}

const Analyser::AnalyserImpl *Analyser::pFunc() const
{
    return reinterpret_cast<Analyser::AnalyserImpl const *>(Logger::pFunc());
}

Analyser::Analyser()
    : Logger(new AnalyserImpl())
{
    pFunc()->mAnalyser = this;
}

Analyser::~Analyser()
{
    delete pFunc();
}

AnalyserPtr Analyser::create() noexcept
{
    return std::shared_ptr<Analyser> {new Analyser {}};
}

void Analyser::analyseModel(const ModelPtr &model)
{
    // Make sure that we have a model and that it is valid before analysing it.

    pFunc()->removeAllIssues();

    if (model == nullptr) {
        auto issue = Issue::IssueImpl::create();

        issue->mPimpl->setDescription("The model is null.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::INVALID_ARGUMENT);

        pFunc()->addIssue(issue);

        return;
    }

    auto validator = Validator::create();

    validator->validateModel(model);

    if (validator->issueCount() > 0) {
        // The model is not valid, so retrieve the validation issues and make
        // them our own.

        for (size_t i = 0; i < validator->issueCount(); ++i) {
            pFunc()->addIssue(validator->issue(i));
        }

        pFunc()->mAnalyserModel->mPimpl->mType = AnalyserModel::Type::INVALID;
    }

    // Check for non-validation errors that will render the given model invalid
    // for analysis.

    if (model->hasUnlinkedUnits()) {
        auto issue = Issue::IssueImpl::create();

        issue->mPimpl->setDescription("The model has units which are not linked together.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANALYSER_UNLINKED_UNITS);

        pFunc()->addIssue(issue);
    }

    // Analyse the model, but only if we didn't come across any issues.

    if (issueCount() == 0) {
        pFunc()->analyseModel(model);
    }
}

bool Analyser::addExternalVariable(const VariablePtr &variable)
{
    if (containsExternalVariable(variable)) {
        return false;
    }

    pFunc()->mExternalVariables.push_back(AnalyserExternalVariable::create(variable));

    return true;
}

bool Analyser::addExternalVariable(const AnalyserExternalVariablePtr &externalVariable)
{
    if (containsExternalVariable(externalVariable)) {
        return false;
    }

    pFunc()->mExternalVariables.push_back(externalVariable);

    return true;
}

bool Analyser::removeExternalVariable(size_t index)
{
    if (index < pFunc()->mExternalVariables.size()) {
        pFunc()->mExternalVariables.erase(pFunc()->mExternalVariables.begin() + ptrdiff_t(index));

        return true;
    }

    return false;
}

bool Analyser::removeExternalVariable(const VariablePtr &variable)
{
    auto result = pFunc()->findExternalVariable(variable);

    if (result != pFunc()->mExternalVariables.end()) {
        pFunc()->mExternalVariables.erase(result);

        return true;
    }

    return false;
}

bool Analyser::removeExternalVariable(const AnalyserExternalVariablePtr &externalVariable)
{
    auto result = pFunc()->findExternalVariable(externalVariable);

    if (result != pFunc()->mExternalVariables.end()) {
        pFunc()->mExternalVariables.erase(result);

        return true;
    }

    return false;
}

void Analyser::removeAllExternalVariables()
{
    pFunc()->mExternalVariables.clear();
}

bool Analyser::containsExternalVariable(const VariablePtr &variable) const
{
    return pFunc()->findExternalVariable(variable) != pFunc()->mExternalVariables.end();
}

bool Analyser::containsExternalVariable(const AnalyserExternalVariablePtr &externalVariable) const
{
    return pFunc()->findExternalVariable(externalVariable) != pFunc()->mExternalVariables.end();
}

AnalyserExternalVariablePtr Analyser::externalVariable(size_t index) const
{
    if (index < pFunc()->mExternalVariables.size()) {
        return pFunc()->mExternalVariables[index];
    }

    return nullptr;
}

AnalyserExternalVariablePtr Analyser::externalVariable(const VariablePtr &variable) const
{
    auto result = pFunc()->findExternalVariable(variable);

    if (result != pFunc()->mExternalVariables.end()) {
        return *result;
    }

    return nullptr;
}

size_t Analyser::externalVariableCount() const
{
    return pFunc()->mExternalVariables.size();
}

AnalyserModelPtr Analyser::analyserModel() const
{
    return pFunc()->mAnalyserModel;
}

} // namespace libcellml
