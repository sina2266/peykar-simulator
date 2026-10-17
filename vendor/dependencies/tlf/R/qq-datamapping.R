#' @title QQDataMapping
#' @description  R6 class for mapping `x`, `y` and `GroupMapping` variables to `data`
#' @export
#' @family DataMapping classes
QQDataMapping <- R6::R6Class(
  "QQDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `defaultAes` variables.
    #' Dummy variable `defaultAes` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      validateIsOfType(data, "data.frame")
      mapData <- super$checkMapData(data, metaData)
      if (isEmpty(self$x)) {
        # Get theoritical quantiles for normal distribution
        xQuantiles <- stats::qqnorm(data[, self$y], plot.it = FALSE)$x
        mapData[, DefaultDataMappingValues$qqPlot] <- xQuantiles
      }
      self$data <- mapData
      return(mapData)
    }
  )
)
