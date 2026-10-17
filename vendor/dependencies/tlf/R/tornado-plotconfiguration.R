#' @title TornadoPlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object for tornado plots
#' @export
#' @family PlotConfiguration classes
TornadoPlotConfiguration <- R6::R6Class(
  "TornadoPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    #' @field bar logical setting if tornado is uses a bar plot instead of regular points
    bar = NULL,
    #' @field colorPalette color palette property from `ggplot2`
    colorPalette = NULL,
    #' @field dodge space between the bars/points
    dodge = NULL,
    #' @field defaultYScale Default yAxis scale value when creating a `TornadoPlotConfiguration` object
    defaultYScale = "discrete",

    #' @description Create a new `TornadoPlotConfiguration` object
    #' @param bar logical setting if tornado is uses a bar plot instead of regular points
    #' @param colorPalette color palette property from `ggplot2`
    #' @param dodge space between the bars/points
    #' @param ... parameters inherited from `PlotConfiguration`
    #' @return A new `TornadoPlotConfiguration` object
    initialize = function(bar = TRUE,
                          colorPalette = NULL,
                          dodge = 0.5,
                          ...) {
      validateIsLogical(bar)
      validateIsString(colorPalette, nullAllowed = TRUE)
      validateIsNumeric(dodge)
      super$initialize(...)
      self$bar <- bar
      self$colorPalette <- colorPalette
      self$dodge <- dodge
    }
  )
)
