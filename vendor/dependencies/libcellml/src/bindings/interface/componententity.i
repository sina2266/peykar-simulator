%module(package="libcellml") componententity

#define LIBCELLML_EXPORT

%import "importedentity.i"
%import "namedentity.i"
%import "types.i"

%feature("docstring") libcellml::ComponentEntity
"Abstract class that provides component managing functionality.";

%feature("docstring") libcellml::ComponentEntity::addComponent
"Add a copy of the given component as a child component of this component
entity.";

%feature("docstring") libcellml::ComponentEntity::containsComponent
"Tests if a component, specified by an index, name, or with a Component object,
 is contained within this entity.

A second argument can be given to specify whether or not child components
should be searched for the component to remove.";

%feature("docstring") libcellml::ComponentEntity::component
"Returns a component, specified by an index or name.

A second argument can be given to specify whether or not child components
should be searched for the component to remove.

Only the first matching component is returned.";

%feature("docstring") libcellml::ComponentEntity::removeComponent
"Removes a component, specified by an index, name, or with a Component object.

A second argument can be given to specify whether or not child components
should be searched for the component to remove.

Only the first matching component is removed.

Returns `True` on success.";

%feature("docstring") libcellml::ComponentEntity::removeAllComponents
"Removes all components that have been added to this entity.";

%feature("docstring") libcellml::ComponentEntity::takeComponent
"Removes and returns a component, specified by an index or name.

Only the first matching component is removed and  returned.";

%feature("docstring") libcellml::ComponentEntity::replaceComponent
"Removes a component, specified by an index, name, or with a Component object,
and replaces it with the given component object (second argument).

A third argument can be given to specify whether or not child components
should be searched for the component to remove.

Only the first matching component is replaced.

Returns `True` on success.";

%feature("docstring") libcellml::ComponentEntity::componentCount
"Returns the number of components the component contains.  ";

%feature("docstring") libcellml::ComponentEntity::encapsulationId
"Returns the encapsulation identifier for this entity.

The encapsulation identifier is placed on the XML element for this entity. For the
:class:`Model` class this is the ``encapsulation`` element that is the root
element for the model's structure.  For the :class:`Component` class this is
the ``component_ref`` element that references the component it represents in
the structure."

%feature("docstring") libcellml::ComponentEntity::setEncapsulationId
"Sets the encapsulation identifier for this entity.

The encapsulation identifier is placed on the XML element for this entity. For the
:class:`Model` class this is the ``encapsulation`` element that is the root
element for the model's structure.  For the :class:`Component` class this is
the ``component_ref`` element that references the component it represents in
the structure."

%feature("docstring") libcellml::ComponentEntity::removeEncapsulationId
"Removes the encapsulation identifier for this entity."


#if defined(SWIGPYTHON)
    %ignore libcellml::ComponentEntity::replaceComponent(size_t index, const ComponentPtr &component);

    // Allow any type of input to be converted to bool
    %typemap(typecheck,precedence=SWIG_TYPECHECK_BOOL) bool { $1 = 1; }
    %typemap(in) bool { $1 = PyObject_IsTrue($input) == 1; }

    // Treat negative size_t as invalid index (instead of unknown method)
    %extend libcellml::ComponentEntity {
        ComponentPtr component(long index) const {
            if (index < 0) return nullptr;
            return $self->component(size_t(index));
        }
        bool removeComponent(long index) {
            if (index < 0) return false;
            return $self->removeComponent(size_t(index));
        }
        ComponentPtr takeComponent(long index) {
            if (index < 0) return nullptr;
            return $self->takeComponent(size_t(index));
        }
        bool replaceComponent(long index, const ComponentPtr &c) {
            if (index < 0) return false;
            return $self->replaceComponent(size_t(index), c);
        }
    }
#endif

%{
#include "libcellml/componententity.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%ignore libcellml::ComponentEntity::ComponentEntity();

%include "libcellml/types.h"
%include "libcellml/componententity.h"
