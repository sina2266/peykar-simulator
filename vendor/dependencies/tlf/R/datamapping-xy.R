#' @title XYDataMapping
#' @description  R6 class for mapping `x` and `y` variable to `data`
#' @export
#' @family DataMapping classes
XYDataMapping <- R6::R6Class(
  "XYDataMapping",
  public = list(
    #' @field x Name of x variable to map
    x = NULL,
    #' @field y Name of y variable to map
    y = NULL,
    #' @field data data.frame used for mapping
    data = NULL,

    #' @description Create a new `XYDataMapping` object
    #' @param x Name of x variable to map
    #' @param y Name of y variable to map
    #' @return A new `XYDataMapping` object
    initialize = function(x,
                          y = NULL) {
      if (isOfType(x, "numeric")) {
        warning(paste("'x' is a numeric, dataMapping will select the column of indice", x, "in data"))
      }
      if (isOfType(y, "numeric")) {
        warning(paste("'y' is a numeric, dataMapping will select the column of indice", y, "in data"))
      }

      self$x <- x
      self$y <- y
    },

    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `defaultAes` variables.
    #' Dummy variable `defaultAes` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      if (isOfType(self$x, "character")) {
        .validateMapping(self$x, data)
      }
      if (isOfType(self$y, "character")) {
        .validateMapping(self$y, data)
      }

      # Drop option simplify data.frame into vectors
      # False enforces data to stay as data.frame if x or y is empty
      self$data <- data[, c(self$x, self$y), drop = FALSE]

      # Dummy variable for default aesthetics
      self$data$legendLabels <- factor("")
      return(self$data)
    }
  )
)
