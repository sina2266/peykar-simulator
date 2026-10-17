messages <- list()

messages$errorMsvcrNotFound <- function() {
  paste(
    paste(rSharpEnv$msvcrFileName, "was not found on this Windows system."),
    "You are probably missing the Visual C++ Redistributable.",
    "Go to https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170 and download the latest 'Microsoft Visual C++ Redistributable'.",
    sep = "\n"
  )
}

messages$errorDotnetRuntimeNotFound <- function() {
  paste(
    paste0(
      "No .NET ",
      rSharpEnv$requiredDotnetVersion,
      " runtime was found."
    ),
    "rSharp is installed, but calls into .NET will fail until the runtime is available.",
    paste0(
      "Install .NET ",
      rSharpEnv$requiredDotnetVersion,
      ": go to https://learn.microsoft.com/en-us/dotnet/core/install/ and follow the installation instructions."
    ),
    sep = "\n"
  )
}

messages$errorRuntimeInitFailed <- function(details) {
  paste(
    "The .NET runtime could not be initialised.",
    "rSharp is installed, but calls into .NET will fail until a working runtime is available.",
    paste0(
      "Install .NET ",
      rSharpEnv$requiredDotnetVersion,
      ": go to https://learn.microsoft.com/en-us/dotnet/core/install/ and follow the installation instructions."
    ),
    paste0("Details: ", details),
    sep = "\n"
  )
}

messages$errorPackageSettingNotFound <- function(settingName, globalEnv) {
  paste0(
    "No global setting with the name '",
    settingName,
    "' exists. Available global settings are:\n",
    paste0(names(globalEnv), collapse = ", ")
  )
}

messages$errorNoExternalPointer <- function() {
  "Trying to create an object, but external pointer to .NET is invalid!"
}

messages$errorPropertyReadOnly <- function(
  propertyName,
  optionalMessage = NULL
) {
  paste0(
    "Property '",
    propertyName,
    "' is read-only and cannot be modified.",
    if (!is.null(optionalMessage)) {
      paste0(" ", optionalMessage)
    }
  )
}

messages$errorMethodNotFound <- function(methodName, typeName) {
  paste0(
    "Method '",
    methodName,
    "' does not exist in the type '",
    typeName,
    "'."
  )
}

messages$errorNotANetObjectClass <- function() {
  "Trying to create an object, but the class is not a subclass of `NetObject`!"
}
