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

#include <algorithm>
#include <string>
#include <vector>

#include <libxml/parser.h>

void structuredErrorCallback(void *userData, XML_ERROR_CALLBACK_ARGUMENT_TYPE error)
{
    if (userData != nullptr && error != nullptr) {
        // Suppress any error messages raised from using LibXml2.
    }
}

TEST(Parser, parseValidXmlDirectlyUsingLibxml)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"/>\n";

    // parse the string using libcellml
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(model);
    EXPECT_EQ(e, a);

    // and now parse directly using libxml2
    xmlInitParser();
    xmlParserCtxtPtr context = xmlNewParserCtxt();
    xmlSetStructuredErrorFunc(context, structuredErrorCallback);
    xmlDocPtr doc = xmlCtxtReadDoc(context, reinterpret_cast<const xmlChar *>(e.c_str()), "/", nullptr, 0);
    xmlFreeParserCtxt(context);
    EXPECT_NE(nullptr, doc);
    xmlFreeDoc(doc);
    xmlSetStructuredErrorFunc(nullptr, nullptr);
    xmlCleanupParser();
}

TEST(Parser, parseInvalidXmlDirectlyUsingLibxml)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"><component></model>";

    // parse the string using libcellml
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    EXPECT_NE(size_t(0), parser->issueCount());

    // and now parse directly using libxml2
    xmlInitParser();
    xmlParserCtxtPtr context = xmlNewParserCtxt();
    xmlSetStructuredErrorFunc(context, structuredErrorCallback);
    xmlDocPtr doc = xmlCtxtReadDoc(context, reinterpret_cast<const xmlChar *>(e.c_str()), "/", nullptr, 0);
    xmlFreeParserCtxt(context);
    xmlSetStructuredErrorFunc(nullptr, nullptr);
    xmlCleanupParser();

    EXPECT_EQ(nullptr, doc);
}
