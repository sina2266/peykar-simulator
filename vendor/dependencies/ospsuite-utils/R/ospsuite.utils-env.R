#' Environment that holds various global variables and settings for the
#' `ospsuite.utils`.
#'
#' It is not exported and should not be directly manipulated by
#' other packages.
#'
#' @keywords internal
#' @noRd
ospsuiteUtilsEnv <- new.env(parent = emptyenv())

# name of the package
# this will be used to retrieve information on the package at runtime
ospsuiteUtilsEnv$packageName <- "ospsuite.utils"

# name of the suite to which this package belongs
ospsuiteUtilsEnv$suiteName <- "Open Systems Pharmacology"

# default values for the `formatNumerics()` helper function
ospsuiteUtilsEnv$formatNumericsDigits <- 2L

# Small value added to zero when calculating log
ospsuiteUtilsEnv$LOG_SAFE_EPSILON <- 1e-20

# Logging settings, masking corresponds to patterns logged but not displayed on console
ospsuiteUtilsEnv$logging <- list(
  folder = NULL,
  errorMasking = NULL,
  warningMasking = NULL,
  infoMasking = NULL
)

#' Names of the settings stored in `ospsuiteEnv`. Can be used with
#' `getOSPSuiteUtilsSetting()`
#'
#' @include utilities.R
#'
#' @examples
#' ospsuiteUtilsSettingNames
#'
#' @export
ospsuiteUtilsSettingNames <- enum(names(ospsuiteUtilsEnv))

#' Get the value of a global `{ospsuite.utils}` package setting.
#'
#' @param settingName String name of the setting
#'
#' @return
#' Value of the setting stored in `ospsuiteEnv`. If the setting does not exist,
#' an error is thrown.
#'
#' @examples
#' getOSPSuiteUtilsSetting("packageName")
#' getOSPSuiteUtilsSetting("suiteName")
#' getOSPSuiteUtilsSetting("formatNumericsDigits")
#' @export
getOSPSuiteUtilsSetting <- function(settingName) {
  if (!(any(names(ospsuiteUtilsEnv) == settingName))) {
    stop(messages$errorPackageSettingNotFound(settingName, ospsuiteUtilsEnv))
  }

  obj <- ospsuiteUtilsEnv[[settingName]]

  # Evaluate if the object is a function. This is required since some
  # properties are defined as a function reference.
  if (is.function(obj)) {
    return(obj())
  }

  return(obj)
}
