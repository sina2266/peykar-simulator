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

#include "libcellml/analyserequationast.h"

#include "analyserequationast_p.h"

#include "libcellml/undefines.h"

namespace libcellml {

void AnalyserEquationAst::AnalyserEquationAstImpl::populate(AnalyserEquationAst::Type type,
                                                            const AnalyserEquationAstPtr &parent)
{
    mType = type;
    mParent = parent;
}

void AnalyserEquationAst::AnalyserEquationAstImpl::populate(AnalyserEquationAst::Type type,
                                                            const std::string &value,
                                                            const AnalyserEquationAstPtr &parent)
{
    mType = type;
    mValue = value;
    mParent = parent;
}

void AnalyserEquationAst::AnalyserEquationAstImpl::populate(AnalyserEquationAst::Type type,
                                                            const VariablePtr &variable,
                                                            const AnalyserEquationAstPtr &parent)
{
    mType = type;
    mVariable = variable;
    mParent = parent;
}

AnalyserEquationAst::AnalyserEquationAst()
    : mPimpl(new AnalyserEquationAstImpl())
{
}

AnalyserEquationAst::~AnalyserEquationAst()
{
    delete mPimpl;
}

AnalyserEquationAstPtr AnalyserEquationAst::create() noexcept
{
    return std::shared_ptr<AnalyserEquationAst> {new AnalyserEquationAst {}};
}

AnalyserEquationAst::Type AnalyserEquationAst::type() const
{
    return mPimpl->mType;
}

static const std::map<AnalyserEquationAst::Type, std::string> typeToString = {
    // Equality.

    {AnalyserEquationAst::Type::EQUALITY, "equality"},

    // Relational and logical operators.

    {AnalyserEquationAst::Type::EQ, "eq"},
    {AnalyserEquationAst::Type::NEQ, "neq"},
    {AnalyserEquationAst::Type::LT, "lt"},
    {AnalyserEquationAst::Type::LEQ, "leq"},
    {AnalyserEquationAst::Type::GT, "gt"},
    {AnalyserEquationAst::Type::GEQ, "geq"},
    {AnalyserEquationAst::Type::AND, "and"},
    {AnalyserEquationAst::Type::OR, "or"},
    {AnalyserEquationAst::Type::XOR, "xor"},
    {AnalyserEquationAst::Type::NOT, "not"},

    // Arithmetic operators.

    {AnalyserEquationAst::Type::PLUS, "plus"},
    {AnalyserEquationAst::Type::MINUS, "minus"},
    {AnalyserEquationAst::Type::TIMES, "times"},
    {AnalyserEquationAst::Type::DIVIDE, "divide"},
    {AnalyserEquationAst::Type::POWER, "power"},
    {AnalyserEquationAst::Type::ROOT, "root"},
    {AnalyserEquationAst::Type::ABS, "abs"},
    {AnalyserEquationAst::Type::EXP, "exp"},
    {AnalyserEquationAst::Type::LN, "ln"},
    {AnalyserEquationAst::Type::LOG, "log"},
    {AnalyserEquationAst::Type::CEILING, "ceiling"},
    {AnalyserEquationAst::Type::FLOOR, "floor"},
    {AnalyserEquationAst::Type::MIN, "min"},
    {AnalyserEquationAst::Type::MAX, "max"},
    {AnalyserEquationAst::Type::REM, "rem"},

    // Calculus elements.

    {AnalyserEquationAst::Type::DIFF, "diff"},

    // Trigonometric operators.

    {AnalyserEquationAst::Type::SIN, "sin"},
    {AnalyserEquationAst::Type::COS, "cos"},
    {AnalyserEquationAst::Type::TAN, "tan"},
    {AnalyserEquationAst::Type::SEC, "sec"},
    {AnalyserEquationAst::Type::CSC, "csc"},
    {AnalyserEquationAst::Type::COT, "cot"},
    {AnalyserEquationAst::Type::SINH, "sinh"},
    {AnalyserEquationAst::Type::COSH, "cosh"},
    {AnalyserEquationAst::Type::TANH, "tanh"},
    {AnalyserEquationAst::Type::SECH, "sech"},
    {AnalyserEquationAst::Type::CSCH, "csch"},
    {AnalyserEquationAst::Type::COTH, "coth"},
    {AnalyserEquationAst::Type::ASIN, "asin"},
    {AnalyserEquationAst::Type::ACOS, "acos"},
    {AnalyserEquationAst::Type::ATAN, "atan"},
    {AnalyserEquationAst::Type::ASEC, "asec"},
    {AnalyserEquationAst::Type::ACSC, "acsc"},
    {AnalyserEquationAst::Type::ACOT, "acot"},
    {AnalyserEquationAst::Type::ASINH, "asinh"},
    {AnalyserEquationAst::Type::ACOSH, "acosh"},
    {AnalyserEquationAst::Type::ATANH, "atanh"},
    {AnalyserEquationAst::Type::ASECH, "asech"},
    {AnalyserEquationAst::Type::ACSCH, "acsch"},
    {AnalyserEquationAst::Type::ACOTH, "acoth"},

    // Piecewise statement.

    {AnalyserEquationAst::Type::PIECEWISE, "piecewise"},
    {AnalyserEquationAst::Type::PIECE, "piece"},
    {AnalyserEquationAst::Type::OTHERWISE, "otherwise"},

    // Token elements.

    {AnalyserEquationAst::Type::CI, "ci"},
    {AnalyserEquationAst::Type::CN, "cn"},

    // Qualifier elements.

    {AnalyserEquationAst::Type::DEGREE, "degree"},
    {AnalyserEquationAst::Type::LOGBASE, "logbase"},
    {AnalyserEquationAst::Type::BVAR, "bvar"},

    // Constants.

    {AnalyserEquationAst::Type::TRUE, "true"},
    {AnalyserEquationAst::Type::FALSE, "false"},
    {AnalyserEquationAst::Type::E, "e"},
    {AnalyserEquationAst::Type::PI, "pi"},
    {AnalyserEquationAst::Type::INF, "inf"},
    {AnalyserEquationAst::Type::NAN, "nan"}};

std::string AnalyserEquationAst::typeAsString(Type type)
{
    return typeToString.at(type);
}

void AnalyserEquationAst::setType(Type type)
{
    mPimpl->mType = type;
}

std::string AnalyserEquationAst::value() const
{
    return mPimpl->mValue;
}

void AnalyserEquationAst::setValue(const std::string &value)
{
    mPimpl->mValue = value;
}

VariablePtr AnalyserEquationAst::variable() const
{
    return mPimpl->mVariable;
}

void AnalyserEquationAst::setVariable(const VariablePtr &variable)
{
    mPimpl->mVariable = variable;
}

AnalyserEquationAstPtr AnalyserEquationAst::parent() const
{
    return mPimpl->mParent.lock();
}

void AnalyserEquationAst::setParent(const AnalyserEquationAstPtr &parent)
{
    mPimpl->mParent = parent;
}

AnalyserEquationAstPtr AnalyserEquationAst::leftChild() const
{
    if (mPimpl->mOwnedLeftChild != nullptr) {
        return mPimpl->mOwnedLeftChild;
    }

    return mPimpl->mLeftChild;
}

void AnalyserEquationAst::setLeftChild(const AnalyserEquationAstPtr &leftChild)
{
    mPimpl->mOwnedLeftChild = nullptr;
    mPimpl->mLeftChild = leftChild;
}

AnalyserEquationAstPtr AnalyserEquationAst::rightChild() const
{
    if (mPimpl->mOwnedRightChild != nullptr) {
        return mPimpl->mOwnedRightChild;
    }

    return mPimpl->mRightChild;
}

void AnalyserEquationAst::setRightChild(const AnalyserEquationAstPtr &rightChild)
{
    mPimpl->mOwnedRightChild = nullptr;
    mPimpl->mRightChild = rightChild;
}

void AnalyserEquationAst::swapLeftAndRightChildren()
{
    auto oldOwnedLeftChild = mPimpl->mOwnedLeftChild;
    auto oldOwnedRightChild = mPimpl->mOwnedRightChild;

    mPimpl->mOwnedLeftChild = oldOwnedRightChild;
    mPimpl->mOwnedRightChild = oldOwnedLeftChild;

    auto oldLeftChild = mPimpl->mLeftChild;
    auto oldRightChild = mPimpl->mRightChild;

    mPimpl->mLeftChild = oldRightChild;
    mPimpl->mRightChild = oldLeftChild;
}

} // namespace libcellml
