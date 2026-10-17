#' @title getBoxWhiskerMeasure
#' @description
#' Get a summary table of Box Whisker percentiles
#'
#' @inheritParams plotBoxWhisker
#' @param y Name of `y` variable in `data`.
#' @param group Name of grouping variable in `data`.
#' @param quantiles Numeric values between 0 and 1 defining the quantiles to summarize
#' @return A data.frame of summary statistics
#' @export
#' @examples
#' # Get box-and-whisker plots of log-normal distributed data
#' boxData <- data.frame(x = c(rep("A", 500), rep("B", 500)), y = rlnorm(1000))
#'
#' getBoxWhiskerMeasure(data = boxData, dataMapping = BoxWhiskerDataMapping$new(x = "x", y = "y"))
#'
getBoxWhiskerMeasure <- function(data,
                                 dataMapping = NULL,
                                 y = NULL,
                                 group = NULL,
                                 quantiles = c(0.05, 0.25, 0.5, 0.75, 0.95)) {
  # If no data mapping is input, use default
  variableNames <- names(data)
  y <- y %||% utils::tail(variableNames, 1)
  dataMapping <- dataMapping %||% BoxWhiskerDataMapping$new(
    x = group,
    y = y
  )

  validateIsOfType(dataMapping, "BoxWhiskerDataMapping")

  # Redfine group and y while removing NA values
  y <- data[!is.na.data.frame(data[, dataMapping$y]), dataMapping$y]
  group <- ifNotNull(
    dataMapping$x,
    data[!is.na.data.frame(data[, dataMapping$y]), dataMapping$x],
    as.factor(rep("", length(y)))
  )

  summary <- tapply(y, group, FUN = function(x) {
    .summaryStat(x, quantiles)
  })
  # As a data.frame summary row names are already group names
  summary <- as.data.frame(t(sapply(summary, FUN = rbind)))

  # Re-label variables
  names(summary) <- c(
    "N",
    sapply(quantiles, function(x) {
      paste0(100 * x, "th percentile")
    }),
    "mean",
    "standard deviation",
    "geo mean",
    "geo standard deviation"
  )


  return(summary)
}


.summaryStat <- function(x, quantiles = c(0.05, 0.25, 0.5, 0.75, 0.95), na.rm = FALSE) {
  n <- length(x)
  summaryQuantiles <- as.numeric(stats::quantile(x, probs = quantiles))
  .summaryStat <- c(mean(x, na.rm = na.rm), stats::sd(x, na.rm = na.rm), exp(mean(log(x), na.rm = na.rm)), exp(stats::sd(log(x), na.rm = na.rm)))

  summary <- c(n, summaryQuantiles, .summaryStat)

  return(summary)
}
