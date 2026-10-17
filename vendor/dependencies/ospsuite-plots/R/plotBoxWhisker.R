#' @title Generate Box-Whisker Plots
#' @description Produces box-and-whisker plots for visualizing the distribution of data.
#' For more details and examples, see the vignettes:
#' * \code{vignette("Box-Whisker Plots", package = "ospsuite.plots")}
#' * \code{vignette("ospsuite.plots", package = "ospsuite.plots")}
#'
#' @param data A `data.frame` containing the data to aggregate.
#' @param mapping A list of aesthetic mappings to use for the plot.
#' @inheritParams plotTimeProfile
#' @inheritParams plotYVsX
#' @param percentiles A numeric vector with percentiles used for the box whiskers and boxes,
#' e.g., c(0.05, 0.25, 0.5, 0.75, 0.95). Default defined by `ospsuite.plots` option.
#' @param statFun (default NULL) A function to calculate whiskers and box ranges,
#' which overwrites the `percentiles` variable if provided.
#' @param outliers Logical indicating whether outliers should be included in the boxplot.
#' Outliers are flagged when outside the range from the "25th" percentile - 1.5 x IQR to
#' the "75th" percentile + 1.5 x IQR, as suggested by McGill et al.
#' @param statFunOutlier (default NULL) A function to calculate outliers,
#' which overwrites the default calculation if provided.
#' @param geomBoxplotAttributes A `list` of arguments passed to the `geom_boxplot` call.
#' @param geomPointAttributes A `list` of arguments passed to the `ggplot2::geom_point` call.
#' @param xScale Either 'linear', 'log', 'discrete', or 'auto' (default).
#' Auto selects linear for continuous data and discrete for categorical data.
#' @param xScaleArgs A list of arguments passed to `ggplot2::scale_x_continuous()`,
#' `ggplot2::scale_x_log10()`, or `ggplot2::scale_x_discrete()`.
#' @param residualScale Deprecated. Retained for backward compatibility only.
#'   Non-`NULL` values trigger a warning and have no effect.
#'
#' @return A `ggplot` object representing the box-whisker plot.
#' @references McGill, R., Tukey, J. W., & Larsen, W. A. (1978). Variations of box plots.
#' The American Statistician, 32(1), 12-16.
#' @examples
#' \dontrun{
#' # Basic box-whisker plot
#' plotBoxWhisker(
#'   data = myData,
#'   mapping = aes(x = group, y = value)
#' )
#'
#' # Box-whisker plot with custom percentiles
#' plotBoxWhisker(
#'   data = myData,
#'   mapping = aes(x = treatment, y = response),
#'   percentiles = c(0.1, 0.25, 0.5, 0.75, 0.9)
#' )
#'
#' # Box-whisker plot with custom stat function
#' customStatFun <- function(x) {
#'   return(quantile(x, probs = c(0.05, 0.25, 0.5, 0.75, 0.95), na.rm = TRUE))
#' }
#' plotBoxWhisker(
#'   data = myData,
#'   mapping = aes(x = dose_group, y = concentration),
#'   statFun = customStatFun,
#'   outliers = TRUE
#' )
#' }
#' @export
#' @family plot functions
plotBoxWhisker <- function(
  data,
  mapping,
  metaData = NULL,
  plotObject = NULL,
  percentiles = getOspsuite.plots.option(optionKey = OptionKeys$percentiles),
  yScale = AxisScales$linear,
  yScaleArgs = list(),
  xScale = "auto",
  xScaleArgs = list(),
  statFun = NULL,
  outliers = FALSE,
  statFunOutlier = NULL,
  geomBoxplotAttributes = getDefaultGeomAttributes("Boxplot"),
  geomPointAttributes = getDefaultGeomAttributes("Boxplot"),
  residualScale = NULL
) {
  if (!is.null(residualScale)) {
    warning(messages$warningResidualScaleDeprecated())
  }
  ## Validation -----------
  checkmate::assertClass(plotObject, classes = "ggplot", null.ok = TRUE)
  checkmate::assertList(metaData, types = "list", null.ok = TRUE)
  checkmate::assertList(geomBoxplotAttributes, names = "named")
  checkmate::assertList(geomPointAttributes, names = "named")
  checkmate::assertDouble(
    percentiles,
    lower = 0,
    upper = 1,
    any.missing = FALSE,
    sorted = TRUE,
    null.ok = !is.null(statFun)
  )

  xScale <- match.arg(xScale, c("auto", AxisScales$discrete, AxisScales$linear, AxisScales$log))
  if (!is.null(yScale)) yScale <- match.arg(yScale, c(AxisScales$linear, AxisScales$log))

  checkmate::assertList(xScaleArgs, null.ok = FALSE, min.len = 0)
  checkmate::assertList(yScaleArgs, null.ok = FALSE, min.len = 0)

  checkmate::assertFunction(statFun, null.ok = !is.null(percentiles))
  checkmate::assertFlag(outliers)
  checkmate::assertFunction(statFunOutlier, null.ok = TRUE)

  ## map Data ----------

  mappedData <- MappedDataBoxplot$new(
    data = data,
    mapping = mapping,
    xScale = xScale,
    yScale = yScale,
    groupAesthetics = "fill",
    isObserved = TRUE
  )
  mappedData$addMetaData(metaData)
  mappedData$doAdjustmentsWithMetaData(
    originalmapping = mapping,
    xScale = xScale,
    xScaleArgs = xScaleArgs
  )

  #-  create default plot ------
  if (is.null(plotObject)) {
    plotObject <- initializePlot(mappedData)
    if (!mappedData$hasXmapping) {
      plotObject$labels[["x"]] <-
        plotObject$labels[["x"]] %||% " "
    }
  }

  # set aggregation function ---------

  if (is.null(statFun)) {
    statFun <- function(y) {
      r <- stats::quantile(y, probs = percentiles, names = FALSE, na.rm = TRUE)
      names(r) <- c("ymin", "lower", "middle", "upper", "ymax")
      return(r)
    }

    # add names of box whisker limits to ggplot for use in function getBoxWhiskerLimits
    statFunExport <- function(y) {
      y <- y[!is.na(y)]
      rQuantiles <- stats::quantile(
        y,
        probs = percentiles,
        names = FALSE,
        na.rm = TRUE
      )
      names(rQuantiles) <- paste(
        scales::label_ordinal()(x = percentiles * 100),
        "percentile"
      )

      r <- c(
        N = length(y),
        rQuantiles,
        "arith mean" = mean(y),
        "arith standard deviation" = stats::sd(y),
        "arith CV" = stats::sd(y) / mean(y),
        "geo mean" = exp(mean(log(y))),
        "geo standard deviation" = exp(stats::sd(log(y))),
        "geo CV" = sqrt(exp((log(stats::sd(y)))^2) - 1)
      )

      return(r)
    }

    plotObject$statFun <- statFunExport
  } else {
    plotObject$statFun <- statFun
  }

  # add x and y scale
  plotObject <- addXYScale(
    plotObject = plotObject,
    xScale = mappedData$xScale,
    xScaleArgs = xScaleArgs,
    yScale = yScale,
    yScaleArgs = yScaleArgs
  )

  plotObject <- plotObject +
    do.call(
      what = ggplot2::stat_summary,
      args = c(
        list(
          mapping = mappedData$boxwhiskerMapping,
          fun.data = statFun,
          geom = "boxplot",
          na.rm = TRUE
        ),
        geomBoxplotAttributes
      )
    )

  # outlier
  if (outliers) {
    # set function for outlier
    if (is.null(statFunOutlier)) {
      statFunOutlier <-
        function(x) {
          q <- stats::quantile(
            x,
            probs = c(0.25, 0.75),
            names = FALSE,
            na.rm = TRUE
          )
          iqr <- diff(range(q))
          pp <- subset(
            x,
            x < (q[1] - (1.5 * iqr)) |
              x > (q[2] + (1.5 * iqr))
          )
          if (length(pp) < 1) {
            return(as.double(NA))
          } else {
            return(pp)
          }
        }
    }
    plotObject$statFunOutlier <- statFunOutlier

    plotObject <- plotObject +
      do.call(
        what = ggplot2::stat_summary,
        args = c(
          list(
            fun = statFunOutlier,
            geom = GeomPointOsp,
            na.rm = TRUE
          ),
          geomPointAttributes
        )
      )
  }

  return(plotObject)
}
