#' @title plotBoxWhisker
#' @description
#' Producing box-and-whisker plots
#'
#' @inheritParams addScatter
#' @param outliers Logical defining if outliers should be included in boxplot
#' @param dataMapping
#' A `BoxWhiskerDataMapping` object mapping `x`, `y` and aesthetic groups to their variable names of `data`.
#' @param plotConfiguration
#' An optional `BoxWhiskerConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/box-whisker-vignette.html>
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce box-and-whisker plots of log-normal distributed data
#' boxData <- data.frame(x = c(rep("A", 500), rep("B", 500)), y = rlnorm(1000))
#'
#' plotBoxWhisker(data = boxData, dataMapping = BoxWhiskerDataMapping$new(x = "x", y = "y"))
#'
#' # Remove outliers from boxplot
#' plotBoxWhisker(
#'   data = boxData,
#'   dataMapping = BoxWhiskerDataMapping$new(x = "x", y = "y"),
#'   outliers = FALSE
#' )
#'
plotBoxWhisker <- function(data,
                           metaData = NULL,
                           outliers = NULL,
                           dataMapping = NULL,
                           plotConfiguration = NULL,
                           plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, BoxWhiskerDataMapping, data)
  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, BoxWhiskerPlotConfiguration,
    data, metaData, dataMapping
  )
  # Overwrites plotConfiguration$outliers if outliers is not null
  validateIsLogical(outliers, nullAllowed = TRUE)
  plotConfiguration$outliers <- outliers
  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  #----- Build layers of molecule plot -----
  plotObject <- .addBoxWhisker(data, metaData, dataMapping, plotConfiguration, plotObject)
  if (plotConfiguration$outliers) {
    plotObject <- .addOutliers(data, metaData, dataMapping, plotConfiguration, plotObject)
  }
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}

#' @title .addBoxWhisker
#' @description
#' Add a boxplot layer to a `ggplot` object (without outliers)
#'
#' @inheritParams plotBoxWhisker
#' @return A `ggplot` object
#' @keywords internal
.addBoxWhisker <- function(data, metaData, dataMapping, plotConfiguration, plotObject) {
  # Get the box plot quantiles from dataMapping
  mapData <- dataMapping$getBoxWhiskerLimits(data)
  # Convert the mapping into characters usable by aes
  mapLabels <- .getAesStringMapping(dataMapping)

  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = 0,
    plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
    propertyNames = c("size", "alpha", "linetype")
  )

  plotObject <- plotObject +
    ggplot2::geom_boxplot(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        ymin = .data$ymin,
        lower = .data$lower,
        middle = .data$middle,
        upper = .data$upper,
        ymax = .data$ymax,
        fill = .data[[mapLabels$fill]],
        color = .data[[mapLabels$color]]
      ),
      alpha = aestheticValues$alpha,
      size = aestheticValues$size,
      linetype = aestheticValues$linetype,
      show.legend = TRUE,
      stat = "identity"
    )

  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "ribbons",
    propertyNames = c("color", "fill"),
    data = mapData,
    mapLabels = mapLabels
  )
  return(plotObject)
}

#' @title .addOutliers
#' @description
#' Add scatter points for outliers to `ggplot` object
#'
#' @inheritParams plotBoxWhisker
#' @return A `ggplot` object
#' @keywords internal
.addOutliers <- function(data, metaData, dataMapping, plotConfiguration, plotObject) {
  mapData <- dataMapping$getOutliers(data)
  # Convert the mapping into characters usable by aes
  mapLabels <- .getAesStringMapping(dataMapping)

  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = 0,
    plotConfigurationProperty = plotObject$plotConfiguration$points,
    propertyNames = c("size", "alpha", "shape", "color")
  )

  # addScatterLayer cannot be used in this case,
  # because position dodge is needed to align boxes and outlier points
  # no matter the number of groups, the value of 0.9 will be always fix
  # otherwise, points won't be centered anymore
  # besides, mapData includes NA instead of non-outlying data,
  # na.rm removes these points without sending warning
  plotObject <- plotObject +
    geomTLFPoint(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = .data$maxOutliers,
        group = .data[[mapLabels$fill]],
        color = .data[[mapLabels$color]]
      ),
      size = aestheticValues$size,
      shape = aestheticValues$shape,
      color = aestheticValues$color,
      alpha = aestheticValues$alpha,
      show.legend = TRUE,
      na.rm = TRUE,
      position = position_dodge(width = 0.9)
    ) +
    geomTLFPoint(
      data = mapData,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = .data$minOutliers,
        group = .data[[mapLabels$fill]],
        color = .data[[mapLabels$color]]
      ),
      size = aestheticValues$size,
      shape = aestheticValues$shape,
      color = aestheticValues$color,
      alpha = aestheticValues$alpha,
      show.legend = TRUE,
      na.rm = TRUE,
      position = position_dodge(width = 0.9)
    )
  return(plotObject)
}
