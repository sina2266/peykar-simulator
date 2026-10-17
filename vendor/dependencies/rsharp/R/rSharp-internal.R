#' Create if possible an S4 `cobjRef` object.
#'
#' Create if possible and adequate the S4 object that wraps the external pointer to a `cobjRef` object.
#'
#' @param obj the presumed external pointer.
#' @param clrtype character; the name of the type for the object. If NULL, rSharp retrieves the type name.
#' @return a cobjRef S4 object if the argument is indeed an external pointer,
#' otherwise returned unchanged.
#' @import methods
.mkClrObjRef <- function(obj, clrtype = NULL) {
  # Create a new `cobjRef` instance if obj is a pointer
  if (inherits(obj, "externalptr")) {
    # If no type is provided, retrieve it from .NET
    if (is.null(clrtype)) {
      clrtype <- .clrTypeNameExtPtr(obj)
    }
    return(new("cobjRef", clrobj = obj, clrtype = clrtype))
  } else {
    # Otherwise return unchanged
    return(obj)
  }
}

#' System function to get a direct access to an object
#'
#' This function is highly unlikely to be of any use to an end user, even an advanced one.
#' This is indirectly needed to unlock the benefits of using R.NET convert data structures between R and .NET.
#'
#' @return a `cobjRef` S4 object
.getCurrentConvertedObject <- function() {
  o <- .External("r_get_object_direct", PACKAGE = rSharpEnv$nativePkgName)
  .mkClrObjRef(o)
}

#' Gets the type name of an object
#'
#' Gets the type name of an object, given the SEXP external pointer to this .NET object.
#'
#' @param extPtr external pointer to a .NET object (not a `cobjRef` S4 or a `NetObject` object)
#' @return a character string, the type name
#' @examples
#' \dontrun{
#' testClassName <- getRSharpSetting("testObjectTypeName")
#' testObj <- newObjectFromName(testClassName)
#' .clrTypeNameExtPtr(testObj$pointer)
#' }
.clrTypeNameExtPtr <- function(extPtr) {
  .validateIsExtPtr(extPtr)
  .External(
    "r_get_typename_externalptr",
    extPtr,
    PACKAGE = rSharpEnv$nativePkgName
  )
}

#' Extract the pointers from the arguments
#'
#' @details
#' Iterates through the list of arguments and replaces `NetObjects` by their
#' pointers. Required to pass to ClrFacade.
#' @noRd
#'
#' @param args List of arguments
.extractPointersFromArgs <- function(args) {
  # Extract the pointer for R6 objects
  for (i in seq_along(args)) {
    if (inherits(args[[i]], "NetObject")) {
      args[[i]] <- args[[i]]$pointer
    }
    # Check if the entry is a list. If so, check if the first element is a NetObject
    # and extract the pointers for all elements
    if (is.list(args[[i]]) && length(args[[i]]) > 0) {
      if (inherits(args[[i]][[1]], "NetObject")) {
        args[[i]] <- lapply(args[[i]], function(x) {
          if (inherits(x, "NetObject")) x$pointer else x
        })
      }
    }
  }
  return(args)
}
