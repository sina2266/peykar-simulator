# Since Theme$new() requires functions created in utilities-xx.R files
# and tlfEnv$currentTheme is defined in this file
# enum tlfSettingsNames is defined after this line to include all the tlfEnv fields
tlfEnv$currentTheme <- Theme$new()

# Default theme is minimal when package is loaded
useMinimalTheme()

#' @title tlfSettingsNames
#' @description
#' Names of the default/global settings stored in tlfEnv.
#' Can be used with `getTLFSettings()`
#' @import ospsuite.utils
#' @export
#' @family enum helpers
tlfSettingsNames <- enum(names(tlfEnv))

# Quiets concerns of R CMD check for ggplot2
if (getRversion() >= "2.15.1") utils::globalVariables(c("count", "ncount", "width"))

# Save a sync version of tlf-env as an .RData file
newEnv <- tlfEnv
save(
  "newEnv",
  file = file.path(system.file(package = "tlf"), "extdata", "tlf-env.RData")
)
rm(list = "newEnv")
