#' Create if possible an object of the R6 class `NetObject`
#'
#' @details
#'
#' Create if possible and adequate the R6 object of the class `NetObject`
#' that wraps the external pointer to a .NET object.
#' If `obj` is not a pointer, returns `obj` unchanged.
#'
#' @param obj the presumed external pointer.
#' @param recursive logical; if TRUE, the function is applied recursively to the list elements.
#' @return A `NetObject` R6 object if the argument is indeed an external pointer,
#' otherwise returned unchanged. If `recursive` is TRUE and `obj` is a list, the function is applied
#' recursively to the list elements.
#' @export
#' @examplesIf dotnetAvailable()
#' castToRObject(1)
#' castToRObject("a")
#' castToRObject(TRUE)
#' castToRObject(FALSE)
#' castToRObject(1L)
#' castToRObject(1.1)
#' castToRObject(1.1 + 1i)
#' castToRObject(list(1, 2, 3))
#' castToRObject(data.frame(a = 1:3, b = c("a", "b", "c")))
castToRObject <- function(obj, recursive = TRUE) {
  # if object is not a list or recursive is FALSE, process is as is
  # special case for data frames, which are recognized as lists
  if (is.data.frame(obj)) {
    return(obj)
  }
  if (!is.list(obj) || !recursive) {
    # Create a new NetObject instance if obj is a pointer
    if (is(obj, "externalptr")) {
      return(NetObject$new(obj))
    }

    # If obj is the S4 object of type cobjRef, return create a new NetObject
    if (inherits(obj, "cobjRef")) {
      return(NetObject$new(obj@clrobj))
    }

    # Otherwise return the object unchanged
    return(obj)
  }

  # if object is a list and recursive is TRUE, process recursively
  lapply(obj, castToRObject, recursive = TRUE)
}

#' Create a new NetObject R6 object given the type name.
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param R6objectClass the R6 class of the object to be created, defaults to `NetObject`.
#' Can be used to create custom R6 classes that inherit from `NetObject`.
#' @param ... additional method arguments passed to the object constructor via the call to .External
#'
#' @return a `NetObject` R6 object
#' @export
#' @examplesIf dotnetAvailable()
#' testClassName <- getRSharpSetting("testObjectTypeName")
#' testObj <- newObjectFromName(testClassName)
#' # object with a constructor that has parameters
#' testObj <- newObjectFromName(testClassName, as.integer(123))
newObjectFromName <- function(typename, ..., R6objectClass = NetObject) {
  # R6objectClass must be a subclass of NetObject - don't know how to validate!
  # if (!inherits(R6objectClass, "NetObject")) {
  #   stop(messages$errorNotANetObjectClass())
  # }
  o <- newPointerFromName(typename, ...)
  R6objectClass$new(o)
}

#' Create a new external pointer to a .NET object given the type name.
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param ... additional method arguments passed to the object constructor via the call to .External
#'
#' @return an external pointer to a .NET object
#' @export
#'
#' @examplesIf dotnetAvailable()
#' testClassName <- getRSharpSetting("testObjectTypeName")
#' testPtr <- newPointerFromName(testClassName)
#' # Now we can create a NetObject from the pointer
#' testObj <- NetObject$new(testPtr)
newPointerFromName <- function(typename, ...) {
  .ensureRuntime()
  # Extract the pointer for R6 objects
  args <- .extractPointersFromArgs(list(...))
  # Calling via `do.call` to pass the arguments
  o <- do.call(
    ".External",
    c(
      list("r_create_clr_object", typename),
      args,
      PACKAGE = rSharpEnv$nativePkgName
    )
  )
  if (is.null(o)) {
    stop("Failed to create instance of type '", typename, "'")
  }
  o
}
