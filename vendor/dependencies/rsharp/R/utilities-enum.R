#' Gets the names of a .NET Enum value type
#'
#' @param enumType a .NET object, System.Type or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @return a character vector of the names for the enum
#' @export
#' @examplesIf dotnetAvailable()
#' enumName <- "ClrFacade.TestEnum"
#' getEnumNames(enumName)
#' # Get enum names from object
#' enumObj <- newObjectFromName(enumName)
#' getEnumNames(enumObj)
getEnumNames <- function(enumType) {
  return(callStatic(rSharpEnv$clrFacadeTypeName, "GetEnumNames", enumType))
}
