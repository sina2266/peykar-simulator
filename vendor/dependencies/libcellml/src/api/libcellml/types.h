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

#include <memory>

#include "libcellml/enums.h"
#include "libcellml/exportdefinitions.h"

namespace libcellml {

// General classes.
class Analyser; /**< Forward declaration of Analyser class. */
using AnalyserPtr = std::shared_ptr<Analyser>; /**< Type definition for shared analyser pointer. */
class AnalyserEquation; /**< Forward declaration of AnalyserEquation class. */
using AnalyserEquationPtr = std::shared_ptr<AnalyserEquation>; /**< Type definition for shared analyser equation pointer. */
class AnalyserEquationAst; /**< Forward declaration of AnalyserEquationAst class. */
using AnalyserEquationAstPtr = std::shared_ptr<AnalyserEquationAst>; /**< Type definition for shared analyser equation AST pointer. */
class AnalyserExternalVariable; /**< Forward declaration of AnalyserExternalVariable class. */
using AnalyserExternalVariablePtr = std::shared_ptr<AnalyserExternalVariable>; /**< Type definition for shared analyser external variable pointer. */
class AnalyserModel; /**< Forward declaration of AnalyserModel class. */
using AnalyserModelPtr = std::shared_ptr<AnalyserModel>; /**< Type definition for shared analyser model pointer. */
class AnalyserVariable; /**< Forward declaration of AnalyserVariable class. */
using AnalyserVariablePtr = std::shared_ptr<AnalyserVariable>; /**< Type definition for shared analyser variable pointer. */
class Annotator; /**< Forward declaration of Annotator class. */
using AnnotatorPtr = std::shared_ptr<Annotator>; /**< Type definition for @c std::shared Annotator pointer. */

class AnyCellmlElement; /**< Forward declaration of AnyCellmlElement class. */
using AnyCellmlElementPtr = std::shared_ptr<AnyCellmlElement>; /**< Type definition for @c std::shared AnyCellmlElement pointer. */

class Generator; /**< Forward declaration of Generator class. */
using GeneratorPtr = std::shared_ptr<Generator>; /**< Type definition for shared generator pointer. */
class GeneratorVariableTracker; /**< Forward declaration of GeneratorVariableTracker class. */
using GeneratorVariableTrackerPtr = std::shared_ptr<GeneratorVariableTracker>; /**< Type definition for shared generator context pointer. */
class GeneratorProfile; /**< Forward declaration of GeneratorProfile class. */
using GeneratorProfilePtr = std::shared_ptr<GeneratorProfile>; /**< Type definition for shared generator variable pointer. */
class Importer; /**< Forward declaration of Importer class. */
using ImporterPtr = std::shared_ptr<Importer>; /**< Type definition for shared importer pointer. */
class Issue; /**< Forward declaration of Issue class. */
using IssuePtr = std::shared_ptr<Issue>; /**< Type definition for shared issue pointer. */
class Logger; /**< Forward declaration of Parser class. */
using LoggerPtr = std::shared_ptr<Logger>; /**< Type definition for shared parser pointer. */
class Parser; /**< Forward declaration of Parser class. */
using ParserPtr = std::shared_ptr<Parser>; /**< Type definition for shared parser pointer. */
class Printer; /**< Forward declaration of Printer class. */
using PrinterPtr = std::shared_ptr<Printer>; /**< Type definition for shared printer pointer. */
class Strict; /**< Forward declaration of Strict class. */
using StrictPtr = std::shared_ptr<Strict>; /**< Type definition for shared strict pointer. */
class Validator; /**< Forward declaration of Validator class. */
using ValidatorPtr = std::shared_ptr<Validator>; /**< Type definition for shared validator pointer. */

// CellML entities.
class Component; /**< Forward declaration of Component class. */
using ComponentPtr = std::shared_ptr<Component>; /**< Type definition for shared component pointer. */
class ComponentEntity; /**< Forward declaration of ComponentEntity class. */
using ComponentEntityPtr = std::shared_ptr<ComponentEntity>; /**< Type definition for shared component entity pointer. */
class Entity; /**< Forward declaration of Entity class. */
using EntityPtr = std::shared_ptr<Entity>; /**< Type definition for shared entity pointer. */
class ImportedEntity; /**< Forward declaration of ImportedEntity class. */
using ImportedEntityPtr = std::shared_ptr<ImportedEntity>; /**< Type definition for shared imported entity pointer. */
class ImportSource; /**< Forward declaration of ImportSource class. */
using ImportSourcePtr = std::shared_ptr<ImportSource>; /**< Type definition for shared import source pointer. */
class Model; /**< Forward declaration of Model class. */
using ModelPtr = std::shared_ptr<Model>; /**< Type definition for shared model pointer. */
class ParentedEntity; /**< Forward declaration of parented entity class. */
using ParentedEntityPtr = std::shared_ptr<ParentedEntity>; /**< Type definition for shared parented entity pointer. */
class Reset; /**< Forward declaration of Reset class. */
using ResetPtr = std::shared_ptr<Reset>; /**< Type definition for shared reset pointer. */
class UnitsItem; /**< Forward declaration of UnitsItem class. */
using UnitsItemPtr = std::shared_ptr<UnitsItem>; /**< Type definition for shared units item pointer. */
class Units; /**< Forward declaration of Units class. */
using UnitsPtr = std::shared_ptr<Units>; /**< Type definition for shared units pointer. */
class Variable; /**< Forward declaration of Variable class. */
using VariablePtr = std::shared_ptr<Variable>; /**< Type definition for shared variable pointer. */
class VariablePair; /**< Forward declaration of VariablePair class. */
using VariablePairPtr = std::shared_ptr<VariablePair>; /**< Type definition for shared variable pair pointer. */

/**
 * @brief The UnitsItem class
 *
 * The UnitsItem class contains a @ref Units to the parent Units item, and
 * the index to the @ref UnitsItem item within the @ref Units.
 */
class LIBCELLML_EXPORT UnitsItem
{
public:
    ~UnitsItem(); /**< Destructor. */
    UnitsItem() = delete; /**< Constructor, @private. */
    UnitsItem(const UnitsItem &rhs) = delete; /**< Copy constructor, @private. */
    UnitsItem(UnitsItem &&rhs) noexcept = delete; /**< Move constructor, @private. */
    UnitsItem &operator=(UnitsItem rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a units item reference object.
     *
     * Factory method to create a @ref UnitsItemPtr.  Create a units item with @ref Units
     * and index with::
     *
     *   auto unitsItem = libcellml::UnitsItemPtr::create(units, index);
     *
     * @return A smart pointer to a @ref UnitsItemPtr object.
     */
    static UnitsItemPtr create(const UnitsPtr &units, size_t index) noexcept;

    /**
     * @brief Get the unit.
     *
     * Get the unit in the @ref Units.
     *
     * @return The unit.
     */
    UnitsPtr units() const;

    /**
     * @brief Get the index.
     *
     * Get the index for the unit in the @ref Units.
     *
     * @return The index.
     */
    size_t index() const;

    /**
     * @brief Test to see if this unit reference is valid.
     *
     * Test to see if the @ref Units used in this unit reference exists and
     * has a unit at the given index.
     *
     * @return @c true if this unit reference is valid, @c false otherwise.
     */
    bool isValid() const;

private:
    explicit UnitsItem(const UnitsPtr &units, size_t index); /**< Constructor with two variables as parameters. */

    struct UnitsItemImpl; /**< Forward declaration for pImpl idiom, @private. */
    UnitsItemImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

/**
 * @brief The VariablePair class
 *
 * A VariablePair is a class containing two @ref Variable.
 * It may be used to define:
 *  - a connection between parent components; or
 *  - an equivalence between two variables;
 */
class LIBCELLML_EXPORT VariablePair
{
public:
    ~VariablePair(); /**< Destructor. */
    VariablePair() = delete; /**< Constructor, @private. */
    VariablePair(const VariablePair &rhs) = delete; /**< Copy constructor, @private. */
    VariablePair(VariablePair &&rhs) noexcept = delete; /**< Move constructor, @private. */
    VariablePair &operator=(VariablePair rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a variable pair object.
     *
     * Factory method to create a @ref VariablePairPtr.  Create a
     * variable pair with variable 1 and variable 2 with::
     *
     *   VariablePairPtr variablePair = libcellml::VariablePairPtr::create(variable1, variable2);
     *
     * @return A smart pointer to a @ref VariablePairPtr object.
     */
    static VariablePairPtr create(const VariablePtr &variable1, const VariablePtr &variable2) noexcept;

    /**
     * @brief Get the first variable in the pair.
     *
     * Get the first variable in the pair.
     *
     * @return The first variable in the pair.
     */
    VariablePtr variable1() const;

    /**
     * @brief Get the second variable in the pair.
     *
     * Get the second variable in the pair.
     *
     * @return The second variable in the pair.
     */
    VariablePtr variable2() const;

    /**
     * @brief Test to see if this variable pair is valid.
     *
     * Test to see if both the variables in this variable pair are
     * not @c nullptr.
     *
     * @return @c true if this variable pair is valid, @c false otherwise.
     */
    bool isValid() const;

private:
    explicit VariablePair(const VariablePtr &variable1, const VariablePtr &variable2); /**< Constructor with two variables as parameters. */

    struct VariablePairImpl; /**< Forward declaration for pImpl idiom, @private. */
    VariablePairImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

/**
 * @brief The AnyCellmlElement class
 *
 * The AnyCellmlElement class contains a @ref Model, @ref Component, etc.,
 * depending on the @ref CellmlElementType enum that describes which type is
 * stored.
 */
class LIBCELLML_EXPORT AnyCellmlElement
{
    friend class Analyser;
    friend class Annotator;
    friend class Importer;
    friend class Issue;
    friend class Parser;
    friend class Printer;
    friend class Validator;

public:
    ~AnyCellmlElement(); /**< Destructor, @private. */
    AnyCellmlElement(const AnyCellmlElement &rhs) = delete; /**< Copy constructor, @private. */
    AnyCellmlElement(AnyCellmlElement &&rhs) noexcept = delete; /**< Move constructor, @private. */
    AnyCellmlElement &operator=(AnyCellmlElement rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Get the @ref CellmlElementType.
     *
     * Get the @ref CellmlElementType.
     *
     * @return The type.
     */
    CellmlElementType type() const;

    /**
     * @brief Get the component.
     *
     * Get the component.
     *
     * @return The @ref Component, or @c nullptr if the internal type is not
     * @ref CellmlElementType::COMPONENT.
     */
    ComponentPtr component() const;

    /**
     * @brief Get the import source.
     *
     * Get the import source.
     *
     * @return The @ref ImportSource, or @c nullptr if the internal type is not
     * @ref CellmlElementType::IMPORT.
     */
    ImportSourcePtr importSource() const;

    /**
     * @brief Get the model.
     *
     * Get the model.
     *
     * @return The @ref Model, or @c nullptr if the internal type is not
     * @ref CellmlElementType::MODEL.
     */
    ModelPtr model() const;

    /**
     * @brief Get the reset.
     *
     * Get the reset.
     *
     * @return The @ref Reset, or @c nullptr if the internal type is not
     * @ref CellmlElementType::RESET.
     */
    ResetPtr reset() const;

    /**
     * @brief Get the units.
     *
     * Get the units.
     *
     * @return The @ref Units, or @c nullptr if the internal type is not
     * @ref CellmlElementType::UNITS.
     */
    UnitsPtr units() const;

    /**
     * @brief Get the units item.
     *
     * Get the units item.
     *
     * @return The @ref UnitsItem, or @c nullptr if the internal type is not
     * @ref CellmlElementType::UNIT.
     */
    UnitsItemPtr unitsItem() const;

    /**
     * @brief Get the variable.
     *
     * Get the variable.
     *
     * @return The @ref Variable, or @c nullptr if the internal type is not
     * @ref CellmlElementType::VARIABLE.
     */
    VariablePtr variable() const;

    /**
     * @brief Get the variable pair.
     *
     * Get the variable pair.
     *
     * @return The a @ref VariablePair, or @c nullptr if the
     * internal type is not @ref CellmlElementType::CONNECTION or @ref CellmlElementType::MAP_VARIABLES.
     */
    VariablePairPtr variablePair() const;

private:
    AnyCellmlElement(); /**< Constructor, @private. */

    struct AnyCellmlElementImpl; /**< Forward declaration for pImpl idiom. */
    AnyCellmlElementImpl *mPimpl; /**< Private member to implementation pointer. */
};

} // namespace libcellml
