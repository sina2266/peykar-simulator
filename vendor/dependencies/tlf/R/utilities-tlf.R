#' @title setDefaultLegendPosition
#' @description Set default legend position of tlf environment
#' @param position legend position.
#' Use Enum `LegendPositions` to assess available legend positions
#' @export
setDefaultLegendPosition <- function(position) {
  validateIsIncluded(position, LegendPositions)
  tlfEnv$currentTheme$background$legendPosition <- position
  return(invisible())
}

#' @title setDefaultLegendTitle
#' @description Set default legend title of tlf environment
#' @param title Character or `Label` object
#' @export
setDefaultLegendTitle <- function(title) {
  validateIsOfType(title, c("character", "Label"), nullAllowed = TRUE)
  if (isOfType(title, "Label")) {
    tlfEnv$currentTheme$fonts$legendTitle <- title$font
    tlfEnv$currentTheme$background$legendTitle <- title$text
    return(invisible())
  }
  tlfEnv$currentTheme$background$legendTitle <- title
  return(invisible())
}

#' @title setDefaultExportParameters
#' @description Set default tlf properties for exporting/saving plots
#' @param format file format of the exported plots
#' @param width plot width in `unit`
#' @param height plot height in `unit`
#' @param units units of `width` and `height`
#' @param dpi units of `width` and `height`
#' @param name base file name of the exported plots
#' @export
setDefaultExportParameters <- function(format = NULL, width = NULL, height = NULL, units = NULL, dpi = NULL, name = NULL) {
  validateIsIncluded(format, ExportFormats, nullAllowed = TRUE)
  validateIsIncluded(units, ExportUnits, nullAllowed = TRUE)
  validateIsNumeric(width, nullAllowed = TRUE)
  validateIsNumeric(height, nullAllowed = TRUE)
  validateIsNumeric(dpi, nullAllowed = TRUE)
  validateIsString(name, nullAllowed = TRUE)

  inputs <- c("format", "width", "height", "units", "dpi", "name")
  eval(.parseVariableToObject(objectName = "tlfEnv$defaultExportParameters", inputs, keepIfNull = TRUE))
  return(invisible())
}

#' @title setDefaultAggregationFunctions
#' @description Set default aggregation functions of tlf environment
#' @param y function or its name as median aggregation
#' @param ymin function or its name as min aggregation
#' @param ymax function or its name as max aggregation
#' @export
setDefaultAggregationFunctions <- function(y = NULL, ymin = NULL, ymax = NULL) {
  validateIsOfType(c(y, ymin, ymax), c("function", "character"), nullAllowed = TRUE)

  if (isOfType(y, "character")) {
    y <- match.fun(y)
  }
  if (isOfType(ymin, "character")) {
    ymin <- match.fun(ymin)
  }
  if (isOfType(ymax, "character")) {
    ymax <- match.fun(ymax)
  }

  tlfEnv$defaultAggregation$functions$y <- y %||% tlfEnv$defaultAggregation$functions$y
  tlfEnv$defaultAggregation$functions$ymin <- ymin %||% tlfEnv$defaultAggregation$functions$ymin
  tlfEnv$defaultAggregation$functions$ymax <- ymax %||% tlfEnv$defaultAggregation$functions$ymax
}

#' @title setDefaultAggregationLabels
#' @description Set default aggregation labels of tlf environment
#' @param y label for median aggregation
#' @param range label for range aggregation
#' @export
setDefaultAggregationLabels <- function(y = NULL, range = NULL) {
  validateIsString(c(y, range), nullAllowed = TRUE)

  tlfEnv$defaultAggregation$labels$y <- y %||% tlfEnv$defaultAggregation$labels$y
  tlfEnv$defaultAggregation$labels$range <- range %||% tlfEnv$defaultAggregation$labels$range
}

#' @title setDefaultAggregationBins
#' @description Set default aggregation bins of tlf environment
#' @param bins Number of bins if value, edges if vector or binning function if function
#' @export
#' @examples
#' # Set default number of bins
#' plotHistogram(x = rnorm(1000))
#'
#' setDefaultAggregationBins(21)
#' plotHistogram(x = rnorm(1000))
#'
setDefaultAggregationBins <- function(bins = NULL) {
  tlfEnv$defaultAggregation$bins <- bins %||% tlfEnv$defaultAggregation$bins
}

#' @title setDefaultWatermark
#' @description Set default watermark value for current theme
#' @param watermark A character value or `Label` object
#' @export
#' @examples
#' # Set default watermark using a character
#' setDefaultWatermark("Confidential")
#' addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' # Set default watermark using a `Label` object
#' setDefaultWatermark(Label$new(text = "Confidential", color = "red", angle = 30))
#' addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
setDefaultWatermark <- function(watermark = NULL) {
  validateIsOfType(watermark, c("Label", "character"), nullAllowed = TRUE)
  if (isOfType(watermark, "character")) {
    tlfEnv$currentTheme$background$watermark <- watermark
  }
  if (isOfType(watermark, "Label")) {
    tlfEnv$currentTheme$background$watermark <- watermark$text
    tlfEnv$currentTheme$fonts$watermark <- watermark$font
  }
  return(invisible())
}

#' @title setDefaultLogTicks
#' @description Set default values for log ticks
#' @param ticks numeric values where ticks are placed.
#' Ensure that the values are positive (they are meant for log scale)
#' @export
setDefaultLogTicks <- function(ticks) {
  tlfEnv$logTicks <- ticks
  return(invisible())
}

#' @title setDefaultErrorbarCapSize
#' @description Set default cap size of error bars
#' @param size A numeric defining the size of the error bar caps in pts
#' @export
setDefaultErrorbarCapSize <- function(size) {
  validateIsNumeric(size)
  tlfEnv$defaultErrorbarCapSize <- size
  return(invisible())
}

#' @title setDefaultLLOQLinetype
#' @description Set default cap linetype for lloq lines
#' @param linetype linetype to set as default
#' @export
setDefaultLLOQLinetype <- function(linetype) {
  validateEnumValue(linetype, enum = Linetypes, nullAllowed = FALSE)
  tlfEnv$defaultLLOQLinetype <- linetype
  return(invisible())
}

#' @title setDefaultAlphaRatio
#' @description Set the default alpha (transparency) difference ratio between
#' points above and bellow lloq
#' @param alphaRatio alpha ratio to set as default. Must be between 0 and 1.
#' @export
setDefaultAlphaRatio <- function(alphaRatio) {
  validateIsNumeric(alphaRatio)
  stopifnot(alphaRatio > 0 & alphaRatio <= 1)
  tlfEnv$DefaultAlphaRatio <- alphaRatio
  return(invisible())
}

#' @title setDefaultMaxCharacterWidth
#' @description Set the  maximum number of characters per row for axis ticks labels and legend.
#' Long strings will be wraped on spaces or non-word characters.
#' @param maxCharacterWidth the maximum number of characters per row
#' @export
setDefaultMaxCharacterWidth <- function(maxCharacterWidth) {
  validateIsNumeric(maxCharacterWidth)
  tlfEnv$maxCharacterWidth <- maxCharacterWidth
  return(invisible())
}

#' @title getTLFSettings
#' @description
#' Get Names of the default/global settings stored in tlfEnv.
#' Can be used with `getTLFSettings()`
#' @param settingName setting name as defined in enum `tlfSettingsNames`
#' @export
getTLFSettings <- function(settingName) {
  validateEnumValue(settingName, enum = tlfSettingsNames, nullAllowed = FALSE)
  obj <- tlfEnv[[settingName]]

  return(list(
    Name = settingName,
    Class = class(obj),
    Value = obj
  ))
}

#' @title saveTLFSettings
#' Save the current TLF global settings in a `.RData` file
#' @param file `.RData` file containing the settings
#' @export
saveTLFSettings <- function(file) {
  validateIsFileExtension(file, "RData")
  newEnv <- tlfEnv
  save("newEnv", file = file)
}

#' @title loadTLFSettings
#' @description
#' Load TLF global settings from a file
#' @param file `.RData` file containing the settings
#' @export
loadTLFSettings <- function(file) {
  validateIsFileExtension(file, "RData")
  load(file = file)
  for (fieldNames in names(newEnv)) {
    tlfEnv[[fieldNames]] <- newEnv[[fieldNames]]
  }
  return(invisible())
}

#' @title resetTLFSettingsToDefault
#' @description
#' Reset the global settings stored in `tlfEnv` to default values defined by the package.
#' @export
resetTLFSettingsToDefault <- function() {
  loadTLFSettings(system.file("extdata", "tlf-env.RData", package = "tlf"))
  return(invisible())
}
