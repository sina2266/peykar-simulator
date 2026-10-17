#' @title PKRatioPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for PK ratio plots
#' @field defaultYScale Default yAxis scale value when creating a `PKRatioPlotConfiguration` object
#' @export
#' @family PlotConfiguration classes
PKRatioPlotConfiguration <- R6::R6Class(
  "PKRatioPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    defaultYScale = "log"
  )
)

#' @title DDIRatioPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for DDI ratio plots
#' @field defaultXScale Default xAxis scale value when creating a `DDIRatioPlotConfiguration` object
#' @field defaultYScale Default yAxis scale value when creating a `DDIRatioPlotConfiguration` object
#' @field defaultExpand Default expand value when creating a `DDIRatioPlotConfiguration` object
#' @export
#' @family PlotConfiguration classes
DDIRatioPlotConfiguration <- R6::R6Class(
  "DDIRatioPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    defaultXScale = "log",
    defaultYScale = "log",
    defaultExpand = TRUE
  )
)

#' @title ObsVsPredPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for Obs vs Pred plots
#' @field defaultSymmetricAxes Default option setting symmetric xAxis and/or yAxis limits when creating a `ObsVsPredPlotConfiguration` object
#' @field lloqDirection Whether to draw LLOQ lines for x (vertical), y (horizontal) or x and y (both).
#' @field foldLinesLegend Whether to draw fold lines in legend. default to FALSE.
#' @field foldLinesLegendDiagonal Whether to draw diagonal lines in legend for fold lines. default to FALSE.
#' @field foldLineslegendType translation of `foldLinesLegendDiagonal` in geom type.
#' @export
#' @family PlotConfiguration classes
ObsVsPredPlotConfiguration <- R6::R6Class(
  "ObsVsPredPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    defaultSymmetricAxes = TRUE,
    lloqDirection = NULL,
    foldLinesLegend = NULL,
    foldLinesLegendDiagonal = NULL,
    #' @description Create a new `ObsVsPredPlotConfiguration` object
    #' @param lloqDirection Whether to draw LLOQ lines for x (vertical), y (horizontal) or x and y (both).
    #' @param foldLinesLegend Whether to draw fold lines in legend. default to FALSE.
    #' @param foldLinesLegendDiagonal Whether to draw diagonal lines in legend for fold lines. default to FALSE.
    #' @param ... parameters inherited from `PlotConfiguration`
    #' @return A new `CumulativeTimeProfilePlotConfiguration` object
    initialize = function(lloqDirection = "vertical",
                          foldLinesLegend = FALSE,
                          foldLinesLegendDiagonal = FALSE,
                          ...) {
      super$initialize(...)
      validateEnumValue(lloqDirection, Directions)
      self$lloqDirection <- lloqDirection
      self$foldLinesLegend <- foldLinesLegend
      self$foldLinesLegendDiagonal <- foldLinesLegendDiagonal
    }
  ),
  active = list(
    foldLineslegendType = function(foldLinesLegendDiagonal = self$foldLinesLegendDiagonal) {
      if (foldLinesLegendDiagonal) {
        return("abline")
      } else {
        return("path")
      }
    }
  )
)

#' @title ResVsPredPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for Res vs Pred/Time plots
#' @field defaultSymmetricAxes Default option setting symmetric xAxis and/or yAxis limits when creating a `ResVsPredPlotConfiguration` object
#' @export
#' @family PlotConfiguration classes
ResVsPredPlotConfiguration <- R6::R6Class(
  "ResVsPredPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    defaultSymmetricAxes = TRUE
  )
)

#' @title ResVsTimePlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for Res vs Pred/Time plots
#' @export
#' @family PlotConfiguration classes
ResVsTimePlotConfiguration <- R6::R6Class(
  "ResVsTimePlotConfiguration",
  inherit = ResVsPredPlotConfiguration
)

#' @title HistogramPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for histograms
#' @export
#' @family PlotConfiguration classes
HistogramPlotConfiguration <- R6::R6Class(
  "HistogramPlotConfiguration",
  inherit = PlotConfiguration
)

#' @title QQPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for Quantile-Quantile plots
#' @export
#' @family PlotConfiguration classes
QQPlotConfiguration <- R6::R6Class(
  "QQPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    #' @description Create a new `QQPlotConfiguration` object
    #' @param xlabel QQ-plot default display is "Standard Normal Quantiles"
    #' @param ... parameters inherited from `PlotConfiguration`
    #' @return A new `QQPlotConfiguration` object
    initialize = function(xlabel = "Standard Normal Quantiles",
                          ...) {
      super$initialize(xlabel = xlabel, ...)
    }
  )
)

#' @title CumulativeTimeProfilePlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for cumulative time profile plots
#' @field defaultExpand Default expand value when creating a `CumulativeTimeProfilePlotConfiguration` object
#' @field colorPalette color palette property from `ggplot2`
#' @export
#' @family PlotConfiguration classes
CumulativeTimeProfilePlotConfiguration <- R6::R6Class(
  "CumulativeTimeProfilePlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    defaultExpand = TRUE,
    colorPalette = NULL,

    #' @description Create a new `CumulativeTimeProfilePlotConfiguration` object
    #' @param colorPalette color palette property from `ggplot2`
    #' @param ... parameters inherited from `PlotConfiguration`
    #' @return A new `CumulativeTimeProfilePlotConfiguration` object
    initialize = function(colorPalette = NULL, ...) {
      validateIsString(colorPalette, nullAllowed = TRUE)
      super$initialize(...)
      self$colorPalette <- colorPalette
    }
  )
)

#' @title PieChartPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for pie charts
#' @field colorPalette color palette property from `ggplot2`
#' @field chartFont `Font` object defining properties of text within pie chart
#' @field start Offset of starting point from 12 o'clock in radians.
#' Offset is applied clockwise or anticlockwise depending on value of direction
#' @field clockwiseDirection logical defining if values are displayed in clockwise order
#' @export
#' @family PlotConfiguration classes
PieChartPlotConfiguration <- R6::R6Class(
  "PieChartPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    colorPalette = NULL,
    chartFont = NULL,
    start = NULL,
    clockwiseDirection = NULL,

    #' @description Create a new `PieChartPlotConfiguration` object
    #' @param colorPalette color palette property from `ggplot2`
    #' @param chartFont `Font` object defining properties of text within pie chart
    #' @param start Offset of starting point from 12 o'clock in radians.
    #' Offset is applied clockwise or anticlockwise depending on value of direction
    #' @param clockwiseDirection logical defining if values are displayed in clockwise order
    #' @param ... parameters inherited from `PlotConfiguration`
    #' @return A new `PieChartPlotConfiguration` object
    initialize = function(colorPalette = NULL,
                          chartFont = NULL,
                          start = 0,
                          clockwiseDirection = TRUE,
                          ...) {
      validateIsString(colorPalette, nullAllowed = TRUE)
      validateIsOfType(chartFont, "Font", nullAllowed = TRUE)
      validateIsNumeric(start)
      validateIsLogical(clockwiseDirection)
      super$initialize(...)

      self$colorPalette <- colorPalette
      self$chartFont <- chartFont %||% Font$new()
      self$start <- start
      self$clockwiseDirection <- clockwiseDirection
    }
  )
)
