#' @title TornadoDataMapping
#' @description  R6 class for mapping `values`, `labels` to `data`
#' @export
#' @family DataMapping classes
TornadoDataMapping <- R6::R6Class(
  "TornadoDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @field lines numeric vector of limits to plot
    lines = NULL,
    #' @field sorted logical indicating if values should be sorted
    sorted = NULL,

    #' @description Create a new `TornadoDataMapping` object
    #' @param lines numeric vector of limits to plot
    #' @param sorted logical indicating if values should be sorted
    #' @param x Variable including the values of tornado plot
    #' @param y Variable including the labels of tornado plot
    #' @param ... parameters inherited from `XYGDataMapping`
    #' @return A new `TornadoDataMapping` object
    initialize = function(lines = DefaultDataMappingValues$tornado,
                          sorted = NULL,
                          x = NULL,
                          y = NULL,
                          ...) {
      validateIsNumeric(lines, nullAllowed = TRUE)
      validateIsLogical(sorted, nullAllowed = TRUE)
      super$initialize(x = x, y = y, ...)

      # The next lines aim at linking some variables in case they are not
      # explicitely defined by users, this leads to nice plot legends in the end

      # Link color to labels (y) if color is not explicitely mapped
      self$groupMapping$color$label <- self$groupMapping$color$label %||% y
      self$groupMapping$color$group <- self$groupMapping$color$group %||% y
      # Link fill/shape to color if they are not explicitely mapped
      self$groupMapping$fill <- ifNotNull(
        self$groupMapping$fill$label,
        self$groupMapping$fill,
        self$groupMapping$color
      )
      self$groupMapping$shape <- ifNotNull(
        self$groupMapping$shape$label,
        self$groupMapping$shape,
        self$groupMapping$color
      )

      self$lines <- lines
      self$sorted <- sorted %||% TRUE
    },

    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `defaultAes` variables.
    #' Dummy variable `defaultAes` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      mapData <- super$checkMapData(data, metaData)
      # Enforce y to be a discrete variable preventing crashes from numerical values
      if (!isOfLength(self$y, 0)) {
        mapData[, self$y] <- as.factor(mapData[, self$y])
      }
      return(mapData)
    }
  )
)
