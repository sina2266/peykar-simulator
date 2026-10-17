#' @title .createPlotScale
#' @description Translate scale into one of the values available in the enum `Scaling`.
#' @param scale character defining the name of the scale
#' @return A value available in enum `Scaling`
#' @keywords internal
.createPlotScale <- function(scale) {
  validateIsString(scale)
  if (isIncluded(tolower(scale), c("identity", "lin", "linear", "default", "normal"))) {
    return("identity")
  }
  if (isIncluded(tolower(scale), c("log", "logarithmic", "log10"))) {
    return(Scaling$log)
  }
  validateIsIncluded(tolower(scale), Scaling)
  return(tolower(scale))
}

#' @title .createPlotTicks
#' @description Translate ticks and ticklabels into a value directly usable by `ggplot2`
#' to give more flexibility in the next functions
#' @param ticks character, numeric or function defining the ticks
#' @return name of the `ggplot2` scale
#' @keywords internal
.createPlotTicks <- function(ticks) {
  if (isEmpty(ticks)) {
    return(waiver())
  }
  if (isOfType(ticks, c("numeric", "function", "expression"))) {
    return(ticks)
  }
  # If character
  if (isIncluded(ticks, c("default", "identity"))) {
    return(waiver())
  }
  if (isIncluded(ticks, c("none"))) {
    return(NULL)
  }
  return(ticks)
}

#' @title .createPlotTickLabels
#' @description Translate ticks and ticklabels into a value directly usable by `ggplot2`
#' to give more flexibility in the next functions
#' @param ticklabels character, numeric or function defining the ticks
#' @return name of the `ggplot2` scale
#' @keywords internal
.createPlotTickLabels <- function(ticklabels) {
  if (isEmpty(ticklabels)) {
    return(waiver())
  }
  if (isOfType(ticklabels, c("numeric", "function", "expression"))) {
    return(ticklabels)
  }
  if (isIncluded(ticklabels, TickLabelTransforms)) {
    transformedLabels <- switch(ticklabels,
      "default" = waiver(),
      "none" = NULL,
      "identity" = identity,
      "log" = getLogTickLabels,
      "ln" = getLnTickLabels,
      "sqrt" = getSqrtTickLabels,
      "greek" = getGreekTickLabels,
      "pi" = getPiTickLabels
    )
    return(transformedLabels)
  }
  return(ticklabels)
}

#' @title AxisConfiguration
#' @description  R6 class defining the configuration of axis
#' @export
#' @family PlotConfiguration classes
AxisConfiguration <- R6::R6Class(
  "AxisConfiguration",
  public = list(
    #' @description Create a new `AxisConfiguration` object
    #' @param valuesLimits numeric vector of value limits (data outside these limits is removed)
    #' @param axisLimits numeric vector of axis limits (data outside these limits is kept but not plotted)
    #' @param limits `r lifecycle::badge("deprecated")`. Replaced by axisLimits argument.
    #' @param scale character defining axis scale
    #' Use enum `Scaling` to access predefined scales.
    #' @param ticks numeric vector or function defining where to position axis ticks
    #' @param ticklabels character vector or function defining what to print on axis ticks
    #' @param minorTicks numeric vector or function defining where to position minor axis ticks
    #' @param font `Font` object defining the font of ticklabels
    #' @param expand logical defining if data is expanded until axis.
    #' If `TRUE`, data is expanded until axis
    #' If `FALSE`, some space between data and axis is kept
    #' @return A new `AxisConfiguration` object
    initialize = function(valuesLimits = NULL,
                          axisLimits = NULL,
                          limits = lifecycle::deprecated(),
                          scale = Scaling$lin,
                          ticks = NULL,
                          ticklabels = NULL,
                          minorTicks = NULL,
                          font = NULL,
                          expand = FALSE) {
      if (lifecycle::is_present(limits)) {
        lifecycle::deprecate_warn(
          "1.5.0",
          "AxisConfiguration(limits)",
          "AxisConfiguration(axisLimits)"
        )
        axisLimits <- limits
      }

      validateIsNumeric(valuesLimits, nullAllowed = TRUE)
      validateIsNumeric(axisLimits, nullAllowed = TRUE)
      validateIsOfType(font, "Font", nullAllowed = TRUE)
      validateIsLogical(expand)
      private$.valuesLimits <- valuesLimits
      private$.axisLimits <- axisLimits

      scale <- scale %||% Scaling$lin
      private$.scale <- .createPlotScale(scale)
      private$.ticks <- .createPlotTicks(ticks)
      private$.ticklabels <- .createPlotTickLabels(ticklabels)
      private$.minorTicks <- .createPlotTicks(minorTicks)
      private$.expand <- expand

      # Default axis font will use theme
      defaultFont <- Font$new()
      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)
      if (isOfType(self, "XAxisConfiguration")) {
        defaultFont <- currentTheme$fonts$xAxis
      }
      if (isOfType(self, "YAxisConfiguration")) {
        defaultFont <- currentTheme$fonts$yAxis
      }
      private$.font <- font %||% defaultFont
    },

    #' @description Get the `ggplot2` actual `trans` name of scale
    #' @return A character included in `ggplot2` available `trans` names
    ggplotScale = function() {
      switch(private$.scale,
        "log" = "log10",
        "ln" = "log",
        private$.scale
      )
    },

    #' @description Get the `ggplot2` actual function for expansion
    #' @return A `ggplot2` function
    ggplotExpansion = function() {
      if (private$.expand) {
        return(ggplot2::expansion())
      }
      return(ggplot2::waiver())
    },

    #' @description Get tick values for pretty default log plots
    #' @return User defined tick values or tlf default ticks
    prettyTicks = function() {
      # A waiver is a ggplot2 "flag" object, similar to NULL,
      # that indicates the calling function should just use the default value
      if (!isOfType(private$.ticks, "waiver")) {
        return(private$.ticks)
      }
      # Default tick values as a function of scale
      switch(private$.scale,
        "log" = tlfEnv$logTicks,
        "ln" = tlfEnv$lnTicks,
        private$.ticks
      )
    },

    #' @description Get tick values for pretty default log plots
    #' @return User defined tick values or tlf default ticks
    prettyMinorTicks = function() {
      # A waiver is a ggplot2 "flag" object, similar to NULL,
      # that indicates the calling function should just use the default value
      if (!isOfType(private$.minorTicks, "waiver")) {
        return(private$.minorTicks)
      }
      # Default tick values as a function of scale
      if (isIncluded(private$.scale, Scaling$log)) {
        return(tlfEnv$logMinorTicks)
      }
      return(private$.minorTicks)
    },

    #' @description Get tick labels for pretty default log plots
    #' @return User defined tick labels or tlf default ticklabels
    prettyTickLabels = function() {
      # A waiver is a ggplot2 "flag" object, similar to NULL,
      # that indicates the calling function should just use the default value
      if (!isOfType(private$.ticklabels, "waiver")) {
        return(private$.ticklabels)
      }
      # Default tick labels as a function of scale
      # ggplot2 accepts functions as input for labels
      switch(private$.scale,
        "log" = getLogTickLabels,
        "ln" = getLnTickLabels,
        "sqrt" = getSqrtTickLabels,
        "percentiles" = getPercentileTickLabels,
        private$.ticklabels
      )
    }
  ),
  active = list(
    #' @field valuesLimits numeric vector of length 2 defining limits of axis.
    #' A value of `NULL` is allowed and lead to default `ggplot2` behaviour
    valuesLimits = function(value) {
      if (missing(value)) {
        return(private$.valuesLimits)
      }
      validateIsNumeric(value, nullAllowed = TRUE)
      if (isOfLength(value, 0)) {
        private$.valuesLimits <- NULL
        return(invisible())
      }
      validateIsOfLength(value, 2)
      private$.valuesLimits <- value
      return(invisible())
    },
    #' @field axisLimits numeric vector of length 2 defining limits of axis.
    #' A value of `NULL` is allowed and lead to default `ggplot2` behaviour
    axisLimits = function(value) {
      if (missing(value)) {
        return(private$.axisLimits)
      }
      validateIsNumeric(value, nullAllowed = TRUE)
      if (isOfLength(value, 0)) {
        private$.axisLimits <- NULL
        return(invisible())
      }
      validateIsOfLength(value, 2)
      private$.axisLimits <- value
      return(invisible())
    },
    #' @field scale name of axis scale from Enum `Scaling`
    #' A value of `NULL` is allowed and will lead to a default linear scale
    scale = function(value) {
      if (missing(value)) {
        return(private$.scale)
      }
      value <- value %||% Scaling$lin
      private$.scale <- .createPlotScale(value)
      return(invisible())
    },
    #' @field ticks function or values defining where axis ticks are placed
    ticks = function(value) {
      if (missing(value)) {
        return(private$.ticks)
      }
      private$.ticks <- .createPlotTicks(value)
      return(invisible())
    },
    #' @field minorTicks function or values defining where axis minor ticks are placed
    minorTicks = function(value) {
      if (missing(value)) {
        return(private$.minorTicks)
      }
      private$.minorTicks <- .createPlotTicks(value)
      return(invisible())
    },
    #' @field ticklabels function or values defining the axis tick labels
    ticklabels = function(value) {
      if (missing(value)) {
        return(private$.ticklabels)
      }
      private$.ticklabels <- .createPlotTickLabels(value)
      return(invisible())
    },
    #' @field font `Font` object defining the font of the ticklabels
    font = function(value) {
      if (missing(value)) {
        return(private$.font)
      }
      validateIsOfType(value, "Font", nullAllowed = TRUE)
      # Default axis font will use theme
      defaultFont <- Font$new()
      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)
      if (isOfType(self, "XAxisConfiguration")) {
        defaultFont <- currentTheme$fonts$xAxis
      }
      if (isOfType(self, "YAxisConfiguration")) {
        defaultFont <- currentTheme$fonts$yAxis
      }
      private$.font <- value %||% defaultFont
      return(invisible())
    },
    #' @field expand logical defining if data is expanded until axis.
    #' If `TRUE`, data is expanded until axis
    #' If `FALSE`, some space between data and axis is kept
    expand = function(value) {
      if (missing(value)) {
        return(private$.expand)
      }
      validateIsLogical(value)
      private$.expand <- value
      return(invisible())
    }
  ),
  private = list(
    .valuesLimits = NULL,
    .axisLimits = NULL,
    .scale = NULL,
    .ticks = NULL,
    .ticklabels = NULL,
    .minorTicks = NULL,
    .font = NULL,
    .expand = NULL
  )
)

#' @title XAxisConfiguration
#' @description  R6 class defining the configuration of X-axis
#' @export
#' @family PlotConfiguration classes
XAxisConfiguration <- R6::R6Class(
  "XAxisConfiguration",
  inherit = AxisConfiguration,
  public = list(
    #' @description Update axis configuration on a `ggplot` object
    #' @param plotObject `ggplot` object
    #' @param yAxisLimits values of axisLimits for `y` axis to prevent `coord_cartesian` to overwrite its properties
    #' @param ylim `r lifecycle::badge("deprecated")`. Replaced by yAxisLimits argument.
    #' @return A `ggplot` object with updated axis properties
    updatePlot = function(plotObject, yAxisLimits = NULL, ylim = lifecycle::deprecated()) {
      if (lifecycle::is_present(ylim)) {
        lifecycle::deprecate_warn("1.5.0", "XAxisConfiguration(ylim)", "XAxisConfiguration(yAxisLimits)")
        yAxisLimits <- ylim
      }

      validateIsOfType(plotObject, "ggplot")
      # Update font properties
      plotObject <- plotObject + ggplot2::theme(axis.text.x = private$.font$createPlotTextFont())
      # Update limits using coor_cartesian to prevent ggplot to remove data and crash
      suppressMessages(
        plotObject <- plotObject + ggplot2::coord_cartesian(xlim = private$.axisLimits, ylim = yAxisLimits)
      )
      # Update ticks and their labels for discrete scale
      if (isIncluded(private$.scale, Scaling$discrete)) {
        suppressMessages(
          plotObject <- plotObject +
            ggplot2::scale_x_discrete(
              breaks = private$.ticks,
              labels = private$.ticklabels,
              expand = self$ggplotExpansion()
            )
        )
        return(plotObject)
      }
      # Most of ggplot2 scales lead to unwanted warning messages
      # `try` should be added in cases of scale breaking because all the ggplot object elements are not yet in place
      suppressMessages(
        plotObject <- plotObject +
          ggplot2::scale_x_continuous(
            limits = private$.valuesLimits,
            trans = self$ggplotScale(),
            breaks = self$prettyTicks(),
            minor_breaks = self$prettyMinorTicks(),
            labels = self$prettyTickLabels(),
            expand = self$ggplotExpansion(),
            oob = .removeInfiniteValues
          )
      )
      if (!isIncluded(private$.scale, c(Scaling$log, Scaling$ln))) {
        return(plotObject)
      }
      # Checks that the final plot limits include at least one pretty log tick
      plotScaleData <- ggplot2::layer_scales(plotObject)
      xDataRange <- switch(private$.scale,
        "log" = 10^plotScaleData$x$range$range,
        "ln" = exp(plotScaleData$x$range$range)
      )
      if (!isEmpty(private$.valuesLimits)) {
        xDataRange <- private$.valuesLimits
      }

      if (!.isLogTicksIncludedInLimits(xDataRange, private$.scale)) {
        return(plotObject)
      }
      # Add special tick lines for pretty log plots
      suppressMessages({
        plotObject <- switch(private$.scale,
          "log" = plotObject + ggplot2::annotation_logticks(sides = "b", color = private$.font$color),
          "ln" = plotObject + ggplot2::annotation_logticks(base = exp(1), sides = "b", color = private$.font$color),
          plotObject
        )
      })
      return(plotObject)
    }
  )
)

#' @title YAxisConfiguration
#' @description  R6 class defining the configuration of Y-axis
#' @export
#' @family PlotConfiguration classes
YAxisConfiguration <- R6::R6Class(
  "YAxisConfiguration",
  inherit = AxisConfiguration,
  public = list(
    #' @field position character position of the Y-axis
    position = "left",

    #' @description Update axis configuration on a `ggplot` object
    #' @param plotObject `ggplot` object
    #' @param xAxisLimits limits of `x` axis to prevent `coord_cartesian` to overwrite its properties
    #' @param xlim `r lifecycle::badge("deprecated")`. Replaced by xAxisLimits argument.
    #' @return A `ggplot` object with updated axis properties
    updatePlot = function(plotObject, xAxisLimits = NULL, xlim = lifecycle::deprecated()) {
      if (lifecycle::is_present(xlim)) {
        lifecycle::deprecate_warn("1.5.0", "YAxisConfiguration(xlim)", "YAxisConfiguration(xAxisLimits)")
        xAxisLimits <- xlim
      }


      validateIsOfType(plotObject, "ggplot")
      # Update font properties
      plotObject <- plotObject + switch(self$position,
        "left" = ggplot2::theme(axis.text.y = private$.font$createPlotTextFont()),
        "right" = ggplot2::theme(axis.text.y.right = private$.font$createPlotTextFont())
      )

      suppressMessages(
        plotObject <- plotObject + ggplot2::coord_cartesian(xlim = xAxisLimits, ylim = private$.axisLimits)
      )
      # Update ticks and their labels for discrete scale
      if (isIncluded(private$.scale, Scaling$discrete)) {
        suppressMessages(
          plotObject <- plotObject +
            ggplot2::scale_y_discrete(
              position = self$position,
              breaks = private$.ticks,
              labels = private$.ticklabels,
              expand = self$ggplotExpansion()
            )
        )
        return(plotObject)
      }
      # Most of ggplot2 scales lead to unwanted warning messages
      # `try` should be added in cases of scale breaking because all the ggplot object elements are not yet in place
      suppressMessages(
        plotObject <- plotObject +
          ggplot2::scale_y_continuous(
            limits = private$.valuesLimits,
            position = self$position,
            trans = self$ggplotScale(),
            breaks = self$prettyTicks(),
            minor_breaks = self$prettyMinorTicks(),
            labels = self$prettyTickLabels(),
            expand = self$ggplotExpansion(),
            oob = .removeInfiniteValues
          )
      )
      if (!isIncluded(private$.scale, c(Scaling$log, Scaling$ln))) {
        return(plotObject)
      }
      # Checks that the final plot limits include at least one pretty log tick
      plotScaleData <- ggplot2::layer_scales(plotObject)
      yDataRange <- switch(private$.scale,
        "log" = 10^plotScaleData$y$range$range,
        "ln" = exp(plotScaleData$y$range$range)
      )
      if (!isEmpty(private$.valuesLimits)) {
        yDataRange <- private$.valuesLimits
      }

      if (!.isLogTicksIncludedInLimits(yDataRange, private$.scale)) {
        return(plotObject)
      }
      suppressMessages({
        plotObject <- switch(private$.scale,
          "log" = plotObject + ggplot2::annotation_logticks(
            sides = switch(self$position,
              "left" = "l",
              "right" = "r"
            ),
            color = private$.font$color
          ),
          "ln" = plotObject + ggplot2::annotation_logticks(
            base = exp(1),
            sides = switch(self$position,
              "left" = "l",
              "right" = "r"
            ),
            color = private$.font$color
          ),
          plotObject
        )
      })
      return(plotObject)
    }
  )
)
