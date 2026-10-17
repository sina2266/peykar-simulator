#' @title plotTornado
#' @description
#' Producing tornado plots
#'
#' @inheritParams addScatter
#' @param y Character values to plot along the `y` axis. Only used instead of `data` if `data` is `NULL`.
#' @param sorted Optional logical value defining if `y` values are sorted by absolute values of `x`.
#' @param colorPalette Optional character values defining a `ggplot2` colorPalette (e.g. `"Spectral"`)
#' @param bar Optional logical value setting tornado plot as bar plot instead of scatter plot.
#' @param dataMapping
#' A `TornadoDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `TornadoPlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce a tornado plot
#' plotTornado(x = c(2, -1, 3), y = c("A", "B", "C"))
#'
#' # Produce a tornado plot as scatter plot
#' plotTornado(x = c(2, -1, 3), y = c("A", "B", "C"), bar = FALSE)
#'
#' # Produce a tornado plot as is (no sorting)
#' plotTornado(x = c(2, -1, 3), y = c("A", "B", "C"), sorted = FALSE)
#'
plotTornado <- function(data = NULL,
                        metaData = NULL,
                        x = NULL,
                        y = NULL,
                        sorted = NULL,
                        colorPalette = NULL,
                        bar = TRUE,
                        dataMapping = NULL,
                        plotConfiguration = NULL,
                        plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  if (is.null(data)) {
    validateIsNumeric(x)
    y <- y %||% rep("", length(x))
    validateIsSameLength(x, y)
    data <- data.frame(x = x, y = y)

    dataMapping <- dataMapping %||% TornadoDataMapping$new(
      sorted = sorted,
      x = ifNotNull(x, "x"),
      y = ifNotNull(y, "y"),
      data = data
    )
  }
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, TornadoDataMapping, data)

  # Update dataMapping if inputs provided by user
  validateIsLogical(sorted, nullAllowed = TRUE)
  dataMapping$sorted <- sorted %||% dataMapping$sorted

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, TornadoPlotConfiguration,
    data, metaData, dataMapping
  )
  # Update plotConfiguration if inputs provided by user
  validateIsString(colorPalette, nullAllowed = TRUE)
  validateIsLogical(bar)
  plotConfiguration$colorPalette <- colorPalette %||% plotConfiguration$colorPalette
  plotConfiguration$bar <- bar

  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # Option sorting the values, which put the wider spread at the top and smaller at the bottom
  # An additional options can be added to change the type of sort for later versions
  # (e.g. increasing/decreasing absolute values or actual values...)
  if (dataMapping$sorted) {
    mapData[, dataMapping$y] <- stats::reorder(mapData[, dataMapping$y], abs(mapData[, dataMapping$x]))
  }

  # If tornado is a bar plot, plot configuration option "bar" is TRUE.
  # Otherwise, the plot will use points instead
  if (plotConfiguration$bar) {
    aestheticValues <- .getAestheticValuesFromConfiguration(
      n = 1,
      position = 0,
      plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
      propertyNames = c("linetype", "size", "alpha")
    )

    plotObject <- plotObject + ggplot2::geom_col(
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
      position = ggplot2::position_dodge(width = plotConfiguration$dodge),
      na.rm = TRUE
    )

    plotObject <- .updateAesProperties(
      plotObject,
      plotConfigurationProperty = "ribbons",
      propertyNames = c("fill", "color"),
      data = mapData,
      mapLabels = mapLabels
    )
  }

  # If tornado is a scatter plot, plot configuration option "bar" is FALSE.
  if (!plotConfiguration$bar) {
    aestheticValues <- .getAestheticValuesFromConfiguration(
      n = 1,
      position = 0,
      plotConfigurationProperty = plotObject$plotConfiguration$points,
      propertyNames = c("size", "alpha")
    )
    # For tornado with points, their shape will be taken from the theme properties
    plotObject <- plotObject + geomTLFPoint(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = .data[[mapLabels$y]],
        color = .data[[mapLabels$color]],
        shape = .data[[mapLabels$shape]]
      ),
      size = aestheticValues$size,
      alpha = aestheticValues$alpha,
      position = ggplot2::position_dodge(width = plotConfiguration$dodge)
    )

    plotObject <- .updateAesProperties(
      plotObject,
      plotConfigurationProperty = "points",
      propertyNames = c("color", "shape"),
      data = mapData,
      mapLabels = mapLabels
    )
  }

  # Add vertical lines
  for (lineIndex in seq_along(dataMapping$lines)) {
    plotObject <- .addLineLayer(
      plotObject,
      type = "vertical",
      value = dataMapping$lines[[lineIndex]],
      # position corresponds to the number of line layers already added
      position = lineIndex - 1
    )
  }

  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .applyColorPalette(
    plotObject,
    colorPalette = plotConfiguration$colorPalette
  )
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}
