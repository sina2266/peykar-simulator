#' @title plotPieChart
#' @description
#' Producing a Pie Chart
#'
#' @inheritParams addScatter
#' @param colorPalette color palette property from `ggplot2`
#' @param start Offset of starting point from 12 o'clock in radians.
#' Offset is applied clockwise or anticlockwise depending on value of direction
#' @param clockwiseDirection logical defining if values are displayed in clockwise order
#' @param dataMapping
#' A `PieChartDataMapping` object mapping `x` and `fill` aesthetic groups to their variable names of `data`.
#' Values mapped to `y` variable will be displayed as text within the pie chart
#' @param plotConfiguration
#' An optional `PieChartPlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Data for the pie chart
#' values <- runif(5)
#' data <- data.frame(
#'   values = values,
#'   text = paste0(round(100 * values / sum(values)), "%"),
#'   legend = letters[1:5]
#' )
#'
#' # Plot pie chart with its legend
#' plotPieChart(
#'   data = data,
#'   dataMapping = PieChartDataMapping$new(x = "values", fill = "legend")
#' )
#'
#' # Plot pie chart with text within pie
#' plotPieChart(
#'   data = data,
#'   dataMapping = PieChartDataMapping$new(x = "values", y = "text", fill = "legend")
#' )
#'
#' # Reverse direction of pie chart
#' plotPieChart(
#'   data = data,
#'   dataMapping = PieChartDataMapping$new(x = "values", y = "text", fill = "legend"),
#'   clockwiseDirection = FALSE
#' )
#'
#' # Start first slice of pie at 90 degrees
#' plotPieChart(
#'   data = data,
#'   dataMapping = PieChartDataMapping$new(x = "values", y = "text", fill = "legend"),
#'   start = pi / 2
#' )
#'
#' # Leverages ggplot color palettes
#' plotPieChart(
#'   data = data,
#'   dataMapping = PieChartDataMapping$new(x = "values", y = "text", fill = "legend"),
#'   colorPalette = ColorPalettes$Set1
#' )
#'
plotPieChart <- function(data = NULL,
                         metaData = NULL,
                         dataMapping = NULL,
                         colorPalette = NULL,
                         start = NULL,
                         clockwiseDirection = NULL,
                         plotConfiguration = NULL,
                         plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  validateIsNotEmpty(data)
  validateIsOfType(data, "data.frame")
  dataMapping <- .setDataMapping(dataMapping, PieChartDataMapping, data)
  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, PieChartPlotConfiguration,
    data, metaData, dataMapping
  )
  # Update plotConfiguration if inputs provided by user
  validateIsString(colorPalette, nullAllowed = TRUE)
  validateIsNumeric(start, nullAllowed = TRUE)
  validateIsLogical(clockwiseDirection, nullAllowed = TRUE)

  plotConfiguration$colorPalette <- colorPalette %||% plotConfiguration$colorPalette
  plotConfiguration$start <- start %||% plotConfiguration$start
  plotConfiguration$clockwiseDirection <- clockwiseDirection %||% plotConfiguration$clockwiseDirection

  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  #----- Build layers of molecule plot -----
  # position defines if bars are stacked or plotted side by side

  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
    propertyNames = c("color", "size", "alpha", "linetype")
  )

  # 1- Pie Chart
  plotObject <- plotObject +
    ggplot2::geom_col(
      data = mapData,
      mapping = ggplot2::aes(
        x = "",
        y = .data[[mapLabels$x]],
        fill = .data[[mapLabels$fill]]
      ),
      color = aestheticValues$color,
      alpha = aestheticValues$alpha,
      position = ggplot2::position_fill(reverse = TRUE)
    )

  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "ribbons",
    propertyNames = "fill",
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .applyColorPalette(
    plotObject,
    colorPalette = plotConfiguration$colorPalette
  )

  # 2- If defined include text in y within pie chart
  if (!isEmpty(dataMapping$y)) {
    # Get properties of R6 font object
    chartFont <- plotConfiguration$chartFont
    textAlignment <- switch(chartFont$align,
      "left" = 0.25,
      "center" = 0.5,
      "right" = 0.75
    )

    plotObject <- plotObject + ggplot2::geom_text(
      data = mapData,
      mapping = ggplot2::aes(
        x = "",
        y = .data[[mapLabels$x]],
        label = .data[[mapLabels$y]]
      ),
      position = ggplot2::position_fill(vjust = textAlignment, reverse = TRUE),
      color = chartFont$color,
      family = .checkPlotFontFamily(chartFont$fontFamily),
      size = chartFont$size / ggplot2::.pt,
      angle = chartFont$angle
    )
  }

  # Remove background elements that do not belong to pie charts
  plotObject <- setGrid(plotObject, linetype = Linetypes$blank)
  plotObject <- plotObject +
    ggplot2::theme(
      axis.line.x = ggplot2::element_blank(),
      axis.line.y = ggplot2::element_blank(),
      axis.text = ggplot2::element_blank(),
      axis.title.x = ggplot2::element_blank(),
      axis.title.y = ggplot2::element_blank(),
      axis.ticks = ggplot2::element_blank()
    ) +
    ggplot2::coord_polar(
      theta = "y",
      start = plotConfiguration$start,
      direction = ifelse(plotConfiguration$clockwiseDirection, 1, -1)
    )
  # Remove watermark that can't be used along with coord polar
  plotObject$layers <- plotObject$layers[-1]
  return(plotObject)
}
