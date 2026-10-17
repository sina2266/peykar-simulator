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

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

// To work around multiple inheritance we have to create a combined Units
// and ImportedEntity class that we can bind with Emscripten.
#define JAVASCRIPT_BINDINGS
#include "libcellml/units.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_units) {

    enum_<libcellml::Units::Prefix>("Units.Prefix")
        .value("YOTTA", libcellml::Units::Prefix::YOTTA)
        .value("ZETTA", libcellml::Units::Prefix::ZETTA)
        .value("EXA", libcellml::Units::Prefix::EXA)
        .value("PETA", libcellml::Units::Prefix::PETA)
        .value("TERA", libcellml::Units::Prefix::TERA)
        .value("GIGA", libcellml::Units::Prefix::GIGA)
        .value("MEGA", libcellml::Units::Prefix::MEGA)
        .value("KILO", libcellml::Units::Prefix::KILO)
        .value("HECTO", libcellml::Units::Prefix::HECTO)
        .value("DECA", libcellml::Units::Prefix::DECA)
        .value("DECI", libcellml::Units::Prefix::DECI)
        .value("CENTI", libcellml::Units::Prefix::CENTI)
        .value("MILLI", libcellml::Units::Prefix::MILLI)
        .value("MICRO", libcellml::Units::Prefix::MICRO)
        .value("NANO", libcellml::Units::Prefix::NANO)
        .value("PICO", libcellml::Units::Prefix::PICO)
        .value("FEMTO", libcellml::Units::Prefix::FEMTO)
        .value("ATTO", libcellml::Units::Prefix::ATTO)
        .value("ZEPTO", libcellml::Units::Prefix::ZEPTO)
        .value("YOCTO", libcellml::Units::Prefix::YOCTO)
    ;

    enum_<libcellml::Units::StandardUnit>("Units.StandardUnit")
        .value("AMPERE", libcellml::Units::StandardUnit::AMPERE)
        .value("BECQUEREL", libcellml::Units::StandardUnit::BECQUEREL)
        .value("CANDELA", libcellml::Units::StandardUnit::CANDELA)
        .value("COULOMB", libcellml::Units::StandardUnit::COULOMB)
        .value("DIMENSIONLESS", libcellml::Units::StandardUnit::DIMENSIONLESS)
        .value("FARAD", libcellml::Units::StandardUnit::FARAD)
        .value("GRAM", libcellml::Units::StandardUnit::GRAM)
        .value("GRAY", libcellml::Units::StandardUnit::GRAY)
        .value("HENRY", libcellml::Units::StandardUnit::HENRY)
        .value("HERTZ", libcellml::Units::StandardUnit::HERTZ)
        .value("JOULE", libcellml::Units::StandardUnit::JOULE)
        .value("KATAL", libcellml::Units::StandardUnit::KATAL)
        .value("KELVIN", libcellml::Units::StandardUnit::KELVIN)
        .value("KILOGRAM", libcellml::Units::StandardUnit::KILOGRAM)
        .value("LITRE", libcellml::Units::StandardUnit::LITRE)
        .value("LUMEN", libcellml::Units::StandardUnit::LUMEN)
        .value("LUX", libcellml::Units::StandardUnit::LUX)
        .value("METRE", libcellml::Units::StandardUnit::METRE)
        .value("MOLE", libcellml::Units::StandardUnit::MOLE)
        .value("NEWTON", libcellml::Units::StandardUnit::NEWTON)
        .value("OHM", libcellml::Units::StandardUnit::OHM)
        .value("PASCAL", libcellml::Units::StandardUnit::PASCAL)
        .value("RADIAN", libcellml::Units::StandardUnit::RADIAN)
        .value("SECOND", libcellml::Units::StandardUnit::SECOND)
        .value("SIEMENS", libcellml::Units::StandardUnit::SIEMENS)
        .value("SIEVERT", libcellml::Units::StandardUnit::SIEVERT)
        .value("STERADIAN", libcellml::Units::StandardUnit::STERADIAN)
        .value("TESLA", libcellml::Units::StandardUnit::TESLA)
        .value("VOLT", libcellml::Units::StandardUnit::VOLT)
        .value("WATT", libcellml::Units::StandardUnit::WATT)
        .value("WEBER", libcellml::Units::StandardUnit::WEBER)
    ;

    class_<libcellml::Units, base<libcellml::NamedEntity>>("Units")
        .smart_ptr<std::shared_ptr<libcellml::Units>>("UnitsPtr")
        .constructor(select_overload<libcellml::UnitsPtr()>(&libcellml::Units::create))
        .constructor(select_overload<libcellml::UnitsPtr(const std::string &)>(&libcellml::Units::create))
        .function("isBaseUnit", &libcellml::Units::isBaseUnit)
        .function("addUnitByReferenceStringPrefix", select_overload<void(const std::string &, const std::string &, double, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByReferenceEnumPrefix", select_overload<void(const std::string &, libcellml::Units::Prefix, double, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByReferenceIntPrefix", select_overload<void(const std::string &, int, double, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByReferenceExponent", select_overload<void(const std::string &, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByReference", select_overload<void(const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByStandardUnitStringPrefix", select_overload<void(libcellml::Units::StandardUnit, const std::string &, double, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByStandardUnitEnumPrefix", select_overload<void(libcellml::Units::StandardUnit, libcellml::Units::Prefix, double, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByStandardUnitIntPrefix", select_overload<void(libcellml::Units::StandardUnit, int, double, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByStandardUnitExponent", select_overload<void(libcellml::Units::StandardUnit, double, const std::string &)>(&libcellml::Units::addUnit))
        .function("addUnitByStandardUnit", select_overload<void(libcellml::Units::StandardUnit)>(&libcellml::Units::addUnit))
        .function("unitAttributeReference", &libcellml::Units::unitAttributeReference)
        .function("setUnitAttributeReference", &libcellml::Units::setUnitAttributeReference)
        .function("unitAttributePrefix", &libcellml::Units::unitAttributePrefix)
        .function("unitAttributeExponent", &libcellml::Units::unitAttributeExponent)
        .function("unitAttributeMultiplier", &libcellml::Units::unitAttributeMultiplier)
        .function("removeUnitByIndex", select_overload<bool(size_t)>(&libcellml::Units::removeUnit))
        .function("removeUnitByReference", select_overload<bool(const std::string &)>(&libcellml::Units::removeUnit))
        .function("removeUnitByStandardUnit", select_overload<bool(libcellml::Units::StandardUnit)>(&libcellml::Units::removeUnit))
        .function("removeAllUnits", &libcellml::Units::removeAllUnits)
        .function("setSourceUnits", &libcellml::Units::setSourceUnits)
        .function("unitCount", &libcellml::Units::unitCount)
        .function("requiresImports", &libcellml::Units::requiresImports)
        .function("clone", &libcellml::Units::clone)
        .function("setUnitId", &libcellml::Units::setUnitId)
        .function("unitId", &libcellml::Units::unitId)
        .function("isImport", &libcellml::Units::isImport)
        .function("importSource", &libcellml::Units::importSource)
        .function("setImportSource", &libcellml::Units::setImportSource)
        .function("importReference", &libcellml::Units::importReference)
        .function("setImportReference", &libcellml::Units::setImportReference)
        .function("isResolved", &libcellml::Units::isResolved)
        .function("isDefined", &libcellml::Units::isDefined)
        .class_function("scalingFactor", &libcellml::Units::scalingFactor)
        .class_function("compatible", &libcellml::Units::compatible)
        .class_function("equivalent", &libcellml::Units::equivalent)
    ;


    EM_ASM(
        Module['Units']['Prefix'] = Module['Units.Prefix'];
        delete Module['Units.Prefix'];
        Module['Units']['StandardUnit'] = Module['Units.StandardUnit'];
        delete Module['Units.StandardUnit'];
    );
}
