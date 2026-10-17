#' @title ObservedDataMapping
#' @description  R6 class for mapping `x`, `y`, of observed data for a time profile plot
#' @export
#' @family DataMapping classes
ObservedDataMapping <- R6::R6Class(
  "ObservedDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @field error mapping error bars around scatter points
    error = NULL,
    #' @field mdv mapping missing dependent variable
    mdv = NULL,
    #' @field ymin mapping error bars around scatter points
    ymin = NULL,
    #' @field ymax mapping error bars around scatter points
    ymax = NULL,
    #' @field y2Axis Name of y2Axis variable to map
    y2Axis = NULL,
    #' @field lloq mapping lloq lines
    lloq = NULL,

    #' @description Create a new `ObservedDataMapping` object
    #' @param x Name of x variable to map
    #' @param y Name of y variable to map
    #' @param ymin mapping lower end of error bars around scatter points
    #' @param ymax mapping upper end of error bars around scatter points
    #' @param y2Axis Name of y2Axis variable to map
    #' @param color R6 class `Grouping` object or its input
    #' @param shape R6 class `Grouping` object or its input
    #' @param group R6 class `Grouping` object or its input
    #' @param error mapping error bars around scatter points
    #' @param uncertainty `r lifecycle::badge("deprecated")` uncertainty were
    #' replaced by `error` argument. Mapping error bars around scatter points.
    #' @param mdv mapping missing dependent variable
    #' @param data data.frame to map used by `.smartMapping`
    #' @param lloq mapping lloq lines
    #' @return A new `ObservedDataMapping` object
    initialize = function(x,
                          y,
                          ymin = NULL,
                          ymax = NULL,
                          y2Axis = NULL,
                          group = NULL,
                          color = NULL,
                          shape = NULL,
                          error = NULL,
                          uncertainty = lifecycle::deprecated(),
                          mdv = NULL,
                          data = NULL,
                          lloq = NULL) {
      if (lifecycle::is_present(uncertainty)) {
        lifecycle::deprecate_warn(
          when = "1.5.0",
          what = "ObservedDataMapping(uncertainty)",
          with = "ObservedDataMapping(error)"
        )
        error <- uncertainty
      }

      validateIsString(error, nullAllowed = TRUE)
      validateIsString(ymin, nullAllowed = TRUE)
      validateIsString(ymax, nullAllowed = TRUE)
      validateIsString(mdv, nullAllowed = TRUE)
      validateIsString(lloq, nullAllowed = TRUE)
      # .smartMapping is available in utilities-mapping.R
      smartMap <- .smartMapping(data)
      super$initialize(
        x = x %||% smartMap$x,
        y = y %||% smartMap$y,
        color = color,
        shape = shape,
        group = group,
        data = data
      )

      # If defined, ymin and ymax are used as is
      # If not, error/uncertainty are used and
      # creates ymin and ymax as y +/- error
      self$error <- error
      self$ymin <- ymin %||% ifNotNull(self$error, "ymin")
      self$ymax <- ymax %||% ifNotNull(self$error, "ymax")
      self$mdv <- mdv
      self$y2Axis <- y2Axis
      self$lloq <- lloq
    },

    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `defaultAes` variables.
    #' Dummy variable `defaultAes` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      validateIsOfType(data, "data.frame")
      .validateMapping(self$error, data, nullAllowed = TRUE)
      .validateMapping(self$mdv, data, nullAllowed = TRUE)
      .validateMapping(self$y2Axis, data, nullAllowed = TRUE)
      .validateMapping(self$lloq, data, nullAllowed = TRUE)

      # Using super method, fetches x, y and groups
      mapData <- super$checkMapData(data, metaData)

      # ymin and ymax for error bars
      # This section may change depending on how we want to include options
      if (!isEmpty(self$error)) {
        mapData[, self$error] <- data[, self$error]
        # Symetric error bars
        mapData[, self$ymax] <- data[, self$y] + data[, self$error]
        mapData[, self$ymin] <- data[, self$y] - data[, self$error]
      }
      # Overwrite ymin and ymax if user defined
      if (isIncluded(self$ymax, names(data))) {
        mapData[, self$ymax] <- data[, self$ymax]
      }
      if (isIncluded(self$ymin, names(data))) {
        mapData[, self$ymin] <- data[, self$ymin]
      }
      if (!isEmpty(self$y2Axis)) {
        mapData[, self$y2Axis] <- as.logical(data[, self$y2Axis])
      }
      # MDV is a Nonmem notation in which values with MDV==1 are removed
      if (!isEmpty(self$mdv)) {
        mapData[, self$mdv] <- as.logical(data[, self$mdv])
        mapData <- mapData[!mapData[, self$mdv], ]
      }
      # LLOQ alows to add lines on the plot and apply an alpha scale on the points
      if (!isEmpty(self$lloq)) {
        mapData[, self$lloq] <- data[, self$lloq]
      }
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
