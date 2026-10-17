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

#include "test_utils.h"

#include "gtest/gtest.h"

#include <libcellml>

TEST(Component, construct)
{
    const std::string validName = "valid_name";

    libcellml::ComponentPtr c1 = libcellml::Component::create();
    c1->setName(validName);

    EXPECT_EQ("valid_name", c1->name());

    auto c2 = libcellml::Component::create(validName);
    EXPECT_EQ("valid_name", c2->name());
}

TEST(Component, validName)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\"/>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName(in);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("valid_name", c->name());
}

TEST(Component, invalidName)
{
    const std::string in = "invalid name -";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"invalid name -\"/>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName(in);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("invalid name -", c->name());
}

TEST(Component, setAndUnsetName)
{
    const std::string in = "name";
    const std::string eName =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"name\"/>\n"
        "</model>\n";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName(in);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ("name", c->name());
    EXPECT_EQ(eName, a);
    c->setName("");
    EXPECT_EQ("", c->name());
    a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Component, addAndCountChildren)
{
    libcellml::ComponentPtr parent = libcellml::Component::create();
    parent->setName("parent");
    libcellml::ComponentPtr child1 = libcellml::Component::create();
    child1->setName("child1");
    libcellml::ComponentPtr child2 = libcellml::Component::create();
    child2->setName("child2");
    libcellml::ComponentPtr child3 = libcellml::Component::create();
    child3->setName("child3");
    libcellml::ComponentPtr child4 = libcellml::Component::create();
    child4->setName("child4");

    EXPECT_EQ(size_t(0), parent->componentCount());

    parent->addComponent(child1);
    parent->addComponent(child2);
    parent->addComponent(child3);
    parent->addComponent(child4);
    EXPECT_EQ(size_t(4), parent->componentCount());

    child3->addComponent(child4);
    parent->addComponent(child3);
    EXPECT_EQ(size_t(4), parent->componentCount());

    EXPECT_EQ(size_t(1), child3->componentCount());
}

TEST(Component, addComponentToItself)
{
    libcellml::ComponentPtr component = libcellml::Component::create();
    component->setName("component");

    EXPECT_EQ(size_t(0), component->componentCount());

    EXPECT_FALSE(component->addComponent(component));
    EXPECT_EQ(size_t(0), component->componentCount());
}

TEST(Component, contains)
{
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c21 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    c2->addComponent(c21);

    EXPECT_FALSE(c->containsComponent("child1"));

    c->addComponent(c1);
    c->addComponent(c2);
    EXPECT_TRUE(c->containsComponent(c2));
    EXPECT_TRUE(c->containsComponent(c21));
}

TEST(Component, containsEncapsulatedComponent)
{
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName("child0");
    libcellml::ComponentPtr c1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr c2 = libcellml::Component::create("child2");
    libcellml::ComponentPtr c3 = libcellml::Component::create("child3");
    libcellml::ComponentPtr c4 = libcellml::Component::create("child4");
    libcellml::ComponentPtr c5 = libcellml::Component::create("child5");
    libcellml::ComponentPtr c6 = libcellml::Component::create("child6");

    c->addComponent(c1);
    c->addComponent(c2);
    c1->addComponent(c4);
    c1->addComponent(c5);
    c1->addComponent(c6);

    EXPECT_FALSE(m->containsComponent("child4", false));
    EXPECT_TRUE(m->containsComponent(c4, true));
}

TEST(Component, addChildrenAndSerialise)
{
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child0\"/>\n"
        "  <component name=\"child1\"/>\n"
        "  <component name=\"child2\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"child0\">\n"
        "      <component_ref component=\"child1\">\n"
        "        <component_ref component=\"child2\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"child0\"/>\n"
        "  <component name=\"child1\"/>\n"
        "  <component name=\"child2\"/>\n"
        "  <component name=\"child3\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"child0\">\n"
        "      <component_ref component=\"child1\">\n"
        "        <component_ref component=\"child2\">\n"
        "          <component_ref component=\"child3\"/>\n"
        "        </component_ref>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModel();
    libcellml::ComponentPtr c0 = libcellml::Component::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    c0->setName("child0");
    c3->setName("child3");

    c1->addComponent(c2);
    c0->addComponent(c1);

    c1->setName("child1");
    c2->setName("child2");

    m->addComponent(c0);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);

    c2->addComponent(c3);
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);
}

TEST(Component, removeComponentMethods)
{
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "  <component name=\"child2\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref>\n"
        "      <component_ref component=\"child2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "  <component name=\"child2\"/>\n"
        "  <component name=\"child1\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref>\n"
        "      <component_ref component=\"child2\"/>\n"
        "      <component_ref component=\"child1\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string e3 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    c->addComponent(c1);
    c->addComponent(c2);

    EXPECT_EQ(size_t(2), c->componentCount());
    EXPECT_TRUE(c->removeComponent(0));
    EXPECT_EQ(size_t(1), c->componentCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);
    // No longer a component at position 1.
    EXPECT_FALSE(c->removeComponent(1));

    // This will add three children 'c1'.
    c->addComponent(c1);
    c->addComponent(c1);
    c->addComponent(c1);
    // Remove the occurrence of "child1".
    EXPECT_TRUE(c->removeComponent("child1"));
    // Can remove a second occurrence of "child1".
    EXPECT_TRUE(c->removeComponent(c1));
    // Still have component 'c2', and one last 'c1'.
    EXPECT_EQ(size_t(2), c->componentCount());
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);

    // Expect no change
    EXPECT_FALSE(c->removeComponent("child3"));
    EXPECT_FALSE(c->removeComponent(c3));
    EXPECT_EQ(size_t(2), c->componentCount());

    c->removeAllComponents();
    a = printer->printModel(m);
    EXPECT_EQ(e3, a);
    EXPECT_EQ(size_t(0), c->componentCount());
}

TEST(Component, removeEncapsulatedComponent)
{
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName("child0");
    libcellml::ComponentPtr c1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr c2 = libcellml::Component::create("child2");
    libcellml::ComponentPtr c3 = libcellml::Component::create("child3");
    libcellml::ComponentPtr c4 = libcellml::Component::create("child4");
    libcellml::ComponentPtr c5 = libcellml::Component::create("child5");
    libcellml::ComponentPtr c6 = libcellml::Component::create("child6");

    c->addComponent(c1);
    c->addComponent(c2);
    c1->addComponent(c4);
    c1->addComponent(c5);
    c1->addComponent(c6);

    EXPECT_FALSE(m->removeComponent("child4", false));
    EXPECT_TRUE(m->removeComponent("child4", true));
    EXPECT_FALSE(m->removeComponent(c5, false));
    EXPECT_TRUE(m->removeComponent(c5, true));
}

TEST(Component, componentMethods)
{
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "  <component name=\"childA\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref>\n"
        "      <component_ref component=\"childA\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\"/>\n"
        "  <component name=\"gus\"/>\n"
        "  <component name=\"childB\"/>\n"
        "  <component name=\"child3\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref component=\"gus\">\n"
        "        <component_ref component=\"childB\"/>\n"
        "        <component_ref component=\"child3\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    c3->setName("child3");
    c->addComponent(c1);

    libcellml::ComponentPtr cA = c->component(0);
    cA->setName("childA");

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);

    // Using const version of overloaded method
    const libcellml::ComponentPtr cS = c->component(0);
    // Can do this as we just have a const pointer
    cS->setName("gus");
    EXPECT_EQ("gus", cS->name());
    EXPECT_EQ(nullptr, c->component(4));

    libcellml::ComponentPtr cAr = c->component("gus");
    EXPECT_EQ("gus", cAr->name());

    // Modify a deeper Component
    c->setName("parent");
    c1->addComponent(c2);
    c1->addComponent(c3);

    libcellml::ComponentPtr cB = c->component(0);
    libcellml::ComponentPtr cBB = cB->component(0);
    cBB->setName("childB");

    EXPECT_EQ(nullptr, c->component(3));

    libcellml::ComponentPtr cSn = c->component("gus");
    EXPECT_EQ("gus", cSn->name());

    EXPECT_EQ(nullptr, c->component("child1", false));

    a = printer->printModel(m);
    EXPECT_EQ(e2, a);
}

TEST(Component, takeComponentMethods)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    c1->setName("child1");
    c2->setName("child2");
    c->addComponent(c1);
    c->addComponent(c2);

    libcellml::ComponentPtr c02 = c->takeComponent(1);
    EXPECT_EQ(size_t(1), c->componentCount());

    EXPECT_EQ(c->takeComponent(4), nullptr);

    EXPECT_EQ("child2", c02->name());
    EXPECT_EQ(nullptr, c02->parent());

    libcellml::ComponentPtr c01 = c->takeComponent("child1");
    EXPECT_EQ(size_t(0), c->componentCount());

    EXPECT_EQ("child1", c01->name());
    EXPECT_EQ(nullptr, c01->parent());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Component, takeEncapsulatedComponent)
{
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName("child0");
    libcellml::ComponentPtr c1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr c2 = libcellml::Component::create("child2");
    libcellml::ComponentPtr c3 = libcellml::Component::create("child3");
    libcellml::ComponentPtr c4 = libcellml::Component::create("child4");
    libcellml::ComponentPtr c5 = libcellml::Component::create("child5");
    libcellml::ComponentPtr c6 = libcellml::Component::create("child6");

    c->addComponent(c1);
    c->addComponent(c2);
    c1->addComponent(c4);
    c1->addComponent(c5);
    c1->addComponent(c6);

    EXPECT_EQ(nullptr, m->takeComponent("child4", false));
    EXPECT_EQ(c4, m->takeComponent("child4", true));
}

TEST(Component, addComponentMultipleTimes)
{
    auto model = libcellml::Model::create("model");
    auto tomato = libcellml::Component::create("tomato");

    EXPECT_TRUE(model->addComponent(tomato));
    EXPECT_TRUE(model->addComponent(tomato));

    auto apple = model->takeComponent("tomato");
    apple->setName("apple");
    EXPECT_EQ("apple", model->component(0)->name());
    EXPECT_EQ("apple", tomato->name());
}

TEST(Component, replaceComponentMethods)
{
    const std::string e_orig =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\"/>\n"
        "  <component/>\n"
        "  <component name=\"child2\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref/>\n"
        "      <component_ref component=\"child2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string e_after =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\"/>\n"
        "  <component/>\n"
        "  <component name=\"child3\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref/>\n"
        "      <component_ref component=\"child3\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string e_post =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\"/>\n"
        "  <component name=\"child4\"/>\n"
        "  <component name=\"child3\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref component=\"child4\"/>\n"
        "      <component_ref component=\"child3\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::ComponentPtr c4 = libcellml::Component::create();

    c->setName("parent");
    c2->setName("child2");
    c3->setName("child3");
    c4->setName("child4");
    c->addComponent(c1);
    c->addComponent(c2);

    // Test replace by index
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e_orig, a);
    EXPECT_FALSE(c->replaceComponent(5, c3));

    EXPECT_TRUE(c->replaceComponent(1, c3));

    a = printer->printModel(m);
    EXPECT_EQ(e_after, a);

    // Test replace by name
    // Try to replace non-existent child.
    EXPECT_FALSE(c->replaceComponent("child5", c4));

    EXPECT_TRUE(c->replaceComponent("", c4));

    a = printer->printModel(m);
    EXPECT_EQ(e_post, a);

    // Test replace by ptr
    EXPECT_FALSE(c->replaceComponent(c1, c2));
    EXPECT_TRUE(c->replaceComponent(c4, c1));
    a = printer->printModel(m);
    EXPECT_EQ(e_after, a);
}

TEST(Component, replaceEncapsulatedComponent)
{
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName("child0");
    libcellml::ComponentPtr c1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr c2 = libcellml::Component::create("child2");
    libcellml::ComponentPtr c3 = libcellml::Component::create("child3");
    libcellml::ComponentPtr c4 = libcellml::Component::create("child4");
    libcellml::ComponentPtr c5 = libcellml::Component::create("child5");
    libcellml::ComponentPtr c6 = libcellml::Component::create("child6");

    c->addComponent(c1);
    c->addComponent(c2);
    c1->addComponent(c4);
    c1->addComponent(c5);
    c1->addComponent(c6);

    EXPECT_FALSE(m->replaceComponent("child4", c3, false));
    EXPECT_FALSE(m->replaceComponent(c4, c3, false));

    EXPECT_TRUE(m->replaceComponent("child4", c3));
    EXPECT_TRUE(m->replaceComponent(c6, c4));
}

TEST(Component, constructors)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"my_name\"/>\n"
        "  <component/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"my_name\">\n"
        "      <component_ref/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";
    const std::string n = "my_name";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::ComponentPtr c1;
    libcellml::ComponentPtr c2;

    c->setName(n);
    c->addComponent(libcellml::Component::create());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Testing assignment for component
    c1 = c;
    EXPECT_EQ("my_name", c1->name());

    // Testing move assignment for component
    c2 = std::move(c1);
    EXPECT_EQ("my_name", c2->name());
    // c1 is now in a valid but undefined state. For us this means we cannot use it, the pointer to implementation
    // is now a nullptr and anything that tries to dereference it will segfault, like name().

    // Testing move constructor for component
    libcellml::ComponentPtr c3 = std::move(c2);
    EXPECT_EQ("my_name", c3->name());
}

TEST(Component, onlyOneParentAtAnyGivenTime)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr parent = libcellml::Component::create();
    parent->setName("parent_component");
    libcellml::ComponentPtr child1 = libcellml::Component::create();
    child1->setName("child1");
    libcellml::ComponentPtr child2 = libcellml::Component::create();
    child2->setName("child2");
    libcellml::ComponentPtr child3 = libcellml::Component::create();
    child3->setName("child3");

    parent->addComponent(child1);
    parent->addComponent(child2);
    parent->addComponent(child3);
    model->addComponent(parent);

    EXPECT_EQ(size_t(3), parent->componentCount());
    EXPECT_EQ(size_t(0), child2->componentCount());

    child2->addComponent(child3);

    EXPECT_EQ(size_t(2), parent->componentCount());
    EXPECT_EQ(size_t(1), child2->componentCount());
}

TEST(Component, addVariableMultipleTimes)
{
    auto model = libcellml::Model::create("model");
    auto tomato = libcellml::Component::create("tomato");
    auto apple = libcellml::Component::create("apple");
    auto pip = libcellml::Variable::create("pip");

    EXPECT_TRUE(model->addComponent(tomato));
    EXPECT_TRUE(model->addComponent(apple));

    // Add a pip to the tomato.
    EXPECT_TRUE(tomato->addVariable(pip));

    // Try to add the same pip again.
    EXPECT_TRUE(tomato->addVariable(pip));

    // We have added another variable.
    EXPECT_EQ(size_t(2), tomato->variableCount());

    // Add the pip to the apple, which will effectively move it from the tomato
    // to the apple. But leave one pip in the tomato.
    EXPECT_TRUE(apple->addVariable(pip));
    EXPECT_EQ(size_t(1), tomato->variableCount());
    EXPECT_EQ(size_t(1), apple->variableCount());

    // check some names
    pip->setName("not_a_pip");
    EXPECT_EQ("not_a_pip", tomato->variable(0)->name());
    EXPECT_EQ("not_a_pip", apple->variable(0)->name());
}

TEST(Component, preventSegfaultInAddSomething)
{
    auto model = libcellml::Model::create("model");
    auto component = libcellml::Component::create("component");

    // Tests to show segfaults fixed when dodgy things are added to other things,
    // now they return false instead of segfault.
    EXPECT_FALSE(component->addVariable(nullptr));
    EXPECT_FALSE(component->addReset(nullptr));
    EXPECT_FALSE(component->addComponent(nullptr));
    EXPECT_FALSE(model->addComponent(nullptr));
    EXPECT_FALSE(model->addUnits(nullptr));
}

TEST(Component, isDefined)
{
    libcellml::ComponentPtr c = libcellml::Component::create("a_component");

    EXPECT_TRUE(c->isDefined());
}

TEST(Component, isNotDefinedWithUndefinedUnits)
{
    libcellml::ComponentPtr c = libcellml::Component::create("a_component");
    libcellml::VariablePtr v = libcellml::Variable::create("a_variable");
    libcellml::UnitsPtr u = libcellml::Units::create("improper_units");
    u->addUnit("battle");

    c->addVariable(v);
    v->setUnits(u);

    EXPECT_FALSE(c->isDefined());
}

TEST(Component, definedComponentImport)
{
    libcellml::ComponentPtr c = libcellml::Component::create("a_component");
    libcellml::ComponentPtr importComponent = libcellml::Component::create("import_component");
    libcellml::VariablePtr v = libcellml::Variable::create("a_variable");
    libcellml::UnitsPtr u = libcellml::Units::create("only_standard_units");

    libcellml::ImportSourcePtr importSource = libcellml::ImportSource::create();
    importSource->setUrl("I_am_a_url");

    importComponent->setImportSource(importSource);
    importComponent->setImportReference("ref");

    importComponent->addVariable(v);
    v->setUnits(u);

    c->addComponent(importComponent);

    EXPECT_FALSE(c->isDefined());
}
