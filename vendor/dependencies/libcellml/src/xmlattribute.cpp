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

#include "xmlattribute.h"

#include <libxml/tree.h>
#include <string>
#include <vector>

#include "namespaces.h"

namespace libcellml {

/**
 * @brief The XmlAttribute::XmlAttributeImpl struct.
 *
 * This struct is the private implementation struct for the XmlAttribute class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
struct XmlAttribute::XmlAttributeImpl
{
    xmlAttrPtr mXmlAttributePtr;
};

XmlAttribute::XmlAttribute()
    : mPimpl(new XmlAttributeImpl())
{
}

XmlAttribute::~XmlAttribute()
{
    delete mPimpl;
}

void XmlAttribute::setXmlAttribute(const xmlAttrPtr &attribute)
{
    mPimpl->mXmlAttributePtr = attribute;
}

std::string XmlAttribute::namespaceUri() const
{
    if (mPimpl->mXmlAttributePtr->ns == nullptr) {
        return {};
    }
    return reinterpret_cast<const char *>(mPimpl->mXmlAttributePtr->ns->href);
}

std::string XmlAttribute::namespacePrefix() const
{
    if (mPimpl->mXmlAttributePtr->ns == nullptr) {
        return {};
    }
    return reinterpret_cast<const char *>(mPimpl->mXmlAttributePtr->ns->prefix);
}

bool XmlAttribute::inNamespaceUri(const char *ns) const
{
    return xmlStrcmp(reinterpret_cast<const xmlChar *>(namespaceUri().c_str()), reinterpret_cast<const xmlChar *>(ns)) == 0;
}

bool XmlAttribute::isType(const char *name, const char *ns) const
{
    return (xmlStrcmp(reinterpret_cast<const xmlChar *>(namespaceUri().c_str()), reinterpret_cast<const xmlChar *>(ns)) == 0)
           && (xmlStrcmp(mPimpl->mXmlAttributePtr->name, reinterpret_cast<const xmlChar *>(name)) == 0);
}

bool XmlAttribute::isCellmlType(const char *name) const
{
    return isType(name, CELLML_2_0_NS);
}

std::string XmlAttribute::name() const
{
    return reinterpret_cast<const char *>(mPimpl->mXmlAttributePtr->name);
}

std::string XmlAttribute::value() const
{
    std::string valueString;
    xmlChar *value = xmlGetProp(mPimpl->mXmlAttributePtr->parent, mPimpl->mXmlAttributePtr->name);
    valueString = std::string(reinterpret_cast<const char *>(value));
    xmlFree(value);
    return valueString;
}

XmlAttributePtr XmlAttribute::next() const
{
    xmlAttrPtr next = mPimpl->mXmlAttributePtr->next;
    XmlAttributePtr nextHandle = nullptr;
    if (next != nullptr) {
        nextHandle = std::make_shared<XmlAttribute>();
        nextHandle->setXmlAttribute(next);
    }
    return nextHandle;
}

void XmlAttribute::removeAttribute()
{
    xmlRemoveProp(mPimpl->mXmlAttributePtr);
}

void XmlAttribute::setNamespacePrefix(const std::string &prefix)
{
    std::vector<xmlChar> buffer;
    xmlNodePtr parent = mPimpl->mXmlAttributePtr->parent;

    buffer.resize(prefix.length() + 1);
    xmlChar *fullElemName = xmlBuildQName(mPimpl->mXmlAttributePtr->name, reinterpret_cast<const xmlChar *>(prefix.c_str()), buffer.data(), static_cast<int>(buffer.size()));

    auto oldAttribute = mPimpl->mXmlAttributePtr;
    mPimpl->mXmlAttributePtr = xmlSetProp(parent, fullElemName, reinterpret_cast<const xmlChar *>(value().c_str()));
    xmlRemoveProp(oldAttribute);
    xmlFree(fullElemName);
}

} // namespace libcellml
