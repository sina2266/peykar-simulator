#' @title plotObservedTimeProfile
#' @description
#' Producing Time Profile plots for observed data
#'
#' @inheritParams addScatter
#' @param dataMapping
#' An `ObservedDataMapping` object mapping `x`, `y`, `ymin`, `ymax` and aesthetic groups to their variable names of `observedData`.
#' @param plotConfiguration
#' An optional `TimeProfilePlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce a Time profile plot with observed data
#' obsData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#' plotObservedTimeProfile(
#'   data = obsData,
#'   dataMapping = ObservedDataMapping$new(x = "x", y = "y")
#' )
plotObservedTimeProfile <- function(data,
                                    metaData = NULL,
                                    dataMapping = NULL,
                                    plotConfiguration = NULL,
                                    plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, ObservedDataMapping, data)
  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, TimeProfilePlotConfiguration,
    data, metaData, dataMapping
  )
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  requireDualAxis <- dataMapping$requireDualAxis(data)

  if (!requireDualAxis) {
    plotObject <- .plotObservedTimeProfileCore(
      data = data,
      metaData = metaData,
      dataMapping = dataMapping,
      plotConfiguration = plotConfiguration,
      plotObject = plotObject
    )
    return(plotObject)
  }

  leftPlotObject <- .plotObservedTimeProfileCore(
    data = dataMapping$getLeftAxis(data),
    metaData = metaData,
    dataMapping = dataMapping,
    plotConfiguration = plotConfiguration,
    plotObject = plotObject
  )
  rightPlotObject <- .plotObservedTimeProfileCore(
    data = dataMapping$getRightAxis(data),
    metaData = metaData,
    dataMapping = dataMapping,
    plotConfiguration = plotConfiguration,
    plotObject = plotObject
  )
  plotObject <- getDualAxisPlot(leftPlotObject, rightPlotObject)
  return(plotObject)
}


#' @title .plotObservedTimeProfileCore
#' @description Producing Core of Time Profile plots for observed data
#' @inheritParams plotObservedTimeProfile
#' @return A `ggplot` object
#' @keywords internal
.plotObservedTimeProfileCore <- function(data = NULL,
                                         metaData = NULL,
                                         dataMapping = NULL,
                                         plotConfiguration = NULL,
                                         plotObject = NULL) {
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)
  #----- Build layers of molecule plot -----
  # 1- LLOQ lines if available
  if (!isEmpty(dataMapping$lloq)) {
    plotObject <- .addLLOQLayer(
      plotObject,
      data = mapData,
      mapLabels = mapLabels,
      direction = plotConfiguration$lloqDirection
    )
  }

  # 2- Error bars if available
  if (!any(isEmpty(dataMapping$ymin), isEmpty(dataMapping$ymax))) {
    plotObject <- .addErrorbarLayer(
      plotObject,
      data = mapData,
      mapLabels = mapLabels
    )
  }
  # 3- Scatter points
  plotObject <- .addScatterLayer(
    plotObject,
    data = mapData,
    mapLabels = mapLabels
  )

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
