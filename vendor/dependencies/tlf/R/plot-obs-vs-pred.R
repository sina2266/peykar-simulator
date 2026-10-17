#' @title plotObsVsPred
#' @description
#' Producing observed vs predicted plots
#'
#' @inheritParams addScatter
#' @param foldDistance Numeric values of fold distance lines to display in log plots.
#' This argument is internally translated into `lines` field of `dataMapping`.
#' __Caution__: this argument is meant for log scaled plots and since fold distance is a ratio it is expected positive.
#' In particular, line of identity corresponds to a `foldDistance` of `1`.
#' @param smoother Optional name of smoother function:
#' \itemize{
#' \item `"loess"` for loess regression
#' \item `"lm"` for linear regression
#' }
#' @param dataMapping
#' A `ObsVsPredDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `ObsVsPredConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce Obs vs Pred plot
#' obsVsPredData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#'
#' plotObsVsPred(data = obsVsPredData, dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"))
#'
#' # Produce Obs vs Pred plot with linear regression
#' plotObsVsPred(
#'   data = obsVsPredData,
#'   dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
#'   smoother = "lm"
#' )
#'
#' # Produce Obs vs Pred plot with user-defined fold distance lines
#' plotObsVsPred(
#'   data = obsVsPredData,
#'   dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
#'   plotConfiguration = ObsVsPredPlotConfiguration$new(
#'     xScale = Scaling$log, xAxisLimits = c(0.05, 50),
#'     yScale = Scaling$log, yAxisLimits = c(0.05, 50)
#'   ),
#'   foldDistance = c(1, 10)
#' )
#'
plotObsVsPred <- function(data,
                          metaData = NULL,
                          dataMapping = NULL,
                          plotConfiguration = NULL,
                          foldDistance = NULL,
                          smoother = NULL,
                          plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, ObsVsPredDataMapping, data)
  validateIsNumeric(foldDistance, nullAllowed = TRUE)
  # foldDistance only updates dataMapping$lines if defined
  if (!isEmpty(foldDistance)) {
    dataMapping$lines <- getLinesFromFoldDistance(foldDistance)
  }
  validateIsIncluded(smoother, c("loess", "lm"), nullAllowed = TRUE)
  dataMapping$smoother <- smoother %||% dataMapping$smoother

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, ObsVsPredPlotConfiguration,
    data, metaData, dataMapping
  )
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # Each new layer is added on top of previous
  # Thus, scatter points are added as last layer to prevent them being hidden by lines or errorbars

  # 1- if available, add LLOQ lines
  if (!isEmpty(mapData$lloq)) {
    plotObject <- .addLLOQLayer(
      plotObject,
      data = mapData,
      mapLabels = mapLabels,
      direction = plotConfiguration$lloqDirection
    )
  }

  # 2- Diagonal lines
  lineIndex <- 0
  if (!isEmpty(dataMapping$lines)) {
    # Add foldDistance legend only if user specified folddistance values
    for (lineIndex in seq_along(dataMapping$lines)) {
      lineValue <- .getAblineValues(dataMapping$lines[[lineIndex]], plotConfiguration$yAxis$scale)

      plotObject <- .addLineLayer(
        plotObject,
        type = "obsvspredDiagonal",
        value = lineValue,
        # position corresponds to the number of line layers already added
        position = lineIndex - 1
      )
    }

    # Defines fold lines linetypes
    positions <- seq_along(dataMapping$lines) - 1

    values <- setNames(unlist(Linetypes[seq_along(positions)]), positions)
    breaks <- positions[foldDistance != 1]

    # Do not plot the legend for x=y line
    labels <- if (length(breaks) > 0) {
      paste0(foldDistance[foldDistance != 1], "-fold")
    } else {
      NULL
    }

    plotObject <-
      plotObject +
      scale_linetype_manual(
        breaks = breaks,
        values = values,
        labels = labels
      )
  }


  # 3- Smoother line
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = lineIndex,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = c("color", "linetype", "size", "alpha")
  )
  if (isIncluded(dataMapping$smoother, "loess")) {
    plotObject <- plotObject +
      ggplot2::geom_smooth(
        data = mapData,
        method = "loess",
        se = FALSE,
        formula = "y ~ x",
        na.rm = TRUE,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          y = .data[[mapLabels$y]]
        ),
        color = aestheticValues$color,
        linetype = aestheticValues$linetype,
        alpha = aestheticValues$alpha,
        size = aestheticValues$size
      )
  }
  if (isIncluded(dataMapping$smoother, "lm")) {
    plotObject <- plotObject +
      ggplot2::geom_smooth(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          y = .data[[mapLabels$y]]
        ),
        method = "lm",
        se = FALSE,
        formula = "y ~ x",
        na.rm = TRUE,
        color = aestheticValues$color,
        linetype = aestheticValues$linetype,
        alpha = aestheticValues$alpha,
        size = aestheticValues$size
      )
  }
  # 4- Error bars
  plotObject <- .addErrorbarLayer(plotObject, data = mapData, mapLabels = mapLabels, direction = "horizontal")
  # 5- Scatter points
  plotObject <- .addScatterLayer(plotObject, data = mapData, mapLabels = mapLabels)

  # .addScatterLayer adds shapes legend on the lines so we need to override it
  # while doing so, it reset the legend title so it is forced to plotConfiguration$legend$title$text.
  # Drawing area is also extendend and linewidth is fixed to prevent confusion between lines when big

  # Legend display
  if (!plotConfiguration$foldLinesLegend) {
    plotObject <- plotObject + guides(linetype = "none")
  } else {
    plotObject <-
      plotObject +
      guides(linetype = guide_legend(
        title = plotConfiguration$legend$title$text,
        override.aes = list(
          shape = NULL,
          linewidth = 0.5
        ),
        keywidth = unit(2, "lines")
      ))
  }


  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "points",
    propertyNames = c("color", "shape"),
    data = mapData,
    mapLabels = mapLabels
  )

  # Update axes limits if option symmetric and user did not define specific limits
  plotObject <- .updateSameAxes(plotObject, mapData, dataMapping)
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}
