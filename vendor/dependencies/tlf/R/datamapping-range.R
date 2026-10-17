#' @title RangeDataMapping
#' @description  R6 class for mapping `x`, `ymin` and `ymax` variable to `data`
#' @export
#' @family DataMapping classes
RangeDataMapping <- R6::R6Class(
  "RangeDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @field ymin Name of ymin variable to map
    ymin = NULL,
    #' @field ymax Name of ymax variable to map
    ymax = NULL,

    #' @description Create a new `RangeDataMapping` object
    #' @param x Name of x variable to map
    #' @param ymin Name of ymin variable to map
    #' @param ymax Name of ymax variable to map
    #' @param groupMapping R6 class `GroupMapping` object
    #' @param color R6 class `Grouping` object or its input
    #' @param fill R6 class `Grouping` object or its input
    #' @param linetype R6 class `Grouping` object or its input
    #' @param shape R6 class `Grouping` object or its input
    #' @param size R6 class `Grouping` object or its input
    #' @param group R6 class `Grouping` object or its input
    #' @param data data.frame to map used by `.smartMapping`
    #' @return A new `RangeDataMapping` object
    initialize = function(x = NULL,
                          ymin = NULL,
                          ymax = NULL,
                          groupMapping = NULL,
                          color = NULL,
                          fill = NULL,
                          linetype = NULL,
                          shape = NULL,
                          size = NULL,
                          group = NULL,
                          data = NULL) {
      # .smartMapping is available in utilities-mapping.R
      smartMap <- .smartMapping(data)
      super$initialize(x %||% smartMap$x,
        y = NULL,
        groupMapping = groupMapping, color = color, fill = fill,
        linetype = linetype, shape = shape, size = size, group = group
      )

      self$ymin <- ymin %||% smartMap$ymin
      self$ymax <- ymax %||% smartMap$ymax
    },

    #' @description Check that `data` variables include map variables
    #' @param data data.frame to check
    #' @param metaData list containing information on `data`
    #' @return A data.frame with map and `legendLabels` variables.
    #' Dummy variable `legendLabels` is necessary to allow further modification of plots.
    checkMapData = function(data, metaData = NULL) {
      .validateMapping(self$x, data, nullAllowed = TRUE)
      if (isOfType(self$ymin, "character")) {
        .validateMapping(self$ymin, data)
      }
      if (isOfType(self$ymax, "character")) {
        .validateMapping(self$ymax, data)
      }

      # Drop option simplify data.frame into vectors
      # False enforces data to stay as data.frame if x or y is empty
      self$data <- data[, c(self$x, self$ymin, self$ymax), drop = FALSE]

      # All possible Groupings are listed in the enum LegendTypes
      for (groupType in LegendTypes) {
        if (isEmpty(self$groupMapping[[groupType]]$group)) {
          next
        }
        grouping <- self$groupMapping[[groupType]]

        groupVariables <- grouping$group
        if (isOfType(groupVariables, "data.frame")) {
          # Last group variable is the label in group data.frames
          # and need to be removed from the check
          groupVariables <- names(groupVariables)
          groupVariables <- utils::head(groupVariables, -1)
        }
        .validateMapping(groupVariables, data)
        # Enforce grouping variables to be factors
        self$data[, grouping$label] <- as.factor(grouping$getCaptions(data, metaData))

        # Dummy variable for default aesthetics that will be used to define legend labels
        legendLabels <- self$data$legendLabels %||% grouping$getCaptions(data, metaData)

        # Prevent duplication of legend if groupings are the same
        if (isTRUE(all.equal(legendLabels, grouping$getCaptions(data, metaData)))) {
          self$data$legendLabels <- legendLabels
          next
        }
        self$data$legendLabels <- as.factor(paste(as.character(self$data$legendLabels),
          as.character(grouping$getCaptions(data, metaData)),
          sep = "-"
        ))
      }

      if (is.null(self$data$legendLabels)) {
        self$data$legendLabels <- factor("")
      }
      return(self$data)
    }
  )
)
