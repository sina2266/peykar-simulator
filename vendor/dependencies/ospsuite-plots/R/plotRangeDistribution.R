#' @title Plot Range Plot
#' @description Creates a range plot using specified data and mapping, allowing for different binning strategies and scaling options. This function provides a flexible way to visualize data distributions over specified ranges with optional statistical summaries.
#'
#' @param data A data frame containing the data to be plotted. This data should include the variables specified in the `mapping` argument.
#' @param mapping A mapping object (created using `ggplot2::aes()`) that defines how variables in `data` are mapped to aesthetics such as x and y axes, color, fill, etc.
#' @param metaData Optional metadata to be added to the plot. This can include additional information relevant to the data being plotted.
#' @param modeOfBinning A character string specifying the mode of binning. It determines how the data will be divided into bins. Options include:
#' - `Equal Frequency Binning`
#' - `Equal Width Binning
#' - `Custom Binning`
#' Default is `BINNINGMODE$number`.
#' @param numberOfBins An integer specifying the number of bins to use for equal frequency or width binning. Default is 20.
#' @param breaks Optional numeric vector specifying custom breaks for binning when `modeOfBinning` is set to `Custom Binning`. This allows for precise control over how data is grouped.
#' @param asStepPlot A logical indicating whether to create a step plot. If TRUE, the plot will display steps between the data points rather than continuous lines. Default is FALSE.
#' @param statFun An optional function for statistical summary, which takes a vector of y-values and returns a summary (e.g., quantiles). If NULL, defaults to calculating quantiles based on the specified `percentiles`.
#' @param percentiles A numeric vector of percentiles to be used in the statistical summary, which defines the range of values to be displayed on the plot. Default is the 5th, 50th, and 95th percentiles.
#' @param yScale A character string specifying the y-axis scale. Options are "linear" or "log". This determines how the y values are displayed on the plot. Default is "linear".
#' @param yScaleArgs A list of additional arguments for the y-axis scale, which can be used to customize the appearance and behavior of the y-axis.
#' @param xScale A character string specifying the x-axis scale. Options are "linear" or "log". This determines how the x values are displayed on the plot. Default is "linear".
#' @param xScaleArgs A list of additional arguments for the x-axis scale, which can be used to customize the appearance and behavior of the x-axis.
#' @param geomRibbonAttributes A list of attributes for the ribbon geometry in the plot, allowing customization of the visual appearance, such as colors and transparency.
#' @param geomLineAttributes A list of attributes for the line geometry in the plot, allowing customization of line characteristics such as color, size, and type.
#' @param identifier columnName of individual identifiers, default "IndividualId"
#' @return A ggplot object representing the range plot. The returned object can be further customized or rendered using `print()` or similar functions.
#' @export
#' @family plot functions
plotRangeDistribution <- function(
  data,
  mapping,
  metaData = NULL,
  modeOfBinning = BINNINGMODE$number,
  numberOfBins = 20,
  breaks = NA,
  asStepPlot = FALSE,
  statFun = NULL,
  percentiles = getOspsuite.plots.option(
    optionKey = OptionKeys$defaultPercentiles
  ),
  yScale = "linear",
  yScaleArgs = list(), # nolint
  xScale = "linear",
  xScaleArgs = list(), # nolint
  geomRibbonAttributes = getDefaultGeomAttributes("Ribbon"),
  geomLineAttributes = getDefaultGeomAttributes("Line"),
  identifier = "IndividualId"
) {
  ## Validation -----------
  checkmate::assertDataFrame(data, min.rows = 1)
  xScale <- match.arg(xScale, c(AxisScales$linear, AxisScales$log))
  if (!is.null(yScale)) yScale <- match.arg(yScale, c(AxisScales$linear, AxisScales$log))
  checkmate::assertList(xScaleArgs, null.ok = FALSE, min.len = 0)
  checkmate::assertList(yScaleArgs, null.ok = FALSE, min.len = 0)
  checkmate::assertList(geomRibbonAttributes, names = "named")
  checkmate::assertList(geomLineAttributes, names = "named")

  checkmate::assertFunction(statFun, null.ok = !is.null(percentiles))
  checkmate::assertDouble(
    percentiles,
    lower = 0,
    upper = 1,
    any.missing = FALSE,
    sorted = TRUE,
    len = 3,
    null.ok = !is.null(statFun)
  )

  ## map Data ----------
  mappedData <- MappedDataRangeDistribution$new(
    data = data,
    mapping = mapping,
    xScale = xScale,
    yScale = yScale,
    groupAesthetics = c("fill", "color"),
    modeOfBinning = modeOfBinning,
    numberOfBins = numberOfBins,
    breaks = breaks
  )
  mappedData$addMetaData(metaData)
  mappedData$setBins()
  mappedData$setBorderDataTable(identifier = identifier)
  mappedData$setXMapping(asStepPlot)

  if (is.null(statFun)) {
    statFun <- function(y) {
      r <- stats::quantile(y, probs = percentiles, names = FALSE, na.rm = TRUE)
      names(r) <- c("ymin", "y", "ymax")
      return(r)
    }
  }

  #-  create default plot ------
  plotObject <- initializePlot(mappedData)

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
          fun.data = statFun,
          geom = "ribbon",
          na.rm = TRUE
        ),
        geomRibbonAttributes
      )
    )

  plotObject <- plotObject +
    do.call(
      what = ggplot2::stat_summary,
      args = c(
        list(
          fun.data = statFun,
          geom = "line",
          na.rm = TRUE
        ),
        geomLineAttributes
      )
    )

  # return also border data.table
  plotObject$border <- mappedData$border

  return(plotObject)
}
