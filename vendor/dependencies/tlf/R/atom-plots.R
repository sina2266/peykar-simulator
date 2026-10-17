# Atom plots

#' @title initializePlot
#' @description
#' Initialize a `ggplot` object and set its labels, grid, background and watermark
#'
#' @param plotConfiguration
#' An optional `PlotConfiguration` object defining labels, grid, background and watermark
#'
#' @return A `ggplot` graphical object
#'
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/atom-plots.html>
#'
#' @family atom plots
#' @export
#' @import ospsuite.utils
#' @examples
#' # Initialize an empty plot
#' p <- initializePlot()
#'
#' # Implement a customized configuration using PlotConfiguration
#' config <- PlotConfiguration$new(title = "My Plot", xlabel = "x variable", ylabel = "y variable")
#' p <- initializePlot(config)
#'
initializePlot <- function(plotConfiguration = NULL) {
  validateIsOfType(plotConfiguration, "PlotConfiguration", nullAllowed = TRUE)
  plotConfiguration <- plotConfiguration %||% PlotConfiguration$new()

  plotObject <- ggplot2::ggplot()
  plotObject$plotConfiguration <- plotConfiguration

  plotObject <- setWatermark(plotObject)
  plotObject <- setBackgroundPlotArea(plotObject)
  plotObject <- setBackgroundPanelArea(plotObject)
  plotObject <- setXGrid(plotObject)
  plotObject <- setYGrid(plotObject)
  plotObject <- setPlotLabels(plotObject)
  plotObject <- setLegend(plotObject)

  return(plotObject)
}

#' @title addScatter
#' @description
#' Add a scatter plot layer to a `ggplot` object
#'
#' @param data A data.frame to use for plot.
#' @param metaData A named list of information about `data` such as the `dimension` and `unit` of its variables.
#' @param x Numeric values to plot along the `x` axis. Only used instead of `data` if `data` is `NULL`.
#' @param y Numeric values to plot along the `y` axis. Only used instead of `data` if `data` is `NULL`.
#' @param dataMapping A `XYGDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param caption Optional character values defining the legend captions of the plot.
#' @param color Optional character values defining the colors of the plot layer.
#' See `grDevices::colors()` to get names of colors
#' @param shape Optional character values defining the shapes/symbols of the plot layer.
#' See enum `Shapes` to get names of shapes.
#' @param linetype Optional character values defining the linetype of the plot layer.
#' See enum `Linetypes` to get names of linetype.
#' @param size Optional numeric values defining the size of the plot layer.
#' @param plotConfiguration
#' An optional `PlotConfiguration` object defining labels, grid, background and watermark.
#' @param plotObject
#' An optional `ggplot` object on which to add the plot layer
#' @return A `ggplot` object
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/atom-plots.html>
#'
#' @family atom plots
#' @export
#' @examples
#' # Add scatter using x and y
#' addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' # Add scatter using a data.frame
#' time <- seq(0, 30, 0.1)
#' scatterData <- data.frame(x = time, y = cos(time))
#'
#' addScatter(
#'   data = scatterData,
#'   dataMapping = XYGDataMapping$new(x = "x", y = "y")
#' )
#'
#' # Or for simple cases a smart mapping will get directly x and y from data
#' addScatter(data = scatterData)
#'
#' # Add a scatter with caption
#' addScatter(data = scatterData, caption = "My scatter plot")
#'
#' # Add a scatter with specific properties
#' addScatter(
#'   data = scatterData,
#'   color = "blue", shape = "diamond", size = 2, caption = "My data"
#' )
#'
#' # Add a scatter with specific properties
#' p <- addScatter(
#'   data = scatterData,
#'   color = "blue", shape = "diamond", size = 2, caption = "My data"
#' )
#' addScatter(
#'   x = c(0, 1), y = c(1, 0),
#'   color = "red", shape = "circle", size = 3,
#'   plotObject = p
#' )
#'
addScatter <- function(data = NULL,
                       metaData = NULL,
                       x = NULL,
                       y = NULL,
                       caption = NULL,
                       color = NULL,
                       shape = NULL,
                       size = NULL,
                       linetype = NULL,
                       dataMapping = NULL,
                       plotConfiguration = NULL,
                       plotObject = NULL) {
  validateIsOfType(dataMapping, XYGDataMapping, nullAllowed = TRUE)
  validateIsOfType(plotConfiguration, PlotConfiguration, nullAllowed = TRUE)

  # If data is not input, creates data from x and y inputs
  if (isEmpty(data)) {
    validateIsSameLength(x, y)
    data <- as.data.frame(cbind(x = x, y = y))
    dataMapping <- dataMapping %||% XYGDataMapping$new(x = ifNotNull(x, "x"), y = ifNotNull(y, "y"), data = data)
  }
  # Enforce data to be a data.frame for dataMapping
  if (!isOfType(data, "data.frame")) {
    data <- data.frame(data, check.names = FALSE)
  }

  dataMapping <- dataMapping %||% XYGDataMapping$new(x = "x", y = "y", data = data)
  plotConfiguration <- plotConfiguration %||% PlotConfiguration$new(data = data, metaData = metaData, dataMapping = dataMapping)
  # Update plotConfiguration if user defined aesthetics
  eval(.parseVariableToObject("plotConfiguration$points", c("color", "shape", "linetype", "size"), keepIfNull = TRUE))

  # If no plot, initialize empty plot
  plotObject <- plotObject %||% initializePlot(plotConfiguration)

  # If no mapping, nor x or y, return plotObject
  if (any(isEmpty(dataMapping$x), isEmpty(dataMapping$y))) {
    warning("No mapping found for x nor y, scatter layer was not added")
    return(plotObject)
  }

  # Get mapping and convert labels into characters usable by aes
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)
  # If no specific mapping, use default captions
  if (min(levels(factor(mapData$legendLabels)) == "") == 1) {
    mapData$legendLabels <- .getlegendLabelsCaption(plotObject)
  }
  mapData$legendLabels <- caption %||% mapData$legendLabels

  plotObject <- plotObject +
    geomTLFPoint(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = .data[[mapLabels$y]],
        shape = .data$legendLabels,
        color = .data$legendLabels,
        size = .data$legendLabels
      ),
      alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$points$alpha, aesthetic = "alpha"),
      show.legend = TRUE,
      na.rm = TRUE
    ) +
    ggplot2::geom_path(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = .data[[mapLabels$y]],
        linetype = .data$legendLabels,
        color = .data$legendLabels
      ),
      linewidth = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$size, aesthetic = "size"),
      alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$points$alpha, aesthetic = "alpha"),
      show.legend = TRUE,
      na.rm = TRUE
    ) +
    ggplot2::geom_ribbon(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        ymin = .data[[mapLabels$y]],
        ymax = .data[[mapLabels$y]],
        fill = .data$legendLabels
      ),
      # alpha is 0 so that line can be seen in legend
      alpha = 0,
      show.legend = TRUE,
      na.rm = TRUE
    )

  # Prepare data for merging previous and current legend
  newLabels <- levels(factor(mapData$legendLabels))
  # Sample LegendType properties based Theme if not input
  try(plotObject <- .mergeLegend(plotObject,
    newLabels = newLabels,
    aestheticSelections = plotConfiguration$points
  ))
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}

#' @title addLine
#' @inheritParams addScatter
#' @description
#' Add a line layer to a `ggplot` object.
#'
#' @return A `ggplot` object
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/atom-plots.html>
#'
#' @family atom plots
#' @export
#' @examples
#' # Add line using x and y
#' addLine(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' # Add line using a data.frame
#' time <- seq(0, 30, 0.1)
#' lineData <- data.frame(x = time, y = cos(time))
#'
#' addLine(
#'   data = lineData,
#'   dataMapping = XYGDataMapping$new(x = "x", y = "y")
#' )
#'
#' # Or for simple cases a smart mapping will get directly x and y from data
#' addLine(data = lineData)
#'
#' # Add a line with caption
#' addLine(data = lineData, caption = "My line plot")
#'
#' # Add a line with specific properties
#' addLine(
#'   data = lineData,
#'   color = "blue", linetype = "longdash", size = 0.5, caption = "My data"
#' )
#'
#' # Add a line with specific properties
#' p <- addLine(
#'   data = lineData,
#'   color = "blue", linetype = "longdash", size = 0.5, caption = "My data"
#' )
#' addLine(
#'   x = c(0, 1), y = c(1, 0),
#'   color = "red", linetype = "solid", size = 1,
#'   plotObject = p
#' )
#'
addLine <- function(data = NULL,
                    metaData = NULL,
                    x = NULL,
                    y = NULL,
                    caption = NULL,
                    color = NULL,
                    shape = NULL,
                    size = NULL,
                    linetype = NULL,
                    dataMapping = NULL,
                    plotConfiguration = NULL,
                    plotObject = NULL) {
  validateIsOfType(dataMapping, XYGDataMapping, nullAllowed = TRUE)
  validateIsOfType(plotConfiguration, PlotConfiguration, nullAllowed = TRUE)

  # If data is not input,  creates new data and its mapping from x and y input
  if (isEmpty(data)) {
    data <- as.data.frame(cbind(x = x, y = y))
    dataMapping <- dataMapping %||% XYGDataMapping$new(x = ifNotNull(x, "x"), y = ifNotNull(y, "y"), data = data)
  }

  # Enforce data to be a data.frame for dataMapping
  if (!isOfType(data, "data.frame")) {
    data <- data.frame(data, check.names = FALSE)
  }

  dataMapping <- dataMapping %||% XYGDataMapping$new(x = "x", y = "y", data = data)
  plotConfiguration <- plotConfiguration %||% PlotConfiguration$new(data = data, metaData = metaData, dataMapping = dataMapping)
  # Update plotConfiguration if user defined aesthetics
  eval(.parseVariableToObject("plotConfiguration$lines", c("color", "shape", "linetype", "size"), keepIfNull = TRUE))

  # If no plot, initialize empty plot
  plotObject <- plotObject %||% initializePlot(plotConfiguration)

  # If no mapping, return plot
  if (all(isEmpty(dataMapping$x), isEmpty(dataMapping$y))) {
    warning("No mapping found for both x and y, line layer was not added")
    return(plotObject)
  }

  # Get mapping and convert labels into characters usable by aes
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)
  # If no specific mapping, use default captions
  if (min(levels(factor(mapData$legendLabels)) == "") == 1) {
    mapData$legendLabels <- .getlegendLabelsCaption(plotObject)
  }
  mapData$legendLabels <- caption %||% mapData$legendLabels

  # y-intercept
  # geom_blank is used to fill the missing aes properties
  # This prevents messing up the legend
  if (isEmpty(dataMapping$x) && !isEmpty(dataMapping$y)) {
    plotObject <- plotObject +
      ggplot2::geom_hline(
        data = mapData,
        mapping = ggplot2::aes(
          yintercept = .data[[mapLabels$y]],
          linetype = .data$legendLabels,
          color = .data$legendLabels,
        ),
        linewidth = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$size, aesthetic = "size"),
        alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$alpha, aesthetic = "alpha"),
        show.legend = TRUE
      ) +
      ggplot2::geom_blank(
        data = mapData,
        mapping = ggplot2::aes(
          shape = .data$legendLabels,
          fill = .data$legendLabels
        ),
        # Use NA instead of TRUE to prevent having an unwanted point in the legend key
        show.legend = NA
      )
  }

  # x-intercept
  if (isEmpty(dataMapping$y) && !isEmpty(dataMapping$x)) {
    plotObject <- plotObject +
      ggplot2::geom_vline(
        data = mapData,
        mapping = ggplot2::aes(
          xintercept = .data[[mapLabels$x]],
          linetype = .data$legendLabels,
          color = .data$legendLabels
        ),
        linewidth = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$size, aesthetic = "size"),
        alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$alpha, aesthetic = "alpha"),
        show.legend = TRUE
      ) +
      ggplot2::geom_blank(
        data = mapData,
        mapping = ggplot2::aes(
          shape = .data$legendLabels,
          fill = .data$legendLabels
        ),
        # Use NA instead of TRUE to prevent having an unwanted point in the legend key
        show.legend = NA
      )
  }

  # Case of a line defined by x and y
  # geom_path is used instead of geom_line,
  # consequently values are connected by their order of appearance and not according to x values
  if (all(!isEmpty(dataMapping$x), !isEmpty(dataMapping$y))) {
    plotObject <- plotObject +
      geomTLFPoint(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          y = .data[[mapLabels$y]],
          shape = .data$legendLabels,
          color = .data$legendLabels,
          size = .data$legendLabels
        ),
        alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$alpha, aesthetic = "alpha"),
        show.legend = TRUE,
        na.rm = TRUE
      ) +
      ggplot2::geom_path(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          y = .data[[mapLabels$y]],
          linetype = .data$legendLabels,
          color = .data$legendLabels,
        ),
        linewidth = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$size, aesthetic = "size"),
        alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$lines$alpha, aesthetic = "alpha"),
        show.legend = TRUE,
        na.rm = TRUE
      ) +
      ggplot2::geom_ribbon(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          ymin = .data[[mapLabels$y]],
          ymax = .data[[mapLabels$y]],
          fill = .data$legendLabels
        ),
        # alpha is 0 so that line can be seen in legend
        alpha = 0,
        show.legend = TRUE,
        na.rm = TRUE
      )
  }

  # Prepare data for merging previous and current legend
  newLabels <- levels(factor(mapData$legendLabels))
  # Sample LegendType properties based Theme if not input
  try(plotObject <- .mergeLegend(plotObject,
    newLabels = newLabels,
    aestheticSelections = plotConfiguration$lines
  ))
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}

#' @title addRibbon
#' @description
#' Add a ribbon layer to a `ggplot` object.
#'
#' @inheritParams addScatter
#' @param ymin Numeric values to plot along the `y` axis. Only used instead of `data` if `data` is `NULL`.
#' @param ymax Numeric values to plot along the `y` axis. Only used instead of `data` if `data` is `NULL`.
#' @param dataMapping A `RangeDataMapping` object mapping `x`, `ymin`, `ymax` and aesthetic groups to their variable names of `data`.
#' @param fill Optional character values defining the colors of the plot layer.
#' See `grDevices::colors()` to get names of colors
#' @param alpha Numeric value between 0 and 1 corresponding to transparency of the ribbon.
#' The closer to 0, the more transparent the ribbon is.
#' The closer to 1, the more opaque the ribbon is.
#' @return A `ggplot` object
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/atom-plots.html>
#'
#' @family atom plots
#' @export
#' @examples
#' # Add ribbon using x, ymin and ymax
#' addRibbon(
#'   x = c(1, 2, 1, 2, 3),
#'   ymin = c(5, 0, 2, 3, 4),
#'   ymax = c(6, 2, 6, 2.5, 5)
#' )
#'
#' # Add ribbon using a data.frame
#' time <- seq(0, 30, 0.1)
#' ribbonData <- data.frame(x = time, ymin = cos(time) - 1, ymax = cos(time) + 1)
#'
#' addRibbon(
#'   data = ribbonData,
#'   dataMapping = RangeDataMapping$new(x = "x", ymin = "ymin", ymax = "ymax")
#' )
#'
#' # Or for simple cases a smart mapping will get directly x, ymin and ymax from data
#' addRibbon(data = ribbonData)
#'
#' # Add a ribbon with caption
#' addRibbon(data = ribbonData, caption = "My ribbon plot")
#'
#' # Add a ribbon with specific properties
#' addRibbon(data = ribbonData, fill = "blue", alpha = 0.5, caption = "My data")
#'
#' # Add a ribbon with specific properties
#' p <- addRibbon(data = ribbonData, fill = "blue", alpha = 0.5, caption = "My data")
#' addRibbon(
#'   x = c(0, 1), ymin = c(-0.5, -0.5), ymax = c(0.5, 0.5),
#'   fill = "red", alpha = 1,
#'   plotObject = p
#' )
#'
addRibbon <- function(data = NULL,
                      metaData = NULL,
                      x = NULL,
                      ymin = NULL,
                      ymax = NULL,
                      caption = NULL,
                      fill = NULL,
                      color = NULL,
                      size = NULL,
                      linetype = NULL,
                      alpha = NULL,
                      dataMapping = NULL,
                      plotConfiguration = NULL,
                      plotObject = NULL) {
  validateIsOfType(dataMapping, RangeDataMapping, nullAllowed = TRUE)
  validateIsOfType(plotConfiguration, PlotConfiguration, nullAllowed = TRUE)

  # If data is not input, creates data and its mapping from x, ymin and ymax input
  if (isEmpty(data)) {
    data <- as.data.frame(cbind(x = x, ymin = ymin %||% 0, ymax = ymax %||% 0))

    # y-intercept ribbon
    if (isEmpty(x)) {
      # Redefine data.frame for y-intercept ribbon
      data <- rbind.data.frame(cbind.data.frame(x = -Inf, data), cbind.data.frame(x = Inf, data))
    }
    dataMapping <- dataMapping %||% RangeDataMapping$new(x = "x", ymin = "ymin", ymax = "ymax", data = data)
  }
  # Enforce data to be a data.frame for dataMapping
  if (!isOfType(data, "data.frame")) {
    data <- data.frame(data, check.names = FALSE)
  }

  dataMapping <- dataMapping %||% RangeDataMapping$new(x = x, ymin = ymin, ymax = ymax, data = data)
  plotConfiguration <- plotConfiguration %||% PlotConfiguration$new(data = data, metaData = metaData, dataMapping = dataMapping)
  # Update plotConfiguration if user defined aesthetics
  eval(.parseVariableToObject("plotConfiguration$ribbons", c("color", "fill", "linetype", "size", "alpha"), keepIfNull = TRUE))

  # If no plot, initialize empty plot
  plotObject <- plotObject %||% initializePlot(plotConfiguration)

  # If no mapping, return plot
  if (all(isEmpty(dataMapping$x), isEmpty(dataMapping$ymin), isEmpty(dataMapping$ymax))) {
    warning("No mapping found for x, ymin and ymax, ribbon layer was not added")
    return(plotObject)
  }

  # Get mapping and convert labels into characters usable by aes
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)
  # If no specific mapping, use default captions
  if (min(levels(factor(mapData$legendLabels)) == "") == 1) {
    mapData$legendLabels <- .getlegendLabelsCaption(plotObject)
  }
  mapData$legendLabels <- caption %||% mapData$legendLabels

  plotObject <- plotObject +
    ggplot2::geom_ribbon(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        ymin = .data[[mapLabels$ymin]],
        ymax = .data[[mapLabels$ymax]],
        fill = .data$legendLabels
      ),
      alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$ribbons$alpha, position = 0, aesthetic = "alpha"),
      show.legend = TRUE,
      na.rm = TRUE
    ) +
    ggplot2::geom_blank(
      data = mapData,
      mapping = aes(shape = .data$legendLabels, color = .data$legendLabels, size = .data$legendLabels, linetype = .data$legendLabels)
    )

  # Prepare data for merging previous and current legend
  newLabels <- levels(factor(mapData$legendLabels))
  # Sample LegendType properties based Theme if not input
  try(plotObject <- .mergeLegend(plotObject,
    newLabels = newLabels,
    aestheticSelections = plotConfiguration$ribbons
  ))
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}

#' @title addErrorbar
#' @description
#' Add an errorbar layer to a `ggplot` object.
#'
#' @inheritParams addRibbon
#' @inheritParams addScatter
#' @param capSize Numeric extent of the error bars caps
#' Caution the value corresponds to the ratio of the mean spacing between plotted error bars.
#' For instance, an `extent` of `1` will fill the caps until the next error bar
#' @return A `ggplot` object
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/atom-plots.html>
#'
#' @family atom plots
#' @export
#' @examples
#' # Add errorbar using x, ymin and ymax
#' addErrorbar(
#'   x = c(1, 2, 1, 2, 3),
#'   ymin = c(5, 0, 2, 3, 4),
#'   ymax = c(6, 2, 6, 2.5, 5)
#' )
#'
#' # Add errorbar using a data.frame
#' time <- seq(0, 30, 0.5)
#' errorbarData <- data.frame(x = time, ymin = cos(time) - 1, ymax = cos(time) + 1)
#'
#' addErrorbar(
#'   data = errorbarData,
#'   dataMapping = RangeDataMapping$new(x = "x", ymin = "ymin", ymax = "ymax")
#' )
#'
#' # Or for simple cases a smart mapping will get directly x, ymin and ymax from data
#' addErrorbar(data = errorbarData)
#'
#' # Add a errorbar with caption
#' addErrorbar(data = errorbarData, caption = "My errorbar plot")
#'
#' # Add a errorbar with specific properties
#' addErrorbar(data = errorbarData, color = "blue", size = 0.5, caption = "My data")
#'
#' # Add a errorbar with specific properties
#' p <- addErrorbar(
#'   data = errorbarData,
#'   color = "blue", size = 0.5, caption = "My data"
#' )
#' addScatter(
#'   x = time, y = cos(time),
#'   color = "red", size = 1, caption = "My data",
#'   plotObject = p
#' )
#'
addErrorbar <- function(data = NULL,
                        metaData = NULL,
                        x = NULL,
                        ymin = NULL,
                        ymax = NULL,
                        caption = NULL,
                        color = NULL,
                        size = NULL,
                        linetype = NULL,
                        capSize = NULL,
                        dataMapping = NULL,
                        plotConfiguration = NULL,
                        plotObject = NULL) {
  validateIsOfType(dataMapping, c("RangeDataMapping", "ObservedDataMapping"), nullAllowed = TRUE)
  validateIsOfType(plotConfiguration, PlotConfiguration, nullAllowed = TRUE)

  # If data is not input, creates data and its mapping from x, ymin and ymax input
  if (isEmpty(data)) {
    data <- as.data.frame(cbind(x = x, ymin = ymin %||% 0, ymax = ymax %||% 0))
    dataMapping <- dataMapping %||% RangeDataMapping$new(x = "x", ymin = "ymin", ymax = "ymax", data = data)
  }
  # Enforce data to be a data.frame for dataMapping
  if (!isOfType(data, "data.frame")) {
    data <- data.frame(data, check.names = FALSE)
  }

  dataMapping <- dataMapping %||% RangeDataMapping$new(x = x, ymin = ymin, ymax = ymax, data = data)
  plotConfiguration <- plotConfiguration %||% PlotConfiguration$new(data = data, metaData = metaData, dataMapping = dataMapping)
  # Update plotConfiguration if user defined aesthetics
  eval(.parseVariableToObject("plotConfiguration$errorbars", c("color", "linetype", "size"), keepIfNull = TRUE))

  # If no plot, initialize empty plot
  plotObject <- plotObject %||% initializePlot(plotConfiguration)

  # If no mapping, return plot
  if (all(isEmpty(dataMapping$x), isEmpty(dataMapping$ymin), isEmpty(dataMapping$ymax))) {
    warning("No mapping found for x, ymin and ymax, error bar layer was not added")
    return(plotObject)
  }

  # Get mapping and convert labels into characters usable by aes
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)
  # If no specific mapping, use default captions
  if (min(levels(factor(mapData$legendLabels)) == "") == 1) {
    mapData$legendLabels <- .getlegendLabelsCaption(plotObject)
  }
  mapData$legendLabels <- caption %||% mapData$legendLabels
  legendLength <- length(unique(mapData$legendLabels))

  eval(.parseVariableToObject("plotObject$plotConfiguration$errorbars", c("color", "size", "linetype"), keepIfNull = TRUE))

  plotObject <- plotObject +
    ggplot2::geom_linerange(
      data = mapData,
      mapping = aes(
        x = .data[[mapLabels$x]],
        ymin = .data[[mapLabels$ymin]],
        ymax = .data[[mapLabels$ymax]],
        color = .data[[mapLabels$color]],
        group = .data[[mapLabels$color]]
      ),
      linewidth = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$size, position = 0, aesthetic = "size"),
      linetype = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$linetype, aesthetic = "linetype"),
      alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$alpha, aesthetic = "alpha"),
      color = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$color, aesthetic = "color"),
      na.rm = TRUE,
      show.legend = FALSE
    ) +
    ggplot2::geom_point(
      data = mapData,
      mapping = aes(
        x = .data[[mapLabels$x]],
        y = .data[[mapLabels$ymin]],
        color = .data[[mapLabels$color]],
        group = .data[[mapLabels$color]]
      ),
      size = capSize %||% tlfEnv$defaultErrorbarCapSize,
      shape = "_",
      alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$alpha, aesthetic = "alpha"),
      color = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$color, aesthetic = "color"),
      na.rm = TRUE,
      show.legend = FALSE
    ) +
    ggplot2::geom_point(
      data = mapData,
      mapping = aes(
        x = .data[[mapLabels$x]],
        y = .data[[mapLabels$ymax]],
        color = .data[[mapLabels$color]],
        group = .data[[mapLabels$color]]
      ),
      size = capSize %||% tlfEnv$defaultErrorbarCapSize,
      shape = "_",
      alpha = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$alpha, aesthetic = "alpha"),
      color = .getAestheticValues(n = 1, selectionKey = plotConfiguration$errorbars$color, aesthetic = "color"),
      na.rm = TRUE,
      show.legend = FALSE
    )

  # Try is used to prevent crashes in the final plot due to ggplot2 peculiarities regarding scale functions
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}

#' @keywords internal
.getlegendLabelsCaption <- function(plotObject) {
  legendLabelsCaptionCount <- which(paste0("data ", seq(1, 100)) %in% plotObject$plotConfiguration$legend$caption$label)
  if (length(legendLabelsCaptionCount) == 0) {
    legendLabelsCaptionCount <- 0
  }
  return(paste0("data ", max(legendLabelsCaptionCount) + 1))
}
