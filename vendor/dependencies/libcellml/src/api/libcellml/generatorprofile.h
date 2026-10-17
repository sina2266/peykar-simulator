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

#include <string>

#include "libcellml/exportdefinitions.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The GeneratorProfile class.
 *
 * The GeneratorProfile class is for representing a CellML Generator Profile.
 */
class LIBCELLML_EXPORT GeneratorProfile
{
public:
    /**
     * @brief The type of a profile.
     *
     * A profile can be of one of the following types:
     *  - C: a profile that targets the C language; or
     *  - PYTHON: a profile that targets the Python language.
     */
    enum class Profile
    {
        C,
        PYTHON
    };

    ~GeneratorProfile(); /**< Destructor, @private. */
    GeneratorProfile(const GeneratorProfile &rhs) = delete; /**< Copy constructor, @private. */
    GeneratorProfile(GeneratorProfile &&rhs) noexcept = delete; /**< Move constructor, @private. */
    GeneratorProfile &operator=(GeneratorProfile rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @ref GeneratorProfile object.
     *
     * Factory method to create a @ref GeneratorProfile.  Create a
     * generator profile with::
     *
     *   GeneratorProfilePtr generatorProfile = libcellml::GeneratorProfile::create();
     *
     * @return A smart pointer to a @ref GeneratorProfile object.
     */
    static GeneratorProfilePtr create(Profile profile = Profile::C) noexcept;

    // Whether the profile is official.

    /**
     * @brief Get the @ref Profile for this @ref GeneratorProfile.
     *
     * Return the @ref Profile for this @ref GeneratorProfile.
     *
     * @return The @ref Profile for this @ref GeneratorProfile.
     */
    Profile profile() const;

    /**
     * @brief Get the string version of a @ref Profile.
     *
     * Return the string version of a @ref Profile.
     *
     * @param profile The profile for which we want the string version.
     *
     * @return The string version of the @ref Profile.
     */
    static std::string profileAsString(Profile profile);

    /**
     * @brief Set the @ref Profile.
     *
     * Set this @ref GeneratorProfile to the given @ref Profile .
     *
     * @param profile The @ref Profile to use.
     */
    void setProfile(Profile profile);

    // Whether the profile requires an interface to be generated.

    /**
     * @brief Test if this @ref GeneratorProfile requires an interface to be
     * generated.
     *
     * Test if this @ref GeneratorProfile has an requires an interface to be
     * generated.
     *
     * @return @c true if the @ref GeneratorProfile has an requires an interface
     * to be generated,
     * @c false otherwise.
     */
    bool hasInterface() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has an requires an interface
     * to be generated.
     *
     * Set whether this @ref GeneratorProfile has an requires an interface to be
     * generated.
     *
     * @param hasInterface A @c bool to determine whether this
     * @ref GeneratorProfile has an requires an interface to be generated.
     */
    void setHasInterface(bool hasInterface);

    // Equality.

    /**
     * @brief Get the @c std::string representing the MathML "equality"
     * operator.
     *
     * Return the @c std::string representing the MathML "equality" operator.
     *
     * @return The @c std::string representing the MathML "equality" operator.
     */
    std::string equalityString() const;

    /**
     * @brief Set the @c std::string representing the MathML "equality"
     * operator.
     *
     * Set the @c std::string representing the MathML "equality" operator.
     *
     * @param equalityString The @c std::string representing the MathML
     * "equality" operator.
     */
    void setEqualityString(const std::string &equalityString);

    // Relational and logical operators.

    /**
     * @brief Get the @c std::string representing the MathML "equal to"
     * operator.
     *
     * Return the @c std::string representing the MathML "equal to" operator.
     *
     * @return The @c std::string representing the MathML "equal to" operator.
     */
    std::string eqString() const;

    /**
     * @brief Set the @c std::string representing the MathML "equal to"
     * operator.
     *
     * Set the @c std::string representing the MathML "equal to" operator.
     *
     * @param eqString The @c std::string representing the MathML "equal to"
     * operator.
     */
    void setEqString(const std::string &eqString);

    /**
     * @brief Get the @c std::string representing the MathML "not equal to"
     * operator.
     *
     * Return the @c std::string representing the MathML "not equal to"
     * operator.
     *
     * @return The @c std::string representing the MathML "not equal to"
     * operator.
     */
    std::string neqString() const;

    /**
     * @brief Set the @c std::string representing the MathML "not equal to"
     * operator.
     *
     * Set the @c std::string representing the MathML "not equal to" operator.
     *
     * @param neqString The @c std::string representing the MathML "not equal
     * to" operator.
     */
    void setNeqString(const std::string &neqString);

    /**
     * @brief Get the @c std::string representing the MathML "less than"
     * operator.
     *
     * Return the @c std::string representing the MathML "less than" operator.
     *
     * @return The @c std::string representing the MathML "less than" operator.
     */
    std::string ltString() const;

    /**
     * @brief Set the @c std::string representing the MathML "less than"
     * operator.
     *
     * Set the @c std::string representing the MathML "less than" operator.
     *
     * @param ltString The @c std::string representing the MathML "less than"
     * operator.
     */
    void setLtString(const std::string &ltString);

    /**
     * @brief Get the @c std::string representing the MathML "less than or
     * equal to" operator.
     *
     * Return the @c std::string representing the MathML "less than or equal
     * to" operator.
     *
     * @return The @c std::string representing the MathML "less than or equal
     * to" operator.
     */
    std::string leqString() const;

    /**
     * @brief Set the @c std::string representing the MathML "less than or
     * equal to" operator.
     *
     * Set the @c std::string representing the MathML "less than or equal to"
     * operator.
     *
     * @param leqString The @c std::string representing the MathML "less than or
     * equal to" operator.
     */
    void setLeqString(const std::string &leqString);

    /**
     * @brief Get the @c std::string representing the MathML "greater than"
     * operator.
     *
     * Return the @c std::string representing the MathML "greater than"
     * operator.
     *
     * @return The @c std::string representing the MathML "greater than"
     * operator.
     */
    std::string gtString() const;

    /**
     * @brief Set the @c std::string representing the MathML "greater than"
     * operator.
     *
     * Set the @c std::string representing the MathML "greater than" operator.
     *
     * @param gtString The @c std::string representing the MathML "greater than"
     * operator.
     */
    void setGtString(const std::string &gtString);

    /**
     * @brief Get the @c std::string representing the MathML "greater than or
     * equal to" operator.
     *
     * Return the @c std::string representing the MathML "greater than or equal
     * to" operator.
     *
     * @return The @c std::string representing the MathML "greater than or
     * equal to" operator.
     */
    std::string geqString() const;

    /**
     * @brief Set the @c std::string representing the MathML "greater than or
     * equal to" operator.
     *
     * Set the @c std::string representing the MathML "greater than or equal to"
     * operator.
     *
     * @param geqString The @c std::string representing the MathML "greater than
     * or equal to" operator.
     */
    void setGeqString(const std::string &geqString);

    /**
     * @brief Get the @c std::string representing the MathML "and" operator.
     *
     * Return the @c std::string representing the MathML "and" operator.
     *
     * @return The @c std::string representing the MathML "and" operator.
     */
    std::string andString() const;

    /**
     * @brief Set the @c std::string representing the MathML "and" operator.
     *
     * Set the @c std::string representing the MathML "and" operator.
     *
     * @param andString The @c std::string representing the MathML "and"
     * operator.
     */
    void setAndString(const std::string &andString);

    /**
     * @brief Get the @c std::string representing the MathML "or" operator.
     *
     * Return the @c std::string representing the MathML "or" operator.
     *
     * @return The @c std::string representing the MathML "or" operator.
     */
    std::string orString() const;

    /**
     * @brief Set the @c std::string representing the MathML "or" operator.
     *
     * Set the @c std::string representing the MathML "or" operator.
     *
     * @param orString The @c std::string representing the MathML "or" operator.
     */
    void setOrString(const std::string &orString);

    /**
     * @brief Get the @c std::string representing the MathML "exclusive or"
     * operator.
     *
     * Return the @c std::string representing the MathML "exclusive or"
     * operator.
     *
     * @return The @c std::string representing the MathML "exclusive or"
     * operator.
     */
    std::string xorString() const;

    /**
     * @brief Set the @c std::string representing the MathML "exclusive or"
     * operator.
     *
     * Set the @c std::string representing the MathML "exclusive or" operator.
     *
     * @param xorString The @c std::string representing the MathML "exclusive
     * or" operator.
     */
    void setXorString(const std::string &xorString);

    /**
     * @brief Get the @c std::string representing the MathML "not" operator.
     *
     * Return the @c std::string representing the MathML "not" operator.
     *
     * @return The @c std::string representing the MathML "not" operator.
     */
    std::string notString() const;

    /**
     * @brief Set the @c std::string representing the MathML "not" operator.
     *
     * Set the @c std::string representing the MathML "not" operator.
     *
     * @param notString The @c std::string representing the MathML "not"
     * operator.
     */
    void setNotString(const std::string &notString);

    /**
     * @brief Test if this @ref GeneratorProfile has an "equal to" operator.
     *
     * Test if this @ref GeneratorProfile has an "equal to" operator.
     *
     * @return @c true if the @ref GeneratorProfile has an "equal to" operator,
     * @c false otherwise.
     */
    bool hasEqOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has an "equal to" operator.
     *
     * Set whether this @ref GeneratorProfile has an "equal to" operator.
     *
     * @param hasEqOperator A @c bool to determine whether this
     * @ref GeneratorProfile has an "equal to" operator.
     */
    void setHasEqOperator(bool hasEqOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "not equal to" operator.
     *
     * Test if this @ref GeneratorProfile has a "not equal to" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "not equal to"
     * operator, @c false otherwise.
     */
    bool hasNeqOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "not equal to"
     * operator.
     *
     * Set whether this @ref GeneratorProfile has a "not equal to" operator.
     *
     * @param hasNeqOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "not equal to" operator.
     */
    void setHasNeqOperator(bool hasNeqOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "less than" operator.
     *
     * Test if this @ref GeneratorProfile has a "less than" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "less than" operator,
     * @c false otherwise.
     */
    bool hasLtOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "less than" operator.
     *
     * Set whether this @ref GeneratorProfile has a "less than" operator.
     *
     * @param hasLtOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "less than" operator.
     */
    void setHasLtOperator(bool hasLtOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "less than or equal to"
     * operator.
     *
     * Test if this @ref GeneratorProfile has a "less than or equal to" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "less than or equal to"
     * operator,
     * @c false otherwise.
     */
    bool hasLeqOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "less than or equal to"
     *  operator.
     *
     * Set whether this @ref GeneratorProfile has a "less than or equal to"
     * operator.
     *
     * @param hasLeqOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "less than or equal to" operator.
     */
    void setHasLeqOperator(bool hasLeqOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "greater than" operator.
     *
     * Test if this @ref GeneratorProfile has a "greater than" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "greater than" operator,
     * @c false otherwise.
     */
    bool hasGtOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "greater than"
     * operator.
     *
     * Set whether this @ref GeneratorProfile has a "greater than" operator.
     *
     * @param hasGtOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "greater than" operator.
     */
    void setHasGtOperator(bool hasGtOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "greater than or equal to"
     * operator.
     *
     * Test if this @ref GeneratorProfile has a "greater than or equal to" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "greater than or equal
     * to" operator,
     * @c false otherwise.
     */
    bool hasGeqOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "greater than or equal
     * to" operator.
     *
     * Set whether this @ref GeneratorProfile has a "greater than or equal to"
     * operator.
     *
     * @param hasGeqOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "greater than or equal to" operator.
     */
    void setHasGeqOperator(bool hasGeqOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has an "and" operator.
     *
     * Test if this @ref GeneratorProfile has an "and" operator.
     *
     * @return @c true if the @ref GeneratorProfile has an "and" operator,
     * @c false otherwise.
     */
    bool hasAndOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has an "and" operator.
     *
     * Set whether this @ref GeneratorProfile has an "and" operator.
     *
     * @param hasAndOperator A @c bool to determine whether this
     * @ref GeneratorProfile has an "and" operator.
     */
    void setHasAndOperator(bool hasAndOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has an "or" operator.
     *
     * Test if this @ref GeneratorProfile has an "or" operator.
     *
     * @return @c true if the @ref GeneratorProfile has an "or" operator,
     * @c false otherwise.
     */
    bool hasOrOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has an "or" operator.
     *
     * Set whether this @ref GeneratorProfile has an "or" operator.
     *
     * @param hasOrOperator A @c bool to determine whether this
     * @ref GeneratorProfile has an "or" operator.
     */
    void setHasOrOperator(bool hasOrOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "exclusive or" operator.
     *
     * Test if this @ref GeneratorProfile has a "exclusive or" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "exclusive or" operator,
     * @c false otherwise.
     */
    bool hasXorOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "exclusive or"
     * operator.
     *
     * Set whether this @ref GeneratorProfile has a "exclusive or" operator.
     *
     * @param hasXorOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "exclusive or" operator.
     */
    void setHasXorOperator(bool hasXorOperator);

    /**
     * @brief Test if this @ref GeneratorProfile has a "not" operator.
     *
     * Test if this @ref GeneratorProfile has a "not" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "not" operator,
     * @c false otherwise.
     */
    bool hasNotOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "not" operator.
     *
     * Set whether this @ref GeneratorProfile has a "not" operator.
     *
     * @param hasNotOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "not" operator.
     */
    void setHasNotOperator(bool hasNotOperator);

    // Arithmetic operators.

    /**
     * @brief Get the @c std::string representing the MathML "plus" operator.
     *
     * Return the @c std::string representing the MathML "plus" operator.
     *
     * @return The @c std::string representing the MathML "plus" operator.
     */
    std::string plusString() const;

    /**
     * @brief Set the @c std::string representing the MathML "plus" operator.
     *
     * Set the @c std::string representing the MathML "plus" operator.
     *
     * @param plusString The @c std::string representing the MathML "plus"
     * operator.
     */
    void setPlusString(const std::string &plusString);

    /**
     * @brief Get the @c std::string representing the MathML "minus" operator.
     *
     * Return the @c std::string representing the MathML "minus" operator.
     *
     * @return The @c std::string representing the MathML "minus" operator.
     */
    std::string minusString() const;

    /**
     * @brief Set the @c std::string representing the MathML "minus" operator.
     *
     * Set the @c std::string representing the MathML "minus" operator.
     *
     * @param minusString The @c std::string representing the MathML "minus"
     * operator.
     */
    void setMinusString(const std::string &minusString);

    /**
     * @brief Get the @c std::string representing the MathML "times" operator.
     *
     * Return the @c std::string representing the MathML "times" operator.
     *
     * @return The @c std::string representing the MathML "times" operator.
     */
    std::string timesString() const;

    /**
     * @brief Set the @c std::string representing the MathML "times" operator.
     *
     * Set the @c std::string representing the MathML "times" operator.
     *
     * @param timesString The @c std::string representing the MathML "times"
     * operator.
     */
    void setTimesString(const std::string &timesString);

    /**
     * @brief Get the @c std::string representing the MathML "divide" operator.
     *
     * Return the @c std::string representing the MathML "divide" operator.
     *
     * @return The @c std::string representing the MathML "divide" operator.
     */
    std::string divideString() const;

    /**
     * @brief Set the @c std::string representing the MathML "divide" operator.
     *
     * Set the @c std::string representing the MathML "divide" operator.
     *
     * @param divideString The @c std::string representing the MathML "divide"
     * operator.
     */
    void setDivideString(const std::string &divideString);

    /**
     * @brief Get the @c std::string representing the MathML "power" operator or
     * function.
     *
     * Return the @c std::string representing the MathML "power" operator or
     * function.
     *
     * @return The @c std::string representing the MathML "power" operator or
     * function.
     */
    std::string powerString() const;

    /**
     * @brief Set the @c std::string representing the MathML "power" operator or
     * function.
     *
     * Set the @c std::string representing the MathML "power" operator or
     * function.
     *
     * @param powerString The @c std::string representing the MathML "power"
     * operator or function.
     */
    void setPowerString(const std::string &powerString);

    /**
     * @brief Get the @c std::string representing the MathML "square root"
     * function.
     *
     * Return the @c std::string representing the MathML "square root" function.
     *
     * @return The @c std::string representing the MathML "square root"
     * function.
     */
    std::string squareRootString() const;

    /**
     * @brief Set the @c std::string representing the MathML "square root"
     * function.
     *
     * Set the @c std::string representing the MathML "square root" function.
     *
     * @param squareRootString The @c std::string representing the MathML
     * "square root" function.
     */
    void setSquareRootString(const std::string &squareRootString);

    /**
     * @brief Get the @c std::string representing the MathML "square" function.
     *
     * Return the @c std::string representing the MathML "square" function.
     *
     * @return The @c std::string representing the MathML "square" function.
     */
    std::string squareString() const;

    /**
     * @brief Set the @c std::string representing the MathML "square" function.
     *
     * Set the @c std::string representing the MathML "square" function.
     *
     * @param squareString The @c std::string representing the MathML "square"
     * function.
     */
    void setSquareString(const std::string &squareString);

    /**
     * @brief Get the @c std::string representing the MathML "absolute value"
     * function.
     *
     * Return the @c std::string representing the MathML "absolute value"
     * function.
     *
     * @return The @c std::string representing the MathML "absolute value"
     * function.
     */
    std::string absoluteValueString() const;

    /**
     * @brief Set the @c std::string representing the MathML "absolute value"
     * function.
     *
     * Set the @c std::string representing the MathML "absolute value" function.
     *
     * @param absoluteValueString The @c std::string representing the MathML
     * "absolute value" function.
     */
    void setAbsoluteValueString(const std::string &absoluteValueString);

    /**
     * @brief Get the @c std::string representing the MathML "exponential"
     * function.
     *
     * Return the @c std::string representing the MathML "exponential"
     * function.
     *
     * @return The @c std::string representing the MathML "exponential"
     * function.
     */
    std::string exponentialString() const;

    /**
     * @brief Set the @c std::string representing the MathML "exponential"
     * function.
     *
     * Set the @c std::string representing the MathML "exponential" function.
     *
     * @param exponentialString The @c std::string representing the MathML
     * "exponential" function.
     */
    void setExponentialString(const std::string &exponentialString);

    /**
     * @brief Get the @c std::string representing the MathML "natural logarithm"
     * function.
     *
     * Return the @c std::string representing the MathML "natural logarithm"
     * function.
     *
     * @return The @c std::string representing the MathML "natural logarithm"
     * function.
     */
    std::string naturalLogarithmString() const;

    /**
     * @brief Set the @c std::string representing the MathML "natural logarithm"
     * function.
     *
     * Set the @c std::string representing the MathML "natural logarithm"
     * function.
     *
     * @param naturalLogarithmString The @c std::string representing the MathML
     * "natural logarithm" function.
     */
    void setNaturalLogarithmString(const std::string &naturalLogarithmString);

    /**
     * @brief Get the @c std::string representing the MathML "common logarithm"
     * function.
     *
     * Return the @c std::string representing the MathML "common logarithm"
     * function.
     *
     * @return The @c std::string representing the MathML "common logarithm"
     * function.
     */
    std::string commonLogarithmString() const;

    /**
     * @brief Set the @c std::string representing the MathML "common logarithm"
     * function.
     *
     * Set the @c std::string representing the MathML "common logarithm"
     * function.
     *
     * @param commonLogarithmString The @c std::string representing the MathML
     * "common logarithm" function.
     */
    void setCommonLogarithmString(const std::string &commonLogarithmString);

    /**
     * @brief Get the @c std::string representing the MathML "ceiling" function.
     *
     * Return the @c std::string representing the MathML "ceiling" function.
     *
     * @return The @c std::string representing the MathML "ceiling" function.
     */
    std::string ceilingString() const;

    /**
     * @brief Set the @c std::string representing the MathML "ceiling" function.
     *
     * Set the @c std::string representing the MathML "ceiling" function.
     *
     * @param ceilingString The @c std::string representing the MathML "ceiling"
     * function.
     */
    void setCeilingString(const std::string &ceilingString);

    /**
     * @brief Get the @c std::string representing the MathML "floor" function.
     *
     * Return the @c std::string representing the MathML "floor" function.
     *
     * @return The @c std::string representing the MathML "floor" function.
     */
    std::string floorString() const;

    /**
     * @brief Set the @c std::string representing the MathML "floor" function.
     *
     * Set the @c std::string representing the MathML "floor" function.
     *
     * @param floorString The @c std::string representing the MathML "floor"
     * function.
     */
    void setFloorString(const std::string &floorString);

    /**
     * @brief Get the @c std::string representing the MathML "minimum" function.
     *
     * Return the @c std::string representing the MathML "minimum" function.
     *
     * @return The @c std::string representing the MathML "minimum" function.
     */
    std::string minString() const;

    /**
     * @brief Set the @c std::string representing the MathML "minimum" function.
     *
     * Set the @c std::string representing the MathML "minimum" function.
     *
     * @param minString The @c std::string representing the MathML "minimum"
     * function.
     */
    void setMinString(const std::string &minString);

    /**
     * @brief Get the @c std::string representing the MathML "maximum" function.
     *
     * Return the @c std::string representing the MathML "maximum" function.
     *
     * @return The @c std::string representing the MathML "maximum" function.
     */
    std::string maxString() const;

    /**
     * @brief Set the @c std::string representing the MathML "maximum" function.
     *
     * Set the @c std::string representing the MathML "maximum" function.
     *
     * @param maxString The @c std::string representing the MathML "maximum"
     * function.
     */
    void setMaxString(const std::string &maxString);

    /**
     * @brief Get the @c std::string representing the MathML "remainder"
     * function.
     *
     * Return the @c std::string representing the MathML "remainder" function.
     *
     * @return The @c std::string representing the MathML "remainder" function.
     */
    std::string remString() const;

    /**
     * @brief Set the @c std::string representing the MathML "remainder"
     * function.
     *
     * Set the @c std::string representing the MathML "remainder" function.
     *
     * @param remString The @c std::string representing the MathML "remainder"
     * function.
     */
    void setRemString(const std::string &remString);

    /**
     * @brief Test if this @ref GeneratorProfile has a "power" operator.
     *
     * Test if this @ref GeneratorProfile has a "power" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "power" operator,
     * @c false otherwise.
     */
    bool hasPowerOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "power" operator.
     *
     * Set whether this @ref GeneratorProfile has a "power" operator.
     *
     * @param hasPowerOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "power" operator.
     */
    void setHasPowerOperator(bool hasPowerOperator);

    // Trigonometric operators.

    /**
     * @brief Get the @c std::string representing the MathML "sine" function.
     *
     * Return the @c std::string representing the MathML "sine" function.
     *
     * @return The @c std::string representing the MathML "sine" function.
     */
    std::string sinString() const;

    /**
     * @brief Set the @c std::string representing the MathML "sine" function.
     *
     * Set the @c std::string representing the MathML "sine" function.
     *
     * @param sinString The @c std::string representing the MathML "sine"
     * function.
     */
    void setSinString(const std::string &sinString);

    /**
     * @brief Get the @c std::string representing the MathML "cosine" function.
     *
     * Return the @c std::string representing the MathML "cosine" function.
     *
     * @return The @c std::string representing the MathML "cosine" function.
     */
    std::string cosString() const;

    /**
     * @brief Set the @c std::string representing the MathML "cosine" function.
     *
     * Set the @c std::string representing the MathML "cosine" function.
     *
     * @param cosString The @c std::string representing the MathML "cosine"
     * function.
     */
    void setCosString(const std::string &cosString);

    /**
     * @brief Get the @c std::string representing the MathML "tangent" function.
     *
     * Return the @c std::string representing the MathML "tangent" function.
     *
     * @return The @c std::string representing the MathML "tangent" function.
     */
    std::string tanString() const;

    /**
     * @brief Set the @c std::string representing the MathML "tangent" function.
     *
     * Set the @c std::string representing the MathML "tangent" function.
     *
     * @param tanString The @c std::string representing the MathML "tangent"
     * function.
     */
    void setTanString(const std::string &tanString);

    /**
     * @brief Get the @c std::string representing the MathML "secant" function.
     *
     * Return the @c std::string representing the MathML "secant" function.
     *
     * @return The @c std::string representing the MathML "secant" function.
     */
    std::string secString() const;

    /**
     * @brief Set the @c std::string representing the MathML "secant" function.
     *
     * Set the @c std::string representing the MathML "secant" function.
     *
     * @param secString The @c std::string representing the MathML "secant"
     * function.
     */
    void setSecString(const std::string &secString);

    /**
     * @brief Get the @c std::string representing the MathML "cosecant"
     * function.
     *
     * Return the @c std::string representing the MathML "cosecant" function.
     *
     * @return The @c std::string representing the MathML "cosecant" function.
     */
    std::string cscString() const;

    /**
     * @brief Set the @c std::string representing the MathML "cosecant"
     * function.
     *
     * Set the @c std::string representing the MathML "cosecant" function.
     *
     * @param cscString The @c std::string representing the MathML "cosecant"
     * function.
     */
    void setCscString(const std::string &cscString);

    /**
     * @brief Get the @c std::string representing the MathML "cotangent"
     * function.
     *
     * Return the @c std::string representing the MathML "cotangent" function.
     *
     * @return The @c std::string representing the MathML "cotangent" function.
     */
    std::string cotString() const;

    /**
     * @brief Set the @c std::string representing the MathML "cotangent"
     * function.
     *
     * Set the @c std::string representing the MathML "cotangent" function.
     *
     * @param cotString The @c std::string representing the MathML "cotangent"
     * function.
     */
    void setCotString(const std::string &cotString);

    /**
     * @brief Get the @c std::string representing the MathML "hyperbolic sine"
     * function.
     *
     * Return the @c std::string representing the MathML "hyperbolic sine"
     * function.
     *
     * @return The @c std::string representing the MathML "hyperbolic sine"
     * function.
     */
    std::string sinhString() const;

    /**
     * @brief Set the @c std::string representing the MathML "hyperbolic sine"
     * function.
     *
     * Set the @c std::string representing the MathML "hyperbolic sine"
     * function.
     *
     * @param sinhString The @c std::string representing the MathML "hyperbolic
     * sine" function.
     */
    void setSinhString(const std::string &sinhString);

    /**
     * @brief Get the @c std::string representing the MathML "hyperbolic cosine"
     * function.
     *
     * Return the @c std::string representing the MathML "hyperbolic cosine"
     * function.
     *
     * @return The @c std::string representing the MathML "hyperbolic cosine"
     * function.
     */
    std::string coshString() const;

    /**
     * @brief Set the @c std::string representing the MathML "hyperbolic cosine"
     * function.
     *
     * Set the @c std::string representing the MathML "hyperbolic cosine"
     * function.
     *
     * @param coshString The @c std::string representing the MathML "hyperbolic
     * cosine" function.
     */
    void setCoshString(const std::string &coshString);

    /**
     * @brief Get the @c std::string representing the MathML "hyperbolic
     * tangent" function.
     *
     * Return the @c std::string representing the MathML "hyperbolic tangent"
     * function.
     *
     * @return The @c std::string representing the MathML "hyperbolic tangent"
     * function.
     */
    std::string tanhString() const;

    /**
     * @brief Set the @c std::string representing the MathML "hyperbolic
     * tangent" function.
     *
     * Set the @c std::string representing the MathML "hyperbolic tangent"
     * function.
     *
     * @param tanhString The @c std::string representing the MathML "hyperbolic
     * tangent" function.
     */
    void setTanhString(const std::string &tanhString);

    /**
     * @brief Get the @c std::string representing the MathML "hyperbolic secant"
     * function.
     *
     * Return the @c std::string representing the MathML "hyperbolic secant"
     * function.
     *
     * @return The @c std::string representing the MathML "hyperbolic secant"
     * function.
     */
    std::string sechString() const;

    /**
     * @brief Set the @c std::string representing the MathML "hyperbolic secant"
     * function.
     *
     * Set the @c std::string representing the MathML "hyperbolic secant"
     * function.
     *
     * @param sechString The @c std::string representing the MathML "hyperbolic
     * secant" function.
     */
    void setSechString(const std::string &sechString);

    /**
     * @brief Get the @c std::string representing the MathML "hyperbolic
     * cosecant" function.
     *
     * Return the @c std::string representing the MathML "hyperbolic cosecant"
     * function.
     *
     * @return The @c std::string representing the MathML "hyperbolic cosecant"
     * function.
     */
    std::string cschString() const;

    /**
     * @brief Set the @c std::string representing the MathML "hyperbolic
     * cosecant" function.
     *
     * Set the @c std::string representing the MathML "hyperbolic cosecant"
     * function.
     *
     * @param cschString The @c std::string representing the MathML "hyperbolic
     * cosecant" function.
     */
    void setCschString(const std::string &cschString);

    /**
     * @brief Get the @c std::string representing the MathML "hyperbolic
     * cotangent" function.
     *
     * Return the @c std::string representing the MathML "hyperbolic cotangent"
     * function.
     *
     * @return The @c std::string representing the MathML "hyperbolic cotangent"
     * function.
     */
    std::string cothString() const;

    /**
     * @brief Set the @c std::string representing the MathML "hyperbolic
     * cotangent" function.
     *
     * Set the @c std::string representing the MathML "hyperbolic cotangent"
     * function.
     *
     * @param cothString The @c std::string representing the MathML "hyperbolic
     * cotangent" function.
     */
    void setCothString(const std::string &cothString);

    /**
     * @brief Get the @c std::string representing the MathML "arc sine"
     * function.
     *
     * Return the @c std::string representing the MathML "arc sine" function.
     *
     * @return The @c std::string representing the MathML "arc sine" function.
     */
    std::string asinString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc sine"
     * function.
     *
     * Set the @c std::string representing the MathML "arc sine" function.
     *
     * @param asinString The @c std::string representing the MathML "arc sine"
     * function.
     */
    void setAsinString(const std::string &asinString);

    /**
     * @brief Get the @c std::string representing the MathML "arc cosine"
     * function.
     *
     * Return the @c std::string representing the MathML "arc cosine" function.
     *
     * @return The @c std::string representing the MathML "arc cosine" function.
     */
    std::string acosString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc cosine"
     * function.
     *
     * Set the @c std::string representing the MathML "arc cosine" function.
     *
     * @param acosString The @c std::string representing the MathML "arc cosine"
     * function.
     */
    void setAcosString(const std::string &acosString);

    /**
     * @brief Get the @c std::string representing the MathML "arc tangent"
     * function.
     *
     * Return the @c std::string representing the MathML "arc tangent" function.
     *
     * @return The @c std::string representing the MathML "arc tangent"
     * function.
     */
    std::string atanString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc tangent"
     * function.
     *
     * Set the @c std::string representing the MathML "arc tangent" function.
     *
     * @param atanString The @c std::string representing the MathML "arc
     * tangent" function.
     */
    void setAtanString(const std::string &atanString);

    /**
     * @brief Get the @c std::string representing the MathML "arc secant"
     * function.
     *
     * Return the @c std::string representing the MathML "arc secant" function.
     *
     * @return The @c std::string representing the MathML "arc secant" function.
     */
    std::string asecString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc secant"
     * function.
     *
     * Set the @c std::string representing the MathML "arc secant" function.
     *
     * @param asecString The @c std::string representing the MathML "arc secant"
     * function.
     */
    void setAsecString(const std::string &asecString);

    /**
     * @brief Get the @c std::string representing the MathML "arc cosecant"
     * function.
     *
     * Return the @c std::string representing the MathML "arc cosecant"
     * function.
     *
     * @return The @c std::string representing the MathML "arc cosecant"
     * function.
     */
    std::string acscString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc cosecant"
     * function.
     *
     * Set the @c std::string representing the MathML "arc cosecant" function.
     *
     * @param acscString The @c std::string representing the MathML "arc
     * cosecant" function.
     */
    void setAcscString(const std::string &acscString);

    /**
     * @brief Get the @c std::string representing the MathML "arc cotangent"
     * function.
     *
     * Return the @c std::string representing the MathML "arc cotangent"
     * function.
     *
     * @return The @c std::string representing the MathML "arc cotangent"
     * function.
     */
    std::string acotString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc cotangent"
     * function.
     *
     * Set the @c std::string representing the MathML "arc cotangent" function.
     *
     * @param acotString The @c std::string representing the MathML "arc
     * cotangent" function.
     */
    void setAcotString(const std::string &acotString);

    /**
     * @brief Get the @c std::string representing the MathML "arc hyperbolic
     * sine" function.
     *
     * Return the @c std::string representing the MathML "arc hyperbolic sine"
     * function.
     *
     * @return The @c std::string representing the MathML "arc hyperbolic sine"
     * function.
     */
    std::string asinhString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc hyperbolic
     * sine" function.
     *
     * Set the @c std::string representing the MathML "arc hyperbolic sine"
     * function.
     *
     * @param asinhString The @c std::string representing the MathML "arc
     * hyperbolic sine" function.
     */
    void setAsinhString(const std::string &asinhString);

    /**
     * @brief Get the @c std::string representing the MathML "arc hyperbolic
     * cosine" function.
     *
     * Return the @c std::string representing the MathML "arc hyperbolic cosine"
     * function.
     *
     * @return The @c std::string representing the MathML "arc hyperbolic
     * cosine" function.
     */
    std::string acoshString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc hyperbolic
     * cosine" function.
     *
     * Set the @c std::string representing the MathML "arc hyperbolic cosine"
     * function.
     *
     * @param acoshString The @c std::string representing the MathML "arc
     * hyperbolic cosine" function.
     */
    void setAcoshString(const std::string &acoshString);

    /**
     * @brief Get the @c std::string representing the MathML "arc hyperbolic
     * tangent" function.
     *
     * Return the @c std::string representing the MathML "arc hyperbolic
     * tangent" function.
     *
     * @return The @c std::string representing the MathML "arc hyperbolic
     * tangent" function.
     */
    std::string atanhString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc hyperbolic
     * tangent" function.
     *
     * Set the @c std::string representing the MathML "arc hyperbolic tangent"
     * function.
     *
     * @param atanhString The @c std::string representing the MathML "arc
     * hyperbolic tangent" function.
     */
    void setAtanhString(const std::string &atanhString);

    /**
     * @brief Get the @c std::string representing the MathML "arc hyperbolic
     * secant" function.
     *
     * Return the @c std::string representing the MathML "arc hyperbolic
     * secant" function.
     *
     * @return The @c std::string representing the MathML "arc hyperbolic
     * secant" function.
     */
    std::string asechString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc hyperbolic
     * secant" function.
     *
     * Set the @c std::string representing the MathML "arc hyperbolic secant"
     * function.
     *
     * @param asechString The @c std::string representing the MathML "arc
     * hyperbolic secant" function.
     */
    void setAsechString(const std::string &asechString);

    /**
     * @brief Get the @c std::string representing the MathML "arc hyperbolic
     * cosecant" function.
     *
     * Return the @c std::string representing the MathML "arc hyperbolic
     * cosecant" function.
     *
     * @return The @c std::string representing the MathML "arc hyperbolic
     * cosecant" function.
     */
    std::string acschString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc hyperbolic
     * cosecant" function.
     *
     * Set the @c std::string representing the MathML "arc hyperbolic cosecant"
     * function.
     *
     * @param acschString The @c std::string representing the MathML "arc
     * hyperbolic cosecant" function.
     */
    void setAcschString(const std::string &acschString);

    /**
     * @brief Get the @c std::string representing the MathML "arc hyperbolic
     * cotangent" function.
     *
     * Return the @c std::string representing the MathML "arc hyperbolic
     * cotangent" function.
     *
     * @return The @c std::string representing the MathML "arc hyperbolic
     * cotangent" function.
     */
    std::string acothString() const;

    /**
     * @brief Set the @c std::string representing the MathML "arc hyperbolic
     * cotangent" function.
     *
     * Set the @c std::string representing the MathML "arc hyperbolic cotangent"
     * function.
     *
     * @param acothString The @c std::string representing the MathML "arc
     * hyperbolic cotangent" function.
     */
    void setAcothString(const std::string &acothString);

    // Piecewise statement.

    /**
     * @brief Get the @c std::string representing the MathML "if" part of a
     * "conditional" statement or operator.
     *
     * Return the @c std::string representing the MathML "if" part of a
     * "conditional" statement or operator.
     *
     * @return The @c std::string representing the MathML "if" part of a
     * "conditional" statement or operator.
     */
    std::string conditionalOperatorIfString() const;

    /**
     * @brief Set the @c std::string representing the MathML "if" part of a
     * "conditional" statement or operator.
     *
     * Set the @c std::string representing the MathML "if" part of a
     * "conditional" statement or operator.
     *
     * @param conditionalOperatorIfString The @c std::string representing the
     * MathML "if" part of a "conditional" statement or operator.
     */
    void setConditionalOperatorIfString(const std::string &conditionalOperatorIfString);

    /**
     * @brief Get the @c std::string representing the MathML "else" part of a
     * "conditional" statement or operator.
     *
     * Return the @c std::string representing the MathML "else" part of a
     * "conditional" statement or operator.
     *
     * @return The @c std::string representing the MathML "else" part of a
     * "conditional" statement or operator.
     */
    std::string conditionalOperatorElseString() const;

    /**
     * @brief Set the @c std::string representing the MathML "else" part of a
     * "conditional" statement or operator.
     *
     * Set the @c std::string representing the MathML "else" part of a
     * "conditional" statement or operator.
     *
     * @param conditionalOperatorElseString The @c std::string representing the
     * MathML "else" part of a "conditional" statement or operator.
     */
    void setConditionalOperatorElseString(const std::string &conditionalOperatorElseString);

    /**
     * @brief Get the @c std::string representing the MathML "if" part of a
     * "piecewise" statement.
     *
     * Return the @c std::string representing the MathML "if" part of a
     * "piecewise" statement.
     *
     * @return The @c std::string representing the MathML "if" part of a
     * "piecewise" statement.
     */
    std::string piecewiseIfString() const;

    /**
     * @brief Set the @c std::string representing the MathML "if" part of a
     * "piecewise" statement.
     *
     * Set the @c std::string representing the MathML "if" part of a "piecewise"
     * statement.
     *
     * @param piecewiseIfString The @c std::string representing the MathML "if"
     * part of a "piecewise" statement.
     */
    void setPiecewiseIfString(const std::string &piecewiseIfString);

    /**
     * @brief Get the @c std::string representing the MathML "else" part of a
     * "piecewise" statement.
     *
     * Return the @c std::string representing the MathML "else" part of a
     * "piecewise" statement.
     *
     * @return The @c std::string representing the MathML "else" part of a
     * "piecewise" statement.
     */
    std::string piecewiseElseString() const;

    /**
     * @brief Set the @c std::string representing the MathML "else" part of a
     * "piecewise" statement.
     *
     * Set the @c std::string representing the MathML "else" part of a
     * "piecewise" statement.
     *
     * @param piecewiseElseString The @c std::string representing the MathML
     * "else" part of a "piecewise" statement.
     */
    void setPiecewiseElseString(const std::string &piecewiseElseString);

    /**
     * @brief Test if this @ref GeneratorProfile has a "conditional" operator.
     *
     * Test if this @ref GeneratorProfile has a "conditional" operator.
     *
     * @return @c true if the @ref GeneratorProfile has a "conditional" operator,
     * @c false otherwise.
     */
    bool hasConditionalOperator() const;

    /**
     * @brief Set whether this @ref GeneratorProfile has a "conditional" operator.
     *
     * Set whether this @ref GeneratorProfile has a "conditional" operator.
     *
     * @param hasConditionalOperator A @c bool to determine whether this
     * @ref GeneratorProfile has a "conditional" operator.
     */
    void setHasConditionalOperator(bool hasConditionalOperator);

    // Constants.

    /**
     * @brief Get the @c std::string representing the MathML "true" boolean.
     *
     * Return the @c std::string representing the MathML "true" boolean.
     *
     * @return The @c std::string representing the MathML "true" boolean.
     */
    std::string trueString() const;

    /**
     * @brief Set the @c std::string representing the MathML "true" boolean.
     *
     * Set the @c std::string representing the MathML "true" boolean.
     *
     * @param trueString The @c std::string representing the MathML "true"
     * boolean.
     */
    void setTrueString(const std::string &trueString);

    /**
     * @brief Get the @c std::string representing the MathML "false" boolean.
     *
     * Return the @c std::string representing the MathML "false" boolean.
     *
     * @return The @c std::string representing the MathML "false" boolean.
     */
    std::string falseString() const;

    /**
     * @brief Set the @c std::string representing the MathML "false" boolean.
     *
     * Set the @c std::string representing the MathML "false" boolean.
     *
     * @param falseString The @c std::string representing the MathML "false"
     * boolean.
     */
    void setFalseString(const std::string &falseString);

    /**
     * @brief Get the @c std::string representing the MathML "Euler's number".
     *
     * Return the @c std::string representing the MathML "Euler's number".
     *
     * @return The @c std::string representing the MathML "Euler's number".
     */
    std::string eString() const;

    /**
     * @brief Set the @c std::string representing the MathML "Euler's number".
     *
     * Set the @c std::string representing the MathML "Euler's number".
     *
     * @param eString The @c std::string representing the MathML "Euler's
     * number".
     */
    void setEString(const std::string &eString);

    /**
     * @brief Get the @c std::string representing the MathML "π" constant.
     *
     * Return the @c std::string representing the MathML "π" constant.
     *
     * @return The @c std::string representing the MathML "π" constant.
     */
    std::string piString() const;

    /**
     * @brief Set the @c std::string representing the MathML "π" constant.
     *
     * Set the @c std::string representing the MathML "π" constant.
     *
     * @param piString The @c std::string representing the MathML "π" constant.
     */
    void setPiString(const std::string &piString);

    /**
     * @brief Get the @c std::string representing the MathML "infinity" value.
     *
     * Return the @c std::string representing the MathML "infinity" value.
     *
     * @return The @c std::string representing the MathML "infinity" value.
     */
    std::string infString() const;

    /**
     * @brief Set the @c std::string representing the MathML "infinity" value.
     *
     * Set the @c std::string representing the MathML "infinity" value.
     *
     * @param infString The @c std::string representing the MathML "infinity"
     * value.
     */
    void setInfString(const std::string &infString);

    /**
     * @brief Get the @c std::string representing the MathML "not-a-number"
     * value.
     *
     * Return the @c std::string representing the MathML "not-a-number" value.
     *
     * @return The @c std::string representing the MathML "not-a-number" value.
     */
    std::string nanString() const;

    /**
     * @brief Set the @c std::string representing the MathML "not-a-number"
     * value.
     *
     * Set the @c std::string representing the MathML "not-a-number" value.
     *
     * @param nanString The @c std::string representing the MathML
     * "not-a-number" value.
     */
    void setNanString(const std::string &nanString);

    // Arithmetic functions.

    /**
     * @brief Get the @c std::string for the "equal to" function implementation.
     *
     * Return the @c std::string for the "equal to" function implementation.
     *
     * @return The @c std::string for the "equal to" function implementation.
     */
    std::string eqFunctionString() const;

    /**
     * @brief Set the @c std::string for the "equal to" function implementation.
     *
     * Set the @c std::string for the "equal to" function implementation.
     *
     * @param eqFunctionString The @c std::string for the "equal to" function
     * implementation.
     */
    void setEqFunctionString(const std::string &eqFunctionString);

    /**
     * @brief Get the @c std::string for the "not equal to" function
     * implementation.
     *
     * Return the @c std::string for the "not equal to" function implementation.
     *
     * @return The @c std::string for the "not equal to" function
     * implementation.
     */
    std::string neqFunctionString() const;

    /**
     * @brief Set the @c std::string for the "not equal to" function
     * implementation.
     *
     * Set the @c std::string for the "not equal to" function implementation.
     *
     * @param neqFunctionString The @c std::string for the "not
     * equal to" function implementation.
     */
    void setNeqFunctionString(const std::string &neqFunctionString);

    /**
     * @brief Get the @c std::string for the "less than" function
     * implementation.
     *
     * Return the @c std::string for the "less than" function implementation.
     *
     * @return The @c std::string for the "less than" function implementation.
     */
    std::string ltFunctionString() const;

    /**
     * @brief Set the @c std::string for the "less than" function
     * implementation.
     *
     * Set the @c std::string for the "less than" function implementation.
     *
     * @param ltFunctionString The @c std::string for the "less than" function
     * implementation.
     */
    void setLtFunctionString(const std::string &ltFunctionString);

    /**
     * @brief Get the @c std::string for the "less than or equal to" function
     * implementation.
     *
     * Return the @c std::string for the "less than or equal to" function
     * implementation.
     *
     * @return The @c std::string for the "less than or equal to" function
     * implementation.
     */
    std::string leqFunctionString() const;

    /**
     * @brief Set the @c std::string for the "less than or equal to" function
     * implementation.
     *
     * Set the @c std::string for the "less than or equal to" function
     * implementation.
     *
     * @param leqFunctionString The @c std::string for the "less than or equal
     * to" function implementation.
     */
    void setLeqFunctionString(const std::string &leqFunctionString);

    /**
     * @brief Get the @c std::string for the "greater than" function
     * implementation.
     *
     * Return the @c std::string for the "greater than" function implementation.
     *
     * @return The @c std::string for the "greater than" function
     * implementation.
     */
    std::string gtFunctionString() const;

    /**
     * @brief Set the @c std::string for the "greater than" function
     * implementation.
     *
     * Set the @c std::string for the "greater than" function implementation.
     *
     * @param gtFunctionString The @c std::string for the "greater than"
     * function implementation.
     */
    void setGtFunctionString(const std::string &gtFunctionString);

    /**
     * @brief Get the @c std::string for the "greater than or equal to" function
     * implementation.
     *
     * Return the @c std::string for the "greater than or equal to" function
     * implementation.
     *
     * @return The @c std::string for the "greater than or equal to" function
     * implementation.
     */
    std::string geqFunctionString() const;

    /**
     * @brief Set the @c std::string for the "greater than or equal to" function
     * implementation.
     *
     * Set the @c std::string for the "greater than or equal to" function
     * implementation.
     *
     * @param geqFunctionString The @c std::string for the "greater than or
     * equal to" function implementation.
     */
    void setGeqFunctionString(const std::string &geqFunctionString);

    /**
     * @brief Get the @c std::string for the "and" function implementation.
     *
     * Return the @c std::string for the "and" function implementation.
     *
     * @return The @c std::string for the "and" function implementation.
     */
    std::string andFunctionString() const;

    /**
     * @brief Set the @c std::string for the "and" function implementation.
     *
     * Set the @c std::string for the "and" function implementation.
     *
     * @param andFunctionString The @c std::string for the "and"
     * function implementation.
     */
    void setAndFunctionString(const std::string &andFunctionString);

    /**
     * @brief Get the @c std::string for the "or" function implementation.
     *
     * Return the @c std::string for the "or" function implementation.
     *
     * @return The @c std::string for the "or" function implementation.
     */
    std::string orFunctionString() const;

    /**
     * @brief Set the @c std::string for the "or" function implementation.
     *
     * Set the @c std::string for the "or" function implementation.
     *
     * @param orFunctionString The @c std::string for the "or"
     * function implementation.
     */
    void setOrFunctionString(const std::string &orFunctionString);

    /**
     * @brief Get the @c std::string for the "exclusive or" function
     * implementation.
     *
     * Return the @c std::string for the "exclusive or" function implementation.
     *
     * @return The @c std::string for the "exclusive or" function
     * implementation.
     */
    std::string xorFunctionString() const;

    /**
     * @brief Set the @c std::string for the "exclusive or" function
     * implementation.
     *
     * Set the @c std::string for the "exclusive or" function implementation.
     *
     * @param xorFunctionString The @c std::string for the "exclusive or"
     * function implementation.
     */
    void setXorFunctionString(const std::string &xorFunctionString);

    /**
     * @brief Get the @c std::string for the "not" function implementation.
     *
     * Return the @c std::string for the "not" function implementation.
     *
     * @return The @c std::string for the "not" function implementation.
     */
    std::string notFunctionString() const;

    /**
     * @brief Set the @c std::string for the "not" function implementation.
     *
     * Set the @c std::string for the "not" function implementation.
     *
     * @param notFunctionString The @c std::string for the "not" function
     * implementation.
     */
    void setNotFunctionString(const std::string &notFunctionString);

    /**
     * @brief Get the @c std::string for the "minimum" function implementation.
     *
     * Return the @c std::string for the "minimum" function implementation.
     *
     * @return The @c std::string for the "minimum" function implementation.
     */
    std::string minFunctionString() const;

    /**
     * @brief Set the @c std::string for the "minimum" function implementation.
     *
     * Set the @c std::string for the "minimum" function implementation.
     *
     * @param minFunctionString The @c std::string for the "minimum" function
     * implementation.
     */
    void setMinFunctionString(const std::string &minFunctionString);

    /**
     * @brief Get the @c std::string for the "maximum" function implementation.
     *
     * Return the @c std::string for the "maximum" function implementation.
     *
     * @return The @c std::string for the "maximum" function implementation.
     */
    std::string maxFunctionString() const;

    /**
     * @brief Set the @c std::string for the "maximum" function implementation.
     *
     * Set the @c std::string for the "maximum" function implementation.
     *
     * @param maxFunctionString The @c std::string for the "maximum" function
     * implementation.
     */
    void setMaxFunctionString(const std::string &maxFunctionString);

    // Trigonometric function implementations.

    /**
     * @brief Get the @c std::string for the "secant" function implementation.
     *
     * Return the @c std::string for the "secant" function implementation.
     *
     * @return The @c std::string for the "secant" function implementation.
     */
    std::string secFunctionString() const;

    /**
     * @brief Set the @c std::string for the "secant" function implementation.
     *
     * Set the @c std::string for the "secant" function implementation.
     *
     * @param secFunctionString The @c std::string for the "secant" function
     * implementation.
     */
    void setSecFunctionString(const std::string &secFunctionString);

    /**
     * @brief Get the @c std::string for the "cosecant" function implementation.
     *
     * Return the @c std::string for the "cosecant" function implementation.
     *
     * @return The @c std::string for the "cosecant" function implementation.
     */
    std::string cscFunctionString() const;

    /**
     * @brief Set the @c std::string for the "cosecant" function implementation.
     *
     * Set the @c std::string for the "cosecant" function implementation.
     *
     * @param cscFunctionString The @c std::string for the "cosecant" function
     * implementation.
     */
    void setCscFunctionString(const std::string &cscFunctionString);

    /**
     * @brief Get the @c std::string for the "cotangent" function
     * implementation.
     *
     * Return the @c std::string for the "cotangent" function implementation.
     *
     * @return The @c std::string for the "cotangent" function implementation.
     */
    std::string cotFunctionString() const;

    /**
     * @brief Set the @c std::string for the "cotangent" function
     * implementation.
     *
     * Set the @c std::string for the "cotangent" function implementation.
     *
     * @param cotFunctionString The @c std::string for the "cotangent" function
     * implementation.
     */
    void setCotFunctionString(const std::string &cotFunctionString);

    /**
     * @brief Get the @c std::string for the "hyperbolic secant" function
     * implementation.
     *
     * Return the @c std::string for the "hyperbolic secant" function
     * implementation.
     *
     * @return The @c std::string for the "hyperbolic secant" function
     * implementation.
     */
    std::string sechFunctionString() const;

    /**
     * @brief Set the @c std::string for the "hyperbolic secant" function
     * implementation.
     *
     * Set the @c std::string for the "hyperbolic secant" function
     * implementation.
     *
     * @param sechFunctionString The @c std::string for the "hyperbolic secant"
     * function implementation.
     */
    void setSechFunctionString(const std::string &sechFunctionString);

    /**
     * @brief Get the @c std::string for the "hyperbolic cosecant" function
     * implementation.
     *
     * Return the @c std::string for the "hyperbolic cosecant" function
     * implementation.
     *
     * @return The @c std::string for the "hyperbolic cosecant" function
     * implementation.
     */
    std::string cschFunctionString() const;

    /**
     * @brief Set the @c std::string for the "hyperbolic cosecant" function
     * implementation.
     *
     * Set the @c std::string for the "hyperbolic cosecant" function
     * implementation.
     *
     * @param cschFunctionString The @c std::string for the "hyperbolic
     * cosecant" function implementation.
     */
    void setCschFunctionString(const std::string &cschFunctionString);

    /**
     * @brief Get the @c std::string for the "hyperbolic cotangent" function
     * implementation.
     *
     * Return the @c std::string for the "hyperbolic cotangent" function
     * implementation.
     *
     * @return The @c std::string for the "hyperbolic cotangent" function
     * implementation.
     */
    std::string cothFunctionString() const;

    /**
     * @brief Set the @c std::string for the "hyperbolic cotangent" function
     * implementation.
     *
     * Set the @c std::string for the "hyperbolic cotangent" function
     * implementation.
     *
     * @param cothFunctionString The @c std::string for the "hyperbolic
     * cotangent" function implementation.
     */
    void setCothFunctionString(const std::string &cothFunctionString);

    /**
     * @brief Get the @c std::string for the "arc secant" function
     * implementation.
     *
     * Return the @c std::string for the "arc secant" function implementation.
     *
     * @return The @c std::string for the "arc secant" function implementation.
     */
    std::string asecFunctionString() const;

    /**
     * @brief Set the @c std::string for the "arc secant" function
     * implementation.
     *
     * Set the @c std::string for the "arc secant" function implementation.
     *
     * @param asecFunctionString The @c std::string for the "arc secant"
     * function implementation.
     */
    void setAsecFunctionString(const std::string &asecFunctionString);

    /**
     * @brief Get the @c std::string for the "arc cosecant" function
     * implementation.
     *
     * Return the @c std::string for the "arc cosecant" function implementation.
     *
     * @return The @c std::string for the "arc cosecant" function
     * implementation.
     */
    std::string acscFunctionString() const;

    /**
     * @brief Set the @c std::string for the "arc cosecant" function
     * implementation.
     *
     * Set the @c std::string for the "arc cosecant" function implementation.
     *
     * @param acscFunctionString The @c std::string for the "arc cosecant"
     * function implementation.
     */
    void setAcscFunctionString(const std::string &acscFunctionString);

    /**
     * @brief Get the @c std::string for the "arc cotangent" function
     * implementation.
     *
     * Return the @c std::string for the "arc cotangent" function
     * implementation.
     *
     * @return The @c std::string for the "arc cotangent" function
     * implementation.
     */
    std::string acotFunctionString() const;

    /**
     * @brief Set the @c std::string for the "arc cotangent" function implementation.
     *
     * Set the @c std::string for the "arc cotangent" function implementation.
     *
     * @param acotFunctionString The @c std::string for the "arc cotangent"
     * function implementation.
     */
    void setAcotFunctionString(const std::string &acotFunctionString);

    /**
     * @brief Get the @c std::string for the "arc hyperbolic secant" function
     * implementation.
     *
     * Return the @c std::string for the "arc hyperbolic secant" function
     * implementation.
     *
     * @return The @c std::string for the "arc hyperbolic secant" function
     * implementation.
     */
    std::string asechFunctionString() const;

    /**
     * @brief Set the @c std::string for the "arc hyperbolic secant" function
     * implementation.
     *
     * Set the @c std::string for the "arc hyperbolic secant" function
     * implementation.
     *
     * @param asechFunctionString The @c std::string for the "arc hyperbolic
     * secant" function implementation.
     */
    void setAsechFunctionString(const std::string &asechFunctionString);

    /**
     * @brief Get the @c std::string for the "arc hyperbolic cosecant" function
     * implementation.
     *
     * Return the @c std::string for the "arc hyperbolic cosecant" function
     * implementation.
     *
     * @return The @c std::string for the "arc hyperbolic cosecant" function
     * implementation.
     */
    std::string acschFunctionString() const;

    /**
     * @brief Set the @c std::string for the "arc hyperbolic cosecant" function
     * implementation.
     *
     * Set the @c std::string for the "arc hyperbolic cosecant" function
     * implementation.
     *
     * @param acschFunctionString The @c std::string for the "arc hyperbolic
     * cosecant" function implementation.
     */
    void setAcschFunctionString(const std::string &acschFunctionString);

    /**
     * @brief Get the @c std::string for the "arc hyperbolic cotangent" function
     * implementation.
     *
     * Return the @c std::string for the "arc hyperbolic cotangent" function
     * implementation.
     *
     * @return The @c std::string for the "arc hyperbolic cotangent" function
     * implementation.
     */
    std::string acothFunctionString() const;

    /**
     * @brief Set the @c std::string for the "arc hyperbolic cotangent" function
     * implementation.
     *
     * Set the @c std::string for the "arc hyperbolic cotangent" function
     * implementation.
     *
     * @param acothFunctionString The @c std::string for the "arc hyperbolic
     * cotangent" function implementation.
     */
    void setAcothFunctionString(const std::string &acothFunctionString);

    // Miscellaneous.

    /**
     * @brief Get the @c std::string for a comment.
     *
     * Return the @c std::string for a comment.
     *
     * @return The @c std::string for a comment.
     */
    std::string commentString() const;

    /**
     * @brief Set the @c std::string for a comment.
     *
     * Set the @c std::string for a comment. To be useful, the string should
     * contain the [CODE] tag, which will be replaced with a (proper) comment.
     *
     * @param commentString The @c std::string to use for a comment.
     */
    void setCommentString(const std::string &commentString);

    /**
     * @brief Get the @c std::string for an origin comment.
     *
     * Return the @c std::string for an origin comment.
     *
     * @return The @c std::string for an origin comment.
     */
    std::string originCommentString() const;

    /**
     * @brief Set the @c std::string for an origin comment.
     *
     * Set the @c std::string for an origin comment. To be useful, the string
     * should contain the [PROFILE_INFORMATION] and [LIBCELLML_VERSION] tags,
     * which will be replaced with some profile information and the version of
     * libCellML used.
     *
     * @param originCommentString The @c std::string to use for an origin
     * comment.
     */
    void setOriginCommentString(const std::string &originCommentString);

    /**
     * @brief Get the @c std::string for the interface file name.
     *
     * Return the @c std::string for the interface file name.
     *
     * @sa setImplementationHeaderString
     *
     * @return The @c std::string for the interface file name.
     */
    std::string interfaceFileNameString() const;

    /**
     * @brief Set the @c std::string for the interface file name.
     *
     * Set the @c std::string for the interface file name.
     *
     * @param interfaceFileNameString The @c std::string to use the interface
     * file name.
     */
    void setInterfaceFileNameString(const std::string &interfaceFileNameString);

    /**
     * @brief Get the @c std::string for the interface of a header.
     *
     * Return the @c std::string for the interface of a header.
     *
     * @return The @c std::string for the interface of a header.
     */
    std::string interfaceHeaderString() const;

    /**
     * @brief Set the @c std::string for the interface of a header.
     *
     * Set the @c std::string for the interface of a header.
     *
     * @param interfaceHeaderString The @c std::string to use the interface of a
     * header.
     */
    void setInterfaceHeaderString(const std::string &interfaceHeaderString);

    /**
     * @brief Get the @c std::string for an implementation header.
     *
     * Return the @c std::string for an implementation header.
     *
     * @return The @c std::string for an implementation header.
     */
    std::string implementationHeaderString() const;

    /**
     * @brief Set the @c std::string for an implementation header.
     *
     * Set the @c std::string for an implementation header. To be useful, the
     * string should contain the [INTERFACE_FILE_NAME] tag, which will be
     * replaced with the interface file name.
     *
     * @sa interfaceFileNameString
     *
     * @param implementationHeaderString The @c std::string to use for an
     * implementation header.
     */
    void setImplementationHeaderString(const std::string &implementationHeaderString);

    /**
     * @brief Get the @c std::string for the interface of the version constant.
     *
     * Return the @c std::string for the interface of the version constant.
     *
     * @return The @c std::string for the interface of the version constant.
     */
    std::string interfaceVersionString() const;

    /**
     * @brief Set the @c std::string for the interface of the version constant.
     *
     * Set the @c std::string for the interface of the version constant.
     *
     * @param interfaceVersionString The @c std::string to use for the interface
     * of the version constant.
     */
    void setInterfaceVersionString(const std::string &interfaceVersionString);

    /**
     * @brief Get the @c std::string for the implementation of the version
     * constant.
     *
     * Return the @c std::string for the implementation of the version constant.
     *
     * @return The @c std::string for the implementation of the version
     * constant.
     */
    std::string implementationVersionString() const;

    /**
     * @brief Set the @c std::string for the implementation of the version
     * constant.
     *
     * Set the @c std::string for the implementation of the version constant.
     *
     * @param implementationVersionString The @c std::string to use for the
     * implementation of the version constant.
     */
    void setImplementationVersionString(const std::string &implementationVersionString);

    /**
     * @brief Get the @c std::string for the interface of the libCellML version
     * constant.
     *
     * Return the @c std::string for the interface of the libCellML version
     * constant.
     *
     * @return The @c std::string for the interface of the libCellML version
     * constant.
     */
    std::string interfaceLibcellmlVersionString() const;

    /**
     * @brief Set the @c std::string for the interface of the libCellML version
     * constant.
     *
     * Set the @c std::string for the interface of the libCellML version
     * constant.
     *
     * @param interfaceLibcellmlVersionString The @c std::string to use for the
     * interface of the libCellML version constant.
     */
    void setInterfaceLibcellmlVersionString(const std::string &interfaceLibcellmlVersionString);

    /**
     * @brief Get the @c std::string for the implementation of the libCellML
     * version constant.
     *
     * Return the @c std::string for the implementation of the libCellML version
     * constant.
     *
     * @return The @c std::string for the implementation of the libCellML
     * version constant.
     */
    std::string implementationLibcellmlVersionString() const;

    /**
     * @brief Set the @c std::string for the implementation of the libCellML
     * version constant.
     *
     * Set the @c std::string for the implementation of the libCellML version
     * constant. To be useful, the string should contain the [LIBCELLML_VERSION]
     * tag, which will be replaced with the version of libCellML used.
     *
     * @param implementationLibcellmlVersionString The @c std::string to use for
     * the implementation of the libCellML version constant.
     */
    void setImplementationLibcellmlVersionString(const std::string &implementationLibcellmlVersionString);

    /**
     * @brief Get the @c std::string for the interface of the state count
     * constant.
     *
     * Return the @c std::string for the interface of the state count constant.
     *
     * @return The @c std::string for the interface of the state count constant.
     */
    std::string interfaceStateCountString() const;

    /**
     * @brief Set the @c std::string for the interface of the state count
     * constant.
     *
     * Set the @c std::string for the interface of the state count constant.
     *
     * @param interfaceStateCountString The @c std::string to use for the
     * interface of the state count constant.
     */
    void setInterfaceStateCountString(const std::string &interfaceStateCountString);

    /**
     * @brief Get the @c std::string for the implementation of the state count
     * constant.
     *
     * Return the @c std::string for the implementation of the state count
     * constant.
     *
     * @return The @c std::string for the implementation of the state count
     * constant.
     */
    std::string implementationStateCountString() const;

    /**
     * @brief Set the @c std::string for the implementation of the state count
     * constant.
     *
     * Set the @c std::string for the implementation of the state count
     * constant. To be useful, the string should contain the [STATE_COUNT] tag,
     * which will be replaced with the number of states in the model.
     *
     * @param implementationStateCountString The @c std::string to use for the
     * implementation of the state count constant.
     */
    void setImplementationStateCountString(const std::string &implementationStateCountString);

    /**
     * @brief Get the @c std::string for the interface of the constant count
     * constant.
     *
     * Return the @c std::string for the interface of the constant count
     * constant.
     *
     * @return The @c std::string for the interface of the constant count
     * constant.
     */
    std::string interfaceConstantCountString() const;

    /**
     * @brief Set the @c std::string for the interface of the constant count
     * constant.
     *
     * Set the @c std::string for the interface of the constant count constant.
     *
     * @param interfaceConstantCountString The @c std::string to use for the
     * interface of the constant count constant.
     */
    void setInterfaceConstantCountString(const std::string &interfaceConstantCountString);

    /**
     * @brief Get the @c std::string for the implementation of the constant
     * count constant.
     *
     * Return the @c std::string for the implementation of the constant count
     * constant.
     *
     * @return The @c std::string for the implementation of the constant count
     * constant.
     */
    std::string implementationConstantCountString() const;

    /**
     * @brief Set the @c std::string for the implementation of the constant
     * count constant.
     *
     * Set the @c std::string for the implementation of the constant count
     * constant. To be useful, the string should contain the [CONSTANT_COUNT]
     * tag, which will be replaced with the number of states in the model.
     *
     * @param implementationConstantCountString The @c std::string to use for
     * the implementation of the constant count constant.
     */
    void setImplementationConstantCountString(const std::string &implementationConstantCountString);

    /**
     * @brief Get the @c std::string for the interface of the computed constant count
     * constant.
     *
     * Return the @c std::string for the interface of the computed constant count
     * constant.
     *
     * @return The @c std::string for the interface of the computed constant count
     * constant.
     */
    std::string interfaceComputedConstantCountString() const;

    /**
     * @brief Set the @c std::string for the interface of the computed constant count
     * constant.
     *
     * Set the @c std::string for the interface of the computed constant count constant.
     *
     * @param interfaceComputedConstantCountString The @c std::string to use for the
     * interface of the computed constant count constant.
     */
    void setInterfaceComputedConstantCountString(const std::string &interfaceComputedConstantCountString);

    /**
     * @brief Get the @c std::string for the implementation of the computed constant
     * count constant.
     *
     * Return the @c std::string for the implementation of the computed constant count
     * constant.
     *
     * @return The @c std::string for the implementation of the computed constant count
     * constant.
     */
    std::string implementationComputedConstantCountString() const;

    /**
     * @brief Set the @c std::string for the implementation of the computed constant
     * count constant.
     *
     * Set the @c std::string for the implementation of the computed constant count
     * constant. To be useful, the string should contain the [COMPUTED_CONSTANT_COUNT]
     * tag, which will be replaced with the number of states in the model.
     *
     * @param implementationComputedConstantCountString The @c std::string to use for
     * the implementation of the computed constant count constant.
     */
    void setImplementationComputedConstantCountString(const std::string &implementationComputedConstantCountString);

    /**
     * @brief Get the @c std::string for the interface of the algebraic variable count
     * constant.
     *
     * Return the @c std::string for the interface of the algebraic variable count
     * constant.
     *
     * @return The @c std::string for the interface of the algebraic variable count
     * constant.
     */
    std::string interfaceAlgebraicVariableCountString() const;

    /**
     * @brief Set the @c std::string for the interface of the algebraic variable count
     * constant.
     *
     * Set the @c std::string for the interface of the algebraic variable count constant.
     *
     * @param interfaceAlgebraicVariableCountString The @c std::string to use for the
     * interface of the algebraic variable count constant.
     */
    void setInterfaceAlgebraicVariableCountString(const std::string &interfaceAlgebraicVariableCountString);

    /**
     * @brief Get the @c std::string for the implementation of the algebraic
     * variable count constant.
     *
     * Return the @c std::string for the implementation of the algebraic variable count
     * constant.
     *
     * @return The @c std::string for the implementation of the algebraic variable count
     * constant.
     */
    std::string implementationAlgebraicVariableCountString() const;

    /**
     * @brief Set the @c std::string for the implementation of the algebraic
     * variable count constant.
     *
     * Set the @c std::string for the implementation of the algebraic variable count
     * constant. To be useful, the string should contain the [ALGEBRAIC_VARIABLE_COUNT]
     * tag, which will be replaced with the number of states in the model.
     *
     * @param implementationAlgebraicVariableCountString The @c std::string to use for
     * the implementation of the algebraic variable count constant.
     */
    void setImplementationAlgebraicVariableCountString(const std::string &implementationAlgebraicVariableCountString);

    /**
     * @brief Get the @c std::string for the interface of the external variable count
     * constant.
     *
     * Return the @c std::string for the interface of the external variable count
     * constant.
     *
     * @return The @c std::string for the interface of the external variable count
     * constant.
     */
    std::string interfaceExternalVariableCountString() const;

    /**
     * @brief Set the @c std::string for the interface of the external variable count
     * constant.
     *
     * Set the @c std::string for the interface of the external variable count constant.
     *
     * @param interfaceExternalVariableCountString The @c std::string to use for the
     * interface of the external variable count constant.
     */
    void setInterfaceExternalVariableCountString(const std::string &interfaceExternalVariableCountString);

    /**
     * @brief Get the @c std::string for the implementation of the external
     * variable count constant.
     *
     * Return the @c std::string for the implementation of the external variable count
     * constant.
     *
     * @return The @c std::string for the implementation of the external variable count
     * constant.
     */
    std::string implementationExternalVariableCountString() const;

    /**
     * @brief Set the @c std::string for the implementation of the external
     * variable count constant.
     *
     * Set the @c std::string for the implementation of the external variable count
     * constant. To be useful, the string should contain the [EXTERNAL_VARIABLE_COUNT]
     * tag, which will be replaced with the number of states in the model.
     *
     * @param implementationExternalVariableCountString The @c std::string to use for
     * the implementation of the external variable count constant.
     */
    void setImplementationExternalVariableCountString(const std::string &implementationExternalVariableCountString);

    /**
     * @brief Get the @c std::string for the data structure for the variable
     * information object.
     *
     * Return the @c std::string for the data structure for the variable
     * information object.
     *
     * @return The @c std::string for the data structure for the variable
     * information object.
     */
    std::string variableInfoObjectString() const;

    /**
     * @brief Set the @c std::string for the data structure for the variable
     * information object.
     *
     * Set the @c std::string for the data structure for the variable
     * information object. To be useful, the string should contain the
     * [NAME_SIZE], [UNITS_SIZE] and [COMPONENT_SIZE] tags, which will be
     * replaced with the maximum size of a string for holding the name of a
     * component, variable and units, respectively.
     *
     * @param variableInfoObjectString The @c std::string to use for the data
     * structure for the variable information object.
     */
    void setVariableInfoObjectString(const std::string &variableInfoObjectString);

    /**
     * @brief Get the @c std::string for the interface of some information about
     * the variable of integration.
     *
     * Return the @c std::string for the interface of some information about the
     * variable of integration.
     *
     * @return The @c std::string for the interface of some information about
     * the variable of integration.
     */
    std::string interfaceVoiInfoString() const;

    /**
     * @brief Set the @c std::string for the interface of some information about
     * the variable of integration.
     *
     * Set the @c std::string for the interface of some information about the
     * variable of integration.
     *
     * @param interfaceVoiInfoString The @c std::string to use for the interface
     * of some information about the variable of integration.
     */
    void setInterfaceVoiInfoString(const std::string &interfaceVoiInfoString);

    /**
     * @brief Get the @c std::string for the implementation of some information
     * about the variable of integration.
     *
     * Return the @c std::string for the implementation of some information
     * about the variable of integration.
     *
     * @return The @c std::string for the implementation of some information
     * about the variable of integration.
     */
    std::string implementationVoiInfoString() const;

    /**
     * @brief Set the @c std::string for the implementation of some information
     * about the variable of integration.
     *
     * Set the @c std::string for the implementation of some information about
     * the variable of integration. To be useful, the string should contain the
     * [CODE] tag, which will be replaced with some information about the
     * variable of integration.
     *
     * @param implementationVoiInfoString The @c std::string to use for the
     * implementation of some information about the variable of integration.
     */
    void setImplementationVoiInfoString(const std::string &implementationVoiInfoString);

    /**
     * @brief Get the @c std::string for the interface of some information about
     * the different states.
     *
     * Return the @c std::string for the interface of some information about the
     * different states.
     *
     * @return The @c std::string for the interface of some information about
     * the different states.
     */
    std::string interfaceStateInfoString() const;

    /**
     * @brief Set the @c std::string for the interface of some information about
     * the different states.
     *
     * Set the @c std::string for the interface of some information about the
     * different states.
     *
     * @param interfaceStateInfoString The @c std::string to use for the
     * interface of some information about the different states.
     */
    void setInterfaceStateInfoString(const std::string &interfaceStateInfoString);

    /**
     * @brief Get the @c std::string for the implementation of some information
     * about the different states.
     *
     * Return the @c std::string for the implementation of some information
     * about the different states.
     *
     * @return The @c std::string for the implementation of some information
     * about the different states.
     */
    std::string implementationStateInfoString() const;

    /**
     * @brief Set the @c std::string for the implementation of some information
     * about the different states.
     *
     * Set the @c std::string for the implementation of some information about
     * the different states. To be useful, the string should contain the [CODE]
     * tag, which will be replaced with some information about the different
     * states.
     *
     * @param implementationStateInfoString The @c std::string to use for the
     * implementation of some information about the different states.
     */
    void setImplementationStateInfoString(const std::string &implementationStateInfoString);

    /**
     * @brief Get the @c std::string for the interface of some information about
     * the different constants.
     *
     * Return the @c std::string for the interface of some information about the
     * different constants.
     *
     * @return The @c std::string for the interface of some information about
     * the different constants.
     */
    std::string interfaceConstantInfoString() const;

    /**
     * @brief Set the @c std::string for the interface of some information about
     * the different constants.
     *
     * Set the @c std::string for the interface of some information about the
     * different constants.
     *
     * @param interfaceConstantInfoString The @c std::string to use for the
     * interface of some information about the different constants.
     */
    void setInterfaceConstantInfoString(const std::string &interfaceConstantInfoString);

    /**
     * @brief Get the @c std::string for the implementation of some information
     * about the different constants.
     *
     * Return the @c std::string for the implementation of some information
     * about the different constants.
     *
     * @return The @c std::string for the implementation of some information
     * about the different constants.
     */
    std::string implementationConstantInfoString() const;

    /**
     * @brief Set the @c std::string for the implementation of some information
     * about the different constants.
     *
     * Set the @c std::string for the implementation of some information about
     * the different constants. To be useful, the string should contain the [CODE]
     * tag, which will be replaced with some information about the different
     * constants.
     *
     * @param implementationConstantInfoString The @c std::string to use for the
     * implementation of some information about the different constants.
     */
    void setImplementationConstantInfoString(const std::string &implementationConstantInfoString);

    /**
     * @brief Get the @c std::string for the interface of some information about
     * the different computed constants.
     *
     * Return the @c std::string for the interface of some information about the
     * different computed constants.
     *
     * @return The @c std::string for the interface of some information about
     * the different computed constants.
     */
    std::string interfaceComputedConstantInfoString() const;

    /**
     * @brief Set the @c std::string for the interface of some information about
     * the different computed constants.
     *
     * Set the @c std::string for the interface of some information about the
     * different computed constants.
     *
     * @param interfaceComputedConstantInfoString The @c std::string to use for the
     * interface of some information about the different computed constants.
     */
    void setInterfaceComputedConstantInfoString(const std::string &interfaceComputedConstantInfoString);

    /**
     * @brief Get the @c std::string for the implementation of some information
     * about the different computed constants.
     *
     * Return the @c std::string for the implementation of some information
     * about the different computed constants.
     *
     * @return The @c std::string for the implementation of some information
     * about the different computed constants.
     */
    std::string implementationComputedConstantInfoString() const;

    /**
     * @brief Set the @c std::string for the implementation of some information
     * about the different computed constants.
     *
     * Set the @c std::string for the implementation of some information about
     * the different computed constants. To be useful, the string should contain the [CODE]
     * tag, which will be replaced with some information about the different
     * computed constants.
     *
     * @param implementationComputedConstantInfoString The @c std::string to use for the
     * implementation of some information about the different computed constants.
     */
    void setImplementationComputedConstantInfoString(const std::string &implementationComputedConstantInfoString);

    /**
     * @brief Get the @c std::string for the interface of some information about
     * the different algebraic variables.
     *
     * Return the @c std::string for the interface of some information about the
     * different algebraic variables.
     *
     * @return The @c std::string for the interface of some information about
     * the different algebraic variables.
     */
    std::string interfaceAlgebraicVariableInfoString() const;

    /**
     * @brief Set the @c std::string for the interface of some information about
     * the different algebraic variables.
     *
     * Set the @c std::string for the interface of some information about the
     * different algebraic variables.
     *
     * @param interfaceAlgebraicVariableInfoString The @c std::string to use for the
     * interface of some information about the different algebraic variables.
     */
    void setInterfaceAlgebraicVariableInfoString(const std::string &interfaceAlgebraicVariableInfoString);

    /**
     * @brief Get the @c std::string for the implementation of some information
     * about the different algebraic variables.
     *
     * Return the @c std::string for the implementation of some information
     * about the different algebraic variables.
     *
     * @return The @c std::string for the implementation of some information
     * about the different algebraic variables.
     */
    std::string implementationAlgebraicVariableInfoString() const;

    /**
     * @brief Set the @c std::string for the implementation of some information
     * about the different algebraic variables.
     *
     * Set the @c std::string for the implementation of some information about
     * the different algebraic variables. To be useful, the string should contain the [CODE]
     * tag, which will be replaced with some information about the different
     * algebraic variables.
     *
     * @param implementationAlgebraicVariableInfoString The @c std::string to use for the
     * implementation of some information about the different algebraic variables.
     */
    void setImplementationAlgebraicVariableInfoString(const std::string &implementationAlgebraicVariableInfoString);

    /**
     * @brief Get the @c std::string for the interface of some information about
     * the different external variables.
     *
     * Return the @c std::string for the interface of some information about the
     * different external variables.
     *
     * @return The @c std::string for the interface of some information about
     * the different external variables.
     */
    std::string interfaceExternalVariableInfoString() const;

    /**
     * @brief Set the @c std::string for the interface of some information about
     * the different external variables.
     *
     * Set the @c std::string for the interface of some information about the
     * different external variables.
     *
     * @param interfaceExternalVariableInfoString The @c std::string to use for the
     * interface of some information about the different external variables.
     */
    void setInterfaceExternalVariableInfoString(const std::string &interfaceExternalVariableInfoString);

    /**
     * @brief Get the @c std::string for the implementation of some information
     * about the different external variables.
     *
     * Return the @c std::string for the implementation of some information
     * about the different external variables.
     *
     * @return The @c std::string for the implementation of some information
     * about the different external variables.
     */
    std::string implementationExternalVariableInfoString() const;

    /**
     * @brief Set the @c std::string for the implementation of some information
     * about the different external variables.
     *
     * Set the @c std::string for the implementation of some information about
     * the different external variables. To be useful, the string should contain the [CODE]
     * tag, which will be replaced with some information about the different
     * external variables.
     *
     * @param implementationExternalVariableInfoString The @c std::string to use for the
     * implementation of some information about the different external variables.
     */
    void setImplementationExternalVariableInfoString(const std::string &implementationExternalVariableInfoString);

    /**
     * @brief Get the @c std::string for an entry in an array for some
     * information about a variable.
     *
     * Return the @c std::string for an entry in an array for some information
     * about a variable.
     *
     * @return The @c std::string for an entry in an array for some information
     * about a variable.
     */
    std::string variableInfoEntryString() const;

    /**
     * @brief Set the @c std::string for an entry in an array for some
     * information about a variable.
     *
     * Set the @c std::string for an entry in an array for some information
     * about a variable. To be useful, the string should contain the
     * [COMPONENT], [NAME] and [UNITS] tags, which will be replaced with the
     * name of the component, name and units of a variable.
     *
     * @param variableInfoEntryString The @c std::string to use for an entry in
     * an array for some information about a variable.
     */
    void setVariableInfoEntryString(const std::string &variableInfoEntryString);

    /**
     * @brief Get the @c std::string for the name of the variable of
     * integration.
     *
     * Return the @c std::string for the name of the variable of integration.
     *
     * @return The @c std::string for the name of the variable of integration.
     */
    std::string voiString() const;

    /**
     * @brief Set the @c std::string for the name of the variable of
     * integration.
     *
     * Set the @c std::string for the name of the variable of integration.
     *
     * @param voiString The @c std::string to use for the name of the variable
     * of integration.
     */
    void setVoiString(const std::string &voiString);

    /**
     * @brief Get the @c std::string for the name of the states array.
     *
     * Return the @c std::string for the name of the states array.
     *
     * @return The @c std::string for the name of the states array.
     */
    std::string statesArrayString() const;

    /**
     * @brief Set the @c std::string for the name of the states array.
     *
     * Set the @c std::string for the name of the states array.
     *
     * @param statesArrayString The @c std::string to use for the name of the
     * states array.
     */
    void setStatesArrayString(const std::string &statesArrayString);

    /**
     * @brief Get the @c std::string for the name of the rates array.
     *
     * Return the @c std::string for the name of the rates array.
     *
     * @return The @c std::string for the name of the rates array.
     */
    std::string ratesArrayString() const;

    /**
     * @brief Set the @c std::string for the name of the rates array.
     *
     * Set the @c std::string for the name of the rates array.
     *
     * @param ratesArrayString The @c std::string to use for the name of the
     * rates array.
     */
    void setRatesArrayString(const std::string &ratesArrayString);

    /**
     * @brief Get the @c std::string for the name of the constants array.
     *
     * Return the @c std::string for the name of the constants array.
     *
     * @return The @c std::string for the name of the constants array.
     */
    std::string constantsArrayString() const;

    /**
     * @brief Set the @c std::string for the name of the constants array.
     *
     * Set the @c std::string for the name of the constants array.
     *
     * @param constantsArrayString The @c std::string to use for the name of the
     * constants array.
     */
    void setConstantsArrayString(const std::string &constantsArrayString);

    /**
     * @brief Get the @c std::string for the name of the computed constants array.
     *
     * Return the @c std::string for the name of the computed constants array.
     *
     * @return The @c std::string for the name of the computed constants array.
     */
    std::string computedConstantsArrayString() const;

    /**
     * @brief Set the @c std::string for the name of the computed constants array.
     *
     * Set the @c std::string for the name of the computed constants array.
     *
     * @param computedConstantsArrayString The @c std::string to use for the name of the
     * computed constants array.
     */
    void setComputedConstantsArrayString(const std::string &computedConstantsArrayString);

    /**
     * @brief Get the @c std::string for the name of the algebraic variables array.
     *
     * Return the @c std::string for the name of the algebraic variables array.
     *
     * @return The @c std::string for the name of the algebraic variables array.
     */
    std::string algebraicVariablesArrayString() const;

    /**
     * @brief Set the @c std::string for the name of the algebraic variables array.
     *
     * Set the @c std::string for the name of the algebraic variables array.
     *
     * @param algebraicVariablesArrayString The @c std::string to use for the name of the
     * algebraic variables array.
     */
    void setAlgebraicVariablesArrayString(const std::string &algebraicVariablesArrayString);

    /**
     * @brief Get the @c std::string for the name of the external variables array.
     *
     * Return the @c std::string for the name of the external variables array.
     *
     * @return The @c std::string for the name of the external variables array.
     */
    std::string externalVariablesArrayString() const;

    /**
     * @brief Set the @c std::string for the name of the external variables array.
     *
     * Set the @c std::string for the name of the external variables array.
     *
     * @param externalVariablesArrayString The @c std::string to use for the name of the
     * external variables array.
     */
    void setExternalVariablesArrayString(const std::string &externalVariablesArrayString);

    /**
     * @brief Get the @c std::string for the type definition of an external
     * variable method.
     *
     * Return the @c std::string for the type definition of an external variable
     * method.
     *
     * @param forDifferentialModel Whether the type definition of an external
     * variable method is for a differential model, as opposed to an algebraic
     * model.
     *
     * @return The @c std::string for the type definition of an external
     * variable method.
     */
    std::string externalVariableMethodTypeDefinitionString(bool forDifferentialModel) const;

    /**
     * @brief Set the @c std::string for the type definition of an external
     * variable method.
     *
     * Set this @c std::string for the type definition of an external variable
     * method.
     *
     * @param forDifferentialModel Whether the type definition of an external
     * variable method is for a differential model, as opposed to an algebraic
     * model.
     * @param externalVariableMethodTypeDefinitionString The @c std::string to
     * use for the type definition of an external variable method.
     */
    void setExternalVariableMethodTypeDefinitionString(bool forDifferentialModel,
                                                       const std::string &externalVariableMethodTypeDefinitionString);

    /**
     * @brief Get the @c std::string for the call to the external variable
     * method.
     *
     * Return the @c std::string for the call to the external variable method.
     *
     * @param forDifferentialModel Whether the call to the external variable
     * method is for a differential model, as opposed to an algebraic model.
     *
     * @return The @c std::string for the call to the external variable method.
     */
    std::string externalVariableMethodCallString(bool forDifferentialModel) const;

    /**
     * @brief Set the @c std::string for the call to the external variable
     * method.
     *
     * Set this @c std::string for the call to the external variable method. To
     * be useful, the string should contain the [INDEX] tag, which will be
     * replaced with the index of the external variable.
     *
     * @param forDifferentialModel Whether the call to the external variable
     * method is for a differential model, as opposed to an algebraic model.
     * @param externalVariableMethodCallString The @c std::string to use for the
     * call to the external variable method.
     */
    void setExternalVariableMethodCallString(bool forDifferentialModel,
                                             const std::string &externalVariableMethodCallString);

    /**
     * @brief Get the @c std::string for the data structure for the root finding
     * information object.
     *
     * Return the @c std::string for the data structure for the root finding
     * information object.
     *
     * @param forDifferentialModel Whether the call to the root finding
     * information object is for a differential model, as opposed to an
     * algebraic model.
     * @param withExternalVariables Whether the call to the root finding
     * information object is for a model with external variables.
     *
     * @return The @c std::string for the data structure for the root finding
     * information object.
     */
    std::string rootFindingInfoObjectString(bool forDifferentialModel,
                                            bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the data structure for the root finding
     * information object.
     *
     * Set the @c std::string for the data structure for the root finding
     * information object.
     *
     * @param forDifferentialModel Whether the call to the root finding
     * information object is for a differential model, as opposed to an
     * algebraic model.
     * @param withExternalVariables Whether the call to the root finding
     * information object is for a model with external variables.
     * @param rootFindingInfoObjectString The @c std::string to use for the data
     * structure for the root finding information object.
     */
    void setRootFindingInfoObjectString(bool forDifferentialModel,
                                        bool withExternalVariables,
                                        const std::string &rootFindingInfoObjectString);

    /**
     * @brief Get the @c std::string for the external NLA solve method.
     *
     * Return the @c std::string for the external NLA solve method.
     *
     * @return The @c std::string for the external NLA solve method.
     */
    std::string externNlaSolveMethodString() const;

    /**
     * @brief Set the @c std::string for the external NLA solve method.
     *
     * Set the @c std::string for the external NLA solve method.
     *
     * @param externNlaSolveMethodString The @c std::string to use for the
     * external NLA solve method.
     */
    void setExternNlaSolveMethodString(const std::string &externNlaSolveMethodString);

    /**
     * @brief Get the @c std::string for the call to the find root method.
     *
     * Return the @c std::string for the call to the find root method.
     *
     * @param forDifferentialModel Whether the call to the find root method is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the find root method is
     * for a model with external variables.
     *
     * @return The @c std::string for the call to the find root method.
     */
    std::string findRootCallString(bool forDifferentialModel,
                                   bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the call to the find root method.
     *
     * Set the @c std::string for the call to the find root method. To be
     * useful, the string should contain the [INDEX] tag, which will be replaced
     * with the index of the NLA system for which we want to find the root(s).
     *
     * @param forDifferentialModel Whether the call to the find root method is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the find root method is
     * for a model with external variables.
     * @param findRootCallString The @c std::string to use for the call to the
     * find root method.
     */
    void setFindRootCallString(bool forDifferentialModel,
                               bool withExternalVariables,
                               const std::string &findRootCallString);

    /**
     * @brief Get the @c std::string for the find root method.
     *
     * Return the @c std::string for the find root method.
     *
     * @param forDifferentialModel Whether the call to the find root method is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the find root method is
     * for a model with external variables.
     *
     * @return The @c std::string for the find root method.
     */
    std::string findRootMethodString(bool forDifferentialModel,
                                     bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the find root method.
     *
     * Set the @c std::string for the find root method. To be useful, the string
     * should contain the [INDEX], [SIZE], and [CODE] tags, which will be
     * replaced with the index of the NLA system for which we want to find the
     * root(s), the size of the @c u array, and some code to find the root(s),
     * respectively.
     *
     * @sa uArrayString, setUArrayString
     *
     * @param forDifferentialModel Whether the call to the find root method is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the find root method is
     * for a model with external variables.
     * @param findRootMethodString The @c std::string to use for the find root
     * method.
     */
    void setFindRootMethodString(bool forDifferentialModel,
                                 bool withExternalVariables,
                                 const std::string &findRootMethodString);

    /**
     * @brief Get the @c std::string for the call to the NLA solve method.
     *
     * Return the @c std::string for the call to the NLA solve method.
     *
     * @param forDifferentialModel Whether the call to the NLA solve method is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the NLA solve method is
     * for a model with external variables.
     *
     * @return The @c std::string for the call to the NLA solve method.
     */
    std::string nlaSolveCallString(bool forDifferentialModel,
                                   bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the call to the NLA solve method.
     *
     * Set the @c std::string for the call to the NLA solve method. To be
     * useful, the string should contain the [INDEX] and [SIZE] tags, which will
     * be replaced with the index of the NLA system for which we want to find
     * the root(s) and the size of the @c u array, respectively.
     *
     * @sa uArrayString, setUArrayString
     *
     * @param forDifferentialModel Whether the call to the NLA solve method is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the NLA solve method is
     * for a model with external variables.
     * @param nlaSolveCallString The @c std::string to use for the call to the
     * NLA solve method.
     */
    void setNlaSolveCallString(bool forDifferentialModel,
                               bool withExternalVariables,
                               const std::string &nlaSolveCallString);

    /**
     * @brief Get the @c std::string for the objective function method.
     *
     * Return the @c std::string for the objective function method.
     *
     * @param forDifferentialModel Whether the call to the objective function
     * method is for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the objective function
     * method is for a model with external variables.
     *
     * @return The @c std::string for the objective function method.
     */
    std::string objectiveFunctionMethodString(bool forDifferentialModel,
                                              bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the objective function method.
     *
     * Set the @c std::string for the objective function method. To be useful,
     * the string should contain the [INDEX] and [CODE] tags, which will be
     * replaced with the index of the NLA system for which we want to find the
     * root(s) and some code to compute the objective function, respectively.
     *
     * @param forDifferentialModel Whether the call to the objective function
     * method is for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the call to the objective function
     * method is for a model with external variables.
     * @param objectiveFunctionMethodString The @c std::string to use for the
     * objective function method.
     */
    void setObjectiveFunctionMethodString(bool forDifferentialModel,
                                          bool withExternalVariables,
                                          const std::string &objectiveFunctionMethodString);

    /**
     * @brief Get the @c std::string for the @c u array used in the objective
     * function and find root methods.
     *
     * Return the @c std::string for the @c u array used in the objective function
     * and find root methods. The @c u array is used to keep track of the value of
     * the different unknown variables in a system of non-linear algebraic equations.
     *
     * @return The @c std::string for the @c u array used in the objective function
     * and find root methods.
     */
    std::string uArrayString() const;

    /**
     * @brief Set the @c std::string for the @c u array used in the objective
     * function and find root methods.
     *
     * Set the @c std::string for the @c u array used in the objective function and
     * find root methods. The @c u array is used to keep track of the value of the
     * different unknown variables in a system of non-linear algebraic equations.
     *
     * @param uArrayString The @c std::string to use for the @c u array used in the
     * objective function and find root methods.
     */
    void setUArrayString(const std::string &uArrayString);

    /**
     * @brief Get the @c std::string for the @c f array used in the objective
     * function and find root methods.
     *
     * Return the @c std::string for the @c f array used in the objective function
     * and find root methods. The @c f array is used to keep track of the value of
     * the different objective functions in a system of non-linear algebraic equations.
     *
     * @return The @c std::string for the @c f array used in the objective function
     * and find root methods.
     */
    std::string fArrayString() const;

    /**
     * @brief Set the @c std::string for the @c f array used in the objective
     * function and find root methods.
     *
     * Set the @c std::string for the @c f array used in the objective function and
     * find root methods. The @c f array is used to keep track of the value of the
     * different objective functions in a system of non-linear algebraic equations.
     *
     * @param fArrayString The @c std::string to use for the @c f array used in the
     * objective function and find root methods.
     */
    void setFArrayString(const std::string &fArrayString);

    /**
     * @brief Get the @c std::string for the interface to create the states
     * array.
     *
     * Return the @c std::string for the interface to create the states array.
     *
     * @return The @c std::string for the interface to create the states array.
     */
    std::string interfaceCreateStatesArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the interface to create the states
     * array.
     *
     * Set the @c std::string for the interface to create the states array.
     *
     * @param interfaceCreateStatesArrayMethodString The @c std::string to use
     * for the interface to create the states array.
     */
    void setInterfaceCreateStatesArrayMethodString(const std::string &interfaceCreateStatesArrayMethodString);

    /**
     * @brief Get the @c std::string for the implementation to create the states
     * array.
     *
     * Return the @c std::string for the implementation to create the states
     * array.
     *
     * @return The @c std::string for the implementation to create the states
     * array.
     */
    std::string implementationCreateStatesArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the implementation to create the states
     * array.
     *
     * Set the @c std::string for the implementation to create the states array.
     *
     * @param implementationCreateStatesArrayMethodString The @c std::string to
     * use for the implementation to create the states array.
     */
    void setImplementationCreateStatesArrayMethodString(const std::string &implementationCreateStatesArrayMethodString);

    /**
     * @brief Get the @c std::string for the interface to create the constants
     * array.
     *
     * Return the @c std::string for the interface to create the constants array.
     *
     * @return The @c std::string for the interface to create the constants array.
     */
    std::string interfaceCreateConstantsArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the interface to create the constants
     * array.
     *
     * Set the @c std::string for the interface to create the constants array.
     *
     * @param interfaceCreateConstantsArrayMethodString The @c std::string to use
     * for the interface to create the constants array.
     */
    void setInterfaceCreateConstantsArrayMethodString(const std::string &interfaceCreateConstantsArrayMethodString);

    /**
     * @brief Get the @c std::string for the implementation to create the constants
     * array.
     *
     * Return the @c std::string for the implementation to create the constants
     * array.
     *
     * @return The @c std::string for the implementation to create the constants
     * array.
     */
    std::string implementationCreateConstantsArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the implementation to create the constants
     * array.
     *
     * Set the @c std::string for the implementation to create the constants array.
     *
     * @param implementationCreateConstantsArrayMethodString The @c std::string to
     * use for the implementation to create the constants array.
     */
    void setImplementationCreateConstantsArrayMethodString(const std::string &implementationCreateConstantsArrayMethodString);

    /**
     * @brief Get the @c std::string for the interface to create the computed constants
     * array.
     *
     * Return the @c std::string for the interface to create the computed constants array.
     *
     * @return The @c std::string for the interface to create the computed constants array.
     */
    std::string interfaceCreateComputedConstantsArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the interface to create the computed constants
     * array.
     *
     * Set the @c std::string for the interface to create the computed constants array.
     *
     * @param interfaceCreateComputedConstantsArrayMethodString The @c std::string to use
     * for the interface to create the computed constants array.
     */
    void setInterfaceCreateComputedConstantsArrayMethodString(const std::string &interfaceCreateComputedConstantsArrayMethodString);

    /**
     * @brief Get the @c std::string for the implementation to create the computed constants
     * array.
     *
     * Return the @c std::string for the implementation to create the computed constants
     * array.
     *
     * @return The @c std::string for the implementation to create the computed constants
     * array.
     */
    std::string implementationCreateComputedConstantsArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the implementation to create the computed constants
     * array.
     *
     * Set the @c std::string for the implementation to create the computed constants array.
     *
     * @param implementationCreateComputedConstantsArrayMethodString The @c std::string to
     * use for the implementation to create the computed constants array.
     */
    void setImplementationCreateComputedConstantsArrayMethodString(const std::string &implementationCreateComputedConstantsArrayMethodString);

    /**
     * @brief Get the @c std::string for the interface to create the algebraic variables
     * array.
     *
     * Return the @c std::string for the interface to create the algebraic variables array.
     *
     * @return The @c std::string for the interface to create the algebraic variables array.
     */
    std::string interfaceCreateAlgebraicVariablesArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the interface to create the algebraic variables
     * array.
     *
     * Set the @c std::string for the interface to create the algebraic variables array.
     *
     * @param interfaceCreateAlgebraicVariablesArrayMethodString The @c std::string to use
     * for the interface to create the algebraic variables array.
     */
    void setInterfaceCreateAlgebraicVariablesArrayMethodString(const std::string &interfaceCreateAlgebraicVariablesArrayMethodString);

    /**
     * @brief Get the @c std::string for the implementation to create the algebraic variables
     * array.
     *
     * Return the @c std::string for the implementation to create the algebraic variables
     * array.
     *
     * @return The @c std::string for the implementation to create the algebraic variables
     * array.
     */
    std::string implementationCreateAlgebraicVariablesArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the implementation to create the algebraic variables
     * array.
     *
     * Set the @c std::string for the implementation to create the algebraic variables array.
     *
     * @param implementationCreateAlgebraicVariablesArrayMethodString The @c std::string to
     * use for the implementation to create the algebraic variables array.
     */
    void setImplementationCreateAlgebraicVariablesArrayMethodString(const std::string &implementationCreateAlgebraicVariablesArrayMethodString);

    /**
     * @brief Get the @c std::string for the interface to create the external variables
     * array.
     *
     * Return the @c std::string for the interface to create the external variables array.
     *
     * @return The @c std::string for the interface to create the external variables array.
     */
    std::string interfaceCreateExternalVariablesArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the interface to create the external variables
     * array.
     *
     * Set the @c std::string for the interface to create the external variables array.
     *
     * @param interfaceCreateExternalVariablesArrayMethodString The @c std::string to use
     * for the interface to create the external variables array.
     */
    void setInterfaceCreateExternalVariablesArrayMethodString(const std::string &interfaceCreateExternalVariablesArrayMethodString);

    /**
     * @brief Get the @c std::string for the implementation to create the external variables
     * array.
     *
     * Return the @c std::string for the implementation to create the external variables
     * array.
     *
     * @return The @c std::string for the implementation to create the external variables
     * array.
     */
    std::string implementationCreateExternalVariablesArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the implementation to create the external variables
     * array.
     *
     * Set the @c std::string for the implementation to create the external variables array.
     *
     * @param implementationCreateExternalVariablesArrayMethodString The @c std::string to
     * use for the implementation to create the external variables array.
     */
    void setImplementationCreateExternalVariablesArrayMethodString(const std::string &implementationCreateExternalVariablesArrayMethodString);

    /**
     * @brief Get the @c std::string for the interface to delete an array.
     *
     * Return the @c std::string for the interface to delete an array.
     *
     * @return The @c std::string for the interface to delete an array.
     */
    std::string interfaceDeleteArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the interface to delete an array.
     *
     * Set the @c std::string for the interface to delete an array.
     *
     * @param interfaceDeleteArrayMethodString The @c std::string to use for the
     * interface to delete an array.
     */
    void setInterfaceDeleteArrayMethodString(const std::string &interfaceDeleteArrayMethodString);

    /**
     * @brief Get the @c std::string for the implementation to delete an array.
     *
     * Return the @c std::string for the implementation to delete an array.
     *
     * @return The @c std::string for the implementation to delete an array.
     */
    std::string implementationDeleteArrayMethodString() const;

    /**
     * @brief Set the @c std::string for the implementation to delete an array.
     *
     * Set the @c std::string for the implementation to delete an array.
     *
     * @param implementationDeleteArrayMethodString The @c std::string to use
     * for the implementation to delete an array.
     */
    void setImplementationDeleteArrayMethodString(const std::string &implementationDeleteArrayMethodString);

    /**
     * @brief Get the @c std::string for the interface to initialise variables.
     *
     * Return the @c std::string for the interface to initialise variables.
     *
     * @param forDifferentialModel Whether the interface to initialise variables
     * is for a differential model, as opposed to an algebraic model.
     *
     * @return The @c std::string for the interface to initialise variables.
     */
    std::string interfaceInitialiseArraysMethodString(bool forDifferentialModel) const;

    /**
     * @brief Set the @c std::string for the interface to initialise variables.
     *
     * Set the @c std::string for the interface to initialise variables.
     *
     * @param forDifferentialModel Whether the interface to initialise variables
     * is for a differential model, as opposed to an algebraic model.
     * @param interfaceInitialiseArraysMethodString The @c std::string to use
     * for the interface to initialise variables.
     */
    void setInterfaceInitialiseArraysMethodString(bool forDifferentialModel,
                                                  const std::string &interfaceInitialiseArraysMethodString);

    /**
     * @brief Get the @c std::string for the implementation to initialise
     * variables.
     *
     * Return the @c std::string for the implementation to initialise variables.
     *
     * @param forDifferentialModel Whether the implementation to initialise
     * variables is for a differential model, as opposed to an algebraic model.
     *
     * @return The @c std::string for the implementation to initialise
     * variables.
     */
    std::string implementationInitialiseArraysMethodString(bool forDifferentialModel) const;

    /**
     * @brief Set the @c std::string for the implementation to initialise
     * variables.
     *
     * Set the @c std::string for the implementation to initialise variables. To
     * be useful, the string should contain the [CODE] tag, which will be
     * replaced with some code to initialise variables.
     *
     * @param forDifferentialModel Whether the implementation to initialise
     * variables is for a differential model, as opposed to an algebraic model.
     * @param implementationInitialiseArraysMethodString The @c std::string
     * to use for the implementation to initialise variables.
     */
    void setImplementationInitialiseArraysMethodString(bool forDifferentialModel,
                                                       const std::string &implementationInitialiseArraysMethodString);

    /**
     * @brief Get the @c std::string for the interface to compute computed
     * constants.
     *
     * Return the @c std::string for the interface to compute computed
     * constants.
     *
     * @param forDifferentialModel Whether the interface to compute computed
     * constants is for a differential model, as opposed to an algebraic model.
     *
     * @return The @c std::string for the interface to compute computed
     * constants.
     */
    std::string interfaceComputeComputedConstantsMethodString(bool forDifferentialModel) const;

    /**
     * @brief Set the @c std::string for the interface to compute computed
     * constants.
     *
     * Set the @c std::string for the interface to compute computed constants.
     *
     * @param forDifferentialModel Whether the interface to compute computed
     * constants is for a differential model, as opposed to an algebraic model.
     * @param interfaceComputeComputedConstantsMethodString The @c std::string
     * to use for the interface to compute computed constants.
     */
    void setInterfaceComputeComputedConstantsMethodString(bool forDifferentialModel,
                                                          const std::string &interfaceComputeComputedConstantsMethodString);

    /**
     * @brief Get the @c std::string for the implementation to compute computed
     * constants.
     *
     * Return the @c std::string for the implementation to compute computed
     * constants.
     *
     * @param forDifferentialModel Whether the implementation to compute computed
     * constants is for a differential model, as opposed to an algebraic model.
     *
     * @return The @c std::string for the implementation to compute computed
     * constants.
     */
    std::string implementationComputeComputedConstantsMethodString(bool forDifferentialModel) const;

    /**
     * @brief Set the @c std::string for the implementation to compute computed
     * constants.
     *
     * Set the @c std::string for the implementation to compute computed
     * constants. To be useful, the string should contain the [CODE] tag, which
     * will be replaced with some code to compute computed constants.
     *
     * @param forDifferentialModel Whether the implementation to compute computed
     * constants is for a differential model, as opposed to an algebraic model.
     * @param implementationComputeComputedConstantsMethodString The
     * @c std::string to use for the implementation to compute computed
     * constants.
     */
    void setImplementationComputeComputedConstantsMethodString(bool forDifferentialModel,
                                                               const std::string &implementationComputeComputedConstantsMethodString);

    /**
     * @brief Get the @c std::string for the interface to compute rates.
     *
     * Return the @c std::string for the interface to compute rates.
     *
     * @param withExternalVariables Whether the interface to compute rates is
     * for a model with external variables.
     *
     * @return The @c std::string for the interface to compute rates.
     */
    std::string interfaceComputeRatesMethodString(bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the interface to compute rates.
     *
     * Set the @c std::string for the interface to compute rates.
     *
     * @param withExternalVariables Whether the interface to compute rates is
     * for a model with external variables.
     * @param interfaceComputeRatesMethodString The @c std::string to use for
     * the interface to compute rates.
     */
    void setInterfaceComputeRatesMethodString(bool withExternalVariables,
                                              const std::string &interfaceComputeRatesMethodString);

    /**
     * @brief Get the @c std::string for the implementation to compute rates.
     *
     * Return the @c std::string for the implementation to compute rates.
     *
     * @param withExternalVariables Whether the implementation to compute rates
     * is for a model with external variables.
     *
     * @return The @c std::string for the implementation to compute rates.
     */
    std::string implementationComputeRatesMethodString(bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the implementation to compute rates.
     *
     * Set the @c std::string for the implementation to compute rates. To be
     * useful, the string should contain the [CODE] tag, which will be replaced
     * with some code to compute rates.
     *
     * @param withExternalVariables Whether the implementation to compute rates
     * is for a model with external variables.
     * @param implementationComputeRatesMethodString The @c std::string to use
     * for the implementation to compute rates.
     */
    void setImplementationComputeRatesMethodString(bool withExternalVariables,
                                                   const std::string &implementationComputeRatesMethodString);

    /**
     * @brief Get the @c std::string for the interface to compute variables.
     *
     * Return the @c std::string for the interface to compute variables.
     *
     * @param forDifferentialModel Whether the interface to compute variables is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the interface to compute variables
     * is for a model with external variables.
     *
     * @return The @c std::string for the interface to compute variables.
     */
    std::string interfaceComputeVariablesMethodString(bool forDifferentialModel,
                                                      bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the interface to compute variables.
     *
     * Set the @c std::string for the interface to compute variables.
     *
     * @param forDifferentialModel Whether the interface to compute variables is
     * for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the interface to compute variables
     * is for a model with external variables.
     * @param interfaceComputeVariablesMethodString The @c std::string to use
     * for the interface to compute variables.
     */
    void setInterfaceComputeVariablesMethodString(bool forDifferentialModel,
                                                  bool withExternalVariables,
                                                  const std::string &interfaceComputeVariablesMethodString);

    /**
     * @brief Get the @c std::string for the implementation to compute
     * variables.
     *
     * Return the @c std::string for the implementation to compute variables.
     *
     * @param forDifferentialModel Whether the implementation to compute
     * variables is for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the implementation to compute
     * variables is for a model with external variables.
     *
     * @return The @c std::string for the implementation to compute variables.
     */
    std::string implementationComputeVariablesMethodString(bool forDifferentialModel,
                                                           bool withExternalVariables) const;

    /**
     * @brief Set the @c std::string for the implementation to compute
     * variables.
     *
     * Set the @c std::string for the implementation to compute variables. To be
     * useful, the string should contain the [CODE] tag, which will be replaced
     * with some code to compute rates.
     *
     * @param forDifferentialModel Whether the implementation to compute
     * variables is for a differential model, as opposed to an algebraic model.
     * @param withExternalVariables Whether the implementation to compute
     * variables is for a model with external variables.
     * @param implementationComputeVariablesMethodString The @c std::string to
     * use for the implementation to compute variables.
     */
    void setImplementationComputeVariablesMethodString(bool forDifferentialModel,
                                                       bool withExternalVariables,
                                                       const std::string &implementationComputeVariablesMethodString);

    /**
     * @brief Get the @c std::string for an empty method.
     *
     * Return the @c std::string for an empty method.
     *
     * @return The @c std::string for an empty method.
     */
    std::string emptyMethodString() const;

    /**
     * @brief Set the @c std::string for an empty method.
     *
     * Set the @c std::string for an empty method.
     *
     * @param emptyMethodString The @c std::string to use for an empty method.
     */
    void setEmptyMethodString(const std::string &emptyMethodString);

    /**
     * @brief Get the @c std::string for an indent.
     *
     * Return the @c std::string for an indent.
     *
     * @return The @c std::string for an indent.
     */
    std::string indentString() const;

    /**
     * @brief Set the @c std::string for an indent.
     *
     * Set the @c std::string for an indent.
     *
     * @param indentString The @c std::string to use for an indent.
     */
    void setIndentString(const std::string &indentString);

    /**
     * @brief Get the @c std::string for declaring a variable.
     *
     * Return the @c std::string for declaring a variable.
     *
     * @return The @c std::string for declaring a variable.
     */
    std::string variableDeclarationString() const;

    /**
     * @brief Set the @c std::string for declaring a variable.
     *
     * Set the @c std::string for declaring a variable. To be useful, the string should contain the [CODE] tag, which
     * will be replaced with the variable declaration.
     *
     * @param variableDeclarationString The @c std::string to use for declaring an untracked variable.
     */
    void setVariableDeclarationString(const std::string &variableDeclarationString);

    /**
     * @brief Get the @c std::string for opening an array.
     *
     * Return the @c std::string for opening an array.
     *
     * @return The @c std::string for opening an array.
     */
    std::string openArrayString() const;

    /**
     * @brief Set the @c std::string for opening an array.
     *
     * Set the @c std::string for opening an array.
     *
     * @param openArrayString The @c std::string to use for opening an array.
     */
    void setOpenArrayString(const std::string &openArrayString);

    /**
     * @brief Get the @c std::string for closing an array.
     *
     * Return the @c std::string for closing an array.
     *
     * @return The @c std::string for closing an array.
     */
    std::string closeArrayString() const;

    /**
     * @brief Set the @c std::string for closing an array.
     *
     * Set the @c std::string for closing an array.
     *
     * @param closeArrayString The @c std::string to use for closing an array.
     */
    void setCloseArrayString(const std::string &closeArrayString);

    /**
     * @brief Get the @c std::string for separating elements in an array.
     *
     * Return the @c std::string for separating elements in an array.
     *
     * @return The @c std::string for separating elements in an array.
     */
    std::string arrayElementSeparatorString() const;

    /**
     * @brief Set the @c std::string for separating elements in an array.
     *
     * Set the @c std::string for separating elements in an array.
     *
     * @param arrayElementSeparatorString The @c std::string to use for
     * separating elements in an array.
     */
    void setArrayElementSeparatorString(const std::string &arrayElementSeparatorString);

    /**
     * @brief Get the @c std::string for a command separator.
     *
     * Return the @c std::string for a command separator.
     *
     * @return The @c std::string for a command separator.
     */
    std::string commandSeparatorString() const;

    /**
     * @brief Set the @c std::string for a command separator.
     *
     * Set the @c std::string for a command separator.
     *
     * @param commandSeparatorString The @c std::string to use for a command
     * separator.
     */
    void setCommandSeparatorString(const std::string &commandSeparatorString);

private:
    explicit GeneratorProfile(Profile profile = Profile::C); /**< Constructor, @private. */

    struct GeneratorProfileImpl;
    GeneratorProfileImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
