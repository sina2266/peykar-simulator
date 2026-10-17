#' @title generates residual quantile quantile plot
#' @description
#'
#' For details and examples see the vignettes:
#' * \code{vignette("Goodness of fit", package = "ospsuite.plots")}
#' * \code{vignette("ospsuite.plots", package = "ospsuite.plots")}
#'
#' @inheritParams plotTimeProfile
#' @param data  ´data.frame` with data to plot
#' @param geomQQAttributes A list of arguments passed to `ggplot2::stat_qq()`.
#' @param geomQQLineAttributes A list of arguments passed to `ggplot2::stat_qq_line()`.
#' @param groupAesthetics A character vector of aesthetic names used for grouping data points in the Q-Q plot.
#'   Common options include `"colour"`, `"fill"`, `"shape"`, `"linetype"`, and `"size"`.
#' @param residualScale Deprecated. Retained for backward compatibility only.
#'   Non-`NULL` values trigger a warning and have no effect.
#'
#'
#' @return A `ggplot` object
#' @export
#' @family plot functions
plotQQ <- function(
  data,
  mapping,
  metaData = NULL,
  xScaleArgs = list(),
  yScaleArgs = list(),
  geomQQAttributes = list(),
  geomQQLineAttributes = geomQQAttributes,
  groupAesthetics = c("colour", "fill", "shape"),
  residualScale = NULL
) {
  if (!is.null(residualScale)) {
    warning(messages$warningResidualScaleDeprecated())
  }
  # Check validity
  checkmate::assertDataFrame(data)

  checkmate::assertList(xScaleArgs, null.ok = FALSE, min.len = 0)
  checkmate::assertList(yScaleArgs, null.ok = FALSE, min.len = 0)

  checkmate::assertList(geomQQAttributes, null.ok = FALSE, min.len = 0)
  checkmate::assertList(geomQQLineAttributes, null.ok = FALSE, min.len = 0)
  checkmate::assertCharacter(groupAesthetics, null.ok = TRUE, min.len = 1)

  xScale <- AxisScales$linear
  yScale <- AxisScales$linear

  # data match --------------
  mappedData <- MappedData$new(
    data = data,
    mapping = mapping,
    xScale = xScale,
    yScale = yScale,
    xlimits = xScaleArgs$limits,
    ylimits = yScaleArgs$limits,
    isObserved = TRUE,
    groupAesthetics = groupAesthetics
  )
  mappedData$addMetaData(metaData = metaData)

  #-  initialize plot
  plotObject <- initializePlot(mappedData = mappedData) +
    theme(aspect.ratio = 1)
  plotObject <- plotObject +
    labs(x = "Standard normal quantiles")

  #----- Build layers -----

  plotObject <- addLayer(
    mappedData = mappedData,
    geom = "qq",
    geomAttributes = geomQQAttributes,
    plotObject = plotObject,
    layerToCall = stat_qq_osp # nolint: object_usage_linter
  )

  plotObject <- addLayer(
    mappedData = mappedData,
    geom = "qq",
    geomAttributes = geomQQLineAttributes,
    plotObject = plotObject,
    layerToCall = stat_qq_line
  )

  # set scales ----

  plotObject <- addXYScale(
    plotObject = plotObject,
    xScale = xScale,
    xScaleArgs = xScaleArgs,
    yScale = yScale,
    yScaleArgs = yScaleArgs
  )

  return(plotObject)
}
