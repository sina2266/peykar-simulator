#' @title BoxWhiskerPlotConfiguration
#' @description  R6 class defining the configuration of a `ggplot` object for boxplots
#' @export
#' @family PlotConfiguration classes
BoxWhiskerPlotConfiguration <- R6::R6Class(
  "BoxWhiskerPlotConfiguration",
  inherit = PlotConfiguration,
  public = list(
    #' @field defaultXScale Default xAxis scale value when creating a `BoxWhiskerPlotConfiguration` object
    defaultXScale = "discrete",

    #' @description Create a new `BoxWhiskerPlotConfiguration` object
    #' @param outliers logical defining if outliers should be included in boxplot
    #' @param ... parameters inherited from `PlotConfiguration`
    #' @return A new `BoxWhiskerPlotConfiguration` object
    initialize = function(outliers = TRUE, ...) {
      validateIsLogical(outliers)
      super$initialize(...)
      private$.outliers <- outliers
    }
  ),
  active = list(
    #' @field outliers logical defining if outliers should be included in boxplot
    outliers = function(value) {
      if (missing(value)) {
        return(private$.outliers)
      }
      validateIsLogical(value, nullAllowed = TRUE)
      private$.outliers <- value %||% private$.outliers
      return(invisible())
    }
  ),
  private = list(
    .outliers = NULL
  )
)
