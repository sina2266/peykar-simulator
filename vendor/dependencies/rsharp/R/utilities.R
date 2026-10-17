#' Turn on/off the conversion of advanced data types with R.NET
#'
#' Turn on/off the conversion of advanced data types with R.NET. This will turn off the conversion of classes such as dictionaries into R lists,
#' as these are not bidirectional and you may want to see and manipulate external pointers to dictionaries in some circumstances.
#'
#' @param enable if true enable, otherwise disable
#' @export
#' @examplesIf dotnetAvailable()
#' library(rSharp)
#' cTypename <- getRSharpSetting("testCasesTypeName")
#' callStatic(cTypename, "CreateStringDictionary")
#' setConvertAdvancedTypes(FALSE)
#' callStatic(cTypename, "CreateStringDictionary")
setConvertAdvancedTypes <- function(enable = TRUE) {
  invisible(callStatic(
    "ClrFacade.ClrFacade",
    "SetConvertAdvancedTypes",
    enable
  ))
}

#' Calls the ToString method of an object
#'
#' @description
#'
#' Calls the ToString method of an object as represented in the .NET.
#' This function is here to help quickly test object equivalence from the R interpreter,
#'  for instance on the tricky topic of date-time conversions
#'
#' @param x any R object, which is converted to a .NET object on which to call ToString
#' @return The string representation of the object in .NET
#' @export
#' @examplesIf dotnetAvailable()
#' library(rSharp)
#' dt <- as.POSIXct("2001-01-01 02:03:04", tz = "UTC")
#' toStringNET(dt)
toStringNET <- function(x) {
  return(callStatic(rSharpEnv$clrFacadeTypeName, "ToString", x))
}
