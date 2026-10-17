#' @title plotCumulativeTimeProfile
#' @description
#' Producing Cumulative Time Profile plots
#'
#' @inheritParams addScatter
#' @param colorPalette Optional character values defining a `ggplot2` colorPalette (e.g. `"Set1"` or `"Spectral"`)
#' @param dataMapping
#' A `CumulativeTimeProfileDataMapping` object mapping `x` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `CumulativeTimeProfilePlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#'
#' # Define data to be plotted as cumulative time profile
#' time <- seq(1, 10)
#' data <- data.frame(
#'   x = rep(time),
#'   y = c(exp(-time / 10), 1 - exp(-time / 10)),
#'   legend = rep(c("decreasing area", "increasing area"), each = 10)
#' )
#'
#' # Produce a Cumulative Time Profile plot
#' plotCumulativeTimeProfile(
#'   data = data,
#'   dataMapping = CumulativeTimeProfileDataMapping$new(x = "x", y = "y", fill = "legend")
#' )
#'
#' # Produce a Cumulative Time Profile plot with a ggplot2 color palette
#' plotCumulativeTimeProfile(
#'   data = data,
#'   dataMapping = CumulativeTimeProfileDataMapping$new(
#'     x = "x",
#'     y = "y",
#'     fill = "legend"
#'   ),
#'   colorPalette = "Set1"
#' )
#'
plotCumulativeTimeProfile <- function(data = NULL,
                                      metaData = NULL,
                                      dataMapping = NULL,
                                      colorPalette = NULL,
                                      plotConfiguration = NULL,
                                      plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, CumulativeTimeProfileDataMapping, data)

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, CumulativeTimeProfilePlotConfiguration,
    data, metaData, dataMapping
  )

  # Update plotConfiguration if inputs provided by user
  validateIsString(colorPalette, nullAllowed = TRUE)
  plotConfiguration$colorPalette <- colorPalette %||% plotConfiguration$colorPalette

  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = 0,
    plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
    propertyNames = c("linetype", "size", "alpha")
  )

  plotObject <- plotObject + ggplot2::geom_area(
    data = mapData,
    mapping = ggplot2::aes(
      x = .data[[mapLabels$x]],
      y = .data[[mapLabels$y]],
      fill = .data[[mapLabels$fill]],
      color = .data[[mapLabels$color]]
    ),
    alpha = aestheticValues$alpha,
    linewidth = aestheticValues$size,
    linetype = aestheticValues$linetype,
    # Cumulative componenent
    position = ggplot2::position_stack(),
    na.rm = TRUE
  )

  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "ribbons",
    propertyNames = c("fill", "color"),
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .applyColorPalette(
    plotObject,
    colorPalette = plotConfiguration$colorPalette
  )
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}
