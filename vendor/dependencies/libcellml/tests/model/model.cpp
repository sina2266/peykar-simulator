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

#include <libcellml>

#include "test_utils.h"

TEST(Model, setGetId)
{
    const std::string id = "modelID";
    libcellml::ModelPtr m = libcellml::Model::create();
    m->setId(id);
    EXPECT_EQ(id, m->id());

    m->removeId();
    EXPECT_EQ("", m->id());
}

TEST(Model, name)
{
    const std::string n = "name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"name\"/>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    m->setName(n);

    EXPECT_EQ(n, m->name());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    auto m2 = libcellml::Model::create(n);
    const std::string a2 = printer->printModel(m2);
    EXPECT_EQ(e, a2);
}

TEST(Model, removeName)
{
    const std::string n = "name";
    const std::string eName =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"name\"/>\n";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"/>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    m->setName(n);
    EXPECT_EQ(n, m->name());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(eName, a);

    m->removeName();
    EXPECT_EQ("", m->name());
    a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Model, invalidName)
{
    const std::string n = "invalid name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"invalid name\"/>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    m->setName(n);

    EXPECT_EQ(n, m->name());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Model, addComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    m->addComponent(c);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Model, addValidNamedComponent)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Model, addInvalidNamedComponent)
{
    const std::string in = "invalid name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"invalid name\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Model, addTwoNamedComponents)
{
    const std::string name1 = "component_1";
    const std::string name2 = "component_2";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component_1\"/>\n"
        "  <component name=\"component_2\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    c1->setName(name1);
    m->addComponent(c1);
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    m->addComponent(c2);
    // once the component is added, we should be able to change the handle to the component and have those changes
    // reflected in the model? Yes we are using shared pointers.
    c2->setName(name2); // so should this give an error? Nope

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Model, countComponents)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");

    EXPECT_EQ(size_t(0), m->componentCount());

    m->addComponent(c1);
    m->addComponent(c2);
    EXPECT_EQ(size_t(2), m->componentCount());
}

TEST(Model, containsComponent)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");

    EXPECT_FALSE(m->containsComponent("child1"));

    m->addComponent(c1);
    m->addComponent(c2);
    EXPECT_TRUE(m->containsComponent("child2"));
}

TEST(Model, removeComponent)
{
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child2\"/>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child2\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    m->addComponent(c1);
    m->addComponent(c2);

    EXPECT_EQ(size_t(2), m->componentCount());
    EXPECT_TRUE(m->removeComponent(0));
    EXPECT_EQ(size_t(1), m->componentCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);
    EXPECT_FALSE(m->removeComponent(1));

    m->addComponent(c1);

    // Remove the first occurrence of "child1".
    EXPECT_TRUE(m->removeComponent("child1"));
    EXPECT_EQ(size_t(1), m->componentCount());
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);

    // Expect no change to model.
    EXPECT_FALSE(m->removeComponent("child3"));
    EXPECT_EQ(size_t(1), m->componentCount());
}

TEST(Model, componentMethods)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"childA\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    c1->setName("child1");
    m->addComponent(c1);

    libcellml::ComponentPtr cA = m->component(0);
    cA->setName("childA");

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Using const version of overloaded method
    const libcellml::ComponentPtr cB = m->component(0);
    // Can do this as we just have a const pointer
    cB->setName("gus");
    EXPECT_EQ("gus", cB->name());

    EXPECT_EQ(nullptr, m->component(4));
}

TEST(Model, takeComponentMethods)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"/>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    m->addComponent(c1);
    m->addComponent(c2);

    libcellml::ComponentPtr c02 = m->takeComponent(1);
    EXPECT_EQ(size_t(1), m->componentCount());

    EXPECT_EQ(m->takeComponent(4), nullptr);

    EXPECT_EQ("child2", c02->name());
    EXPECT_EQ(nullptr, c02->parent());

    libcellml::ComponentPtr c01 = m->takeComponent("child1");
    EXPECT_NE(nullptr, c01);
    EXPECT_EQ(size_t(0), m->componentCount());

    EXPECT_EQ("child1", c01->name());
    EXPECT_EQ(nullptr, c01->parent());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Expect no change.
    EXPECT_EQ(size_t(0), m->componentCount());
    EXPECT_EQ(nullptr, m->takeComponent("child4"));
    EXPECT_EQ(size_t(0), m->componentCount());
}

static int count = 0;
class big_and_complicated
{
    // lots of complicated code
public:
    int id;
    big_and_complicated()
        : id(count + 101)
    {
        ++count;
    }

    ~big_and_complicated()
    {
        --count;
    }
};

struct structure
{
    structure()
        : m_data {new big_and_complicated {}}
    {
        std::cout << "structure constructor: " << m_data->id << std::endl;
    }

    structure(const structure &rhs)
        : m_data {new big_and_complicated {}}
    {
        std::cout << "structure copy constructor: " << rhs.m_data->id << std::endl;
        m_data->id = rhs.m_data->id;
    }

    structure(structure &&rhs) noexcept
        : m_data(rhs.m_data)
    {
        std::cout << "structure move constructor: " << m_data->id << std::endl;
        rhs.m_data = nullptr;
    }

    structure &operator=(structure rhs)
    {
        rhs.swap(*this);
        return *this;
    }

    void swap(structure &rhs) noexcept
    {
        std::swap(m_data, rhs.m_data);
    }

    ~structure()
    {
        std::cout << "structure destructor: ";
        if (m_data != nullptr) {
            std::cout << m_data->id << std::endl;
        } else {
            std::cout << std::endl;
        }
        delete m_data;
    }

private:
    big_and_complicated *m_data = nullptr;
};

TEST(Model, replaceComponent)
{
    const std::string e_orig =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child1\"/>\n"
        "  <component name=\"child2\"/>\n"
        "</model>\n";
    const std::string e_after =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child1\"/>\n"
        "  <component name=\"child3\"/>\n"
        "</model>\n";
    const std::string e_post =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child5\"/>\n"
        "  <component name=\"child3\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::ComponentPtr c4 = libcellml::Component::create();
    libcellml::ComponentPtr c5 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    c3->setName("child3");
    c4->setName("child4");
    c5->setName("child5");
    m->addComponent(c1);
    m->addComponent(c2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e_orig, a);

    // Attempt to replace non-existent component.
    EXPECT_FALSE(m->replaceComponent(5, c3));

    // Replace existing component.
    EXPECT_TRUE(m->replaceComponent(1, c3));
    EXPECT_EQ(m, c3->parent());

    a = printer->printModel(m);
    EXPECT_EQ(e_after, a);

    // Nothing happens when trying to replace a component that doesn't match
    // the given name.
    EXPECT_FALSE(m->replaceComponent("child5", c4));
    EXPECT_EQ(nullptr, c4->parent());

    EXPECT_TRUE(m->replaceComponent("child1", c4));
    EXPECT_EQ(m, c4->parent());

    EXPECT_TRUE(m->replaceComponent(c4, c5));
    EXPECT_EQ(m, c5->parent());

    a = printer->printModel(m);
    EXPECT_EQ(e_post, a);
}

TEST(Model, constructors)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"my_name\">\n"
        "  <component/>\n"
        "</model>\n";
    const std::string n = "my_name";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ModelPtr m1;
    libcellml::ModelPtr m2;
    m->setName(n);
    m->addComponent(libcellml::Component::create());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);

    EXPECT_EQ(e, a);

    // Testing copy constructor
    libcellml::ModelPtr &m3(m);
    EXPECT_EQ("my_name", m3->name());

    // Testing model assignment
    m1 = m;
    EXPECT_EQ("my_name", m->name());

    // Testing move assignment for model
    m2 = std::move(m1);
    EXPECT_EQ("my_name", m2->name());

    // Testing move constructor for component
    libcellml::ModelPtr m4 = std::move(m2);
    EXPECT_EQ("my_name", m4->name());
}

TEST(Model, setAndCheckIdsAllEntities)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"mname\" id=\"mid\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\" id=\"i1id\">\n"
        "    <component component_ref=\"a_component_in_that_model\" name=\"c1name\" id=\"c1id\"/>\n"
        "  </import>\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-different-model.xml\" id=\"i2id\">\n"
        "    <units units_ref=\"a_units_in_that_model\" name=\"u1name\" id=\"u1id\"/>\n"
        "  </import>\n"
        "  <units name=\"u2name\" id=\"u2id\"/>\n"
        "  <units name=\"u3name\" id=\"u3id\"/>\n"
        "  <component name=\"c2name\" id=\"c2id\">\n"
        "    <variable name=\"vname\" units=\"u1name\" id=\"vid\"/>\n"
        "    <reset id=\"r1id\">\n"
        "      <test_value id=\"tvid\"/>\n"
        "      <reset_value id=\"rvid\"/>\n"
        "    </reset>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr i1 = libcellml::ImportSource::create();
    libcellml::ImportSourcePtr i2 = libcellml::ImportSource::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::VariablePtr v = libcellml::Variable::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    libcellml::ResetPtr r1 = libcellml::Reset::create();

    i1->setUrl("some-other-model.xml");
    c1->setSourceComponent(i1, "a_component_in_that_model");

    i2->setUrl("some-other-different-model.xml");
    u1->setSourceUnits(i2, "a_units_in_that_model");

    m->setName("mname");
    c1->setName("c1name");
    c2->setName("c2name");
    v->setName("vname");
    u1->setName("u1name");
    u2->setName("u2name");
    u3->setName("u3name");

    m->setId("mid");
    i1->setId("i1id");
    i2->setId("i2id");
    c1->setId("c1id");
    c2->setId("c2id");
    v->setId("vid");
    u1->setId("u1id");
    u2->setId("u2id");
    u3->setId("u3id");
    r1->setId("r1id");

    r1->setTestValueId("tvid");
    r1->setResetValueId("rvid");

    v->setUnits(u1);
    c2->addReset(r1);
    c2->addVariable(v);

    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);
    m->addComponent(c1);
    m->addComponent(c2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(a, e);
}

TEST(Model, equivalentVariableCountReportsCorrectlyAfterUsingRemoveComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <component name=\"c1\">\n"
        "    <variable name=\"v1\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"c2\">\n"
        "    <variable name=\"v2\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"c1\" component_2=\"c2\">\n"
        "    <map_variables variable_1=\"v1\" variable_2=\"v2\"/>\n"
        "  </connection>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(e);

    model->removeComponent("c1");

    EXPECT_EQ(size_t(0), model->component(0)->variable(0)->equivalentVariableCount());
    EXPECT_EQ(nullptr, model->component(0)->variable(0)->equivalentVariable(0));
}

TEST(Model, removeComponentInsensitiveToOrder)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"parsed_model\">\n"
        "  <component name=\"c1\">\n"
        "    <variable name=\"v1\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"c2\">\n"
        "    <variable name=\"v2\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto modelParsed = parser->parseModel(e);
    auto modelApi = libcellml::Model::create("api_model");

    // I want to move a component from modelParsed to modelApi
    auto c1Parsed = modelParsed->component("c1");
    modelApi->addComponent(c1Parsed);
    // Expect one component each
    EXPECT_EQ(size_t(1), modelParsed->componentCount());
    EXPECT_EQ(size_t(1), modelApi->componentCount());

    // Remove it from the parsed model: this does nothing because the parent pointer
    // has already been changed when it was added to the modelApi
    modelParsed->removeComponent(c1Parsed);

    // Still expect one component each
    EXPECT_EQ(size_t(1), modelParsed->componentCount());
    EXPECT_EQ(size_t(1), modelApi->componentCount());

    // If the order of operations is switched the behaviour is the same:

    // Get a pointer to the second component in the parsed model.
    auto c2Parsed = modelParsed->component("c2");
    // Remove it from the parsed model.
    modelParsed->removeComponent(c2Parsed);
    // Add it to the api model.
    modelApi->addComponent(c2Parsed);

    EXPECT_EQ(size_t(0), modelParsed->componentCount());
    EXPECT_EQ(size_t(2), modelApi->componentCount());
}

TEST(Model, cleanEmptyComponents)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"\">\n"
        "    <component component_ref=\"\" name=\"\"/>\n"
        "  </import>\n"
        "  <component name=\"c1\"/>\n"
        "  <component id=\"c2\"/>\n"
        "  <component>\n"
        "    <variable/>\n"
        "  </component>\n"
        "  <component>\n"
        "    <reset/>\n"
        "  </component>\n"
        "  <component>abc</component>\n"
        "</model>\n";
    auto model = libcellml::Model::create();
    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create();
    c2->setId("c2");
    auto c3 = libcellml::Component::create();
    auto v = libcellml::Variable::create();
    c3->addVariable(v);
    auto c4 = libcellml::Component::create();
    auto r = libcellml::Reset::create();
    c4->addReset(r);
    auto c5 = libcellml::Component::create();
    c5->setMath("abc");
    auto c6 = libcellml::Component::create();
    auto importSource = libcellml::ImportSource::create();
    c6->setImportSource(importSource);
    auto c7 = libcellml::Component::create();

    model->addComponent(c1);
    model->addComponent(c2);
    model->addComponent(c3);
    model->addComponent(c4);
    model->addComponent(c5);
    model->addComponent(c6);
    model->addComponent(c7);

    EXPECT_EQ(size_t(7), model->componentCount());
    // Call the Model::clean() function to remove empty component.
    model->clean();
    EXPECT_EQ(size_t(6), model->componentCount());
    // Check the correct component was cleaned.
    auto p = libcellml::Printer::create();
    EXPECT_EQ(e, p->printModel(model));
}

TEST(Model, cleanEmptyUnits)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"\">\n"
        "    <units units_ref=\"\" name=\"\"/>\n"
        "  </import>\n"
        "  <units name=\"u1\"/>\n"
        "  <units id=\"u2\"/>\n"
        "  <units>\n"
        "    <unit units=\"u4\"/>\n"
        "  </units>\n"
        "</model>\n";
    auto model = libcellml::Model::create();
    auto u1 = libcellml::Units::create("u1");
    auto u2 = libcellml::Units::create();
    u2->setId("u2");
    auto u3 = libcellml::Units::create();
    auto importSource = libcellml::ImportSource::create();
    u3->setImportSource(importSource);
    auto u4 = libcellml::Units::create();
    u4->addUnit("u4");
    auto u5 = libcellml::Units::create();
    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);
    model->addUnits(u4);
    model->addUnits(u5);

    EXPECT_EQ(size_t(5), model->unitsCount());
    // Call the Model::clean() function to remove empty components and units.
    model->clean();
    EXPECT_EQ(size_t(4), model->unitsCount());
    // Check the correct units is being cleaned.
    auto p = libcellml::Printer::create();
    EXPECT_EQ(e, p->printModel(model));
}

TEST(Model, cleanEmptyComponentEncapsulation)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "  <component name=\"c2\"/>\n"
        "  <component/>\n"
        "  <component name=\"c4\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref>\n"
        "      <component_ref component=\"c2\"/>\n"
        "      <component_ref>\n"
        "        <component_ref component=\"c4\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto model = libcellml::Model::create();
    auto c1 = libcellml::Component::create();
    auto c2 = libcellml::Component::create("c2");
    auto c3 = libcellml::Component::create();
    auto c4 = libcellml::Component::create("c4");
    auto c5 = libcellml::Component::create();
    auto c6 = libcellml::Component::create();

    model->addComponent(c1);
    c1->addComponent(c2);
    c1->addComponent(c3);
    c3->addComponent(c4);
    c3->addComponent(c5);
    c5->addComponent(c6);

    // Call the Model::clean() function to remove empty components.
    model->clean();
    auto p = libcellml::Printer::create();
    EXPECT_EQ(e, p->printModel(model));
}

TEST(Model, cleanModel)
{
    // Make a model with empty components and empty import sources.
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"dirtyModel\">\n"
        "  <units name=\"namedEmptyUnits\"/>\n"
        "  <units id=\"nonEmptyId\"/>\n"
        "  <component name=\"namedEmptyComponent\"/>\n"
        "  <component name=\"nonEmptyComponent\">\n"
        "    <variable name=\"x\" units=\"requiredUnits\"/>\n"
        "  </component>\n"
        "  <component id=\"nonEmptyComponentId\"/>\n"
        "</model>\n";
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("dirty_model.cellml"));
    auto printer = libcellml::Printer::create();

    EXPECT_EQ(size_t(4), model->componentCount());
    EXPECT_EQ(size_t(3), model->unitsCount());

    // Call the Model::clean() function to remove empty components and units.
    model->clean();

    EXPECT_EQ(size_t(3), model->componentCount());
    EXPECT_EQ(size_t(2), model->unitsCount());
    EXPECT_EQ(e, printer->printModel(model));
}

TEST(Model, cleanEncapsulatedModel)
{
    // Make a model with empty components and empty import sources.
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"dirtyModel\">\n"
        "  <component name=\"component\"/>\n"
        "  <component name=\"emptyChildComponent\"/>\n"
        "  <component name=\"nonEmptyComponent\">\n"
        "    <variable name=\"x\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"component\">\n"
        "      <component_ref component=\"emptyChildComponent\"/>\n"
        "      <component_ref component=\"nonEmptyComponent\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("dirty_encapsulated_model.cellml"));
    auto printer = libcellml::Printer::create();

    model->clean();

    EXPECT_EQ(e, printer->printModel(model));
}

libcellml::ModelPtr commonSetupImportedUnits()
{
    auto model = libcellml::Model::create();
    auto myConcreteUnits = libcellml::Units::create("myConcreteUnits");
    auto myImportedUnits = libcellml::Units::create("myImportedUnits");

    auto import = libcellml::ImportSource::create();
    import->setUrl("import.cellml");
    myImportedUnits->setImportSource(import);

    model->addUnits(myConcreteUnits);
    model->addUnits(myImportedUnits);

    EXPECT_TRUE(model->units("myImportedUnits")->isImport());
    EXPECT_FALSE(model->units("myConcreteUnits")->isImport());

    EXPECT_EQ(size_t(2), model->unitsCount());

    return model;
}

TEST(Model, removeImportedUnitsByName)
{
    auto model = commonSetupImportedUnits();

    EXPECT_TRUE(model->removeUnits("myConcreteUnits"));
    EXPECT_TRUE(model->removeUnits("myImportedUnits"));

    EXPECT_EQ(size_t(0), model->unitsCount());
}

TEST(Model, removeImportedUnitsByIndex)
{
    auto model = commonSetupImportedUnits();

    EXPECT_TRUE(model->removeUnits(0));
    EXPECT_TRUE(model->removeUnits(0));

    EXPECT_EQ(size_t(0), model->unitsCount());
}

TEST(Model, removeImportedUnitsByReference)
{
    auto model = commonSetupImportedUnits();
    auto myConcreteUnits = model->units(0);
    auto myImportedUnits = model->units(1);

    EXPECT_TRUE(model->removeUnits(myConcreteUnits));
    EXPECT_TRUE(model->removeUnits(myImportedUnits));

    EXPECT_EQ(size_t(0), model->unitsCount());
}

TEST(Model, takeImportedUnitsByName)
{
    auto model = commonSetupImportedUnits();

    auto takeUnits1 = model->takeUnits("myImportedUnits");
    auto takeUnits2 = model->takeUnits("myConcreteUnits");

    EXPECT_EQ(size_t(0), model->unitsCount());
}

TEST(Model, takeImportedUnitsByIndex)
{
    auto model = commonSetupImportedUnits();

    auto takeUnits1 = model->takeUnits(0);
    auto takeUnits2 = model->takeUnits(0);

    EXPECT_EQ(size_t(0), model->unitsCount());
}

TEST(Model, replaceImportedUnitsByName)
{
    auto model = libcellml::Model::create();
    auto myConcreteUnits1 = libcellml::Units::create("myConcreteUnits1");
    auto myImportedUnits1 = libcellml::Units::create("myImportedUnits1");
    auto myConcreteUnits2 = libcellml::Units::create("myConcreteUnits2");
    auto myImportedUnits2 = libcellml::Units::create("myImportedUnits2");

    auto import1 = libcellml::ImportSource::create();
    auto import2 = libcellml::ImportSource::create();

    import1->setUrl("import1.cellml");
    import2->setUrl("import2.cellml");

    myImportedUnits1->setImportSource(import1);
    myImportedUnits2->setImportSource(import2);

    model->addUnits(myConcreteUnits1);
    model->addUnits(myImportedUnits2);

    EXPECT_EQ(size_t(2), model->unitsCount());

    // REPLACE concrete -> imported by name.
    EXPECT_TRUE(model->replaceUnits("myConcreteUnits1", myImportedUnits1));
    EXPECT_EQ(size_t(2), model->unitsCount());

    // REPLACE imported -> concrete by name.
    EXPECT_TRUE(model->replaceUnits("myImportedUnits2", myConcreteUnits2));
    EXPECT_EQ(size_t(2), model->unitsCount());
}

TEST(Model, replaceImportedUnitsByIndex)
{
    auto model = libcellml::Model::create();
    auto myConcreteUnits1 = libcellml::Units::create("myConcreteUnits1");
    auto myImportedUnits1 = libcellml::Units::create("myImportedUnits1");
    auto myConcreteUnits2 = libcellml::Units::create("myConcreteUnits2");
    auto myImportedUnits2 = libcellml::Units::create("myImportedUnits2");

    auto import1 = libcellml::ImportSource::create();
    auto import2 = libcellml::ImportSource::create();

    import1->setUrl("import1.cellml");
    import2->setUrl("import2.cellml");

    myImportedUnits1->setImportSource(import1);
    myImportedUnits2->setImportSource(import2);

    model->addUnits(myConcreteUnits1);
    model->addUnits(myImportedUnits2);

    EXPECT_EQ(size_t(2), model->unitsCount());

    // REPLACE concrete -> imported by index.
    EXPECT_TRUE(model->replaceUnits(0, myImportedUnits1));
    EXPECT_EQ(size_t(2), model->unitsCount());

    // REPLACE imported -> concrete by index.
    EXPECT_TRUE(model->replaceUnits(1, myConcreteUnits2));
    EXPECT_EQ(size_t(2), model->unitsCount());
}

TEST(Model, replaceImportedUnitsByReference)
{
    auto model = libcellml::Model::create();
    auto myConcreteUnits1 = libcellml::Units::create("myConcreteUnits1");
    auto myImportedUnits1 = libcellml::Units::create("myImportedUnits1");
    auto myConcreteUnits2 = libcellml::Units::create("myConcreteUnits2");
    auto myImportedUnits2 = libcellml::Units::create("myImportedUnits2");

    auto import1 = libcellml::ImportSource::create();
    auto import2 = libcellml::ImportSource::create();

    import1->setUrl("import1.cellml");
    import2->setUrl("import2.cellml");

    myImportedUnits1->setImportSource(import1);
    myImportedUnits2->setImportSource(import2);

    model->addUnits(myConcreteUnits1);
    model->addUnits(myImportedUnits2);

    EXPECT_EQ(size_t(2), model->unitsCount());

    // REPLACE concrete -> imported by reference.
    EXPECT_TRUE(model->replaceUnits(myConcreteUnits1, myImportedUnits1));
    EXPECT_EQ(size_t(2), model->unitsCount());

    // REPLACE imported -> concrete by reference.
    EXPECT_TRUE(model->replaceUnits(myImportedUnits2, myConcreteUnits2));

    EXPECT_EQ(size_t(2), model->unitsCount());
}

libcellml::ModelPtr commonSetupImportedComponent()
{
    auto model = libcellml::Model::create();
    auto myConcreteComponent = libcellml::Component::create("myConcreteComponent");
    auto myImportedComponent = libcellml::Component::create("myImportedComponent");

    auto import = libcellml::ImportSource::create();
    import->setUrl("import.cellml");
    myImportedComponent->setImportSource(import);

    model->addComponent(myConcreteComponent);
    model->addComponent(myImportedComponent);

    EXPECT_TRUE(model->component("myImportedComponent")->isImport());
    EXPECT_FALSE(model->component("myConcreteComponent")->isImport());

    EXPECT_EQ(size_t(2), model->componentCount());

    return model;
}

TEST(Model, removeImportedComponentByName)
{
    auto model = commonSetupImportedComponent();

    EXPECT_TRUE(model->removeComponent("myConcreteComponent"));
    EXPECT_TRUE(model->removeComponent("myImportedComponent"));

    EXPECT_EQ(size_t(0), model->componentCount());
}

TEST(Model, removeImportedComponentByIndex)
{
    auto model = commonSetupImportedComponent();

    EXPECT_TRUE(model->removeComponent(0));
    EXPECT_TRUE(model->removeComponent(0));

    EXPECT_EQ(size_t(0), model->componentCount());
}

TEST(Model, removeImportedComponentByReference)
{
    auto model = commonSetupImportedComponent();
    auto myConcreteComponent = model->component(0);
    auto myImportedComponent = model->component(1);

    EXPECT_TRUE(model->removeComponent(myConcreteComponent));
    EXPECT_TRUE(model->removeComponent(myImportedComponent));

    EXPECT_EQ(size_t(0), model->componentCount());
}

TEST(Model, takeImportedComponentByName)
{
    auto model = commonSetupImportedComponent();

    auto takeComponent1 = model->takeComponent("myImportedComponent");
    auto takeComponent2 = model->takeComponent("myConcreteComponent");

    EXPECT_EQ(size_t(0), model->componentCount());
}

TEST(Model, takeImportedComponentByIndex)
{
    auto model = commonSetupImportedComponent();

    auto takeComponent1 = model->takeComponent(0);
    auto takeComponent2 = model->takeComponent(0);

    EXPECT_EQ(size_t(0), model->componentCount());
}

TEST(Model, replaceImportedComponentByName)
{
    auto model = libcellml::Model::create();
    auto myConcreteComponent1 = libcellml::Component::create("myConcreteComponent1");
    auto myImportedComponent1 = libcellml::Component::create("myImportedComponent1");
    auto myConcreteComponent2 = libcellml::Component::create("myConcreteComponent2");
    auto myImportedComponent2 = libcellml::Component::create("myImportedComponent2");

    auto import1 = libcellml::ImportSource::create();
    auto import2 = libcellml::ImportSource::create();

    import1->setUrl("import1.cellml");
    import2->setUrl("import2.cellml");

    myImportedComponent1->setImportSource(import1);
    myImportedComponent2->setImportSource(import2);

    model->addComponent(myConcreteComponent1);
    model->addComponent(myImportedComponent2);

    EXPECT_EQ(size_t(2), model->componentCount());

    // REPLACE concrete -> imported by name.
    EXPECT_TRUE(model->replaceComponent("myConcreteComponent1", myImportedComponent1));
    EXPECT_EQ(size_t(2), model->componentCount());

    // REPLACE imported -> concrete by name.
    EXPECT_TRUE(model->replaceComponent("myImportedComponent2", myConcreteComponent2));
    EXPECT_EQ(size_t(2), model->componentCount());
}

TEST(Model, replaceImportedComponentByIndex)
{
    auto model = libcellml::Model::create();
    auto myConcreteComponent1 = libcellml::Component::create("myConcreteComponent1");
    auto myImportedComponent1 = libcellml::Component::create("myImportedComponent1");
    auto myConcreteComponent2 = libcellml::Component::create("myConcreteComponent2");
    auto myImportedComponent2 = libcellml::Component::create("myImportedComponent2");

    auto import1 = libcellml::ImportSource::create();
    auto import2 = libcellml::ImportSource::create();

    import1->setUrl("import1.cellml");
    import2->setUrl("import2.cellml");

    myImportedComponent1->setImportSource(import1);
    myImportedComponent2->setImportSource(import2);

    model->addComponent(myConcreteComponent1);
    model->addComponent(myImportedComponent2);

    EXPECT_EQ(size_t(2), model->componentCount());

    // REPLACE concrete -> imported by index.
    EXPECT_TRUE(model->replaceComponent(0, myImportedComponent1));
    EXPECT_EQ(size_t(2), model->componentCount());

    // REPLACE imported -> concrete by index.
    EXPECT_TRUE(model->replaceComponent(1, myConcreteComponent2));
    EXPECT_EQ(size_t(2), model->componentCount());
}

TEST(Model, replaceImportedComponentByReference)
{
    auto model = libcellml::Model::create();
    auto myConcreteComponent1 = libcellml::Component::create("myConcreteComponent1");
    auto myImportedComponent1 = libcellml::Component::create("myImportedComponent1");
    auto myConcreteComponent2 = libcellml::Component::create("myConcreteComponent2");
    auto myImportedComponent2 = libcellml::Component::create("myImportedComponent2");

    auto import1 = libcellml::ImportSource::create();
    auto import2 = libcellml::ImportSource::create();

    import1->setUrl("import1.cellml");
    import2->setUrl("import2.cellml");

    myImportedComponent1->setImportSource(import1);
    myImportedComponent2->setImportSource(import2);

    model->addComponent(myConcreteComponent1);
    model->addComponent(myImportedComponent2);

    EXPECT_EQ(size_t(2), model->componentCount());

    // REPLACE concrete -> imported by index.
    EXPECT_TRUE(model->replaceComponent(myConcreteComponent1, myImportedComponent1));
    EXPECT_EQ(size_t(2), model->componentCount());

    // REPLACE imported -> concrete by index.
    EXPECT_TRUE(model->replaceComponent(myImportedComponent2, myConcreteComponent2));
    EXPECT_EQ(size_t(2), model->componentCount());
}

TEST(Model, removeAllComponentsImportedChild)
{
    auto model = libcellml::Model::create();
    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");
    auto import = libcellml::ImportSource::create();
    c2->setImportSource(import);
    model->addComponent(c2);
    model->addComponent(c1);
    EXPECT_EQ(size_t(2), model->componentCount());

    model->removeAllComponents();
    EXPECT_EQ(size_t(0), model->componentCount());
}
