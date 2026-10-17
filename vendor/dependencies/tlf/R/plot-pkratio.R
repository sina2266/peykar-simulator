#' @title plotPKRatio
#' @description
#' Producing PK Ratio plots
#'
#' @inheritParams addScatter
#' @param foldDistance Numeric values of fold distance lines to display in log plots.
#' This argument is internally translated into `lines` field of `dataMapping`.
#' __Caution__: this argument is meant for log scaled plots and since fold distance is a ratio it is expected positive.
#' In particular, line of identity corresponds to a `foldDistance` of `1`.
#' @param dataMapping
#' A `PKRatioDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `PKRatioPlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/pk-ratio-vignette.html>
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce PK Ratio plot
#' pkData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#'
#' plotPKRatio(data = pkData, dataMapping = PKRatioDataMapping$new(x = "x", y = "y"))
#'
#' # Produce PK Ratio plot with user-defined horizontal lines
#' plotPKRatio(
#'   data = pkData,
#'   dataMapping = PKRatioDataMapping$new(x = "x", y = "y"),
#'   foldDistance = c(1, 10)
#' )
#'
plotPKRatio <- function(data,
                        metaData = NULL,
                        dataMapping = NULL,
                        plotConfiguration = NULL,
                        foldDistance = NULL,
                        plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, PKRatioDataMapping, data)
  validateIsNumeric(foldDistance, nullAllowed = TRUE)
  if (!isEmpty(foldDistance)) {
    dataMapping$lines <- getLinesFromFoldDistance(foldDistance)
  }
  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, PKRatioPlotConfiguration,
    data, metaData, dataMapping
  )
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # Each new layer is added on top of previous
  # Thus, scatter points are added as last layer to prevent them being hidden by lines or errorbars
  # 1- Horizontal lines
  for (lineIndex in seq_along(dataMapping$lines)) {
    plotObject <- .addLineLayer(
      plotObject,
      type = "horizontal",
      value = dataMapping$lines[[lineIndex]],
      # position corresponds to the number of line layers already added
      position = lineIndex - 1
    )
  }
  # 2- Error bars
  plotObject <- .addErrorbarLayer(plotObject, data = mapData, mapLabels = mapLabels)
  # 3- Scatter points
  plotObject <- .addScatterLayer(plotObject, data = mapData, mapLabels = mapLabels)

  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "points",
    propertyNames = c("color", "shape"),
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}
