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

#include "libcellml/units.h"

#include "libcellml/importsource.h"
#include "libcellml/model.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "commonutils.h"
#include "units_p.h"
#include "utilities.h"

namespace libcellml {

static const std::map<Units::Prefix, const std::string> prefixToString = {
    {Units::Prefix::ATTO, "atto"},
    {Units::Prefix::CENTI, "centi"},
    {Units::Prefix::DECA, "deca"},
    {Units::Prefix::DECI, "deci"},
    {Units::Prefix::EXA, "exa"},
    {Units::Prefix::FEMTO, "femto"},
    {Units::Prefix::GIGA, "giga"},
    {Units::Prefix::HECTO, "hecto"},
    {Units::Prefix::KILO, "kilo"},
    {Units::Prefix::MEGA, "mega"},
    {Units::Prefix::MICRO, "micro"},
    {Units::Prefix::MILLI, "milli"},
    {Units::Prefix::NANO, "nano"},
    {Units::Prefix::PETA, "peta"},
    {Units::Prefix::PICO, "pico"},
    {Units::Prefix::TERA, "tera"},
    {Units::Prefix::YOCTO, "yocto"},
    {Units::Prefix::YOTTA, "yotta"},
    {Units::Prefix::ZEPTO, "zepto"},
    {Units::Prefix::ZETTA, "zetta"}};

static const std::map<Units::StandardUnit, const std::string> standardUnitToString = {
    {Units::StandardUnit::AMPERE, "ampere"},
    {Units::StandardUnit::BECQUEREL, "becquerel"},
    {Units::StandardUnit::CANDELA, "candela"},
    {Units::StandardUnit::COULOMB, "coulomb"},
    {Units::StandardUnit::DIMENSIONLESS, "dimensionless"},
    {Units::StandardUnit::FARAD, "farad"},
    {Units::StandardUnit::GRAM, "gram"},
    {Units::StandardUnit::GRAY, "gray"},
    {Units::StandardUnit::HENRY, "henry"},
    {Units::StandardUnit::HERTZ, "hertz"},
    {Units::StandardUnit::JOULE, "joule"},
    {Units::StandardUnit::KATAL, "katal"},
    {Units::StandardUnit::KELVIN, "kelvin"},
    {Units::StandardUnit::KILOGRAM, "kilogram"},
    {Units::StandardUnit::LITRE, "litre"},
    {Units::StandardUnit::LUMEN, "lumen"},
    {Units::StandardUnit::LUX, "lux"},
    {Units::StandardUnit::METRE, "metre"},
    {Units::StandardUnit::MOLE, "mole"},
    {Units::StandardUnit::NEWTON, "newton"},
    {Units::StandardUnit::OHM, "ohm"},
    {Units::StandardUnit::PASCAL, "pascal"},
    {Units::StandardUnit::RADIAN, "radian"},
    {Units::StandardUnit::SECOND, "second"},
    {Units::StandardUnit::SIEMENS, "siemens"},
    {Units::StandardUnit::SIEVERT, "sievert"},
    {Units::StandardUnit::STERADIAN, "steradian"},
    {Units::StandardUnit::TESLA, "tesla"},
    {Units::StandardUnit::VOLT, "volt"},
    {Units::StandardUnit::WATT, "watt"},
    {Units::StandardUnit::WEBER, "weber"}};

std::vector<UnitDefinition>::const_iterator Units::UnitsImpl::findUnit(const std::string &reference) const
{
    return std::find_if(mUnitDefinitions.begin(), mUnitDefinitions.end(),
                        [=](const UnitDefinition &u) -> bool { return u.mReference == reference; });
}

Units::UnitsImpl *Units::pFunc()
{
    return reinterpret_cast<Units::UnitsImpl *>(Entity::pFunc());
}

const Units::UnitsImpl *Units::pFunc() const
{
    return reinterpret_cast<Units::UnitsImpl const *>(Entity::pFunc());
}

Units::Units()
    : NamedEntity(new Units::UnitsImpl())
{
    pFunc()->mUnits = this;
}

Units::Units(const std::string &name)
    : NamedEntity(new Units::UnitsImpl())
{
    pFunc()->mUnits = this;
    setName(name);
}

Units::~Units()
{
    delete pFunc();
}

bool Units::UnitsImpl::isBaseUnit(const std::string &name) const
{
    return name == "ampere" || name == "candela" || name == "dimensionless" || name == "kelvin" || name == "kilogram" || name == "metre" || name == "mole" || name == "second";
}

bool Units::UnitsImpl::isBaseUnitWithHistory(History &history, const UnitsConstPtr &units) const
{
    if (mUnits->isImport()) {
        ImportSourcePtr importedSource = mUnits->importSource();
        ModelPtr model = importedSource->model();
        if (model != nullptr) {
            auto h = createHistoryEpoch(units, importeeModelUrl(history, mUnits->importSource()->url()));
            if (checkForImportCycles(history, h)) {
                return false;
            }
            history.push_back(h);
            if (model->hasUnits(mUnits->importReference())) {
                auto importedUnits = model->units(mUnits->importReference());
                // Call isBaseUnit recursively until unit is no longer an import.
                return importedUnits->pFunc()->isBaseUnitWithHistory(history, importedUnits);
            }
        }
        return false;
    }

    auto unitsName = mUnits->name();
    bool standardUnitCheck = true;
    if (isStandardUnitName(unitsName)) {
        standardUnitCheck = isBaseUnit(unitsName);
    }
    return (mUnits->unitCount() == 0) && standardUnitCheck;
}

bool Units::UnitsImpl::performTestWithHistory(History &history, const UnitsConstPtr &units, TestType type) const
{
    ModelPtr model;
    if (mUnits->isImport()) {
        model = mUnits->importSource()->model();
        if (model == nullptr) {
            return false;
        }

        auto importedUnits = model->units(mUnits->importReference());
        if (importedUnits == nullptr) {
            return false;
        }

        auto h = createHistoryEpoch(units, importeeModelUrl(history, mUnits->importSource()->url()));
        if (checkForImportCycles(history, h)) {
            return false;
        }

        history.push_back(h);

        return importedUnits->pFunc()->performTestWithHistory(history, importedUnits, type);
    }

    model = std::dynamic_pointer_cast<libcellml::Model>(mUnits->parent());
    for (size_t unitIndex = 0; unitIndex < mUnits->unitCount(); ++unitIndex) {
        std::string reference = mUnits->unitAttributeReference(unitIndex);
        if (isStandardUnitName(reference)) {
            continue;
        }

        if (model != nullptr) {
            auto childUnits = model->units(reference);
            if (childUnits != nullptr) {
                if (!childUnits->pFunc()->performTestWithHistory(history, childUnits, type)) {
                    return false;
                }
            } else if (type == TestType::DEFINED) {
                return false;
            }
        } else if (type == TestType::DEFINED) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Finds and updates the multiplier of the unit.
 *
 * We pass in the unit and use its attributes to find the relevant multiplier.
 * If the units are not base units, we travel up the model hierarchy to find
 * the base units.
 *
 * @param units The units to find the multiplier for.
 * @param direction The direction to update multiplier. Either 1 or -1.
 * @param multiplier The multiplier to find.
 *
 * @return Either @c true or @c false, depending if the units were successfully updated.
 */
bool updateUnitMultiplier(const UnitsPtr &units, int direction, double &multiplier)
{
    double localMultiplier = 0;

    if (units->isImport()) {
        if (units->isResolved()) {
            auto importSource = units->importSource();
            auto importedUnits = importSource->model()->units(units->importReference());
            updateUnitMultiplier(importedUnits, 1, localMultiplier);
            multiplier += localMultiplier * direction;
        } else {
            return false;
        }
    } else if (units->unitCount() > 0) {
        std::string ref;
        std::string pre;
        std::string id;
        double exp;
        double mult;
        double expMult;
        double standardMult = 0.0;
        double prefixMult = 0.0;
        for (size_t i = 0; i < units->unitCount(); ++i) {
            units->unitAttributes(i, ref, pre, exp, expMult, id);
            mult = std::log10(expMult);

            bool ok;
            prefixMult = convertPrefixToInt(pre, &ok);
            if (!ok) {
                return false;
            }

            if (isStandardUnitName(ref)) {
                standardMult = standardMultiplierList.at(ref);
                // Combine the information into a single local multiplier: exponent applies to standard multiplier and prefix multiplier.
                localMultiplier += mult + (standardMult + prefixMult) * exp;
            } else {
                auto model = owningModel(units);
                auto refUnits = model->units(ref);
                if (refUnits == nullptr) {
                    return false;
                }
                double branchMult = 0.0;
                // Return false when we can't find a valid prefix.
                if (!updateUnitMultiplier(refUnits, 1, branchMult)) {
                    return false;
                }
                // Make the direction positive on all branches, direction is only applied at the end.
                localMultiplier += mult + branchMult * exp + prefixMult;
            }
        }
        multiplier += localMultiplier * direction;
    }

    return true;
}

UnitsPtr Units::create() noexcept
{
    return std::shared_ptr<Units> {new Units {}};
}

UnitsPtr Units::create(const std::string &name) noexcept
{
    return std::shared_ptr<Units> {new Units {name}};
}

bool Units::isBaseUnit() const
{
    History history;
    return pFunc()->isBaseUnitWithHistory(history, shared_from_this());
}

bool Units::doEquals(const EntityPtr &other) const
{
    if (!NamedEntity::doEquals(other)) {
        return false;
    }

    auto units = std::dynamic_pointer_cast<Units>(other);

    if ((units == nullptr)
        || (pFunc()->mUnitDefinitions.size() != units->unitCount())
        || !ImportedEntity::doEquals(units)) {
        return false;
    }

    std::string reference;
    std::string prefix;
    double exponent;
    double multiplier;
    std::string id;
    std::vector<size_t> unmatchedUnitIndex(pFunc()->mUnitDefinitions.size());

    std::iota(unmatchedUnitIndex.begin(), unmatchedUnitIndex.end(), 0);

    for (const auto &unitDefinition : pFunc()->mUnitDefinitions) {
        bool unitFound = false;
        size_t index = 0;

        for (index = 0; (index < unmatchedUnitIndex.size()) && !unitFound; ++index) {
            size_t currentIndex = unmatchedUnitIndex.at(index);

            units->unitAttributes(currentIndex, reference, prefix, exponent, multiplier, id);

            if (areNearlyEqual(unitDefinition.mExponent, exponent)
                && (unitDefinition.mId == id)
                && areNearlyEqual(unitDefinition.mMultiplier, multiplier)
                && (unitDefinition.mPrefix == prefix)
                && (unitDefinition.mReference == reference)) {
                unitFound = true;
            }
        }

        if (!unitFound) {
            return false;
        }

        unmatchedUnitIndex.erase(unmatchedUnitIndex.begin() + ptrdiff_t(index) - 1);
    }

    return true;
}

void Units::addUnit(const std::string &reference, const std::string &prefix, double exponent,
                    double multiplier, const std::string &id)
{
    UnitDefinition ud;
    ud.mReference = reference;

    // Allow all nonzero user-specified prefixes.
    if (isCellMLInteger(prefix)) {
        try {
            int prefixInteger = std::stoi(prefix);
            if (prefixInteger != 0) {
                ud.mPrefix = prefix;
            }
        } catch (std::out_of_range &) {
            ud.mPrefix = prefix;
        }
    } else {
        ud.mPrefix = prefix;
    }

    ud.mExponent = exponent;
    ud.mMultiplier = multiplier;
    ud.mId = id;

    pFunc()->mUnitDefinitions.push_back(ud);
}

void Units::addUnit(const std::string &reference, Prefix prefix, double exponent,
                    double multiplier, const std::string &id)
{
    addUnit(reference, prefixToString.at(prefix), exponent, multiplier, id);
}

void Units::addUnit(const std::string &reference, int prefix, double exponent,
                    double multiplier, const std::string &id)
{
    addUnit(reference, convertToString(prefix), exponent, multiplier, id);
}

void Units::addUnit(const std::string &reference, double exponent, const std::string &id)
{
    addUnit(reference, "0", exponent, 1.0, id);
}

void Units::addUnit(const std::string &reference)
{
    addUnit(reference, "0", 1.0, 1.0, "");
}

void Units::addUnit(StandardUnit standardUnit, const std::string &prefix, double exponent,
                    double multiplier, const std::string &id)
{
    addUnit(standardUnitToString.at(standardUnit), prefix, exponent, multiplier, id);
}

void Units::addUnit(StandardUnit standardUnit, Prefix prefix, double exponent,
                    double multiplier, const std::string &id)
{
    addUnit(standardUnitToString.at(standardUnit), prefixToString.at(prefix), exponent, multiplier, id);
}

void Units::addUnit(StandardUnit standardUnit, int prefix, double exponent,
                    double multiplier, const std::string &id)
{
    addUnit(standardUnitToString.at(standardUnit), convertToString(prefix), exponent, multiplier, id);
}

void Units::addUnit(StandardUnit standardUnit, double exponent, const std::string &id)
{
    addUnit(standardUnitToString.at(standardUnit), "0", exponent, 1.0, id);
}

void Units::addUnit(StandardUnit standardUnit)
{
    addUnit(standardUnitToString.at(standardUnit), "0", 1.0, 1.0, "");
}

void Units::unitAttributes(StandardUnit standardUnit, std::string &prefix, double &exponent, double &multiplier, std::string &id) const
{
    std::string dummyReference;
    unitAttributes(static_cast<size_t>(pFunc()->findUnit(standardUnitToString.at(standardUnit)) - pFunc()->mUnitDefinitions.begin()), dummyReference, prefix, exponent, multiplier, id);
}

void Units::unitAttributes(const std::string &reference, std::string &prefix, double &exponent, double &multiplier, std::string &id) const
{
    std::string dummyReference;
    unitAttributes(static_cast<size_t>(pFunc()->findUnit(reference) - pFunc()->mUnitDefinitions.begin()), dummyReference, prefix, exponent, multiplier, id);
}

void Units::unitAttributes(size_t index, std::string &reference, std::string &prefix, double &exponent, double &multiplier, std::string &id) const
{
    UnitDefinition ud;
    if (index < pFunc()->mUnitDefinitions.size()) {
        ud = pFunc()->mUnitDefinitions.at(index);
    }
    reference = ud.mReference;
    prefix = ud.mPrefix;
    exponent = ud.mExponent;
    multiplier = ud.mMultiplier;
    id = ud.mId;
}

std::string Units::unitAttributeReference(size_t index) const
{
    std::string ref;
    std::string pre;
    double exp;
    double mult;
    std::string id;
    unitAttributes(index, ref, pre, exp, mult, id);
    return ref;
}

void Units::setUnitAttributeReference(size_t index, const std::string &reference)
{
    if (index < pFunc()->mUnitDefinitions.size()) {
        UnitDefinition unitDefinition = pFunc()->mUnitDefinitions.at(index);
        unitDefinition.mReference = reference;
        pFunc()->mUnitDefinitions[index] = unitDefinition;
    }
}

std::string Units::unitAttributePrefix(size_t index) const
{
    std::string ref;
    std::string pre;
    double exp;
    double mult;
    std::string id;
    unitAttributes(index, ref, pre, exp, mult, id);
    return pre;
}

double Units::unitAttributeExponent(size_t index) const
{
    std::string ref;
    std::string pre;
    double exp;
    double mult;
    std::string id;
    unitAttributes(index, ref, pre, exp, mult, id);
    return exp;
}

double Units::unitAttributeMultiplier(size_t index) const
{
    std::string ref;
    std::string pre;
    double exp;
    double mult;
    std::string id;
    unitAttributes(index, ref, pre, exp, mult, id);
    return mult;
}

bool Units::setUnitId(size_t index, const std::string &id)
{
    if (index < pFunc()->mUnitDefinitions.size()) {
        pFunc()->mUnitDefinitions[index].mId = id;
        return true;
    }
    return false;
}

std::string Units::unitId(size_t index) const
{
    if (index < pFunc()->mUnitDefinitions.size()) {
        return pFunc()->mUnitDefinitions.at(index).mId;
    }
    return "";
}

bool Units::removeUnit(const std::string &reference)
{
    bool status = false;
    auto result = pFunc()->findUnit(reference);
    if (result != pFunc()->mUnitDefinitions.end()) {
        pFunc()->mUnitDefinitions.erase(result);
        status = true;
    }

    return status;
}

bool Units::removeUnit(size_t index)
{
    bool status = false;
    if (index < pFunc()->mUnitDefinitions.size()) {
        pFunc()->mUnitDefinitions.erase(pFunc()->mUnitDefinitions.begin() + ptrdiff_t(index));
        status = true;
    }

    return status;
}

bool Units::removeUnit(StandardUnit standardUnit)
{
    const std::string reference = standardUnitToString.find(standardUnit)->second;
    return removeUnit(reference);
}

void Units::removeAllUnits()
{
    pFunc()->mUnitDefinitions.clear();
}

void Units::setSourceUnits(ImportSourcePtr &importSource, const std::string &name)
{
    setImportSource(importSource);
    setImportReference(name);
}

size_t Units::unitCount() const
{
    return pFunc()->mUnitDefinitions.size();
}

double Units::scalingFactor(const UnitsPtr &units1, const UnitsPtr &units2, bool checkCompatibility)
{
    if (checkCompatibility && !Units::compatible(units1, units2)) {
        return 0.0;
    }

    bool updateUnits1 = false;
    bool updateUnits2 = false;

    if ((units1 != nullptr) && (units2 != nullptr)) {
        double multiplier = 0.0;
        updateUnits1 = updateUnitMultiplier(units1, -1, multiplier);
        updateUnits2 = updateUnitMultiplier(units2, 1, multiplier);

        if (updateUnits1 && updateUnits2) {
            return std::pow(10, multiplier);
        }
    }

    return 0.0;
}

using UnitsMap = std::map<std::string, double>;

void updateUnitsMapWithStandardUnit(const std::string &name, UnitsMap &unitsMap, double exp)
{
    for (const auto &baseUnitsComponent : standardUnitsList.at(name)) {
        auto unitsMapIter = unitsMap.find(baseUnitsComponent.first);
        if (unitsMapIter == unitsMap.end()) {
            unitsMap.emplace(baseUnitsComponent.first, 0.0);
        }
        unitsMap[baseUnitsComponent.first] += baseUnitsComponent.second * exp;
    }
}

void updateUnitsMap(const UnitsPtr &units, UnitsMap &unitsMap, double exp = 1.0)
{
    if (units->isBaseUnit()) {
        auto unitsName = units->name();
        auto found = unitsMap.find(unitsName);
        if (found == unitsMap.end()) {
            unitsMap.emplace(unitsName, exp);
        } else {
            found->second += exp;
        }
    } else if (isStandardUnit(units)) {
        updateUnitsMapWithStandardUnit(units->name(), unitsMap, exp);
    } else if (units->isImport()) {
        auto importSource = units->importSource();
        auto importedUnits = importSource->model()->units(units->importReference());
        updateUnitsMap(importedUnits, unitsMap);
    } else {
        for (size_t i = 0; i < units->unitCount(); ++i) {
            std::string ref;
            std::string pre;
            std::string id;
            double expMult;
            double uExp;
            units->unitAttributes(i, ref, pre, uExp, expMult, id);
            if (isStandardUnitName(ref)) {
                updateUnitsMapWithStandardUnit(ref, unitsMap, uExp * exp);
            } else {
                auto model = owningModel(units);
                auto refUnits = model->units(ref);
                updateUnitsMap(refUnits, unitsMap, uExp * exp);
            }
        }
    }
}

UnitsMap defineUnitsMap(const UnitsPtr &units)
{
    UnitsMap unitsMap;

    updateUnitsMap(units, unitsMap);

    // Checking for exponents of zero in the map, which can be removed.
    auto it = unitsMap.begin();
    while (it != unitsMap.end()) {
        if (it->second == 0.0) {
            it = unitsMap.erase(it);
        } else if (it->first == "dimensionless") {
            it = unitsMap.erase(it);
        } else {
            ++it;
        }
    }
    return unitsMap;
}

bool Units::requiresImports() const
{
    // Function to check child unit dependencies for imports.
    if (isImport()) {
        return true;
    }

    auto model = owningModel(shared_from_this());
    if (model != nullptr) {
        for (size_t u = 0; u < unitCount(); ++u) {
            const std::string ref = unitAttributeReference(u);
            auto child = model->units(ref);
            if ((child == nullptr) || (this == child.get())) {
                continue;
            }
            if (child->requiresImports()) {
                return true;
            }
        }
    }
    return false;
}

bool Units::compatible(const UnitsPtr &units1, const UnitsPtr &units2)
{
    // Initial checks.
    if ((units1 == nullptr) || (units2 == nullptr)) {
        return false;
    }
    if ((!units1->isDefined()) || (!units2->isDefined())) {
        return false;
    }

    UnitsMap units1Map = defineUnitsMap(units1);
    UnitsMap units2Map = defineUnitsMap(units2);

    if (units1Map.size() == units2Map.size()) {
        for (const auto &units : units1Map) {
            std::string unit = units.first;
            auto found = units2Map.find(unit);

            if (found == units2Map.end()) {
                return false;
            }
            if (!areEqual(found->second, units.second)) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool Units::equivalent(const UnitsPtr &units1, const UnitsPtr &units2)
{
    // Units must be compatible and return a scaling factor of 1.0.
    return Units::scalingFactor(units1, units2) == 1.0;
}

UnitsPtr Units::clone() const
{
    auto units = create();

    units->setId(id());
    units->setName(name());

    if (isImport()) {
        units->setImportSource(importSource());
    }

    units->setImportReference(importReference());

    std::string reference;
    std::string prefix;
    std::string id;
    double exponent;
    double multiplier;
    for (size_t index = 0; index < pFunc()->mUnitDefinitions.size(); ++index) {
        unitAttributes(index, reference, prefix, exponent, multiplier, id);
        units->addUnit(reference, prefix, exponent, multiplier, id);
    }

    return units;
}

bool Units::isDefined() const
{
    History history;
    return pFunc()->performTestWithHistory(history, shared_from_this(), TestType::DEFINED);
}

bool Units::doIsResolved() const
{
    History history;
    return pFunc()->performTestWithHistory(history, shared_from_this(), TestType::RESOLVED);
}

} // namespace libcellml
