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

#include "gtest/gtest.h"

#include "test_utils.h"
#include <libcellml>

TEST(UnitsImport, basics)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"a-model.xml\">\n"
        "    <units units_ref=\"bob\" name=\"\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("a-model.xml");

    libcellml::UnitsPtr u = libcellml::Units::create();

    EXPECT_EQ(u->importSource(), nullptr);
    EXPECT_EQ(u->importReference(), "");

    u->setImportSource(imp);
    u->setImportReference("bob");

    EXPECT_EQ(u->importSource(), imp);
    EXPECT_EQ(u->importReference(), "bob");

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(UnitsImport, importValidName)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <units units_ref=\"a_units_in_that_model\" name=\"units_in_this_model\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::UnitsPtr importedUnits = libcellml::Units::create();

    EXPECT_EQ(importedUnits->importSource(), nullptr);

    EXPECT_FALSE(importedUnits->isImport());

    importedUnits->setName("units_in_this_model");
    importedUnits->setSourceUnits(imp, "a_units_in_that_model");

    EXPECT_EQ(importedUnits->importSource(), imp);

    EXPECT_TRUE(importedUnits->isImport());

    m->addUnits(importedUnits);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(UnitsImport, importInvalidName)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <units units_ref=\"a units in that model\" name=\"units_in_this_model\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::UnitsPtr importedUnits = libcellml::Units::create();

    EXPECT_EQ(importedUnits->importSource(), nullptr);

    importedUnits->setName("units_in_this_model");
    importedUnits->setSourceUnits(imp, "a units in that model");

    EXPECT_EQ(importedUnits->importSource(), imp);

    m->addUnits(importedUnits);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(UnitsImport, nonExistentUrl)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://someplace.world/cellml/model.xml\">\n"
        "    <units units_ref=\"per_mole\" name=\"noble_per_mole\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("http://someplace.world/cellml/model.xml");

    libcellml::UnitsPtr importedUnits = libcellml::Units::create();

    EXPECT_EQ(importedUnits->importSource(), nullptr);

    importedUnits->setName("noble_per_mole");
    importedUnits->setSourceUnits(imp, "per_mole");

    EXPECT_EQ(importedUnits->importSource(), imp);

    EXPECT_EQ(size_t(0), m->unitsCount());
    m->addUnits(importedUnits);
    EXPECT_EQ(size_t(1), m->unitsCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(UnitsImport, noNameAttribute)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://someplace.world/cellml/model.xml\">\n"
        "    <units units_ref=\"per_mole\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();

    libcellml::ModelPtr m = p->parseModel(in);

    EXPECT_EQ(size_t(1), p->errorCount());
    EXPECT_EQ("Import of units does not specify a name attribute.", p->error(0)->description());
}

TEST(UnitsImport, importModifyAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <units units_ref=\"a_units_in_that_model\" name=\"units_in_this_model\"/>\n"
        "  </import>\n"
        "  <units name=\"multiplied_import\">\n"
        "    <unit multiplier=\"5.6\" units=\"units_in_this_model\"/>\n"
        "  </units>\n"
        "  <units name=\"prefixed_import\">\n"
        "    <unit prefix=\"yotta\" units=\"units_in_this_model\"/>\n"
        "  </units>\n"
        "  <units name=\"exponented_import\">\n"
        "    <unit exponent=\"3\" units=\"units_in_this_model\"/>\n"
        "  </units>\n"
        "  <units name=\"all_import\">\n"
        "    <unit exponent=\"-4\" multiplier=\"-1.3\" prefix=\"-17\" units=\"units_in_this_model\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::UnitsPtr importedUnits = libcellml::Units::create();
    importedUnits->setName("units_in_this_model");
    importedUnits->setSourceUnits(imp, "a_units_in_that_model");

    m->addUnits(importedUnits);

    libcellml::UnitsPtr importedUnitsMultiplied = libcellml::Units::create();
    importedUnitsMultiplied->setName("multiplied_import");
    importedUnitsMultiplied->addUnit("units_in_this_model", 0, 1.0, 5.6);

    m->addUnits(importedUnitsMultiplied);

    libcellml::UnitsPtr importedUnitsPrefixed = libcellml::Units::create();
    importedUnitsPrefixed->setName("prefixed_import");
    importedUnitsPrefixed->addUnit("units_in_this_model", libcellml::Units::Prefix::YOTTA);

    m->addUnits(importedUnitsPrefixed);

    libcellml::UnitsPtr importedUnitsExponented = libcellml::Units::create();
    importedUnitsExponented->setName("exponented_import");
    importedUnitsExponented->addUnit("units_in_this_model", 3.0);

    m->addUnits(importedUnitsExponented);

    libcellml::UnitsPtr importedUnitsAll = libcellml::Units::create();
    importedUnitsAll->setName("all_import");
    importedUnitsAll->addUnit("units_in_this_model", -17, -4.0, -1.3);

    m->addUnits(importedUnitsAll);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);

    // check units count
    EXPECT_EQ(size_t(5), model->unitsCount());
}
