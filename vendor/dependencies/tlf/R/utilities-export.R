#' @title setPlotExport
#' @description Set plot export properties
#' @param plotObject Graphical object created from ggplot
#' @param name character defining the name of the file to be saved (without extension)
#' @param format character defining the format of the file to be saved.
#' @param width numeric values defining the width in `units` of the plot dimensions after saving
#' @param height numeric values defining the height in `units` of the plot dimensions after saving
#' @param units character defining the unit of the saving dimension
#' @param dpi numeric value defining plot resolution (dots per inch)
#' @return ggplot object with updated saving properties
#' @export
setPlotExport <- function(plotObject, name = NULL, format = NULL, width = NULL, height = NULL, units = NULL, dpi = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(name, nullAllowed = TRUE)
  validateIsIncluded(format, ExportFormats, nullAllowed = TRUE)
  validateIsIncluded(units, ExportUnits, nullAllowed = TRUE)
  validateIsNumeric(width, nullAllowed = TRUE)
  validateIsNumeric(height, nullAllowed = TRUE)
  validateIsNumeric(dpi, nullAllowed = TRUE)

  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  inputs <- c("name", "format", "width", "height", "units", "dpi")
  eval(.parseVariableToObject(objectName = "newPlotObject$plotConfiguration$export", inputs, keepIfNull = TRUE))
  return(newPlotObject)
}

#' @title setPlotExportFormat
#' @description Set plot export properties
#' @param plotObject Graphical object created from ggplot
#' @param format file format of the exported plot
#' @return ggplot object with updated labels
#' @export
setPlotExportFormat <- function(plotObject, format = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(format, ExportFormats, nullAllowed = TRUE)

  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$export
  export <- newPlotObject$plotConfiguration$export
  export$format <- format %||% export$format

  return(newPlotObject)
}

#' @title setPlotExportDimensions
#' @description Set plot export properties
#' @param plotObject Graphical object created from ggplot
#' @param width plot width in `unit`
#' @param height plot height in `unit`
#' @param units units of `width` and `height`
#' @param dpi numeric value defining plot resolution (dots per inch)
#' @return ggplot object with updated labels
#' @export
setPlotExportDimensions <- function(plotObject, width = NULL, height = NULL, units = NULL, dpi = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(units, ExportUnits, nullAllowed = TRUE)
  validateIsNumeric(width, nullAllowed = TRUE)
  validateIsNumeric(height, nullAllowed = TRUE)
  validateIsNumeric(dpi, nullAllowed = TRUE)

  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)
  inputs <- c("width", "height", "units", "dpi")
  eval(.parseVariableToObject(objectName = "newPlotObject$plotConfiguration$export", inputs, keepIfNull = TRUE))
  return(newPlotObject)
}

#' @title setPlotExportSize
#' @description Set plot export properties
#' @inheritParams setPlotExportDimensions
#' @return ggplot object with updated labels
#' @export
setPlotExportSize <- setPlotExportDimensions

#' @title exportPlot
#' @description Save a `ggplot` object according to its export properties
#' @param plotObject Graphical object created from ggplot
#' @param fileName name of exported file (with extension)
#' @param name character defining the name of the file to be saved (without extension)
#' @param format character defining the format of the file to be saved.
#' @param width numeric values defining the width in `units` of the plot dimensions after saving
#' @param height numeric values defining the height in `units` of the plot dimensions after saving
#' @param units character defining the unit of the saving dimension
#' @param dpi numeric value defining plot resolution (dots per inch)
#' @return The file name of the exported plot
#' @export
#' @import ggplot2
exportPlot <- function(plotObject, fileName = NULL, name = NULL, format = NULL, width = NULL, height = NULL, units = NULL, dpi = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(fileName, nullAllowed = TRUE)
  validateIsString(name, nullAllowed = TRUE)
  validateIsIncluded(format, ExportFormats, nullAllowed = TRUE)
  validateIsIncluded(units, ExportUnits, nullAllowed = TRUE)
  validateIsNumeric(width, nullAllowed = TRUE)
  validateIsNumeric(height, nullAllowed = TRUE)
  validateIsNumeric(dpi, nullAllowed = TRUE)

  # Update overwritten export properties
  plotObject <- setPlotExport(
    plotObject,
    name = name,
    format = format,
    width = width,
    height = height,
    units = units,
    dpi = dpi
  )

  fileName <- plotObject$plotConfiguration$export$savePlot(plotObject, fileName)
  # Return the file name in case users needs it
  # Use invisible to prevent writing the file name in the console every time a plot is exported
  return(invisible(fileName))
}


#' @title exportPlotConfigurationCode
#' @description Export a plot configuration as R code
#' @param plotConfiguration A `PlotConfiguration` object
#' @param name Name of `PlotConfiguration` object in the created code
#' @return R code to recreate the plot configuration as character
#' @export
exportPlotConfigurationCode <- function(plotConfiguration, name = "plotConfiguration") {
  validateIsOfType(plotConfiguration, "PlotConfiguration")
  validateIsString(name)
  plotConfigurationClass <- class(plotConfiguration)[1]

  initializationCode <- c(
    "# Initialize the PlotConfiguration object",
    paste0(name, " <- ", plotConfigurationClass, "$new()")
  )

  labelsCode <- "# Define/Overwrite PlotConfiguration labels properties"
  for (label in c("title", "subtitle", "xlabel", "ylabel")) {
    for (property in c("text", "font$color", "font$size", "font$fontFace", "font$angle")) {
      labelText <- paste0("plotConfiguration$labels$", label, "$", property)
      updatedLabelText <- paste0(name, "$labels$", label, "$", property)
      labelValue <- eval(parse(text = labelText))
      labelsCode <- c(
        labelsCode,
        as.character(.parseValueToObject(updatedLabelText, labelValue))
      )
    }
  }

  backgroundCode <- "# Define/Overwrite PlotConfiguration background properties"
  for (property in c("text", "font$color", "font$size", "font$fontFace", "font$angle")) {
    labelText <- paste0("plotConfiguration$background$watermark$", property)
    updatedLabelText <- paste0(name, "$background$watermark$", property)
    labelValue <- eval(parse(text = labelText))
    backgroundCode <- c(
      backgroundCode,
      as.character(.parseValueToObject(updatedLabelText, labelValue))
    )
  }
  for (backgroundElement in c("plot", "panel", "xAxis", "yAxis", "xGrid", "yGrid")) {
    for (property in c("color", "size", "linetype", "fill")) {
      backgroundText <- paste0("plotConfiguration$background$", backgroundElement, "$", property)
      updatedBackgroundText <- paste0(name, "$background$", backgroundElement, "$", property)
      backgroundValue <- eval(parse(text = backgroundText))
      backgroundCode <- c(
        backgroundCode,
        as.character(.parseValueToObject(updatedBackgroundText, backgroundValue))
      )
    }
  }

  axesCode <- "# Define/Overwrite PlotConfiguration axes properties"
  for (axisElement in c("xAxis", "yAxis")) {
    for (property in c("font$color", "font$size", "font$fontFace", "font$angle", "valuesLimits", "axisLimits", "scale", "ticklabels", "ticks")) {
      axesText <- paste0("plotConfiguration$", axisElement, "$", property)
      updatedAxesText <- paste0(name, "$", axisElement, "$", property)
      axesValue <- eval(parse(text = axesText))
      axesCode <- c(
        axesCode,
        as.character(.parseValueToObject(updatedAxesText, axesValue))
      )
    }
  }

  aestheticSelectionCode <- "# Define/Overwrite PlotConfiguration aesthetics selection properties"
  for (aestheticElement in c("points", "lines", "ribbons", "errorbars")) {
    for (property in c("color", "size", "linetype", "shape", "fill", "alpha")) {
      aestheticText <- paste0("plotConfiguration$", aestheticElement, "$", property)
      updatedAestheticText <- paste0(name, "$", aestheticElement, "$", property)
      aestheticValue <- eval(parse(text = aestheticText))
      aestheticSelectionCode <- c(
        aestheticSelectionCode,
        as.character(.parseValueToObject(updatedAestheticText, aestheticValue))
      )
    }
  }

  legendCode <- "# Define/Overwrite PlotConfiguration legend properties"
  for (property in c("position", "background$fill", "background$color", "background$size", "background$linetype", "title$text", "title$font$color", "title$font$size", "title$font$fontFace", "title$font$angle", "font$color", "font$size", "font$fontFace", "font$angle")) {
    legendText <- paste0("plotConfiguration$legend$", property)
    updatedLegendText <- paste0(name, "$legend$", property)
    legendValue <- eval(parse(text = legendText))
    legendCode <- c(
      legendCode,
      as.character(.parseValueToObject(updatedLegendText, legendValue))
    )
  }

  exportCode <- "# Define/Overwrite PlotConfiguration export properties"
  for (property in c("name", "format", "width", "height", "units", "dpi")) {
    exportText <- paste0("plotConfiguration$export$", property)
    updatedExportText <- paste0(name, "$export$", property)
    exportValue <- eval(parse(text = exportText))
    exportCode <- c(
      exportCode,
      as.character(.parseValueToObject(updatedExportText, exportValue))
    )
  }

  classSpecificCode <- NULL
  if (plotConfigurationClass %in% "BoxWhiskerPlotConfiguration") {
    classSpecificCode <- c(
      "# Define/Overwrite properties specific to BoxWhisker plots",
      paste0(name, "$outliers <- ", plotConfiguration$outliers)
    )
  }
  if (plotConfigurationClass %in% "TornadoPlotConfiguration") {
    classSpecificCode <- "# Define/Overwrite Pproperties specific to Tornado plots"
    for (property in c("bar", "dodge", "colorPalette")) {
      classSpecificText <- paste0("plotConfiguration$", property)
      updatedClassSpecificText <- paste0(name, "$", property)
      classSpecificValue <- eval(parse(text = classSpecificText))
      classSpecificCode <- c(
        classSpecificCode,
        as.character(.parseValueToObject(updatedClassSpecificText, classSpecificValue))
      )
    }
  }

  plotConfigurationCode <- c(
    initializationCode, "\n",
    labelsCode, "\n",
    backgroundCode, "\n",
    axesCode, "\n",
    legendCode, "\n",
    aestheticSelectionCode, "\n",
    exportCode, "\n",
    classSpecificCode
  )

  return(plotConfigurationCode)
}


#' @title updateExportDimensionsForLegend
#' @description Update plot dimensions based on size and position of legend
#' @param plotObject A `ggplot` object
#' @return A `ggplot` object
#' @export
updateExportDimensionsForLegend <- function(plotObject) {
  # Get grob from plot = list of plot properties
  grobObject <- ggplot2::ggplotGrob(plotObject)
  # Look for legend grob that stores the dimensions of the legend
  legendGrobIndex <- which(sapply(grobObject$grobs, function(grob) grob$name) == "guide-box")
  # If no legend, index is empty
  if (isEmpty(legendGrobIndex)) {
    return(plotObject)
  }
  legendGrob <- grobObject$grobs[[legendGrobIndex]]
  # If not empty,
  # - add nothing if legend within
  if (grepl(pattern = "inside", x = getLegendPosition(plotObject))) {
    return(plotObject)
  }
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)
  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$export
  export <- newPlotObject$plotConfiguration$export
  # If unit is in pixels, convert all export dimensions to inches to keep compatibility with older versions of ggplot2
  export$convertPixels()
  # - add legend height to the final plot dimensions if legend above/below
  if (grepl(pattern = "Top", x = getLegendPosition(plotObject)) |
    grepl(pattern = "Bottom", x = getLegendPosition(plotObject))) {
    export$height <- export$height + as.numeric(grid::convertUnit(max(legendGrob$heights), export$units))
    return(newPlotObject)
  }
  # - add legend width to the final plot dimensions if legend left/right
  export$width <- export$width + as.numeric(grid::convertUnit(max(legendGrob$widths), export$units))
  return(newPlotObject)
}
