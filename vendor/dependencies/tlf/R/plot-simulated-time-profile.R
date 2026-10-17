#' @title plotSimulatedTimeProfile
#' @description
#' Producing Time Profile plots
#'
#' @inheritParams addScatter
#' @param dataMapping
#' A `TimeProfileDataMapping` object mapping `x`, `y`, `ymin`, `ymax` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `TimeProfilePlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce a Time profile plot with simulated data
#' simTime <- seq(1, 10, 0.1)
#' simData <- data.frame(
#'   x = simTime,
#'   y = 10 * exp(-simTime),
#'   ymin = 8 * exp(-simTime),
#'   ymax = 12 * exp(-simTime)
#' )
#'
#' plotSimulatedTimeProfile(
#'   data = simData,
#'   dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax")
#' )
plotSimulatedTimeProfile <- function(data = NULL,
                                     metaData = NULL,
                                     dataMapping = NULL,
                                     plotConfiguration = NULL,
                                     plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, TimeProfileDataMapping, data)
  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, TimeProfilePlotConfiguration,
    data, metaData, dataMapping
  )
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  requireDualAxis <- dataMapping$requireDualAxis(data)

  if (!requireDualAxis) {
    plotObject <- .plotSimulatedTimeProfileCore(
      data = data,
      metaData = metaData,
      dataMapping = dataMapping,
      plotConfiguration = plotConfiguration,
      plotObject = plotObject
    )
    return(plotObject)
  }

  leftPlotObject <- .plotSimulatedTimeProfileCore(
    data = dataMapping$getLeftAxis(data),
    metaData = metaData,
    dataMapping = dataMapping,
    plotConfiguration = plotConfiguration,
    plotObject = plotObject
  )
  rightPlotObject <- .plotSimulatedTimeProfileCore(
    data = dataMapping$getRightAxis(data),
    metaData = metaData,
    dataMapping = dataMapping,
    plotConfiguration = plotConfiguration,
    plotObject = plotObject
  )
  plotObject <- getDualAxisPlot(leftPlotObject, rightPlotObject)
  return(plotObject)
}

#' @title .plotSimulatedTimeProfileCore
#' @description Producing Core of Time Profile plots for simulated data
#' @inheritParams plotSimulatedTimeProfile
#' @return A `ggplot` object
#' @keywords internal
.plotSimulatedTimeProfileCore <- function(data = NULL,
                                          metaData = NULL,
                                          dataMapping = NULL,
                                          plotConfiguration = NULL,
                                          plotObject = NULL) {
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # 1- Ribbons if available
  if (!any(isEmpty(dataMapping$ymin), isEmpty(dataMapping$ymax))) {
    aestheticValues <- .getAestheticValuesFromConfiguration(
      n = 1,
      position = 0,
      plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
      propertyNames = c("alpha")
    )
    plotObject <- plotObject +
      ggplot2::geom_ribbon(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          ymin = .data[[mapLabels$ymin]],
          ymax = .data[[mapLabels$ymax]],
          fill = .data[[mapLabels$fill]],
          group = .data[[mapLabels$linetype]]
        ),
        alpha = aestheticValues$alpha,
        na.rm = TRUE,
        show.legend = TRUE
      )
  }

  # 2- Lines
  if (!isEmpty(dataMapping$y)) {
    aestheticValues <- .getAestheticValuesFromConfiguration(
      n = 1,
      position = 0,
      plotConfigurationProperty = plotObject$plotConfiguration$lines,
      propertyNames = c("alpha", "size")
    )
    plotObject <- plotObject +
      ggplot2::geom_path(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          y = .data[[mapLabels$y]],
          color = .data[[mapLabels$color]],
          linetype = .data[[mapLabels$linetype]]
        ),
        linewidth = aestheticValues$size,
        alpha = aestheticValues$alpha,
        na.rm = TRUE,
        show.legend = TRUE,
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
