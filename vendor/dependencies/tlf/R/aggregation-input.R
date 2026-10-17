#' @title AggregationInput
#' @description  R6 class to be used to construct inputs to the AggregationSummary class
#' @import R6
#' @export
AggregationInput <- R6::R6Class(
  "AggregationInput",
  public = list(
    #' @field aggregationFunction list of functions to use for aggregation
    aggregationFunction = NULL,
    #' @field aggregationFunctionName vector of function names
    #' that will be used as variable name of the aggregation
    aggregationFunctionName = NULL,
    #' @field aggregationUnit unit of aggregation output
    aggregationUnit = NULL,
    #' @field aggregationDimension dimension of aggregation output
    aggregationDimension = NULL,

    #' @description Create a new `AggregationInput` object
    #' @param aggregationFunction list of functions to use for aggregation
    #' @param aggregationFunctionName vector of function names
    #' that will be used as variable name of the aggregation
    #' @param aggregationUnit unit of aggregation output
    #' @param aggregationDimension dimension of aggregation output
    #' @return A new `AggregationInput` object
    initialize = function(aggregationFunction = NULL,
                          aggregationFunctionName = NULL,
                          aggregationUnit = NULL,
                          aggregationDimension = NULL) {
      self$aggregationFunction <- aggregationFunction
      self$aggregationFunctionName <- aggregationFunctionName
      self$aggregationUnit <- aggregationUnit
      self$aggregationDimension <- aggregationDimension
    }
  )
)

#' @title mean-1.96sd
#' @description Calculate `mean-1.96SD`
#' @param x Numeric values
#' @return Numeric value corresponding to `mean(x)-1.96*sd(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate mean-1.96SD
#' `mean-1.96sd`(rnorm(1000))
#'
`mean-1.96sd` <- function(x) {
  return(mean(x) - 1.96 * stats::sd(x))
}

#' @title mean+1.96sd
#' @description Calculate `mean+1.96SD`
#' @param x Numeric values
#' @return Numeric value corresponding to `mean(x)+1.96*sd(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate mean+1.96SD
#' `mean+1.96sd`(rnorm(1000))
#'
`mean+1.96sd` <- function(x) {
  return(mean(x) + 1.96 * stats::sd(x))
}

#' @title mean-sd
#' @description Calculate `mean-SD`
#' @param x Numeric values
#' @return Numeric value corresponding to `mean(x)-sd(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate mean-SD
#' `mean-sd`(rnorm(1000))
#'
`mean-sd` <- function(x) {
  return(mean(x) - stats::sd(x))
}

#' @title mean+sd
#' @description Calculate `mean+SD`
#' @param x Numeric values
#' @return Numeric value corresponding to `mean(x)+sd(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate mean-SD
#' `mean+sd`(rnorm(1000))
#'
`mean+sd` <- function(x) {
  return(mean(x) + stats::sd(x))
}

#' @title median-IQR
#' @description Calculate `median-IQR`
#' @param x Numeric values
#' @return Numeric value corresponding to `median(x)-iqr(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate median-IQR
#' `median-IQR`(rnorm(1000))
#'
`median-IQR` <- function(x) {
  quartiles <- stats::quantile(x, probs = c(0.5, 0.75, 0.25))
  res <- quartiles[1] - (quartiles[2] - quartiles[3])
  return(as.numeric(res))
}

#' @title median+IQR
#' @description Calculate `median+IQR`
#' @param x Numeric values
#' @return Numeric value corresponding to `median(x)+iqr(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate median+IQR
#' `median+IQR`(rnorm(1000))
#'
`median+IQR` <- function(x) {
  quartiles <- stats::quantile(x, probs = c(0.5, 0.75, 0.25))
  res <- quartiles[1] + (quartiles[2] - quartiles[3])
  return(as.numeric(res))
}

#' @title median-1.5IQR
#' @description Calculate `median-1.5IQR`
#' @param x Numeric values
#' @return Numeric value corresponding to `median(x)-1.5*iqr(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate median-1.5IQR
#' `median-1.5IQR`(rnorm(1000))
#'
`median-1.5IQR` <- function(x) {
  quartiles <- stats::quantile(x, probs = c(0.5, 0.75, 0.25))
  res <- quartiles[1] - 1.5 * (quartiles[2] - quartiles[3])
  return(as.numeric(res))
}

#' @title median+1.5IQR
#' @description Calculate `median+1.5IQR`
#' @param x Numeric values
#' @return Numeric value corresponding to `median(x)+1.5*iqr(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate median+1.5IQR
#' `median+1.5IQR`(rnorm(1000))
#'
`median+1.5IQR` <- function(x) {
  quartiles <- stats::quantile(x, probs = c(0.5, 0.75, 0.25))
  res <- quartiles[1] + 1.5 * (quartiles[2] - quartiles[3])
  return(as.numeric(res))
}

#' @title Percentile25%-1.5IQR
#' @description Calculate `Percentile25%-1.5IQR`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 0.25)-1.5*iqr(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile25%-1.5IQR
#' `Percentile25%-1.5IQR`(rnorm(1000))
#'
`Percentile25%-1.5IQR` <- function(x) {
  quartiles <- stats::quantile(x, probs = c(0.75, 0.25))
  res <- quartiles[2] - 1.5 * (quartiles[1] - quartiles[2])
  return(as.numeric(res))
}

#' @title Percentile75%+1.5IQR
#' @description Calculate `Percentile75%+1.5IQR`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 0.75)+1.5*iqr(x)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile75%+1.5IQR
#' `Percentile75%+1.5IQR`(rnorm(1000))
#'
`Percentile75%+1.5IQR` <- function(x) {
  quartiles <- stats::quantile(x, probs = c(0.75, 0.25))
  res <- quartiles[1] + 1.5 * (quartiles[1] - quartiles[2])
  return(as.numeric(res))
}

# Assign did not work because the functions need to be documented with @export
# in order to be used, however I did not find the way to do it using assign
# I left the code as a comment in case it may be re-used
# for (percentileValue in predefinedPercentiles) {
#  # Use eval and parse to use evaluated values within functions
#  percentileFunction <- eval(parse(text = paste0("function (x) {as.numeric(stats::quantile(x, probs = ", percentileValue / 100, "))}")))
#  assign(paste0("Percentile-", percentileValue, "%"), percentileFunction)
# }

#' @title Percentile0%
#' @description Calculate `Percentile0%` i.e. `min` value
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 0)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile0%
#' `Percentile0%`(rnorm(1000))
#'
`Percentile0%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 0))
}

#' @title Percentile1%
#' @description Calculate `Percentile1%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 1/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile1%
#' `Percentile1%`(rnorm(1000))
#'
`Percentile1%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 1 / 100))
}

#' @title Percentile2.5%
#' @description Calculate `Percentile2.5%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 2.5/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile2.5%
#' `Percentile2.5%`(rnorm(1000))
#'
`Percentile2.5%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 2.5 / 100))
}

#' @title Percentile5%
#' @description Calculate `Percentile5%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 5/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile5%
#' `Percentile5%`(rnorm(1000))
#'
`Percentile5%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 5 / 100))
}

#' @title Percentile10%
#' @description Calculate `Percentile10%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 10/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile10%
#' `Percentile10%`(rnorm(1000))
#'
`Percentile10%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 10 / 100))
}

#' @title Percentile15%
#' @description Calculate `Percentile15%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 15/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile15%
#' `Percentile15%`(rnorm(1000))
#'
`Percentile15%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 15 / 100))
}

#' @title Percentile20%
#' @description Calculate `Percentile20%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 20/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile20%
#' `Percentile20%`(rnorm(1000))
#'
`Percentile20%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 20 / 100))
}

#' @title Percentile25%
#' @description Calculate `Percentile25%` i.e. 1st quartile value
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 25/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile25%
#' `Percentile25%`(rnorm(1000))
#'
`Percentile25%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 25 / 100))
}

#' @title Percentile50%
#' @description Calculate `Percentile50%` i.e. `median` value
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 50/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile50%
#' `Percentile50%`(rnorm(1000))
#'
`Percentile50%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 50 / 100))
}

#' @title Percentile75%
#' @description Calculate `Percentile75%` i.e. 3rd quartile value
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 75/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile75%
#' `Percentile75%`(rnorm(1000))
#'
`Percentile75%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 75 / 100))
}

#' @title Percentile80%
#' @description Calculate `Percentile80%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 80/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile80%
#' `Percentile80%`(rnorm(1000))
#'
`Percentile80%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 80 / 100))
}

#' @title Percentile85%
#' @description Calculate `Percentile85%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 85/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile85%
#' `Percentile85%`(rnorm(1000))
#'
`Percentile85%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 85 / 100))
}

#' @title Percentile90%
#' @description Calculate `Percentile90%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 90/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile90%
#' `Percentile90%`(rnorm(1000))
#'
`Percentile90%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 90 / 100))
}

#' @title Percentile95%
#' @description Calculate `Percentile95%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 95/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile95%
#' `Percentile95%`(rnorm(1000))
#'
`Percentile95%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 95 / 100))
}

#' @title Percentile97.5%
#' @description Calculate `Percentile97.5%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 97.5/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile97.5%
#' `Percentile97.5%`(rnorm(1000))
#'
`Percentile97.5%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 97.5 / 100))
}

#' @title Percentile99%
#' @description Calculate `Percentile99%`
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 99/100)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile99%
#' `Percentile99%`(rnorm(1000))
#'
`Percentile99%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 99 / 100))
}

#' @title Percentile100%
#' @description Calculate `Percentile100%` i.e. `max` value
#' @param x Numeric values
#' @return Numeric value corresponding to `quantile(x, 1)`
#' @export
#' @family stat functions
#' @examples
#' # Calculate Percentile100%
#' `Percentile100%`(rnorm(1000))
#'
`Percentile100%` <- function(x) {
  as.numeric(stats::quantile(x, probs = 1))
}
