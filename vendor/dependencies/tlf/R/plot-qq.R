#' @title plotQQ
#' @description
#' Producing Histograms
#'
#' @inheritParams addScatter
#' @param dataMapping
#' A `QQDataMapping` object mapping `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `QQPlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce QQ plot of normally distributed data
#' plotQQ(y = rnorm(100))
#'
#' # Produce QQ plot of normally distributed data split by group
#' qqData <- data.frame(
#'   residuals = c(rnorm(100), rnorm(100)),
#'   groups = c(rep("Group A", 100), rep("Group B", 100))
#' )
#' plotQQ(
#'   data = qqData,
#'   dataMapping = QQDataMapping$new(y = "residuals", group = "groups")
#' )
#'
plotQQ <- function(data = NULL,
                   metaData = NULL,
                   y = NULL,
                   dataMapping = NULL,
                   plotConfiguration = NULL,
                   plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  if (is.null(data)) {
    validateIsNumeric(y)
    data <- data.frame(y = y)
    dataMapping <- dataMapping %||% QQDataMapping$new(
      y = "y",
      data = data
    )
  }
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, QQDataMapping, data)

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, QQPlotConfiguration,
    data, metaData, dataMapping
  )
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # 1- Line layer
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = c("color", "size", "alpha", "linetype")
  )

  plotObject <- plotObject +
    ggplot2::geom_qq_line(
      data = mapData,
      mapping = ggplot2::aes(
        sample = .data[[mapLabels$y]]
      ),
      size = aestheticValues$size,
      color = aestheticValues$color,
      linetype = aestheticValues$linetype,
      alpha = aestheticValues$alpha,
      na.rm = TRUE
    )

  # 2- Scatter point layer
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    plotConfigurationProperty = plotObject$plotConfiguration$points,
    propertyNames = c("size", "alpha")
  )

  plotObject <- plotObject +
    geomTLFPoint(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[DefaultDataMappingValues$qqPlot]],
        y = .data[[mapLabels$y]],
        color = .data[[mapLabels$color]],
        shape = .data[[mapLabels$shape]]
      ),
      size = aestheticValues$size,
      alpha = aestheticValues$alpha,
      na.rm = TRUE
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
