messages <- list(
  errorExceedLength = function(values, parentLengths) {
    paste0("Values '", paste0(values, collapse = ", "), "' exceed variable length : '", paste0(parentLengths, collapse = ", "), "'.")
  },
  errorConflictingInput = function(eitherInput, orInput) {
    paste0(
      "Conflicting inputs provided. Inputs '", paste0(eitherInput, collapse = ", "),
      "' and '", paste0(orInput, collapse = ", "), "' can't be provided at the same time."
    )
  },
  errorNrowData = function(plotName) {
    paste0("nrow(data)=0. ", plotName, " layer could not be added.")
  },
  unknownColorPalette = function(palette) {
    paste0(
      "Unknown palette '", palette, "', using palette 'Set1' instead.\n",
      "See enum 'ColorPalettes' to check available color palettes."
    )
  },
  errorValuesNotStrictlyPositive = function(values) {
    paste0(
      "All values need to be STRICTLY positive but ",
      sum(values <= 0),
      " values lower than or equal to zero were found."
    )
  },
  warningValuesNotWitinRange = function(x, left, right, strict = FALSE) {
    paste0(
      "Value(s) '",
      paste0(x[!isBetween(x, left, right, strict = FALSE)], collapse = "', '"),
      "' were not ",
      ifelse(strict, "strictly ", ""),
      "within [", paste0(range(left, right), collapse = ";"), "] range"
    )
  },
  warningAngleNotIncludedInAvailableAngles = function(oldAngle, newAngle) {
    paste0(
      "Angles other than 0, 90, 180 and 270 are not available for title, subtitles, caption and axis titles. ",
      "Replacing '", oldAngle, "' by closest available value: '", newAngle, "'."
    )
  }
)
