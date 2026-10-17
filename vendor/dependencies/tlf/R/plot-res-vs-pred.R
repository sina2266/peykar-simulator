#' @title plotResVsPred
#' @description
#' Producing residuals vs predicted plots
#'
#' @inheritParams plotObsVsPred
#' @param dataMapping
#' A `ResVsPredDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `ResVsPredConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce Obs vs Pred plot
#' resVsPredData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#'
#' plotResVsPred(data = resVsPredData, dataMapping = ResVsPredDataMapping$new(x = "x", y = "y"))
#'
#' # Produce Res vs Pred plot with linear regression
#' plotResVsPred(
#'   data = resVsPredData,
#'   dataMapping = ResVsPredDataMapping$new(x = "x", y = "y"),
#'   smoother = "lm"
#' )
plotResVsPred <- function(data,
                          metaData = NULL,
                          dataMapping = NULL,
                          plotConfiguration = NULL,
                          smoother = NULL,
                          plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, ResVsPredDataMapping, data)
  validateIsIncluded(smoother, c("loess", "lm"), nullAllowed = TRUE)
  dataMapping$smoother <- smoother %||% dataMapping$smoother

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, ResVsPredPlotConfiguration,
    data, metaData, dataMapping
  )
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # Each new layer is added on top of previous
  # Thus, scatter points are added as last layer to prevent them being hidden by lines or errorbars
  # 1- Diagonal lines
  for (lineIndex in seq_along(dataMapping$lines)) {
    plotObject <- .addLineLayer(
      plotObject,
      type = "horizontal",
      value = dataMapping$lines[[lineIndex]],
      # position corresponds to the number of line layers already added
      position = lineIndex - 1
    )
  }
  lineIndex <- ifNotNull(lineIndex, lineIndex, 0)

  # 2- Smoother line
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
  # 2- Scatter points
  plotObject <- .addScatterLayer(plotObject, data = mapData, mapLabels = mapLabels)

  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "points",
    propertyNames = c("color", "shape"),
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .updateSymmetricAxes(plotObject, mapData, dataMapping)
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}

#' @title plotResVsTime
#' @description
#' Producing residuals vs time plots
#'
#' @inheritParams plotObsVsPred
#' @param dataMapping
#' A `ResVsTimeDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `ResVsTimeConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce Obs vs Pred plot
#' resVsTimeData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#'
#' plotResVsTime(data = resVsTimeData, dataMapping = ResVsTimeDataMapping$new(x = "x", y = "y"))
#'
#' # Produce Res vs Time plot with linear regression
#' plotResVsTime(
#'   data = resVsTimeData,
#'   dataMapping = ResVsTimeDataMapping$new(x = "x", y = "y"),
#'   smoother = "lm"
#' )
plotResVsTime <- plotResVsPred
