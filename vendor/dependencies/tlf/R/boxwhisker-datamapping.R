#' @title BoxWhiskerDataMapping
#' @description  R6 class for mapping `y`, `GroupMapping`, `boxWhiskerLimits` and `outlierLimits` to `data`
#' @export
#' @family DataMapping classes
BoxWhiskerDataMapping <- R6::R6Class(
  "BoxWhiskerDataMapping",
  inherit = XYGDataMapping,
  public = list(
    #' @field outlierLimits List of `minOutlierLimit` and `maxOutlierLimit` functions
    #' outside which `data` is flagged as outlier
    outlierLimits = NULL,
    #' @field boxWhiskerLimits List of `ymin`, `lower`, `middle`, `upper` and `ymax` functions
    #' calculated on `data` to obtain box whiskers
    boxWhiskerLimits = NULL,

    #' @description Create a new `BoxWhiskerDataMapping` object
    #' @param x Name of x variable to map
    #' Default value is NULL in case of a unique box in the boxplot.
    #' @param y Name of y variable to map
    #' @param ymin Name of function used for calculating lower whisker.
    #' Default value is `Percentile5%`.
    #' @param lower Name of function used for calculating lower line of box
    #' Default value is `Percentile25%`.
    #' @param middle Name of function used for calculating middle line
    #' Default value is `Percentile55%`.
    #' @param upper Name of function used for calculating upper line of box
    #' Default value is `Percentile75%`.
    #' @param ymax Name of function used for calculating upper whisker
    #' Default value is `Percentile95%`.
    #' @param minOutlierLimit Name of function used for calculating lower outlier limit
    #' Default value is `Percentile25-1.5IQR%`.
    #' @param maxOutlierLimit Name of function used for calculating upper outlier limit
    #' Default value is `Percentile75+1.5IQR%`.
    #' @param ... parameters inherited from `XYGDataMapping`
    #' @return A new `BoxWhiskerDataMapping` object
    initialize = function(x = NULL,
                          y,
                          ymin = tlfStatFunctions$`Percentile5%`,
                          lower = tlfStatFunctions$`Percentile25%`,
                          middle = tlfStatFunctions$`Percentile50%`,
                          upper = tlfStatFunctions$`Percentile75%`,
                          ymax = tlfStatFunctions$`Percentile95%`,
                          minOutlierLimit = tlfStatFunctions$`Percentile25%-1.5IQR`,
                          maxOutlierLimit = tlfStatFunctions$`Percentile75%+1.5IQR`,
                          ...) {
      super$initialize(x = x, y = y, ...)
      self$groupMapping$color <- self$groupMapping$color %||% self$groupMapping$fill

      self$boxWhiskerLimits <- c(ymin, lower, middle, upper, ymax)
      self$outlierLimits <- c(minOutlierLimit, maxOutlierLimit)
    },

    #' @description Get a data.frame with box-whisker limit by group
    #' @param data data.frame to check
    #' @return A data.frame with `ymin`, `lower`, `middle`, `upper`, `ymax` variables.
    getBoxWhiskerLimits = function(data) {
      # Dummy silent variable if x is NULL
      if (isOfLength(self$x, 0)) {
        data$legendLabels <- factor("")
      }

      # Transform names into functions for aggregation summary
      boxWhiskerLimitsFunctions <- sapply(self$boxWhiskerLimits, match.fun)

      # Use aggregation summary to get box specific values
      summaryObject <- AggregationSummary$new(
        data = data,
        xColumnNames = self$x %||% "legendLabels",
        groupingColumnNames = self$groupMapping$fill$label,
        yColumnNames = self$y,
        aggregationFunctionsVector = boxWhiskerLimitsFunctions,
        aggregationFunctionNames = c("ymin", "lower", "middle", "upper", "ymax")
      )

      boxWhiskerLimits <- summaryObject$dfHelper

      # Dummy variable for aesthetics
      boxWhiskerLimits$legendLabels <- factor("")
      if (!isOfLength(self$x, 0)) {
        boxWhiskerLimits[, self$x] <- as.factor(boxWhiskerLimits[, self$x])
      }

      return(boxWhiskerLimits)
    },

    #' @description Get a data.frame flagging outliers
    #' @param data data.frame to check
    #' @return A data.frame with `minOutliers` and `maxOutliers` variables.
    #' Values not flagged are `NA` in the outliers variables
    getOutliers = function(data) {
      data <- self$checkMapData(data)
      # Dummy silent variable if x is NULL
      if (isOfLength(self$x, 0)) {
        data$legendLabels <- factor("")
      }

      # Transform names into functions for aggregation summary
      outlierLimitsFunctions <- sapply(self$outlierLimits, match.fun)

      # Use aggregation summary to get outliers boundaries specific values
      summaryObject <- AggregationSummary$new(
        data = data,
        xColumnNames = self$x %||% "legendLabels",
        groupingColumnNames = self$groupMapping$fill$label,
        yColumnNames = self$y,
        aggregationFunctionsVector = outlierLimitsFunctions,
        aggregationFunctionNames = c("minOutlierLimit", "maxOutlierLimit")
      )

      outlierLimits <- summaryObject$dfHelper

      # Merge outlier limits to data
      outliers <- merge.data.frame(data, outlierLimits)

      # Create the outliers variables by flagging which are lower or higher than limits
      outliers[, "minOutliers"] <- outliers[, self$y]
      outliers[, "maxOutliers"] <- outliers[, self$y]

      minOutliersFlag <- outliers[, self$y] < outliers[, "minOutlierLimit"]
      maxOutliersFlag <- outliers[, self$y] > outliers[, "maxOutlierLimit"]

      outliers$minOutliers[!minOutliersFlag] <- NA
      outliers$maxOutliers[!maxOutliersFlag] <- NA

      # Dummy variable for aesthetics
      outliers$legendLabels <- factor("")
      if (!isOfLength(self$x, 0)) {
        outliers[, self$x] <- as.factor(outliers[, self$x])
      }

      return(outliers)
    }
  )
)
