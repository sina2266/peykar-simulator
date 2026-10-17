// This is a helper macro for extending the bindings to have a Constructor mapped to the static create method.

// The macro create_name_constructor must be used in conjunction with the create_constructor macro.

%define %create_constructor(TYPE...)
%typemap(out) libcellml::TYPE *TYPE() {
  /*
  Here we take the returned value from the Constructor for this object and cast it
  to the pointer that it actually is.  Once that is done we can set the required resultobj.
  */
  std::shared_ptr<  libcellml::TYPE > *smartresult = reinterpret_cast<std::shared_ptr<  libcellml::TYPE > *>(result);
  resultobj = SWIG_NewPointerObj(SWIG_as_voidptr(smartresult), SWIGTYPE_p_std__shared_ptrT_libcellml__ ## TYPE ## _t, SWIG_POINTER_NEW | SWIG_POINTER_OWN);
}

%extend libcellml::TYPE {
    TYPE() {
        /*
        Provide a fake Constructor for the class and return a shared pointer
        as an actual class pointer because that is what we have to return from
        this fake Constructor.  The intention here is that this method is
        only used in one place so there we know it is actually a pointer
        to a shared pointer and cast it back to its proper form then.
        */
        auto ptr = new std::shared_ptr<  libcellml::TYPE >(libcellml::TYPE::create());
        return reinterpret_cast<libcellml::TYPE *>(ptr);
    }
}

%ignore libcellml::TYPE::create;
%enddef

%define %create_name_constructor(TYPE...)
%extend libcellml::TYPE {
    TYPE(const std::string &name) {
        /*
        Provide a fake name Constructor for the class and return a shared pointer
        as an actual class pointer because that is what we have to return from
        this fake Constructor.  The intention here is that this method is
        only used in one place so there we know it is actually a pointer
        to a shared pointer and cast it back to its proper form then.
        */
        auto ptr = new std::shared_ptr<  libcellml::TYPE >(libcellml::TYPE::create(name));
        return reinterpret_cast<libcellml::TYPE *>(ptr);
    }
}
%enddef
