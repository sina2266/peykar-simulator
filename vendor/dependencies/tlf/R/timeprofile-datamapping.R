#' @title TimeProfileDataMapping
#' @description  R6 class defining the configuration of a `ggplot` object for time profile plot
#' @export
#' @family DataMapping classes
TimeProfileDataMapping <- R6::R6Class(
  "TimeProfileDataMapping",
  inherit = RangeDataMapping,
  public = list(
    #' @field y2Axis Name of y2Axis variable to map
    y2Axis = NULL,

    #' @description Create a new `TimeProfileDataMapping` object
    #' @param x Name of x variable to map
    #' @param y Name of y variable to map
    #' @param ymin Name of ymin variable to map
    #' @param ymax Name of ymax variable to map
    #' @param y2Axis Name of y2Axis variable to map
    #' @param group R6 class `Grouping` object or its input
    #' @param color R6 class `Grouping` object or its input
    #' @param fill R6 class `Grouping` object or its input
    #' @param linetype R6 class `Grouping` object or its input
    #' @param data data.frame to map used by `.smartMapping`
    #' @return A new `RangeDataMapping` object
    initialize = function(x = NULL,
                          y = NULL,
                          ymin = NULL,
                          ymax = NULL,
                          group = NULL,
                          y2Axis = NULL,
                          color = NULL,
                          fill = NULL,
                          linetype = NULL,
                          data = NULL) {
      # .smartMapping is available in utilities-mapping.R
      smartMap <- .smartMapping(data)
      super$initialize(
        x = x %||% smartMap$x,
        ymin = ymin %||% smartMap$ymin,
        ymax = ymax %||% smartMap$ymax,
        color = color, fill = fill, linetype = linetype, group = group, data = data
      )
      # Since TimeProfileDataMapping inherits from RangeDataMapping
      # super$initialize introduce a self$y which is NULL
      self$y <- y %||% smartMap$y
      self$y2Axis <- y2Axis
    },
    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `legendLabels` variables.
    #' Dummy variable `legendLabels` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      validateIsOfType(data, "data.frame")
      .validateMapping(self$y, data, nullAllowed = TRUE)
      .validateMapping(self$y2Axis, data, nullAllowed = TRUE)
      mapData <- super$checkMapData(data, metaData)
      # This may change depending of how we want to include options
      if (!isEmpty(self$y)) {
        mapData[, self$y] <- data[, self$y]
      }
      if (!isEmpty(self$y2Axis)) {
        mapData[, self$y2Axis] <- as.logical(data[, self$y2Axis])
      }
      self$data <- mapData
      return(mapData)
    },

    #' @description Assess if `data` require a dual axis plot
    #' @param data data.frame to check
    #' @return A logical
    requireDualAxis = function(data) {
      .validateMapping(self$y2Axis, data, nullAllowed = TRUE)
      if (isEmpty(self$y2Axis)) {
        return(FALSE)
      }
      return(any(as.logical(data[, self$y2Axis]), na.rm = TRUE))
    },

    #' @description Render NA values for all right axis data
    #' @param data A data.frame
    #' @return A data.frame to be plotted in left axis
    getLeftAxis = function(data) {
      if (!self$requireDualAxis(data)) {
        return(data)
      }
      # Ensure NAs in that data don't mess up the selection
      selectedRows <- as.logical(data[, self$y2Axis]) %in% TRUE
      if (isIncluded(self$ymax, names(data))) {
        data[selectedRows, self$ymax] <- NA
      }
      if (isIncluded(self$ymin, names(data))) {
        data[selectedRows, self$ymin] <- NA
      }
      if (isIncluded(self$y, names(data))) {
        data[selectedRows, self$y] <- NA
      }
      return(data)
    },

    #' @description Render NA values for all left axis data
    #' @param data A data.frame
    #' @return A data.frame to be plotted in right axis
    getRightAxis = function(data) {
      # Ensure NAs in that data don't mess up the selection
      selectedRows <- as.logical(data[, self$y2Axis]) %in% FALSE
      if (isIncluded(self$ymax, names(data))) {
        data[selectedRows, self$ymax] <- NA
      }
      if (isIncluded(self$ymin, names(data))) {
        data[selectedRows, self$ymin] <- NA
      }
      if (isIncluded(self$y, names(data))) {
        data[selectedRows, self$y] <- NA
      }
      return(data)
    }
  )
)
