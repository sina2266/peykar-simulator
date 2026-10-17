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

#include "libcellml/annotator.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

#include "libcellml/component.h"
#include "libcellml/importsource.h"
#include "libcellml/model.h"
#include "libcellml/reset.h"
#include "libcellml/types.h"
#include "libcellml/units.h"
#include "libcellml/variable.h"

#include "anycellmlelement_p.h"
#include "commonutils.h"
#include "internaltypes.h"
#include "issue_p.h"
#include "logger_p.h"
#include "utilities.h"

namespace libcellml {

using ItemList = std::multimap<std::string, AnyCellmlElementPtr>;

/**
 * @brief The Annotator::AnnotatorImpl class.
 *
 * This class is the private implementation class for the Annotator class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
class Annotator::AnnotatorImpl: public Logger::LoggerImpl
{
public:
    Annotator *mAnnotator = nullptr;
    ItemList mIdList;
    ModelWeakPtr mModel;
    size_t mCounter = 0xb4da55;
    size_t mHash = 0;

    AnyCellmlElementPtr convertToWeak(const AnyCellmlElementPtr &item);
    AnyCellmlElementPtr convertToShared(const AnyCellmlElementPtr &item);

    void listComponentIdsAndItems(const ComponentPtr &component, ItemList &idList);
    ItemList listIdsAndItems(const ModelPtr &model);

    void update();
    void buildIdList();

    size_t idCount();

    std::string makeUniqueId();

    std::string id(const AnyCellmlElementPtr &item);
    std::string setAutoId(const AnyCellmlElementPtr &item);
    bool isOwnedByModel(const AnyCellmlElementPtr &item) const;
    void removeId(const AnyCellmlElementPtr &item, const std::string &id);
    void setId(const AnyCellmlElementPtr &item, const std::string &id);
    bool itemsEqual(const AnyCellmlElementPtr &itemWeak, const AnyCellmlElementPtr &itemShared);
    bool validItem(const AnyCellmlElementPtr &item);

    void doSetAllAutomaticIds();
    void doSetModelIds();
    void doSetImportSourceIds();
    void doSetUnitsIds();
    void doSetUnitsItemIds();
    void doSetEncapsulationIds();
    void doClearComponentIds(const ComponentPtr &component);

    /**
     * @brief Set ids for a type that resides within the component tree.
     *
     * Set ids for a type that resides within the component tree.
     * If @p all is @c true it will override whatever is given for @p type
     * and set ids to all types.
     * If @p all is @c false only ids for the given @p type will be set.
     *
     * @param component The component in the tree to set ids to.
     * @param type The type of id to set.
     * @param all Set @c true to ignore the type and apply ids to all types.
     */
    void doSetComponentTreeTypeIds(const ComponentPtr &component, CellmlElementType type, bool all = false);

    /**
     * @brief Test to determine if the given @p id at the given @p index exists.
     *
     * Test to determine if the given @p id at the given @p index exists.  Returns @c true
     * if the item does exist at the @p index and @c false otherwise.
     *
     * Logs the following errors (if encountered):
     *  - missing model,
     *  - no id; or
     *  - duplicate ids.
     *
     * @param id A @c std::string representing the @p id to retrieve.
     * @param index The index of the item with @p id.
     * @param unique @c true if only considering unique existence, @c false otherwise.
     *
     * @return @c true if the @p id exists at @p index, @c false otherwise.
     */
    bool exists(const std::string &id, size_t index, bool unique = false);

    size_t generateHash();
    void doUpdateComponentHash(const ComponentPtr &component, std::string &idsString);

    void addIssueNoModel();
    void addIssueInvalidArgument(CellmlElementType type);
    void addIssueNotFound(const std::string &id);
    void addIssueNonUnique(const std::string &id);
};

Annotator::AnnotatorImpl *Annotator::pFunc()
{
    return reinterpret_cast<Annotator::AnnotatorImpl *>(Logger::pFunc());
}

const Annotator::AnnotatorImpl *Annotator::pFunc() const
{
    return reinterpret_cast<Annotator::AnnotatorImpl const *>(Logger::pFunc());
}

Annotator::Annotator()
    : Logger(new Annotator::AnnotatorImpl())
{
    pFunc()->mAnnotator = this;
    pFunc()->mIdList = std::multimap<std::string, AnyCellmlElementPtr>();
}

Annotator::~Annotator()
{
    delete pFunc();
}

AnnotatorPtr Annotator::create() noexcept
{
    return std::shared_ptr<Annotator> {new Annotator {}};
}

template<typename T, typename U>
inline bool equals(const std::weak_ptr<T> &t, const std::weak_ptr<U> &u)
{
    return !t.owner_before(u) && !u.owner_before(t);
}

void Annotator::AnnotatorImpl::listComponentIdsAndItems(const ComponentPtr &component, ItemList &idList)
{
    std::string id = component->id();
    if (!id.empty()) {
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setComponent(component);
        idList.insert(std::make_pair(id, convertToWeak(entry)));
    }
    // Imports.
    ImportSourcePtr importSource = component->importSource();
    if (importSource != nullptr) {
        id = importSource->id();
        if (!id.empty()) {
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setImportSource(importSource);
            idList.insert(std::make_pair(id, convertToWeak(entry)));
        }
    }
    // Component reference in encapsulation structure.
    id = component->encapsulationId();
    if (!id.empty()) {
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setComponentRef(component);
        idList.insert(std::make_pair(id, convertToWeak(entry)));
    }
    // Variables.
    for (size_t v = 0; v < component->variableCount(); ++v) {
        VariablePtr variable = component->variable(v);
        VariableWeakPtr weakVariable = variable;
        id = variable->id();
        if (!id.empty()) {
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setVariable(variable);
            idList.insert(std::make_pair(id, convertToWeak(entry)));
        }
        for (size_t e = 0; e < variable->equivalentVariableCount(); ++e) {
            // Equivalent variable mappings.
            VariablePtr equivalentVariable = variable->equivalentVariable(e);
            VariableWeakPtr weakEquivalentVariable = equivalentVariable;
            id = Variable::equivalenceMappingId(variable, equivalentVariable);
            if (!id.empty()) {
                // Check whether this connection has already been recorded.  Since connections could be recorded from either
                // side this is a separate check for duplicates.  We can't rely on the variables having names to set an order
                // either.
                bool found = false;
                if (idList.count(id) != 0) {
                    // Get the range of items with this identifier:
                    auto rangePair = idList.equal_range(id);
                    for (auto it = rangePair.first; it != rangePair.second; ++it) {
                        // Make sure it's also a MAP_VARIABLES item.
                        if (it->second->type() == CellmlElementType::MAP_VARIABLES) {
                            auto testPair = it->second->variablePair();
                            VariableWeakPtr variable1Weak = testPair->variable1();
                            VariableWeakPtr variable2Weak = testPair->variable2();
                            if (equals(variable1Weak, weakEquivalentVariable) && equals(variable2Weak, weakVariable)) {
                                found = true;
                            }
                        }
                    }
                }
                if (!found) {
                    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                    entry->mPimpl->setMapVariables(variable, equivalentVariable);
                    idList.insert(std::make_pair(id, convertToWeak(entry)));
                }
            }

            // Connections.
            id = Variable::equivalenceConnectionId(variable, equivalentVariable);
            if (!id.empty()) {
                // Check whether this connection has already been recorded.  Since connections could be recorded from either
                // side this is a separate check for duplicates.  We can't rely on the variables having names to set an order
                // either.
                bool found = false;
                if (idList.count(id) != 0) {
                    // Get the range of items with this identifier:
                    auto rangePair = idList.equal_range(id);
                    for (auto it = rangePair.first; it != rangePair.second; ++it) {
                        // Make sure it's also a CONNECTION item.
                        if (it->second->type() == CellmlElementType::CONNECTION) {
                            auto testPair = it->second->variablePair();
                            if ((owningComponent(testPair->variable1()) == owningComponent(equivalentVariable)) && (owningComponent(testPair->variable2()) == owningComponent(variable))) {
                                found = true;
                            } else if ((owningComponent(testPair->variable2()) == owningComponent(equivalentVariable)) && (owningComponent(testPair->variable1()) == owningComponent(variable))) {
                                found = true;
                            }
                        }
                    }
                }
                if (!found) {
                    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                    entry->mPimpl->setConnection(variable, equivalentVariable);
                    idList.insert(std::make_pair(id, convertToWeak(entry)));
                }
            }
        }
    }
    // Resets.
    for (size_t r = 0; r < component->resetCount(); ++r) {
        ResetPtr reset = component->reset(r);
        id = reset->id();
        if (!id.empty()) {
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setReset(reset);
            idList.insert(std::make_pair(id, convertToWeak(entry)));
        }
        id = reset->testValueId();
        if (!id.empty()) {
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setTestValue(reset);
            idList.insert(std::make_pair(id, convertToWeak(entry)));
        }
        id = reset->resetValueId();
        if (!id.empty()) {
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setResetValue(reset);
            idList.insert(std::make_pair(id, convertToWeak(entry)));
        }
    }

    for (size_t c = 0; c < component->componentCount(); ++c) {
        listComponentIdsAndItems(component->component(c), idList);
    }
}

ItemList Annotator::AnnotatorImpl::listIdsAndItems(const ModelPtr &model)
{
    // Collect all existing identifiers in a list and return.
    //    auto model = weakModel.lock();
    ItemList idList;
    // Model.
    std::string id = model->id();
    if (!id.empty()) {
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setModel(model);
        idList.insert(std::make_pair(id, convertToWeak(entry)));
    }

    // Units.
    for (size_t u = 0; u < model->unitsCount(); ++u) {
        UnitsPtr units = model->units(u);
        id = units->id();
        if (!id.empty()) {
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setUnits(units);
            idList.insert(std::make_pair(id, convertToWeak(entry)));
        }
        for (size_t i = 0; i < units->unitCount(); ++i) {
            std::string prefix;
            std::string reference;
            double exponent;
            double multiplier;
            units->unitAttributes(i, reference, prefix, exponent, multiplier, id);
            if (!id.empty()) {
                auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                entry->mPimpl->setUnitsItem(UnitsItem::create(units, i));
                idList.insert(std::make_pair(id, convertToWeak(entry)));
            }
        }
        if (units->isImport()) {
            ImportSourcePtr importSource = units->importSource();
            id = importSource->id();
            if (!id.empty()) {
                auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                entry->mPimpl->setImportSource(importSource);
                idList.insert(std::make_pair(id, convertToWeak(entry)));
            }
        }
    }

    // Components.
    for (size_t c = 0; c < model->componentCount(); ++c) {
        listComponentIdsAndItems(model->component(c), idList);
    }

    // Encapsulation.
    id = model->encapsulationId();
    if (!id.empty()) {
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setEncapsulation(model);
        idList.insert(std::make_pair(id, convertToWeak(entry)));
    }

    return idList;
}

AnyCellmlElementPtr Annotator::AnnotatorImpl::convertToWeak(const AnyCellmlElementPtr &item)
{
    auto converted = AnyCellmlElement::AnyCellmlElementImpl::create();
    auto type = item->type();

    converted->mPimpl->mType = type;

    switch (item->type()) {
    case CellmlElementType::COMPONENT:
    case CellmlElementType::COMPONENT_REF: {
        ComponentWeakPtr weakComponent = item->component();
        converted->mPimpl->mItem = weakComponent;
    } break;
    case CellmlElementType::CONNECTION:
    case CellmlElementType::MAP_VARIABLES:
        // We don't store a weak pointer for connections / map variables because
        // the map is the owner of the VariablePair object.
        converted->mPimpl->mItem = item->variablePair();
        break;
    case CellmlElementType::ENCAPSULATION:
    case CellmlElementType::MODEL: {
        ModelWeakPtr weakModel = item->model();
        converted->mPimpl->mItem = weakModel;
    } break;
    case CellmlElementType::IMPORT: {
        ImportSourceWeakPtr weakImportSource = item->importSource();
        converted->mPimpl->mItem = weakImportSource;
    } break;
    case CellmlElementType::RESET:
    case CellmlElementType::RESET_VALUE:
    case CellmlElementType::TEST_VALUE: {
        ResetWeakPtr weakReset = item->reset();
        converted->mPimpl->mItem = weakReset;
    } break;
    case CellmlElementType::UNIT:
        // We don't store a weak pointer for units item because the map is the
        // owner of the UnitsItem object.
        converted->mPimpl->mItem = item->unitsItem();
        break;
    case CellmlElementType::UNITS: {
        UnitsWeakPtr weakUnits = item->units();
        converted->mPimpl->mItem = weakUnits;
    } break;
    default: { /* CellmlElementType::VARIABLE */
        VariableWeakPtr weakVariable = item->variable();
        converted->mPimpl->mItem = weakVariable;
    } break;
    }

    return converted;
}

AnyCellmlElementPtr Annotator::AnnotatorImpl::convertToShared(const AnyCellmlElementPtr &item)
{
    auto converted = AnyCellmlElement::AnyCellmlElementImpl::create();
    auto type = item->type();

    ComponentPtr component;
    ImportSourcePtr importSource;
    ModelPtr model;
    ResetPtr reset;
    UnitsPtr units;
    UnitsItemPtr unitsItem;
    VariablePtr variable;
    VariablePairPtr variablePair;

    switch (item->type()) {
    case CellmlElementType::COMPONENT:
    case CellmlElementType::COMPONENT_REF:
        component = std::any_cast<ComponentWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setComponent(component, type);
        break;
    case CellmlElementType::CONNECTION:
    case CellmlElementType::MAP_VARIABLES:
        variablePair = item->variablePair();
        converted->mPimpl->setVariablePair(variablePair, type);
        break;
    case CellmlElementType::ENCAPSULATION:
    case CellmlElementType::MODEL:
        model = std::any_cast<ModelWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setModel(model, type);
        break;
    case CellmlElementType::IMPORT:
        importSource = std::any_cast<ImportSourceWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setImportSource(importSource);
        break;
    case CellmlElementType::RESET:
        reset = std::any_cast<ResetWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setReset(reset);
        break;
    case CellmlElementType::RESET_VALUE:
        reset = std::any_cast<ResetWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setResetValue(reset);
        break;
    case CellmlElementType::TEST_VALUE:
        reset = std::any_cast<ResetWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setTestValue(reset);
        break;
    case CellmlElementType::UNIT:
        unitsItem = item->unitsItem();
        converted->mPimpl->setUnitsItem(unitsItem);
        break;
    case CellmlElementType::UNITS:
        units = std::any_cast<UnitsWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setUnits(units);
        break;
    default: /* CellmlElementType::VARIABLE */
        variable = std::any_cast<VariableWeakPtr>(item->mPimpl->mItem).lock();
        converted->mPimpl->setVariable(variable);
        break;
    }

    return converted;
}

void Annotator::AnnotatorImpl::buildIdList()
{
    mIdList.clear();
    mIdList = listIdsAndItems(mModel.lock());
}

size_t Annotator::AnnotatorImpl::idCount()
{
    return mIdList.size();
}

void Annotator::AnnotatorImpl::update()
{
    removeAllIssues();
    size_t hash = generateHash();
    if (mHash != hash) {
        buildIdList();
        mHash = hash;
    }
}

void Annotator::setModel(const ModelPtr &model)
{
    pFunc()->mModel = model;
    pFunc()->mHash = 0;
    pFunc()->update();
}

void Annotator::AnnotatorImpl::addIssueNotFound(const std::string &id)
{
    auto issue = Issue::IssueImpl::create();
    issue->mPimpl->setDescription("Could not find an item with an identifier of '" + id + "' in the model.");
    issue->mPimpl->setLevel(Issue::Level::WARNING);
    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANNOTATOR_ID_NOT_FOUND);
    addIssue(issue);
}

void Annotator::AnnotatorImpl::addIssueNoModel()
{
    auto issue = Issue::IssueImpl::create();
    issue->mPimpl->setDescription("This Annotator object does not have a model to work with.");
    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANNOTATOR_NO_MODEL);
    addIssue(issue);
}

void Annotator::AnnotatorImpl::addIssueInvalidArgument(CellmlElementType type)
{
    auto issue = Issue::IssueImpl::create();
    auto description = "The item is internally inconsistent: the enum type '" + cellmlElementTypeAsString(type) + "' cannot be used with the stored item.";
    issue->mPimpl->setDescription(description);
    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANNOTATOR_INCONSISTENT_TYPE);
    addIssue(issue);
}

void Annotator::AnnotatorImpl::addIssueNonUnique(const std::string &id)
{
    auto issue = Issue::IssueImpl::create();
    issue->mPimpl->setDescription("The identifier '" + id + "' occurs " + std::to_string(mIdList.count(id)) + " times in the model so a unique item cannot be located.");
    issue->mPimpl->setLevel(Issue::Level::WARNING);
    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANNOTATOR_ID_NOT_UNIQUE);
    addIssue(issue);
}

bool Annotator::AnnotatorImpl::exists(const std::string &id, size_t index, bool unique)
{
    if (!mAnnotator->hasModel()) {
        addIssueNoModel();
        return false;
    }

    auto count = mAnnotator->itemCount(id);
    if (count == 1) {
        return true;
    }
    if (unique && count > 1) {
        addIssueNonUnique(id);
        return false;
    }
    if (count <= index) {
        addIssueNotFound(id);
        return false;
    }

    return true;
}

AnyCellmlElementPtr Annotator::item(const std::string &id)
{
    pFunc()->update();
    return pFunc()->exists(id, 0, true) ? std::move(items(id)[0]) : AnyCellmlElement::AnyCellmlElementImpl::create();
}

AnyCellmlElementPtr Annotator::item(const std::string &id, size_t index)
{
    pFunc()->update();
    return pFunc()->exists(id, index) ? std::move(items(id)[index]) : AnyCellmlElement::AnyCellmlElementImpl::create();
}

bool Annotator::isUnique(const std::string &id)
{
    pFunc()->update();
    return pFunc()->mIdList.count(id) == 1;
}

std::vector<AnyCellmlElementPtr> Annotator::items(const std::string &id)
{
    pFunc()->update();
    std::vector<AnyCellmlElementPtr> items;
    auto range = pFunc()->mIdList.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        items.push_back(pFunc()->convertToShared(it->second));
    }
    return items;
}

std::vector<std::string> Annotator::duplicateIds()
{
    pFunc()->update();
    std::vector<std::string> ids;
    for (auto i = pFunc()->mIdList.begin(), end = pFunc()->mIdList.end(); i != end; i = pFunc()->mIdList.upper_bound(i->first)) {
        auto next = i;
        ++next;
        if ((next != end) && (next->first == i->first)) {
            ids.push_back(i->first);
        }
    }
    return ids;
}

std::vector<std::string> Annotator::ids()
{
    pFunc()->update();
    std::vector<std::string> ids;
    for (auto i = pFunc()->mIdList.begin(), end = pFunc()->mIdList.end(); i != end; i = pFunc()->mIdList.upper_bound(i->first)) {
        ids.push_back(i->first);
    }
    return ids;
}

ComponentPtr Annotator::component(const std::string &id, size_t index)
{
    return item(id, index)->component();
}

ComponentPtr Annotator::component(const std::string &id)
{
    return item(id)->component();
}

ComponentPtr Annotator::componentEncapsulation(const std::string &id, size_t index)
{
    return item(id, index)->component();
}

ComponentPtr Annotator::componentEncapsulation(const std::string &id)
{
    return item(id)->component();
}

VariablePairPtr Annotator::connection(const std::string &id, size_t index)
{
    return item(id, index)->variablePair();
}

VariablePairPtr Annotator::connection(const std::string &id)
{
    return item(id)->variablePair();
}

ModelPtr Annotator::encapsulation(const std::string &id, size_t index)
{
    return item(id, index)->model();
}

ModelPtr Annotator::encapsulation(const std::string &id)
{
    return item(id)->model();
}

ImportSourcePtr Annotator::importSource(const std::string &id, size_t index)
{
    return item(id, index)->importSource();
}

ImportSourcePtr Annotator::importSource(const std::string &id)
{
    return item(id)->importSource();
}

VariablePairPtr Annotator::mapVariables(const std::string &id, size_t index)
{
    return item(id, index)->variablePair();
}

VariablePairPtr Annotator::mapVariables(const std::string &id)
{
    return item(id)->variablePair();
}

ModelPtr Annotator::model(const std::string &id, size_t index)
{
    return item(id, index)->model();
}

ModelPtr Annotator::model(const std::string &id)
{
    return item(id)->model();
}

ResetPtr Annotator::reset(const std::string &id, size_t index)
{
    return item(id, index)->reset();
}

ResetPtr Annotator::reset(const std::string &id)
{
    return item(id)->reset();
}

ResetPtr Annotator::resetValue(const std::string &id, size_t index)
{
    return item(id, index)->reset();
}

ResetPtr Annotator::resetValue(const std::string &id)
{
    return item(id)->reset();
}

ResetPtr Annotator::testValue(const std::string &id, size_t index)
{
    return item(id, index)->reset();
}

ResetPtr Annotator::testValue(const std::string &id)
{
    return item(id)->reset();
}

UnitsPtr Annotator::units(const std::string &id, size_t index)
{
    return item(id, index)->units();
}

UnitsPtr Annotator::units(const std::string &id)
{
    return item(id)->units();
}

UnitsItemPtr Annotator::unitsItem(const std::string &id, size_t index)
{
    return item(id, index)->unitsItem();
}

UnitsItemPtr Annotator::unitsItem(const std::string &id)
{
    return item(id)->unitsItem();
}

VariablePtr Annotator::variable(const std::string &id, size_t index)
{
    return item(id, index)->variable();
}

VariablePtr Annotator::variable(const std::string &id)
{
    return item(id)->variable();
}

void Annotator::clearAllIds()
{
    auto model = pFunc()->mModel.lock();
    if (model != nullptr) {
        pFunc()->update();
        model->removeId();
        for (size_t i = 0; i < model->unitsCount(); ++i) {
            auto units = model->units(i);
            units->removeId();
            if (units->isImport()) {
                units->importSource()->removeId();
            }
            for (size_t j = 0; j < units->unitCount(); ++j) {
                units->setUnitId(j, "");
            }
        }
        for (size_t i = 0; i < model->componentCount(); ++i) {
            pFunc()->doClearComponentIds(model->component(i));
        }
        model->removeEncapsulationId();

        pFunc()->mIdList.clear();
        pFunc()->mHash = 0;
    } else {
        pFunc()->addIssueNoModel();
    }
}

void Annotator::clearAllIds(ModelPtr &model)
{
    pFunc()->mModel = model;
    clearAllIds();
}

void Annotator::AnnotatorImpl::doClearComponentIds(const ComponentPtr &component)
{
    component->removeEncapsulationId();
    component->removeId();
    if (component->isImport()) {
        component->importSource()->removeId();
    }
    for (size_t i = 0; i < component->variableCount(); ++i) {
        auto variable = component->variable(i);
        variable->removeId();
        for (size_t j = 0; j < variable->equivalentVariableCount(); ++j) {
            Variable::setEquivalenceConnectionId(variable, variable->equivalentVariable(j), "");
            Variable::setEquivalenceMappingId(variable, variable->equivalentVariable(j), "");
        }
    }
    for (size_t i = 0; i < component->resetCount(); ++i) {
        auto reset = component->reset(i);
        reset->removeId();
        reset->removeResetValueId();
        reset->removeTestValueId();
    }
    for (size_t i = 0; i < component->componentCount(); ++i) {
        doClearComponentIds(component->component(i));
    }
}

bool Annotator::assignAllIds()
{
    auto model = pFunc()->mModel.lock();
    if (model != nullptr) {
        size_t initialSize = pFunc()->idCount();
        pFunc()->doSetAllAutomaticIds();
        return pFunc()->idCount() > initialSize;
    }
    pFunc()->addIssueNoModel();
    return false;
}

bool Annotator::assignAllIds(ModelPtr &model)
{
    if (model == nullptr) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("The Model supplied is a nullptr. No action has been taken.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ANNOTATOR_NULL_MODEL);
        return false;
    }
    setModel(model);
    return assignAllIds();
}

bool Annotator::assignIds(CellmlElementType type)
{
    auto model = pFunc()->mModel.lock();
    if (model == nullptr) {
        pFunc()->addIssueNoModel();
        return false;
    }

    size_t initialSize = pFunc()->idCount();

    switch (type) {
    case CellmlElementType::COMPONENT:
    case CellmlElementType::COMPONENT_REF:
    case CellmlElementType::CONNECTION:
    case CellmlElementType::MAP_VARIABLES:
    case CellmlElementType::RESET:
    case CellmlElementType::RESET_VALUE:
    case CellmlElementType::TEST_VALUE:
    case CellmlElementType::VARIABLE:
        for (size_t index = 0; index < model->componentCount(); ++index) {
            pFunc()->doSetComponentTreeTypeIds(model->component(index), type);
        }
        break;
    case CellmlElementType::ENCAPSULATION:
        pFunc()->doSetEncapsulationIds();
        break;
    case CellmlElementType::IMPORT:
        pFunc()->doSetImportSourceIds();
        break;
    case CellmlElementType::MODEL:
        pFunc()->doSetModelIds();
        break;
    case CellmlElementType::UNIT:
        pFunc()->doSetUnitsItemIds();
        break;
    case CellmlElementType::UNITS:
        pFunc()->doSetUnitsIds();
        break;
    case CellmlElementType::MATH:
    default: /* case CellmlElementType::UNDEFINED */
        break;
    }

    setModel(model);

    return pFunc()->idCount() > initialSize;
}

bool assignEncapsulationId(const ComponentPtr &component, CellmlElementType type, bool all)
{
    bool inHierarchy = std::dynamic_pointer_cast<libcellml::Model>(component->parent()) == nullptr || component->componentCount() > 0;
    return ((type == CellmlElementType::COMPONENT_REF) || all)
           && component->encapsulationId().empty()
           && inHierarchy;
}

void Annotator::AnnotatorImpl::doSetComponentTreeTypeIds(const ComponentPtr &component, CellmlElementType type, bool all)
{
    if (((type == CellmlElementType::COMPONENT) || all)
        && component->id().empty()) {
        auto id = makeUniqueId();
        component->setId(id);
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setComponent(component);
        mIdList.insert(std::make_pair(id, convertToWeak(entry)));
    }
    if (assignEncapsulationId(component, type, all)) {
        auto id = makeUniqueId();
        component->setEncapsulationId(id);
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setComponentRef(component);
        mIdList.insert(std::make_pair(id, convertToWeak(entry)));
    }
    if ((type == CellmlElementType::VARIABLE) || all) {
        for (size_t vIndex = 0; vIndex < component->variableCount(); ++vIndex) {
            auto v = component->variable(vIndex);
            if (v->id().empty()) {
                auto id = makeUniqueId();
                v->setId(id);
                auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                entry->mPimpl->setVariable(v);
                mIdList.insert(std::make_pair(id, convertToWeak(entry)));
            }
        }
    }
    for (size_t vIndex = 0; vIndex < component->variableCount(); ++vIndex) {
        auto v1 = component->variable(vIndex);
        for (size_t eIndex = 0; eIndex < v1->equivalentVariableCount(); ++eIndex) {
            auto v2 = v1->equivalentVariable(eIndex);
            if (((type == CellmlElementType::CONNECTION) || all)
                && Variable::equivalenceConnectionId(v1, v2).empty()) {
                auto id = makeUniqueId();
                Variable::setEquivalenceConnectionId(v1, v2, id);
                auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                entry->mPimpl->setConnection(v1, v2);
                mIdList.insert(std::make_pair(id, convertToWeak(entry)));
            }
            if (((type == CellmlElementType::MAP_VARIABLES) || all)
                && Variable::equivalenceMappingId(v1, v2).empty()) {
                auto id = makeUniqueId();
                Variable::setEquivalenceMappingId(v1, v2, id);
                auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                entry->mPimpl->setMapVariables(v1, v2);
                mIdList.insert(std::make_pair(id, convertToWeak(entry)));
            }
        }
    }
    for (size_t rIndex = 0; rIndex < component->resetCount(); ++rIndex) {
        auto r = component->reset(rIndex);
        if (((type == CellmlElementType::RESET) || all)
            && r->id().empty()) {
            auto id = makeUniqueId();
            r->setId(id);
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setReset(r);
            mIdList.insert(std::make_pair(id, convertToWeak(entry)));
        }
        if (((type == CellmlElementType::RESET_VALUE) || all)
            && r->resetValueId().empty()) {
            auto id = makeUniqueId();
            r->setResetValueId(id);
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setResetValue(r);
            mIdList.insert(std::make_pair(id, convertToWeak(entry)));
        }
        if (((type == CellmlElementType::TEST_VALUE) || all)
            && r->testValueId().empty()) {
            auto id = makeUniqueId();
            r->setTestValueId(id);
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setTestValue(r);
            mIdList.insert(std::make_pair(id, convertToWeak(entry)));
        }
    }

    for (size_t c = 0; c < component->componentCount(); ++c) {
        doSetComponentTreeTypeIds(component->component(c), type, all);
    }
}

void Annotator::AnnotatorImpl::doSetImportSourceIds()
{
    // Import items.
    auto model = mModel.lock();
    auto importSources = getAllImportSources(model);
    for (auto &importSource : importSources) {
        if (importSource->id().empty()) {
            auto id = makeUniqueId();
            importSource->setId(id);
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setImportSource(importSource);
            mIdList.insert(std::make_pair(id, convertToWeak(entry)));
        }
    }
}

void Annotator::AnnotatorImpl::doSetUnitsIds()
{
    // Units and unit items.
    auto model = mModel.lock();
    for (size_t u = 0; u < model->unitsCount(); ++u) {
        auto us = model->units(u);
        if (us->id().empty()) {
            auto id = makeUniqueId();
            us->setId(id);
            auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
            entry->mPimpl->setUnits(us);
            mIdList.insert(std::make_pair(id, convertToWeak(entry)));
        }
    }
}

void Annotator::AnnotatorImpl::doSetUnitsItemIds()
{
    auto model = mModel.lock();
    for (size_t u = 0; u < model->unitsCount(); ++u) {
        auto us = model->units(u);
        for (size_t i = 0; i < us->unitCount(); i++) {
            if (us->unitId(i).empty()) {
                auto id = makeUniqueId();
                us->setUnitId(i, id);
                auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
                entry->mPimpl->setUnitsItem(UnitsItem::create(us, i));
                mIdList.insert(std::make_pair(id, convertToWeak(entry)));
            }
        }
    }
}

void Annotator::AnnotatorImpl::doSetEncapsulationIds()
{
    auto model = mModel.lock();
    if (model->encapsulationId().empty()) {
        auto id = makeUniqueId();
        model->setEncapsulationId(id);
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setEncapsulation(model);
        mIdList.insert(std::make_pair(id, convertToWeak(entry)));
    }
}

void Annotator::AnnotatorImpl::doSetModelIds()
{
    auto model = mModel.lock();
    if (model->id().empty()) {
        auto id = makeUniqueId();
        model->setId(id);
        auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
        entry->mPimpl->setModel(model);
        mIdList.insert(std::make_pair(id, convertToWeak(entry)));
    }
}

void Annotator::AnnotatorImpl::doSetAllAutomaticIds()
{
    doSetModelIds();
    doSetImportSourceIds();
    doSetUnitsIds();
    doSetUnitsItemIds();
    auto model = mModel.lock();
    for (size_t index = 0; index < model->componentCount(); ++index) {
        auto component = model->component(index);
        doSetComponentTreeTypeIds(component, CellmlElementType::UNDEFINED, true);
    }
    doSetEncapsulationIds();
}

std::string Annotator::AnnotatorImpl::makeUniqueId()
{
    // Because the hexadecimal counter starts high enough that it will always have a letter as the first character,
    // we don't need to prefix it with any other string to be valid.  This is stored in the mCounter variable
    // so that "holes" in the automatic identifier list are not filled; they will always build from the previous
    // maximum value.

    std::stringstream stream;
    stream << std::hex << mCounter;
    std::string id = stream.str();
    stream.str(std::string());

    while (mIdList.count(id) != 0) {
        ++mCounter;
        stream << std::hex << mCounter;
        id = stream.str();
        stream.str(std::string());
    }
    // Note: this is only as unique as the information in mIdList permits.  That array must be updated
    // outside this function.
    return id;
}

std::string Annotator::AnnotatorImpl::id(const AnyCellmlElementPtr &item)
{
    std::string id;
    switch (item->type()) {
    case CellmlElementType::COMPONENT:
        id = item->component()->id();
        break;
    case CellmlElementType::COMPONENT_REF:
        id = item->component()->encapsulationId();
        break;
    case CellmlElementType::CONNECTION: {
        auto variablePair = item->variablePair();
        id = Variable::equivalenceConnectionId(variablePair->variable1(), variablePair->variable2());
    } break;
    case CellmlElementType::ENCAPSULATION:
        id = item->model()->encapsulationId();
        break;
    case CellmlElementType::IMPORT:
        id = item->importSource()->id();
        break;
    case CellmlElementType::MAP_VARIABLES: {
        auto variablePair = item->variablePair();
        id = Variable::equivalenceMappingId(variablePair->variable1(), variablePair->variable2());
    } break;
    case CellmlElementType::MODEL:
        id = item->model()->id();
        break;
    case CellmlElementType::RESET:
        id = item->reset()->id();
        break;
    case CellmlElementType::RESET_VALUE:
        id = item->reset()->resetValueId();
        break;
    case CellmlElementType::TEST_VALUE:
        id = item->reset()->testValueId();
        break;
    case CellmlElementType::UNIT: {
        auto unitItem = item->unitsItem();
        id = unitItem->units()->unitId(unitItem->index());
    } break;
    case CellmlElementType::UNITS:
        id = item->units()->id();
        break;
    default: /* CellmlElementType::VARIABLE */
        id = item->variable()->id();
        break;
    }
    return id;
}

void Annotator::AnnotatorImpl::setId(const AnyCellmlElementPtr &item, const std::string &id)
{
    switch (item->type()) {
    case CellmlElementType::COMPONENT:
        item->component()->setId(id);
        break;
    case CellmlElementType::COMPONENT_REF:
        item->component()->setEncapsulationId(id);
        break;
    case CellmlElementType::CONNECTION: {
        auto variablePair = item->variablePair();
        Variable::setEquivalenceConnectionId(variablePair->variable1(), variablePair->variable2(), id);
    } break;
    case CellmlElementType::ENCAPSULATION:
        item->model()->setEncapsulationId(id);
        break;
    case CellmlElementType::IMPORT:
        item->importSource()->setId(id);
        break;
    case CellmlElementType::MAP_VARIABLES: {
        auto variablePair = item->variablePair();
        Variable::setEquivalenceMappingId(variablePair->variable1(), variablePair->variable2(), id);
    } break;
    case CellmlElementType::MODEL:
        item->model()->setId(id);
        break;
    case CellmlElementType::RESET:
        item->reset()->setId(id);
        break;
    case CellmlElementType::RESET_VALUE:
        item->reset()->setResetValueId(id);
        break;
    case CellmlElementType::TEST_VALUE:
        item->reset()->setTestValueId(id);
        break;
    case CellmlElementType::UNIT: {
        auto unitsItem = item->unitsItem();
        unitsItem->units()->setUnitId(unitsItem->index(), id);
    } break;
    case CellmlElementType::UNITS:
        item->units()->setId(id);
        break;
    default: /* CellmlElementType::VARIABLE */
        item->variable()->setId(id);
        break;
    }
}

bool Annotator::AnnotatorImpl::isOwnedByModel(const AnyCellmlElementPtr &item) const
{
    auto model = mModel.lock();
    ModelPtr itemModel = nullptr;
    switch (item->type()) {
    case CellmlElementType::COMPONENT:
    case CellmlElementType::COMPONENT_REF:
        itemModel = owningModel(item->component());
        break;
    case CellmlElementType::CONNECTION:
    case CellmlElementType::MAP_VARIABLES: {
        auto variablePair = item->variablePair();
        if (model == owningModel(variablePair->variable1())) {
            itemModel = owningModel(variablePair->variable2());
        }
    } break;
    case CellmlElementType::ENCAPSULATION:
    case CellmlElementType::MODEL:
        itemModel = item->model();
        break;
    case CellmlElementType::IMPORT:
        itemModel = model;
        break;
    case CellmlElementType::RESET:
    case CellmlElementType::RESET_VALUE:
    case CellmlElementType::TEST_VALUE:
        itemModel = owningModel(item->reset());
        break;
    case CellmlElementType::UNIT:
        // Unit is not actually stored as a weak pointer so we can compare shared pointers directly.
        itemModel = owningModel(item->unitsItem()->units());
        break;
    case CellmlElementType::UNITS:
        itemModel = owningModel(item->units());
        break;
    default: /* CellmlElementType::VARIABLE */
        itemModel = owningModel(item->variable());
        break;
    }

    return itemModel == model;
}

bool Annotator::AnnotatorImpl::itemsEqual(const AnyCellmlElementPtr &itemWeak, const AnyCellmlElementPtr &itemShared)
{
    bool itemsEqual = false;
    auto item = convertToWeak(itemShared);
    switch (itemWeak->type()) {
    case CellmlElementType::COMPONENT:
    case CellmlElementType::COMPONENT_REF:
        itemsEqual = equals(std::any_cast<ComponentWeakPtr>(itemWeak->mPimpl->mItem),
                            std::any_cast<ComponentWeakPtr>(item->mPimpl->mItem));
        break;
    case CellmlElementType::CONNECTION:
    case CellmlElementType::MAP_VARIABLES:
        // Connections and map variables are not stored as a weak pointer so we can compare
        // shared pointers directly.
        itemsEqual = itemWeak->variablePair() == item->variablePair();
        break;
    case CellmlElementType::ENCAPSULATION:
    case CellmlElementType::MODEL:
        itemsEqual = equals(std::any_cast<ModelWeakPtr>(itemWeak->mPimpl->mItem),
                            std::any_cast<ModelWeakPtr>(item->mPimpl->mItem));
        break;
    case CellmlElementType::IMPORT:
        itemsEqual = equals(std::any_cast<ImportSourceWeakPtr>(itemWeak->mPimpl->mItem),
                            std::any_cast<ImportSourceWeakPtr>(item->mPimpl->mItem));
        break;
    case CellmlElementType::RESET:
    case CellmlElementType::RESET_VALUE:
    case CellmlElementType::TEST_VALUE:
        itemsEqual = equals(std::any_cast<ResetWeakPtr>(itemWeak->mPimpl->mItem),
                            std::any_cast<ResetWeakPtr>(item->mPimpl->mItem));
        break;
    case CellmlElementType::UNIT:
        // Unit is not actually stored as a weak pointer so we can compare shared pointers directly.
        itemsEqual = itemWeak->unitsItem() == itemShared->unitsItem();
        break;
    case CellmlElementType::UNITS:
        itemsEqual = equals(std::any_cast<UnitsWeakPtr>(itemWeak->mPimpl->mItem),
                            std::any_cast<UnitsWeakPtr>(item->mPimpl->mItem));
        break;
    default: /* CellmlElementType::VARIABLE */
        itemsEqual = equals(std::any_cast<VariableWeakPtr>(itemWeak->mPimpl->mItem),
                            std::any_cast<VariableWeakPtr>(item->mPimpl->mItem));
        break;
    }

    return itemsEqual;
}

bool Annotator::AnnotatorImpl::validItem(const AnyCellmlElementPtr &item)
{
    bool result = false;
    switch (item->type()) {
    case CellmlElementType::COMPONENT:
    case CellmlElementType::COMPONENT_REF:
        result = item->component() != nullptr;
        break;
    case CellmlElementType::CONNECTION:
    case CellmlElementType::MAP_VARIABLES: {
        auto variablePair = item->variablePair();
        result = (variablePair != nullptr)
                 && (variablePair->variable1() != nullptr)
                 && (variablePair->variable2() != nullptr);
    } break;
    case CellmlElementType::ENCAPSULATION:
    case CellmlElementType::MODEL:
        result = item->model() != nullptr;
        break;
    case CellmlElementType::IMPORT:
        result = item->importSource() != nullptr;
        break;
    case CellmlElementType::RESET:
    case CellmlElementType::RESET_VALUE:
    case CellmlElementType::TEST_VALUE:
        result = item->reset() != nullptr;
        break;
    case CellmlElementType::UNIT: {
        auto unitsItem = item->unitsItem();
        result = (unitsItem != nullptr) && (unitsItem->units() != nullptr);
    } break;
    case CellmlElementType::UNITS:
        result = item->units() != nullptr;
        break;
    default: /* CellmlElementType::VARIABLE */
        result = item->variable() != nullptr;
        break;
    }

    return result;
}

void Annotator::AnnotatorImpl::removeId(const AnyCellmlElementPtr &item, const std::string &id)
{
    auto range = mIdList.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        if ((it->second->type() == item->type()) && itemsEqual(it->second, item)) {
            mIdList.erase(it);
            break;
        }
    }
}

std::string Annotator::AnnotatorImpl::setAutoId(const AnyCellmlElementPtr &item)
{
    std::string newId;
    if (validItem(item)) {
        if (mModel.lock() != nullptr) {
            auto oldId = id(item);

            if (!isOwnedByModel(item)) {
                return newId;
            }

            update();
            newId = makeUniqueId();

            if (!oldId.empty()) {
                removeId(item, oldId);
            }

            setId(item, newId);
            mIdList.insert(std::make_pair(newId, convertToWeak(item)));
        } else {
            addIssueNoModel();
        }
    } else {
        addIssueInvalidArgument(item->type());
    }
    return newId;
}

std::string Annotator::assignId(const AnyCellmlElementPtr &item)
{
    return pFunc()->setAutoId(item);
}

std::string Annotator::assignId(const ModelPtr &model, CellmlElementType type)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setModel(model, type);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const ComponentPtr &component, CellmlElementType type)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setComponent(component, type);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const ResetPtr &reset, CellmlElementType type)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setReset(reset, type);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const ImportSourcePtr &importSource)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setImportSource(importSource);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const UnitsPtr &units)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setUnits(units);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const UnitsPtr &units, size_t index)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setUnitsItem(UnitsItem::create(units, index));
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const UnitsItemPtr &unitsItem)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setUnitsItem(unitsItem);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const VariablePtr &variable)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setVariable(variable);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const VariablePairPtr &pair, CellmlElementType type)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setVariablePair(pair, type);
    return pFunc()->setAutoId(entry);
}

std::string Annotator::assignId(const VariablePtr &variable1, const VariablePtr &variable2, CellmlElementType type)
{
    auto entry = AnyCellmlElement::AnyCellmlElementImpl::create();
    entry->mPimpl->setVariablePair(variable1, variable2, type);
    return pFunc()->setAutoId(entry);
}

size_t Annotator::itemCount(const std::string &id)
{
    pFunc()->update();
    return pFunc()->mIdList.count(id);
}

void Annotator::AnnotatorImpl::doUpdateComponentHash(const ComponentPtr &component, std::string &idsString)
{
    for (size_t i = 0; i < component->variableCount(); ++i) {
        idsString += "v=" + std::to_string(i) + component->variable(i)->id();
    }

    for (size_t i = 0; i < component->resetCount(); ++i) {
        auto reset = component->reset(i);
        idsString += "r=" + std::to_string(i) + reset->id() + "rv=" + reset->resetValueId() + "tv=" + reset->testValueId();
    }

    // Note that MathML identifiers are not yet included.

    for (size_t i = 0; i < component->componentCount(); ++i) {
        auto child = component->component(i);
        idsString += "c=" + std::to_string(i) + child->id() + "ce=" + child->encapsulationId();
        doUpdateComponentHash(child, idsString);
    }
}

size_t Annotator::AnnotatorImpl::generateHash()
{
    // Serialise the stored model into a (very) simplified string of identifier-based items, and create a hash.
    size_t hash = 0;
    auto model = mModel.lock();
    if (model != nullptr) {
        std::string idsString;
        size_t i;
        idsString += "m=" + model->id() + "me=" + model->encapsulationId();

        auto importSources = getAllImportSources(model);
        i = 0;
        for (auto &importSource : importSources) {
            idsString += "i=" + std::to_string(++i) + importSource->id();
        }

        for (i = 0; i < model->unitsCount(); ++i) {
            auto units = model->units(i);
            idsString += "U=" + std::to_string(i) + units->id();
            for (size_t j = 0; j < units->unitCount(); ++j) {
                idsString += "u=" + std::to_string(j) + units->unitId(j);
            }
        }

        for (i = 0; i < model->componentCount(); ++i) {
            auto component = model->component(i);
            idsString += "c=" + std::to_string(i) + component->id();
            idsString += "cr=" + std::to_string(i) + component->encapsulationId();
            doUpdateComponentHash(component, idsString);
        }

        hash = std::hash<std::string> {}(idsString);
    }
    return hash;
}

bool Annotator::hasModel() const
{
    return !pFunc()->mModel.expired();
}

ModelPtr Annotator::model() const
{
    return pFunc()->mModel.lock();
}

} // namespace libcellml
