#' @title .parseVariableToObject
#' @description Create an expression of type `objectName$variableName <- variableName`
#' @param objectName Name of the object whose field is updated
#' @param variableName Name of the variable and field of `objectName`
#' @param keepIfNull logical `objectName$variableName <- variableName %||% objectName$variableName`
#' @return An expression to `eval()`
#' @keywords internal
.parseVariableToObject <- function(objectName, variableName, keepIfNull = FALSE) {
  if (keepIfNull) {
    return(parse(text = paste0(objectName, "$", variableName, " <- ", variableName, " %||% ", objectName, "$", variableName)))
  }
  return(parse(text = paste0(objectName, "$", variableName, " <- ", variableName)))
}

#' @title .parseVariableFromObject
#' @description Create an expression of type `variableName <- objectName$variableName`
#' @param objectName Name of the object whose field is updated
#' @param variableName Name of the variable and field of `objectName`
#' @param keepIfNull logical `variableName <- objectName$variableName %||% variableName`
#' @return An expression to `eval()`
#' @keywords internal
.parseVariableFromObject <- function(objectName, variableName, keepIfNull = FALSE) {
  if (keepIfNull) {
    return(parse(text = paste0(variableName, " <- ", objectName, "$", variableName, " %||% ", variableName)))
  }
  return(parse(text = paste0(variableName, " <- ", objectName, "$", variableName)))
}

#' @title .parseValueToObject
#' @description Create an expression of type `objectName <- value`
#' @param objectName Name of the object to update
#' @param value Value of the variable `objectName`
#' @return An expression to `eval()`
#' @keywords internal
.parseValueToObject <- function(objectName, value) {
  if (isEmpty(value)) {
    return(parse(text = paste0(objectName, " <- NULL")))
  }
  if (isOfType(value, "character")) {
    return(parse(text = paste0(objectName, ' <- "', value, '"')))
  }
  return(parse(text = paste0(objectName, " <- ", value)))
}

#' @title .setDataMapping
#' @description Set `DataMapping` object internally using `tlf` default if `dataMapping` is not provided
#' @param dataMapping A `DataMappingClass` object
#' @param DataMappingClass Required class for `dataMapping`
#' @param data A data.frame potentially used for smart mapping
#' @return A `DataMapping` object
#' @keywords internal
.setDataMapping <- function(dataMapping, DataMappingClass, data = NULL) {
  dataMapping <- dataMapping %||% DataMappingClass$new(data = data)
  validateIsOfType(dataMapping, DataMappingClass)
  return(dataMapping)
}

#' @title .setPlotConfiguration
#' @description Set `PlotConfiguration` object internally using `tlf` default if `plotConfiguration` is not provided
#' @param plotConfiguration A `PlotConfigurationClass` object
#' @param PlotConfigurationClass Required class for `plotConfiguration`
#' @param data A data.frame potentially used for smart plot configuration
#' @param metaData A list of meta data potentially used for smart plot configuration
#' @param dataMapping A `DataMapping` object potentially used for smart plot configuration
#' @return A `PlotConfiguration` object
#' @keywords internal
.setPlotConfiguration <- function(plotConfiguration,
                                  PlotConfigurationClass,
                                  data = NULL,
                                  metaData = NULL,
                                  dataMapping = NULL) {
  plotConfiguration <- plotConfiguration %||%
    PlotConfigurationClass$new(data = data, metaData = metaData, dataMapping = dataMapping)
  validateIsOfType(plotConfiguration, PlotConfigurationClass)
  return(plotConfiguration)
}

#' @title .setPlotObject
#' @description Set a `ggplot` object associated with its `plotConfiguration`
#' @param plotObject A `ggplot` object
#' @param plotConfiguration A `PlotConfiguration` object
#' @return A `ggplot` object
#' @keywords internal
.setPlotObject <- function(plotObject,
                           plotConfiguration = NULL) {
  plotObject <- plotObject %||% initializePlot(plotConfiguration)
  validateIsOfType(plotObject, "ggplot")
  # Check if plotConfiguration attribute exists (ggplot2 4.0.0 compatibility)
  if (is.null(plotObject$plotConfiguration)) {
    stop("plotConfiguration attribute is missing from the plot object")
  }
  return(plotObject)
}


#' @title .updateAxes
#' @description Updates the plot axes
#' @param plotObject A `ggplot` object
#' @return A `ggplot` object
#' @keywords internal
.updateAxes <- function(plotObject) {
  # Try is used to prevent crashes in the final plot due to ggplot2 peculiarities regarding scale functions
  try(suppressMessages(plotObject <- setXAxis(plotObject)))
  try(suppressMessages(plotObject <- setYAxis(plotObject)))
  return(plotObject)
}

#' @title .updateSameAxes
#' @description Updates plot configuration axes to get same limits
#' @param plotObject A `ggplot` object
#' @param data A data.frame
#' @param dataMapping A `DataMapping` object
#' @return A `ggplot` object
#' @keywords internal
.updateSameAxes <- function(plotObject, data, dataMapping) {
  if (!all(
    plotObject$plotConfiguration$defaultSymmetricAxes,
    isEmpty(plotObject$plotConfiguration$xAxis$axisLimits),
    isEmpty(plotObject$plotConfiguration$yAxis$axisLimits)
  )) {
    return(plotObject)
  }
  limits <- .getSameLimitsFromMapping(data, dataMapping)
  plotObject$plotConfiguration$xAxis$axisLimits <- limits
  plotObject$plotConfiguration$yAxis$axisLimits <- limits
  return(plotObject)
}

#' @title .updateSymmetricAxes
#' @description Updates plot configuration axes to get symmetric limits
#' @param plotObject A `ggplot` object
#' @param data A data.frame
#' @param dataMapping A `DataMapping` object
#' @return A `ggplot` object
#' @keywords internal
.updateSymmetricAxes <- function(plotObject, data, dataMapping) {
  if (!all(
    plotObject$plotConfiguration$defaultSymmetricAxes,
    isEmpty(plotObject$plotConfiguration$yAxis$axisLimits)
  )) {
    return(plotObject)
  }
  limits <- getSymmetricLimits(data[, dataMapping$y])
  plotObject$plotConfiguration$yAxis$axisLimits <- limits
  return(plotObject)
}
