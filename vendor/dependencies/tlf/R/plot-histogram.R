#' @title plotHistogram
#' @description
#' Producing Histograms
#'
#' @inheritParams addScatter
#' @param frequency logical defining if histogram displays a frequency in y axis
#' @param bins Number or edges of bins.
#' If `bins` is provided as a single numeric values, `bin` corresponds to number of bins.
#' The bin edges are then equally spaced within the range of data.
#' If `bins` is provided as an array of numeric values, `bin` corresponds to their edges.
#' Default value, `bins=NULL`, uses the value defined by `dataMapping`
#' @param binwidth Numerical value of defining the width of each bin.
#' If defined, `binwidth` can overwrite `bins` if `bins` was not provided or simply provided as a single value.
#' Default value, `binwidth=NULL`, uses the value defined by `dataMapping`
#' @param stack Logical defining for multiple histograms if their bars are stacked
#' Default value, `stack=NULL`, uses the value defined by `dataMapping`
#' @param distribution Name of distribution to fit to the data.
#' Only 2 distributions are currently available: `"normal"` and `"logNormal"`
#' Use `distribution="none"` to prevent fit of distribution
#' Default value, `distribution=NULL`, uses the value defined by `dataMapping`
#' @param dataMapping
#' A `HistogramDataMapping` object mapping `x` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `HistogramPlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/histogram.html>
#' @importFrom stats dnorm setNames
#' @export
#' @family molecule plots
#' @examples
#' # Produce histogram of normally distributed data
#' plotHistogram(x = rnorm(100))
#'
#' # Produce histogram of normally distributed data normalized in y axis
#' plotHistogram(x = rnorm(100), frequency = TRUE)
#'
#' # Produce histogram of normally distributed data with many bins
#' plotHistogram(x = rlnorm(100), bins = 21)
#'
#' # Produce histogram of fitted normally distributed data
#' plotHistogram(x = rlnorm(100), distribution = "normal")
#'
#'
#' # Produce histogram of fitted normally distributed data
#' plotHistogram(x = rlnorm(100), distribution = "normal", frequency = TRUE, stack = TRUE)
#'
plotHistogram <- function(data = NULL,
                          metaData = NULL,
                          x = NULL,
                          dataMapping = NULL,
                          frequency = NULL,
                          bins = NULL,
                          binwidth = NULL,
                          stack = NULL,
                          distribution = NULL,
                          plotConfiguration = NULL,
                          plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  if (is.null(data)) {
    validateIsNumeric(x)
    data <- data.frame(x = x)
    dataMapping <- dataMapping %||% HistogramDataMapping$new(
      x = ifNotNull(x, "x"),
      data = data
    )
  }
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, HistogramDataMapping, data)

  # Update dataMapping if inputs provided by user
  validateIsNumeric(bins, nullAllowed = TRUE)
  validateIsNumeric(binwidth, nullAllowed = TRUE)
  validateIsLogical(stack, nullAllowed = TRUE)
  validateIsIncluded(distribution, c("normal", "logNormal", "none"), nullAllowed = TRUE)
  validateIsLogical(frequency, nullAllowed = TRUE)

  dataMapping$frequency <- frequency %||% dataMapping$frequency
  dataMapping$stack <- stack %||% dataMapping$stack
  dataMapping$distribution <- distribution %||% dataMapping$distribution
  dataMapping$bins <- bins %||% dataMapping$bins
  dataMapping$binwidth <- binwidth %||% dataMapping$binwidth

  # Check for default labeling to update plotConfiguration after using .setPlotConfiguration
  ylabel <- NULL
  if (isEmpty(plotConfiguration)) {
    ylabel <- ifelse(dataMapping$frequency, "Relative frequency", "Count")
  }

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, HistogramPlotConfiguration,
    data, metaData, dataMapping
  )
  # Update default ylabel based on frequency
  plotConfiguration$labels$ylabel <- ylabel %||% plotConfiguration$labels$ylabel
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # position defines if bars are stacked or plotted side by side
  position <- ggplot2::position_nudge()
  if (dataMapping$stack) {
    position <- ggplot2::position_stack()
  }

  # If argument bins is of length > 1,
  # bins corresponds to bin edges instead of number of bins
  edges <- NULL
  if (length(dataMapping$bins) > 1) {
    edges <- dataMapping$bins
  }
  # Manage ggplot aes property depending on stack and frequency options
  # geom_histogram can use computed variables defined between two dots
  # see https://ggplot2.tidyverse.org/reference/geom_histogram.html for more info
  mapping <- ggplot2::aes(
    x = .data[[mapLabels$x]],
    y = ggplot2::after_stat(count),
    fill = .data[[mapLabels$fill]]
  )

  if (dataMapping$frequency) {
    # If histogram bars are not stacked, calculate frequency within each data groups
    # Since there is no direct computed variable
    # ncount variable is scaled by binwidth*dnorm(0) to get an area of ~1
    mapping <- ggplot2::aes(
      x = .data[[mapLabels$x]],
      y = ggplot2::after_stat(ncount * max(width) * dnorm(0)),
      fill = .data[[mapLabels$fill]]
    )
    if (dataMapping$stack) {
      # If histogram bars are stacked,
      # Calculate overall frequency as count per bin / total
      # This results in same histogram shapes no matter the data groups
      mapping <- ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = ggplot2::after_stat(count / sum(count)),
        fill = .data[[mapLabels$fill]]
      )
    }
  }

  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
    propertyNames = c("color", "size", "alpha", "linetype")
  )
  # 1- Histogram
  plotObject <- plotObject +
    ggplot2::geom_histogram(
      data = mapData,
      mapping = mapping,
      position = position,
      bins = dataMapping$bins,
      binwidth = dataMapping$binwidth,
      breaks = edges,
      size = aestheticValues$size,
      color = aestheticValues$color,
      linetype = aestheticValues$linetype,
      alpha = aestheticValues$alpha
    )

  # If distribution is provided by dataMapping, get median and distribution of the data
  fitData <- .getDistributionFit(mapData, dataMapping)
  fitMedian <- .getDistributionMed(mapData, dataMapping)

  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = c("size", "alpha")
  )

  # 2- Lines of distribution fit
  if (!isEmpty(fitData)) {
    plotObject <- plotObject +
      ggplot2::geom_line(
        data = fitData,
        mapping = ggplot2::aes(
          x = .data$x,
          y = .data$y,
          color = .data$legendLabels,
          linetype = .data$legendLabels
        ),
        linewidth = aestheticValues$size,
        alpha = aestheticValues$alpha
      )
  }

  # 3- Vertical lines of median
  for (lineIndex in seq_along(fitMedian)) {
    plotObject <- .addLineLayer(
      plotObject,
      type = "vertical",
      value = fitMedian[lineIndex],
      # position corresponds to the number of line layers already added
      position = lineIndex - 1
    )
  }

  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "ribbons",
    propertyNames = "fill",
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "lines",
    propertyNames = c("color", "linetype"),
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}


#' @title .getDistributionFit
#' @description Get a data.frame from the fit of a distribution provided in `dataMapping`
#' If `normal` distribution is selected, its mean is plotted
#' If `logNormal` distribution is selected, its mode is plotted
#' @param data data.frame containing the data to be used for the plot
#' @param dataMapping A `HistogramDataMapping` object
#' The object defines the distribution to be fitted and the option `stack`.
#' If the bars are stacked, the fit will account for the final histogram
#' @return A data.frame with `x`, `y` and `legendLabels`
#' @keywords internal
.getDistributionFit <- function(data, dataMapping) {
  if (isIncluded(dataMapping$distribution, "none")) {
    return()
  }
  x <- data[, dataMapping$x]
  # Get array of x values
  xFit <- seq(min(x, na.rm = TRUE), max(x, na.rm = TRUE), length.out = 1e3)
  # switch(
  # dataMapping$distribution,
  # "normal" = seq(-max(abs(x), na.rm = TRUE), max(abs(x), na.rm = TRUE), length.out = 1e3),
  # "logNormal" = seq(0, max(abs(x), na.rm = TRUE), length.out = 1e3)
  # )

  # Get binwidth
  bins <- dataMapping$bins
  if (length(bins) > 1) {
    bins <- length(bins) - 1
  }
  binwidth <- (max(x, na.rm = TRUE) - min(x, na.rm = TRUE)) / bins
  binwidth <- dataMapping$binwidth %||% binwidth

  if (dataMapping$stack) {
    yScaling <- binwidth * ifelse(dataMapping$frequency, 1, length(x))
    dataFit <- data.frame(
      x = xFit,
      y = yScaling * switch(dataMapping$distribution,
        "normal" = stats::dnorm(xFit, mean = mean(x, na.rm = TRUE), sd = stats::sd(x, na.rm = TRUE)),
        "logNormal" = stats::dlnorm(xFit, meanlog = mean(log(x), na.rm = TRUE), sdlog = stats::sd(log(x), na.rm = TRUE))
      ),
      legendLabels = paste0(dataMapping$distribution, " distribution fit")
    )
    return(dataFit)
  }

  dataFit <- NULL
  for (groupLevel in levels(data$legendLabels)) {
    selectedGroup <- data$legendLabels %in% groupLevel
    yScaling <- binwidth * ifelse(dataMapping$frequency, 1, length(x[selectedGroup]))
    dataFit <- rbind.data.frame(
      dataFit,
      data.frame(
        x = xFit,
        y = yScaling * switch(dataMapping$distribution,
          "normal" = stats::dnorm(
            xFit,
            mean = mean(x[selectedGroup], na.rm = TRUE),
            sd = stats::sd(x[selectedGroup], na.rm = TRUE)
          ),
          "logNormal" = stats::dlnorm(
            xFit,
            meanlog = mean(log(x[selectedGroup]), na.rm = TRUE),
            sdlog = stats::sd(log(x[selectedGroup]), na.rm = TRUE)
          )
        ),
        legendLabels = groupLevel
      )
    )
  }
  return(dataFit)
}

#' @title .getDistributionMed
#' @description Get an array of values from the fit of a distribution provided in `dataMapping`
#' If `normal` distribution is selected, its mean is plotted
#' If `logNormal` distribution is selected, its mode is plotted
#' @param data data.frame containing the data to be used for the plot
#' @param dataMapping A `HistogramDataMapping` object
#' The object defines the distribution to be fitted and the option `stack`.
#' If the bars are stacked, the fit will account for the final histogram
#' @return Numeric values for vertical lines
#' @keywords internal
.getDistributionMed <- function(data, dataMapping) {
  if (isIncluded(dataMapping$distribution, "none")) {
    return()
  }
  x <- data[, dataMapping$x]
  if (dataMapping$stack) {
    return(
      switch(dataMapping$distribution,
        "normal" = mean(x, na.rm = TRUE),
        "logNormal" = exp(mean(log(x), na.rm = TRUE) - stats::var(log(x), na.rm = TRUE))
      )
    )
  }

  xintercept <- NULL
  for (groupLevel in levels(data$legendLabels)) {
    selectedGroup <- data$legendLabels %in% groupLevel
    xintercept <- c(
      xintercept,
      switch(dataMapping$distribution,
        "normal" = mean(x[selectedGroup], na.rm = TRUE),
        "logNormal" = exp(mean(log(x[selectedGroup]), na.rm = TRUE) - stats::var(log(x[selectedGroup]), na.rm = TRUE))
      )
    )
  }
  return(xintercept)
}
