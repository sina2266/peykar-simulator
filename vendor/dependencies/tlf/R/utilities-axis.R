#' @title setXAxis
#' @description Set X-axis properties of a `ggplot` object
#' @param plotObject A `ggplot` object to set X-axis properties
#' @param scale Scale of axis. Use enum `Scaling` to access names of scales.
#' @param valuesLimits Optional numeric values of values limits
#' @param axisLimits Optional numeric values of axis limits
#' @param limits `r lifecycle::badge("deprecated")`. Replaced by axisLimits argument.
#' @param ticks Optional values or function for axis ticks
#' @param ticklabels Optional values or function for axis ticklabels
#' @param minorTicks Optional values or function for axis minor ticks
#' @param font A `Font` object  defining font of ticklabels
#' @param expand Logical defining if data is expanded until axis
#' @return A `ggplot` object
#' @export
#' @examples
#' myPlot <- addLine(x = c(1, 2, 3), y = c(10, 50, 100))
#'
#' # Set x-axis in log scale
#' setXAxis(myPlot, scale = Scaling$log)
#'
#' # Set x-axis ticklabels to Greek letters
#' setXAxis(myPlot, ticks = c(1, 2, 3), ticklabels = parse(text = c("alpha", "beta", "gamma")))
#'
#' # Set x-axis limits
#' setXAxis(myPlot, axisLimits = c(1, 2.5))
#'
#' # Set x-axis fonts
#' setXAxis(myPlot, font = Font$new(color = "blue", size = 14))
setXAxis <- function(plotObject,
                     scale = NULL,
                     valuesLimits = NULL,
                     axisLimits = NULL,
                     limits = lifecycle::deprecated(),
                     ticks = NULL,
                     ticklabels = NULL,
                     minorTicks = NULL,
                     font = NULL,
                     expand = NULL) {
  if (lifecycle::is_present(limits)) {
    lifecycle::deprecate_warn("1.5.0", "setXAxis(limits)", "setXAxis(axisLimits)")
    axisLimits <- limits
  }

  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(scale, Scaling, nullAllowed = TRUE)
  validateIsNumeric(valuesLimits, nullAllowed = TRUE)
  validateIsNumeric(axisLimits, nullAllowed = TRUE)
  validateIsOfType(font, "Font", nullAllowed = TRUE)
  validateIsLogical(expand, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$xAxis
  xAxis <- newPlotObject$plotConfiguration$xAxis
  eval(.parseVariableToObject("xAxis", c("valuesLimits", "axisLimits", "scale", "ticks", "ticklabels", "minorTicks", "font", "expand"), keepIfNull = TRUE))
  newPlotObject <- xAxis$updatePlot(newPlotObject,
    yAxisLimits = newPlotObject$plotConfiguration$yAxis$axisLimits
  )
  return(newPlotObject)
}

#' @title setYAxis
#' @description Set Y-axis properties of a `ggplot` object
#' @inheritParams setXAxis
#' @return A `ggplot` object
#' @export
#' @examples
#' myPlot <- addLine(x = c(1, 2, 3), y = c(10, 50, 100))
#'
#' # Set y-axis in log scale
#' setYAxis(myPlot, scale = Scaling$log)
#'
#' # Set y-axis ticklabels to Greek letters
#' setYAxis(myPlot, ticks = c(10, 50, 100), ticklabels = parse(text = c("alpha", "beta", "gamma")))
#'
#' # Set y-axis limits
#' setYAxis(myPlot, axisLimits = c(10, 75))
#'
#' # Set y-axis fonts
#' setYAxis(myPlot, font = Font$new(color = "blue", size = 14))
setYAxis <- function(plotObject,
                     scale = NULL,
                     valuesLimits = NULL,
                     axisLimits = NULL,
                     limits = lifecycle::deprecated(),
                     ticks = NULL,
                     ticklabels = NULL,
                     minorTicks = NULL,
                     font = NULL,
                     expand = NULL) {
  if (lifecycle::is_present(limits)) {
    lifecycle::deprecate_warn("1.5.0", "setYAxis(limits)", "setYAxis(axisLimits)")
    axisLimits <- limits
  }

  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(scale, Scaling, nullAllowed = TRUE)
  validateIsNumeric(valuesLimits, nullAllowed = TRUE)
  validateIsNumeric(axisLimits, nullAllowed = TRUE)
  validateIsOfType(font, "Font", nullAllowed = TRUE)
  validateIsLogical(expand, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$yAxis
  yAxis <- newPlotObject$plotConfiguration$yAxis
  eval(.parseVariableToObject("yAxis", c("valuesLimits", "axisLimits", "scale", "ticks", "ticklabels", "minorTicks", "font", "expand"), keepIfNull = TRUE))
  newPlotObject <- yAxis$updatePlot(newPlotObject,
    xAxisLimits = newPlotObject$plotConfiguration$xAxis$axisLimits
  )
  return(newPlotObject)
}

#' @title setY2Axis
#' @description Set right Y-axis properties of a `ggplot` object
#' @inheritParams setXAxis
#' @return A `ggplot` object
#' @export
setY2Axis <- function(plotObject,
                      scale = NULL,
                      valuesLimits = NULL,
                      axisLimits = NULL,
                      limits = lifecycle::deprecated(),
                      ticks = NULL,
                      ticklabels = NULL,
                      minorTicks = NULL,
                      font = NULL,
                      expand = NULL) {
  if (lifecycle::is_present(limits)) {
    lifecycle::deprecate_warn("1.5.0", "setY2Axis(limits)", "setY2Axis(axisLimits)")
    axisLimits <- limits
  }

  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(scale, Scaling, nullAllowed = TRUE)
  validateIsNumeric(valuesLimits, nullAllowed = TRUE)
  validateIsNumeric(axisLimits, nullAllowed = TRUE)
  validateIsOfType(font, "Font", nullAllowed = TRUE)
  validateIsLogical(expand, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$yAxis
  y2Axis <- newPlotObject$plotConfiguration$y2Axis %||% YAxisConfiguration$new()
  y2Axis$position <- "right"
  eval(.parseVariableToObject("y2Axis", c("valuesLimits", "axisLimits", "scale", "ticks", "ticklabels", "minorTicks", "font", "expand"), keepIfNull = TRUE))
  newPlotObject <- y2Axis$updatePlot(newPlotObject,
    xAxisLimits = newPlotObject$plotConfiguration$xAxis$axisLimits
  )
  return(newPlotObject)
}


#' @title getLogTickLabels
#' @description Get ticklabels expressions for log scale plots
#' @param ticks numeric values of the ticks
#' @return Expressions to use in `ticklabels` input parameter of `setXAxis` and `setYAxis` functions
#' @examples
#' ticks <- c(1, 5, 10, 50, 100, 500)
#' getLogTickLabels(ticks)
#' @export
getLogTickLabels <- function(ticks) {
  exponentValues <- floor(log10(ticks))
  # Values to print before 10^ using multiplication dot
  prefixValues <- ticks * 10^(-exponentValues)
  prefixValues <- paste0(prefixValues, "%*%")
  # For 1 the multiplication is redundant and removed
  prefixValues[prefixValues == "1%*%"] <- ""
  return(scales:::parse_safe(text = paste(prefixValues, "10^", exponentValues, sep = "")))
}

#' @title getLnTickLabels
#' @description Get ticklabels expressions for ln scale plots
#' @param ticks numeric values of the ticks
#' @return Expressions to use in `ticklabels` input parameter of `setXAxis` and `setYAxis` functions
#' @examples
#' ticks <- exp(c(1, 5, 10, 50, 100, 500))
#' getLnTickLabels(ticks)
#' @export
getLnTickLabels <- function(ticks) {
  exponentValues <- floor(log(ticks))
  # Values to print before 10^ using multiplication dot
  prefixValues <- ticks * exp(-exponentValues)
  prefixValues <- paste0(prefixValues, "%*%")
  # For 1 the multiplication is redundant and removed
  prefixValues[prefixValues == "1%*%"] <- ""
  return(scales:::parse_safe(text = paste(prefixValues, "e^", exponentValues, sep = "")))
}


#' @title getSqrtTickLabels
#' @description Get ticklabels expressions for sqrt scale plots
#' @param ticks numeric values of the ticks
#' @return Expressions to use in `ticklabels` input parameter of `setXAxis` and `setYAxis` functions
#' @examples
#' ticks <- sqrt(c(1, 5, 10, 50, 100, 500))
#' getSqrtTickLabels(ticks)
#' @export
getSqrtTickLabels <- function(ticks) {
  sqrtValues <- ticks^2
  return(scales:::parse_safe(text = paste("sqrt(", sqrtValues, ")", sep = "")))
}

#' @title getGreekTickLabels
#' @description Get ticklabels expressions for discrete scale plots with greek letters
#' @param ticks numeric values of the ticks
#' @return Expressions to use in `ticklabels` input parameter of `setXAxis` and `setYAxis` functions
#' @examples
#' ticks <- c(1, 5, 10, 50, 100, 500)
#' getGreekTickLabels(ticks)
#' @export
getGreekTickLabels <- function(ticks) {
  # alpha starts at converted integer 945
  if (is.numeric(ticks)) {
    return(sapply(ticks, function(tick) {
      intToUtf8(tick + 944)
    }))
  }
  tickLabels <- sapply(1:length(ticks), function(tick) {
    intToUtf8(tick + 944)
  })
  return(tickLabels)
}

#' @title getPiTickLabels
#' @description Get ticklabels expressions for plots with values as ratios of Pi
#' @param ticks numeric values of the ticks
#' @return Expressions to use in `ticklabels` input parameter of `setXAxis` and `setYAxis` functions
#' @examples
#' ticks <- seq(0, 2 * pi, pi / 2)
#' getPiTickLabels(ticks)
#' @export
getPiTickLabels <- function(ticks) {
  # Get fractions of pi from ticks
  roundPi <- as.character(ticks %/% pi)
  # Remove 1 and -1 from expression
  roundPi[roundPi == "1"] <- ""
  roundPi[roundPi == "-1"] <- "-"
  # Flag when 0 to remove pi from label
  roundPi[roundPi == "0"] <- "x"

  roundPi <- paste(roundPi, "\u03C0", sep = "")
  roundPi[grepl("x", roundPi)] <- ""

  # Round to 3 digits to get fraction values
  # If fraction is recognized, used fraction format
  decPi <- round((ticks %% pi) / pi, 3)
  decPi <- sapply(decPi, function(piFraction) {
    if (piFraction == 0) {
      return("")
    }
    if (piFraction == 0.167) {
      return(" + \u03C0/6")
    }
    if (piFraction == 0.25) {
      return(" + \u03C0/4")
    }
    if (piFraction == 0.333) {
      return(" + \u03C0/3")
    }
    if (piFraction == 0.5) {
      return(" + \u03C0/2")
    }
    if (piFraction == 0.667) {
      return(" + 2\u03C0/3")
    }
    if (piFraction == 0.833) {
      return(" + 5\u03C0/6")
    }
    return(paste0("+", piFraction, "\u03C0"))
  })
  piLabels <- paste(roundPi, decPi, sep = "")
  piLabels[piLabels == ""] <- "0"
  return(piLabels)
}

#' @title getPercentileTickLabels
#' @description Get ticklabels expressions for percentiles of normal distribution scale plots
#' @param ticks numeric values of the ticks
#' @return Expressions to use in `ticklabels` input parameter of `setXAxis` and `setYAxis` functions
#' @examples
#' ticks <- rnorm(5)
#' getPercentileTickLabels(ticks)
#'
#' # Get percentile of normal distribution
#' ticks <- qnorm(seq(1, 9) / 10)
#' getPercentileTickLabels(ticks)
#'
#' @export
getPercentileTickLabels <- function(ticks) {
  return(paste0(round(100 * stats::pnorm(ticks), 3), "%"))
}

#' @title .removeInfiniteValues
#' @description Censor/remove any values outside of range
#' Caution, removing infinite values can cause issues with ribbons
#' which can use such infinite values for filling a range
#' @param x numeric vector of values to manipulate
#' @param range numeric vector of length two giving desired output range
#' @keywords internal
.removeInfiniteValues <- function(x, range = c(0, 1)) {
  scales::censor(x, range, only.finite = FALSE)
}
