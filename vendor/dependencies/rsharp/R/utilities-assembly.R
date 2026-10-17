#' Loads a .NET assembly.
#'
#' @details
#'
#' Note that this is loaded in the single application domain that is created by rSharp, not a separate application domain.
#'
#' @param name a character vector of length one. It can be the full file name of the assembly to load, or a fully qualified assembly name, or as a last resort a partial name.
#'
#' @seealso \code{\link{.C}} which this function wraps
#' @export
#' @return Name of the loaded assembly, if successfull.
#' @examples
#' \dontrun{
#' f <- file.path("SomeDirectory", "YourDotNetBinaryFile.dll")
#' f <- path.expand(f)
#' stopifnot(file.exists(f))
#' loadAssembly(f)
#' }
loadAssembly <- function(name) {
  .ensureRuntime()
  result <- .External(
    "rSharp_load_assembly",
    name,
    PACKAGE = rSharpEnv$nativePkgName
  )
  return(result)
}

#' List the names of loaded assemblies
#'
#' @param fullname should the full name of the assemblies be returned. `FALSE` by default.
#' @param filenames if TRUE, return a data frame where the second column is the URI (usually file path) of the loaded assembly. `FALSE` by default.
#' @return the names of loaded assemblies
#' @export
#' @examplesIf dotnetAvailable()
#' getLoadedAssemblies()
getLoadedAssemblies <- function(fullname = FALSE, filenames = FALSE) {
  assNames <- callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetLoadedAssemblyNames",
    fullname
  )
  if (filenames) {
    data.frame(
      AssemblyName = assNames,
      URI = callStatic(
        rSharpEnv$clrFacadeTypeName,
        "GetLoadedAssemblyURI",
        assNames
      )
    )
  } else {
    assNames
  }
}

#' Get a list of .NET type names exported by an assembly
#'
#' @param assemblyName the name of the assembly
#' @return The names of the types exported by the assembly
#' @export
#' @examplesIf dotnetAvailable()
#' getTypesInAssembly("ClrFacade")
getTypesInAssembly <- function(assemblyName) {
  callStatic(rSharpEnv$clrFacadeTypeName, "GetTypesInAssembly", assemblyName)
}


#' Is the assembly loaded?
#'
#' @param assemblyName The name of the assembly, e.g. 'ClrFacade'
#'
#' @return TRUE if the assembly is loaded, FALSE otherwise
#' @export
#'
#' @examplesIf dotnetAvailable()
#' isAssemblyLoaded("ClrFacade")
isAssemblyLoaded <- function(assemblyName) {
  loadedAssemblies <- getLoadedAssemblies()
  if (assemblyName %in% loadedAssemblies) {
    return(TRUE)
  }

  return(FALSE)
}
