#' @title setGrid
#' @description Set x and y grid properties of a `ggplot` object
#' @param plotObject A `ggplot` object
#' @param color Optional character values defining the color of the grid.
#' See `grDevices::colors()` to get names of colors
#' @param linetype Optional character values defining the linetype of the grid.
#' See enum `Linetypes` to get names of linetype.
#' @param size Optional numeric values defining the size of the grid.
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set grid of a scatter plot
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setGrid(p, color = "red", linetype = "dotted")
#'
setGrid <- function(plotObject,
                    color = NULL,
                    linetype = NULL,
                    size = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(color, nullAllowed = TRUE)
  validateIsIncluded(linetype, Linetypes, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  background <- newPlotObject$plotConfiguration$background
  eval(.parseVariableToObject("background$xGrid", c("color", "linetype", "size"), keepIfNull = TRUE))
  eval(.parseVariableToObject("background$yGrid", c("color", "linetype", "size"), keepIfNull = TRUE))
  newPlotObject <- background$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setXGrid
#' @description Set x grid properties of a `ggplot` object
#' @inheritParams setGrid
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set x grid of a scatter plot
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setXGrid(p, color = "red", linetype = "dotted")
#'
setXGrid <- function(plotObject,
                     color = NULL,
                     linetype = NULL,
                     size = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(color, nullAllowed = TRUE)
  validateIsIncluded(linetype, Linetypes, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  background <- newPlotObject$plotConfiguration$background
  eval(.parseVariableToObject("background$xGrid", c("color", "linetype", "size"), keepIfNull = TRUE))
  newPlotObject <- background$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setYGrid
#' @description Set y grid properties of a `ggplot` object
#' @inheritParams setGrid
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set y grid of a scatter
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setYGrid(p, color = "red", linetype = "dotted")
#'
setYGrid <- function(plotObject,
                     color = NULL,
                     linetype = NULL,
                     size = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(color, nullAllowed = TRUE)
  validateIsIncluded(linetype, Linetypes, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  background <- newPlotObject$plotConfiguration$background
  eval(.parseVariableToObject("background$yGrid", c("color", "linetype", "size"), keepIfNull = TRUE))
  newPlotObject <- background$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setBackground
#' @description Set background properties of a `ggplot` object
#' @param plotObject A `ggplot` object
#' @param fill Optional character values defining the color of the background.
#' See `grDevices::colors()` to get names of colors
#' @param color Optional character values defining the color of the background frame.
#' See `grDevices::colors()` to get names of colors
#' @param linetype Optional character values defining the linetype of the background frame.
#' See enum `Linetypes` to get names of linetype.
#' @param size Optional numeric values defining the size of the background frame.
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set background of a scatter plot
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setBackground(p, fill = "yellowgreen", color = "red", linetype = "dotted")
#'
setBackground <- function(plotObject,
                          fill = NULL,
                          color = NULL,
                          linetype = NULL,
                          size = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(fill, nullAllowed = TRUE)
  validateIsString(color, nullAllowed = TRUE)
  validateIsIncluded(linetype, Linetypes, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  background <- newPlotObject$plotConfiguration$background
  eval(.parseVariableToObject("background$plot", c("fill", "color", "linetype", "size"), keepIfNull = TRUE))
  eval(.parseVariableToObject("background$panel", c("fill", "color", "linetype", "size"), keepIfNull = TRUE))
  newPlotObject <- background$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setBackgroundPanelArea
#' @description Set background panel area properties of a `ggplot` object
#' @inheritParams setBackground
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set background of a scatter plot
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setBackgroundPanelArea(p, fill = "yellowgreen", color = "red", linetype = "dotted")
#'
setBackgroundPanelArea <- function(plotObject,
                                   fill = NULL,
                                   color = NULL,
                                   linetype = NULL,
                                   size = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(fill, nullAllowed = TRUE)
  validateIsString(color, nullAllowed = TRUE)
  validateIsIncluded(linetype, Linetypes, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  background <- newPlotObject$plotConfiguration$background
  eval(.parseVariableToObject("background$panel", c("fill", "color", "linetype", "size"), keepIfNull = TRUE))
  newPlotObject <- background$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setBackgroundPlotArea
#' @description Set background plot area properties of a `ggplot` object
#' @inheritParams setBackground
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set background of a scatter plot
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setBackgroundPlotArea(p, fill = "yellowgreen", color = "red", linetype = "dotted")
#'
setBackgroundPlotArea <- function(plotObject,
                                  fill = NULL,
                                  color = NULL,
                                  linetype = NULL,
                                  size = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(fill, nullAllowed = TRUE)
  validateIsString(color, nullAllowed = TRUE)
  validateIsIncluded(linetype, Linetypes, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  background <- newPlotObject$plotConfiguration$background
  eval(.parseVariableToObject("background$plot", c("fill", "color", "linetype", "size"), keepIfNull = TRUE))
  newPlotObject <- background$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title addWatermark
#' @description
#' Creates a `ggplot` grob based on the label text and its font properties.
#' Then,  adds the grob to the `ggplot` object as a new layer using `ggplot2::annotation_custom`.
#' @param plotObject A `ggplot` object
#' @param watermark A character value or a `Label` object
#' @param color Color of the watermark.
#' @param size Size of the watermark.
#' @param angle Angle of the watermark (in degree).
#' @param alpha Numeric value between 0 and 1 corresponding to transparency of the watermark
#' The closer to 0, the more transparent the watermark is.
#' The closer to 1, the more opaque the watermark is.
#' @return A `ggplot` object
#' @import  ggplot2
#' @export
#' @examples
#' # Add a watermark to an empty plot
#' p <- ggplot2::ggplot()
#' addWatermark(p, "watermark")
#'
#' # Watermark with font properties
#' watermarkLabel <- Label$new(text = "watermark", color = "blue")
#' addWatermark(p, watermarkLabel)
#'
#' # Horizontal watermark
#' addWatermark(p, watermarkLabel, angle = 0)
#'
#' # Watermark totally opaque
#' addWatermark(p, watermarkLabel, alpha = 1)
#'
#' # As multiple layers of watermark:
#' p2 <- addWatermark(p, watermarkLabel, alpha = 1)
#' addWatermark(p2, "other watermark", color = "red", angle = 90)
#'
addWatermark <- function(plotObject,
                         watermark,
                         color = NULL,
                         size = NULL,
                         angle = NULL,
                         alpha = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(watermark, c("character", "Label"))
  validateIsString(color, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)
  validateIsNumeric(alpha, nullAllowed = TRUE)
  validateIsNumeric(angle, nullAllowed = TRUE)
  # Transparency from theme aesthetic map if left undefined
  alpha <- alpha %||% .getAestheticValues(
    n = 1,
    selectionKey = AestheticSelectionKeys$first,
    aesthetic = "alpha"
  )
  # Ensure watermark is a Label class
  if (isOfType(watermark, "character")) {
    watermark <- asLabel(watermark, font = tlfEnv$currentTheme$fonts$watermark)
  }
  eval(.parseVariableToObject("watermark$font", c("color", "size", "angle"), keepIfNull = TRUE))

  watermark <- createWatermarkGrob(label = watermark, alpha = alpha)

  plotObject <- plotObject + ggplot2::annotation_custom(grob = watermark, xmin = I(-Inf), ymin = I(-Inf), xmax = I(Inf), ymax = I(Inf))

  return(plotObject)
}

#' @title setWatermark
#' @description
#' Set the watermark of a `ggplot` object.
#' Unlike `addWatermark`, the watermark layer is overridden by `setWatermark`.
#' @inheritParams addWatermark
#' @return A `ggplot` object
#' @import ggplot2
#' @export
#' @examples
#' # Add a watermark to an empty plot
#' p <- initializePlot()
#' setWatermark(p, "watermark")
#'
#' # Watermark with font properties
#' watermarkLabel <- Label$new(text = "watermark", color = "blue")
#' setWatermark(p, watermarkLabel)
#'
#' # Horizontal watermark
#' setWatermark(p, watermarkLabel, angle = 0)
#'
#' # Watermark totally opaque
#' setWatermark(p, watermarkLabel, alpha = 1)
#'
setWatermark <- function(plotObject,
                         watermark = NULL,
                         color = NULL,
                         size = NULL,
                         angle = NULL,
                         alpha = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(watermark, c("character", "Label"), nullAllowed = TRUE)
  validateIsString(color, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)
  validateIsNumeric(alpha, nullAllowed = TRUE)
  validateIsNumeric(angle, nullAllowed = TRUE)

  alpha <- alpha %||% .getAestheticValues(
    n = 1,
    selectionKey = AestheticSelectionKeys$first,
    aesthetic = "alpha"
  )

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$background$watermark
  watermarkConfiguration <- newPlotObject$plotConfiguration$background$watermark

  if (isOfType(watermark, "character")) {
    watermark <- asLabel(text = watermark, font = watermarkConfiguration$font)
  }
  watermarkConfiguration <- watermark %||% watermarkConfiguration
  eval(.parseVariableToObject("watermarkConfiguration$font", c("color", "size", "angle"), keepIfNull = TRUE))

  # If plot is initialized, addWatermark otherwise update watermark
  if (isOfLength(plotObject$layers, 0)) {
    newPlotObject <- addWatermark(newPlotObject,
      watermark = watermarkConfiguration,
      alpha = alpha
    )
    return(newPlotObject)
  }

  # Using initializePlot, watermark will always be as first layer
  dummyPlot <- addWatermark(ggplot2::ggplot(),
    watermark = watermarkConfiguration,
    alpha = alpha
  )
  newPlotObject$layers[[1]] <- dummyPlot$layers[[1]]
  return(newPlotObject)
}

#' @title createWatermarkGrob
#' @description
#' Creates a `ggplot` grob based on the label text and its font properties.
#' @param label A character value or `Label` object
#' @param alpha Numeric value between 0 and 1 corresponding to transparency of the watermark
#' The closer to 0, the more transparent the watermark is.
#' The closer to 1, the more opaque the watermark is.
#' @return A `ggplot` grob
#' @export
createWatermarkGrob <- function(label, alpha = NULL) {
  validateIsNumeric(alpha, nullAllowed = TRUE)
  # Ensure label is a Label class
  label <- asLabel(label)
  alpha <- alpha %||% .getAestheticValues(
    n = 1,
    selectionKey = AestheticSelectionKeys$first,
    aesthetic = "alpha"
  )

  # Font size is in .pt within annonate and requires a conversion
  watermark <- ggplot2::ggplot() +
    ggplot2::theme_void() +
    ggplot2::annotate(
      geom = "text",
      x = 0,
      y = 0,
      label = label$text %||% "",
      color = label$font$color,
      fontface = label$font$fontFace,
      size = label$font$size / ggplot2::.pt,
      angle = label$font$angle, alpha = alpha
    )
  watermark <- ggplot2::ggplotGrob(watermark)

  return(watermark)
}
