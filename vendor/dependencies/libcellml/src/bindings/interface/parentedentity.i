%module(package="libcellml") parentedentity

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "entity.i"
%import "types.i"

%feature("docstring") libcellml::Entity::parent
"Returns the parent of the CellML Entity (or `None` if not set).";

%feature("docstring") libcellml::Entity::hasParent
"Tests if this entity has a parent. Returns true if it has a parent and false otherwise.";

%feature("docstring") libcellml::Entity::hasAncestor
"Tests if any of the ancestors of this entity have the given entity as a parent.";

%feature("docstring") libcellml::Entity::setParent
"Set the parent of the entity to the given entity.  This should ostensibly be a Model or a Component.";

%{
#include "libcellml/component.h"
#include "libcellml/model.h"
#include "libcellml/parentedentity.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%ignore libcellml::ParentedEntity::ParentedEntity();

%typemap(out) libcellml::ParentedEntityPtr parent {
    auto isAModel = std::dynamic_pointer_cast<libcellml::Model>($1);
    auto isAComponent = std::dynamic_pointer_cast<libcellml::Component>($1);
    if (isAModel != nullptr) {
        std::shared_ptr<  libcellml::Model > *smartresult = result ? new std::shared_ptr<  libcellml::Model >(isAModel) : nullptr;
        $result = SWIG_NewPointerObj(SWIG_as_voidptr(smartresult), SWIGTYPE_p_std__shared_ptrT_libcellml__Model_t, SWIG_POINTER_OWN);
    } else if (isAComponent != nullptr) {
        std::shared_ptr<  libcellml::Component > *smartresult = result ? new std::shared_ptr<  libcellml::Component >(isAComponent) : nullptr;
        $result = SWIG_NewPointerObj(SWIG_as_voidptr(smartresult), SWIGTYPE_p_std__shared_ptrT_libcellml__Component_t, SWIG_POINTER_OWN);
    } else {
        $result = Py_None;
        Py_INCREF($result);
    }
}

%include "libcellml/parentedentity.h"
