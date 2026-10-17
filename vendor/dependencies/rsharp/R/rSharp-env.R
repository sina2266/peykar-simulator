# Environment that holds various global variables and settings for the package.
# It is not exported and should not be directly manipulated by other packages.
rSharpEnv <- new.env(parent = emptyenv())

# name of the package. This will be used to retrieve information on the package at run time
rSharpEnv$packageName <- "rSharp"
# Name of the C++ redistributable library
rSharpEnv$msvcrFileName <- "msvcp140.dll"
# Major version of the .NET runtime the bundled assemblies target. The runtime
# check requires this exact major version: the assemblies are built for
# `net8.0` and their runtimeconfig does not roll forward across major versions,
# so a higher major (for example .NET 10) is not a valid substitute.
rSharpEnv$requiredDotnetVersion <- 8L
# The name of the package
rSharpEnv$pkgName <- "rSharp"

info <- Sys.info()
# The name of the native (C++) library
rSharpEnv$nativePkgName <-
  switch(
    info[['sysname']],
    Windows = rSharpEnv$pkgName,
    Linux = {
      paste0(rSharpEnv$pkgName, ".linux")
    },
    Darwin = {
      paste0(
        rSharpEnv$pkgName,
        ".mac.",
        if (info[['machine']] == 'arm64') 'arm64' else 'x64'
      )
    }
  )

# Name of the .NET library
rSharpEnv$dotnetPkgName <- "ClrFacade"

# Full type name of the main facade to the interop code written in C#
rSharpEnv$clrFacadeTypeName <- "ClrFacade.ClrFacade"
# Full type name of the test cases
rSharpEnv$testCasesTypeName <- "ClrFacade.TestCases"
# Full name of test object class
rSharpEnv$testObjectTypeName <- "ClrFacade.TestObject"
rSharpEnv$testMethodBindingTypeName <- "ClrFacade.TestMethodBinding"


#' Names of the settings stored in rSharpEnv Can be used with `getRSharpSetting()`
#' @export
rSharpSettingNames <- names(rSharpEnv)

#' @title getRSharpSetting
#' @description Get the value of a global rSharp setting.
#'
#' @param settingName String name of the setting
#'
#' @return Value of the setting stored in rSharpEnv. If the setting does not exist, an error is thrown.
#' @export
#'
#' @examples
#' getRSharpSetting("nativePkgName")
getRSharpSetting <- function(settingName) {
  if (!(any(names(rSharpEnv) == settingName))) {
    stop(messages$errorPackageSettingNotFound(settingName, rSharpEnv))
  }

  obj <- rSharpEnv[[settingName]]
  # Evaluate if the object is a function. This is required since some properties are defined as function reference
  if (is.function(obj)) {
    return(obj())
  }

  return(obj)
}
