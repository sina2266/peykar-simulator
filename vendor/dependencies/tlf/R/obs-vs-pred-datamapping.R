#' @title ObsVsPredDataMapping
#' @description  Class for mapping variables in observations vs predictions plot
#' @export
#' @family DataMapping classes
ObsVsPredDataMapping <- R6::R6Class(
  "ObsVsPredDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @field lines list of ratio limits to plot as horizontal lines
    lines = NULL,
    #' @field xmin mapping of upper value of error bars around scatter points
    xmin = NULL,
    #' @field xmax mapping of lower value of error bars around scatter points
    xmax = NULL,
    #' @field smoother regression function name
    smoother = NULL,
    #' @field lloq mapping lloq lines
    lloq = NULL,

    #' @description Create a new `ObsVsPredDataMapping` object
    #' @param x Name of x variable to map
    #' @param y Name of y variable to map
    #' @param xmin mapping of upper value of error bars around scatter points
    #' @param xmax mapping of lower value of error bars around scatter points
    #' @param lines list of lines to plot
    #' @param smoother smoother function or parameter
    #' @param lloq mapping lloq lines
    #' To map a loess smoother to the plot, use `smoother`="loess"
    #' @param ... parameters inherited from `XYGDataMapping`
    #' @return A new `ObsVsPredDataMapping` object
    initialize = function(x = NULL,
                          y = NULL,
                          xmin = NULL,
                          xmax = NULL,
                          lines = DefaultDataMappingValues$obsVsPred,
                          smoother = NULL,
                          lloq = NULL,
                          ...) {
      validateIsIncluded(smoother, c("lm", "loess"), nullAllowed = TRUE)
      validateIsString(xmin, nullAllowed = TRUE)
      validateIsString(xmax, nullAllowed = TRUE)
      validateIsString(lloq, nullAllowed = TRUE)
      super$initialize(x = x, y = y, ...)
      self$lines <- lines
      self$smoother <- smoother
      # If no xmin/xmax defined, map to x to get emtpy errorbars
      self$xmin <- xmin %||% self$x
      self$xmax <- xmax %||% self$x
      self$lloq <- lloq
    },

    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `defaultAes` variables.
    #' Dummy variable `defaultAes` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      validateIsOfType(data, "data.frame")
      .validateMapping(self$xmin, data, nullAllowed = TRUE)
      .validateMapping(self$xmax, data, nullAllowed = TRUE)
      .validateMapping(self$lloq, data, nullAllowed = TRUE)
      mapData <- super$checkMapData(data, metaData)
      mapData[, self$xmin] <- data[, self$xmin]
      mapData[, self$xmax] <- data[, self$xmax]
      if (!isEmpty(self$lloq)) {
        mapData[, self$lloq] <- data[, self$lloq]
      }
      self$data <- mapData
      return(mapData)
    }
  )
)

#' @title ResVsPredDataMapping
#' @description  Class for mapping variables in residuals vs predictions/time plot
#' @export
#' @family DataMapping classes
ResVsPredDataMapping <- R6::R6Class(
  "ResVsPredDataMapping",
  inherit = ObsVsPredDataMapping
)

#' @title ResVsTimeDataMapping
#' @description  Class for mapping variables in residuals vs predictions/time plot
#' @export
#' @family DataMapping classes
ResVsTimeDataMapping <- R6::R6Class(
  "ResVsTimeDataMapping",
  inherit = ResVsPredDataMapping
)
