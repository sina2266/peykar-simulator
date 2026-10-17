#' @title PKRatioDataMapping
#' @description  R6 class for mapping `x`, `y`, `GroupMapping` and pkRatio `lines` variables to `data`
#' @export
#' @family DataMapping classes
PKRatioDataMapping <- R6::R6Class(
  "PKRatioDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @field lines list of ratio limits to plot as horizontal lines
    lines = NULL,
    #' @field ymin mapping of upper value of error bars around scatter points
    ymin = NULL,
    #' @field ymax mapping of lower value of error bars around scatter points
    ymax = NULL,

    #' @description Create a new `PKRatioDataMapping` object
    #' @param x Name of x variable to map
    #' @param y Name of y variable to map
    #' @param ymin mapping of upper value of error bars around scatter points
    #' @param ymax mapping of lower value of error bars around scatter points
    #' @param lines List of ratio limits to display as horizontal lines
    #' @param ... parameters inherited from `XYGDataMapping`
    #' @return A new `PKRatioDataMapping` object
    initialize = function(x = NULL,
                          y = NULL,
                          ymin = NULL,
                          ymax = NULL,
                          lines = DefaultDataMappingValues$pkRatio,
                          ...) {
      validateIsString(ymin, nullAllowed = TRUE)
      validateIsString(ymax, nullAllowed = TRUE)
      super$initialize(x = x, y = y, ...)
      self$lines <- lines
      # If no ymin/ymax defined, map to y to get emtpy errorbars
      self$ymin <- ymin %||% self$y
      self$ymax <- ymax %||% self$y
    },

    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `defaultAes` variables.
    #' Dummy variable `defaultAes` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      validateIsOfType(data, "data.frame")
      .validateMapping(self$ymin, data, nullAllowed = TRUE)
      .validateMapping(self$ymax, data, nullAllowed = TRUE)
      mapData <- super$checkMapData(data, metaData)
      mapData[, self$ymin] <- data[, self$ymin]
      mapData[, self$ymax] <- data[, self$ymax]
      self$data <- mapData
      return(mapData)
    }
  )
)
