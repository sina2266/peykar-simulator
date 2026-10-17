#' Gets the static members for a type
#'
#' @param objOrType a .NET object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @export
#' @examplesIf dotnetAvailable()
#' cTypename <- getRSharpSetting("testCasesTypeName")
#' getStaticMembers(cTypename)
getStaticMembers <- function(objOrType) {
  list(
    Methods = getStaticMethods(objOrType),
    Fields = getStaticFields(objOrType),
    Properties = getStaticProperties(objOrType)
  )
}

#' Gets the signature of a static member of a type
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param memberName The exact name of the member (i.e. field, property, method) to search for
#' @export
getStaticMemberSignature <- function(typename, memberName) {
  callStatic(rSharpEnv$clrFacadeTypeName, "GetSignature", typename, memberName)
}

#' Gets the static fields for a type
#'
#' @param objOrType a `NetObject` object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param contains a string that the property names returned must contain
#' @export
getStaticFields <- function(objOrType, contains = "") {
  callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetStaticFields",
    objOrType,
    contains
  )
}

#' Gets the static properties for a type
#'
#' @inheritParams getStaticFields
#' @export
getStaticProperties <- function(objOrType, contains = "") {
  callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetStaticProperties",
    objOrType,
    contains
  )
}

#' Gets the static methods for a type
#'
#' @inheritParams getStaticFields
#' @export
getStaticMethods <- function(objOrType, contains = "") {
  callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetStaticMethods",
    objOrType,
    contains
  )
}

#' Call a static method on a .NET type
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param methodName the name of a static method of the type
#' @param ... additional method arguments passed to .External (e.g., arguments to the method)
#' @return an object resulting from the call. May be a `NetObject` object, or a native R object for common types. Can be NULL.
#' @export
#' @examplesIf dotnetAvailable()
#' cTypename <- getRSharpSetting("testCasesTypeName")
#' callStatic(cTypename, "IsTrue", TRUE)
callStatic <- function(typename, methodName, ...) {
  .ensureRuntime()
  # Extract the pointer for R6 objects
  args <- .extractPointersFromArgs(list(...))
  # Calling via `do.call` to pass the arguments
  extPtr <- do.call(
    ".External",
    c(
      list("r_call_static_method", typename, methodName),
      args,
      PACKAGE = rSharpEnv$nativePkgName
    )
  )
  return(castToRObject(extPtr))
}

#' Gets the value of a static field or property of a class
#'
#' @param type Type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param name the name of a field/property  of the object
#' @return An object resulting from the call. May be a `NetObject` object, or a native R object for common types. Can be NULL.
#' @export
#' @examplesIf dotnetAvailable()
#' testClassName <- getRSharpSetting("testObjectTypeName")
#' getStatic(testClassName, "StaticPropertyIntegerOne")
getStatic <- function(type, name) {
  return(callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetFieldOrProperty",
    type,
    name
  ))
}

#' Sets the value of a field or property of an object or class
#'
#' @param type Type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param name the name of a field/property of the object
#' @param asInteger Boolean whether to convert the value to an integer.
#' Used for cases where .NET signature requires an integer. Ignored if `value` is not numeric.
#' @param value The value to set the field with
#'
#' @export
#' @examplesIf dotnetAvailable()
#' testClassName <- getRSharpSetting("testObjectTypeName")
#' setStatic(testClassName, "StaticPropertyIntegerOne", as.integer(42))
setStatic <- function(type, name, value, asInteger = FALSE) {
  # Convert character to UTF-8
  if (is(value, "character")) {
    value <- enc2utf8(value)
  }
  # Workaround for cases where .NET signature requires an integer
  if (is.numeric(value) && asInteger) {
    value <- as.integer(value)
  }

  invisible(callStatic(
    rSharpEnv$clrFacadeTypeName,
    "SetFieldOrProperty",
    type,
    name,
    value
  ))
}
