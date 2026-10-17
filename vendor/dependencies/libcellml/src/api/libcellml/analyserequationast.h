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

#pragma once

#include "libcellml/analyser.h"
#include "libcellml/undefines.h"

namespace libcellml {

/**
 * @brief The AnalyserEquationAst class.
 *
 * The AnalyserEquationAst class is for representing an equation abstract syntax
 * tree (AST) in the context of a CellML Analyser.
 */
class LIBCELLML_EXPORT AnalyserEquationAst
{
    friend class Analyser;

public:
    /**
     * @brief The type of a node in an abstract syntax tree (AST).
     *
     * The type of a node in an abstract syntax tree (AST), i.e. whether it is
     * an equality type, a relational or a logical operator, an arithmetic
     * operator, a calculus element, a trigonometric operator, a piecewise
     * statement, a token element, a qualifier element, or a constant.
     */
    enum class Type
    {
        // Equality.

        EQUALITY, /**< The equality operator. */

        // Relational and logical operators.

        EQ, /**< The equal to operator. */
        NEQ, /**< The not equal to operator. */
        LT, /**< The less than operator. */
        LEQ, /**< The less than or equal to operator. */
        GT, /**< The greater than operator. */
        GEQ, /**< The greater than or equal to operator. */
        AND, /**< The and operator. */
        OR, /**< The or operator. */
        XOR, /**< The exclusive or operator. */
        NOT, /**< The not operator. */

        // Arithmetic operators.

        PLUS, /**< The plus operator. */
        MINUS, /**< The minus operator. */
        TIMES, /**< The times operator. */
        DIVIDE, /**< The divide operator. */
        POWER, /**< The power operator. */
        ROOT, /**< The root operator. */
        ABS, /**< The absolute value function. */
        EXP, /**< The exponential function. */
        LN, /**< The natural logarithm function. */
        LOG, /**< The common logarithm function. */
        CEILING, /**< The ceiling function. */
        FLOOR, /**< The floor function. */
        MIN, /**< The minimum function. */
        MAX, /**< The maximum function. */
        REM, /**< The remainder function. */

        // Calculus elements.

        DIFF, /**< The differentiation operator. */

        // Trigonometric operators.

        SIN, /**< The sine function. */
        COS, /**< The cosine function. */
        TAN, /**< The tangent function. */
        SEC, /**< The secant function. */
        CSC, /**< The cosecant function. */
        COT, /**< The cotangent function. */
        SINH, /**< The hyperbolic sine function. */
        COSH, /**< The hyperbolic cosine function. */
        TANH, /**< The hyperbolic tangent function. */
        SECH, /**< The hyperbolic secant function. */
        CSCH, /**< The hyperbolic cosecant function. */
        COTH, /**< The hyperbolic cotangent function. */
        ASIN, /**< The arc sine function. */
        ACOS, /**< The arc cosine function. */
        ATAN, /**< The arc tangent function. */
        ASEC, /**< The arc secant function. */
        ACSC, /**< The arc cosecant function. */
        ACOT, /**< The arc cotangent function. */
        ASINH, /**< The arc hyperbolic sine function. */
        ACOSH, /**< The arc hyperbolic cosine function. */
        ATANH, /**< The arc hyperbolic tangent function. */
        ASECH, /**< The arc hyperbolic secant function. */
        ACSCH, /**< The arc hyperbolic cosecant function. */
        ACOTH, /**< The arc hyperbolic cotangent function. */

        // Piecewise statement.

        PIECEWISE, /**< The "piecewise" statement. */
        PIECE, /**< The "piece" part of a "piecewise" statement. */
        OTHERWISE, /**< The "otherwise" part of a "piecewise" statement. */

        // Token elements.

        CI, /**< An identifier (i.e. the name of a model variable). */
        CN, /**< A number. */

        // Qualifier elements.

        DEGREE, /**< The degree of a root operator (it is only used when its value is not 2). */
        LOGBASE, /**< The base with respect to which the logarithm is taken. */
        BVAR, /**< The bound variable of a differential equation. */

        // Constants.

        TRUE, /**< The "true" boolean. */
        FALSE, /**< The "false" boolean. */
        E, /**< Euler's number. */
        PI, /**< The π constant. */
        INF, /**< The infinity value. */
        NAN /**< The not-a-number value. */
    };

    ~AnalyserEquationAst(); /**< Destructor, @private. */
    AnalyserEquationAst(const AnalyserEquationAst &rhs) = delete; /**< Copy constructor, @private. */
    AnalyserEquationAst(AnalyserEquationAst &&rhs) noexcept = delete; /**< Move constructor, @private. */
    AnalyserEquationAst &operator=(AnalyserEquationAst rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create an @ref AnalyserEquationAst object.
     *
     * Factory method to create an @ref AnalyserEquationAst. Create a blank
     * equation AST with::
     *
     * @code
     *   auto ast = libcellml::AnalyserEquationAst::create();
     * @endcode
     *
     * @return A smart pointer to an @ref AnalyserEquationAst object.
     */
    static AnalyserEquationAstPtr create() noexcept;

    /**
     * @brief Get the @ref Type of this @ref AnalyserEquationAst.
     *
     * Return the @ref Type of this @ref AnalyserEquationAst.
     *
     * @return The @ref Type.
     */
    Type type() const;

    /**
     * @brief Get the string version of a @ref Type.
     *
     * Return the string version of a @ref Type.
     *
     * @param type The type for which we want the string version.
     *
     * @return The string version of the @ref Type.
     */
    static std::string typeAsString(Type type);

    /**
     * @brief Set the type of this @ref AnalyserEquationAst.
     *
     * Set the type of this @ref AnalyserEquationAst.
     *
     * @param type The @ref AnalyserEquationAst type to be set.
     */
    void setType(Type type);

    /**
     * @brief Get the value for this @ref AnalyserEquationAst.
     *
     * Return the value for this @ref AnalyserEquationAst.
     *
     * @return The value.
     */
    std::string value() const;

    /**
     * @brief Set the value for this @ref AnalyserEquationAst.
     *
     * Set the value for this @ref AnalyserEquationAst.
     *
     * @param value The @c std::string to be set as the value for this
     * @ref AnalyserEquationAst.
     */
    void setValue(const std::string &value);

    /**
     * @brief Get the @ref Variable for this @ref AnalyserEquationAst.
     *
     * Return the @ref Variable for this @ref AnalyserEquationAst.
     *
     * @return The variable.
     */
    VariablePtr variable() const;

    /**
     * @brief Set the @ref Variable for this @ref AnalyserEquationAst.
     *
     * Set the @ref Variable for this @ref AnalyserEquationAst.
     *
     * @param variable The @ref Variable to be set as the variable for this
     * @ref AnalyserEquationAst.
     */
    void setVariable(const VariablePtr &variable);

    /**
     * @brief Get the parent of this @ref AnalyserEquationAst.
     *
     * Return the parent of this @ref AnalyserEquationAst.
     *
     * @return The parent.
     */
    AnalyserEquationAstPtr parent() const;

    /**
     * @brief Set the parent of this @ref AnalyserEquationAst.
     *
     * Set the parent of this @ref AnalyserEquationAst.
     *
     * @param parent The @ref AnalyserEquationAst to be set as the parent of this
     * @ref AnalyserEquationAst.
     */
    void setParent(const AnalyserEquationAstPtr &parent);

    /**
     * @brief Get the left child for this @ref AnalyserEquationAst.
     *
     * Return the left child of this @ref AnalyserEquationAst.
     *
     * @return The left child.
     */
    AnalyserEquationAstPtr leftChild() const;

    /**
     * @brief Set the left child of this @ref AnalyserEquationAst.
     *
     * Set the left child of this @ref AnalyserEquationAst.
     *
     * @param leftChild The @ref AnalyserEquationAst to be set as the left child
     * of this @ref AnalyserEquationAst.
     */
    void setLeftChild(const AnalyserEquationAstPtr &leftChild);

    /**
     * @brief Get the right child for this @ref AnalyserEquationAst.
     *
     * Return the right child of this @ref AnalyserEquationAst.
     *
     * @return The right child.
     */
    AnalyserEquationAstPtr rightChild() const;

    /**
     * @brief Set the right child of this @ref AnalyserEquationAst.
     *
     * Set the right child of this @ref AnalyserEquationAst.
     *
     * @param rightChild The @ref AnalyserEquationAst to be set as the right child
     * of this @ref AnalyserEquationAst.
     */
    void setRightChild(const AnalyserEquationAstPtr &rightChild);

    /**
     * @brief Swap the left and right children of this @ref AnalyserEquationAst.
     *
     * Swap the left and right children of this @ref AnalyserEquationAst.
     */
    void swapLeftAndRightChildren();

private:
    AnalyserEquationAst(); /**< Constructor, @private. */

    struct AnalyserEquationAstImpl;
    AnalyserEquationAstImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
