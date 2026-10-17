#' List the public constructors of a CLR Type
#'
#'
#' @param type .NET Type, or a (character) type name that can be successfully parsed
#' @return a list of constructor signatures
#' @export
#' @examplesIf dotnetAvailable()
#' testClassName <- "ClrFacade.TestObject"
#' getConstructors(testClassName)
getConstructors <- function(type) {
  callStatic(rSharpEnv$clrFacadeTypeName, "GetConstructors", type)
}

#' Gets the pointer to the `System.RuntimeType` of a `NetObject` object or a .NET type name.
#' @description
#' Returns a `NetObject` object with external pointer to the object of type `System.RuntimeType` that
#' represents the type of the .NET object. To get a string representation
#' of the type, call `toStringNET` on the returned object.
#'
#'
#' @examplesIf dotnetAvailable()
#' testClassName <- "ClrFacade.TestObject"
#' type <- getType(testClassName)
#' toStringNET(type)
#'
#' testObj <- newObjectFromName(testClassName)
#' type <- getType(testObj)
#' toStringNET(type)
#'
#' @param objOrTypename An object of class `NEtObject` or a character vector of length one.
#'  It can be the full file name of the assembly to load, or a fully qualified
#'  assembly name, or as a last resort a partial name.
#' @return A `NetObject` to the pointer of `System.RuntimeType` of `objOrTypename`.
#' @export
getType <- function(objOrTypename) {
  # Of the input is the assembly name
  if (is.character(objOrTypename)) {
    return(callStatic(rSharpEnv$clrFacadeTypeName, "GetType", objOrTypename))
  }

  # If the input is a NetObject
  if (inherits(objOrTypename, "NetObject")) {
    return(objOrTypename$call("GetType"))
  }
  stop("objOrTypename is neither a `NetObject` object nor a character vector")
}
