#' @title setLegend
#' @param plotObject Graphical object created from ggplot
#' @param position legend position.
#' Use enum `LegendPositions` to access the list of legend positions.
#' @param title character or `Label` object
#' @param font `Font` object defining legend font
#' @param caption data.frame containing the caption properties of the legend
#' @return A `ggplot` object
#' @description
#' Set legend position, title, font and/or caption
#' @export
setLegend <- function(plotObject,
                      position = NULL,
                      title = NULL,
                      font = NULL,
                      caption = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(position, LegendPositions, nullAllowed = TRUE)
  validateIsOfType(title, c("character", "Label"), nullAllowed = TRUE)
  validateIsOfType(font, c("Font"), nullAllowed = TRUE)
  validateIsOfType(caption, "data.frame", nullAllowed = TRUE)

  plotObject <- setLegendPosition(plotObject, position = position)
  plotObject <- setLegendTitle(plotObject, title = title)
  plotObject <- setLegendFont(plotObject, color = font$color, size = font$size, angle = font$angle, fontFace = font$fontFace)
  plotObject <- setLegendCaption(plotObject, caption = caption)
  return(plotObject)
}

#' @title setLegendFont
#' @param plotObject ggplot object
#' @param size numeric defining the size of legend font
#' @param color character defining the color of legend font
#' @param fontFamily character defining the family of legend font
#' @param fontFace character defining the legend font face as defined in helper enum `FontFaces`.
#' @param angle numeric defining the angle of legend font
#' @param align character defining the alignment of legend font as defined in helper enum `Alignments`.
#' @return A ggplot object
#' @description Set legend font properties
#' @export
setLegendFont <- function(plotObject,
                          color = NULL,
                          size = NULL,
                          fontFamily = NULL,
                          fontFace = NULL,
                          angle = NULL,
                          align = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsString(color, nullAllowed = TRUE)
  validateIsString(fontFamily, nullAllowed = TRUE)
  validateIsNumeric(size, nullAllowed = TRUE)
  validateIsNumeric(angle, nullAllowed = TRUE)
  validateIsIncluded(fontFace, FontFaces, nullAllowed = TRUE)
  validateIsIncluded(align, Alignments, nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration
  legend <- newPlotObject$plotConfiguration$legend
  eval(.parseVariableToObject("legend$font", c("size", "color", "fontFace", "fontFamily", "angle", "align"), keepIfNull = TRUE))
  newPlotObject <- legend$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setLegendTitle
#' @param plotObject ggplot object
#' @param title character or `Label` object
#' @return A ggplot object
#' @description Set legend title
#' @export
setLegendTitle <- function(plotObject,
                           title = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(title, c("character", "Label"), nullAllowed = TRUE)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)
  legend <- newPlotObject$plotConfiguration$legend
  if (isOfType(title, "character")) {
    title <- asLabel(title, font = legend$title$font)
  }
  legend$title <- title %||% legend$title
  newPlotObject <- legend$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title setLegendPosition
#' @description Set the legend position
#' @param plotObject `ggplot` graphical object
#' @param position legend position as defined in helper enum `LegendPositions`
#' @return A `ggplot` graphical object
#' @seealso LegendPositions
#' @export
#' @import ggplot2
setLegendPosition <- function(plotObject,
                              position = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsIncluded(position, LegendPositions, nullAllowed = TRUE)

  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  legend <- newPlotObject$plotConfiguration$legend
  legend$position <- position %||% legend$position

  newPlotObject <- legend$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title getLegendPosition
#' @description Get the legend position
#' @param plotObject `ggplot` graphical object
#' @return Position of legend as defined in enum `LegendPositions`
#' @seealso LegendPositions
#' @export
getLegendPosition <- function(plotObject) {
  validateIsOfType(plotObject, "ggplot")
  return(plotObject$plotConfiguration$legend$position)
}

#' @title setLegendCaption
#' @param plotObject `ggplot` graphical object
#' @param caption data.frame containing the caption properties of the legend
#' @return A `ggplot` graphical object
#' @description
#' Set the legend caption
#' @export
#' @import ggplot2
#' @import utils
setLegendCaption <- function(plotObject, caption = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(caption, "data.frame", nullAllowed = TRUE)

  # Empty or null captions
  if (isOfLength(caption, 0)) {
    return(plotObject)
  }

  validateIsIncluded(names(caption), CaptionProperties)

  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  legend <- newPlotObject$plotConfiguration$legend
  legend$caption <- caption

  # Re-order based on order variable
  orderedName <- legend$caption$name[legend$caption$order]
  orderedLabel <- legend$caption$label[legend$caption$order]
  orderedVisibility <- legend$caption$visibility[legend$caption$order]

  # The next lines need to be looped on every element of LegendTypes
  # Otherwise, there can be a legend per LegendTypes
  for (aestype in LegendTypes) {
    # breaks and values are remapped by alphabetical order on ggplot2 versions <3.3.0
    # For these versions, the order is reverse engineered
    aesValues <- c(
      legend$caption[legend$caption$order[orderedVisibility], aestype],
      legend$caption[legend$caption$order[!orderedVisibility], aestype]
    )
    if (utils::packageVersion("ggplot2") < "3.3.0") {
      aesValues <- legend$caption[order(legend$caption$name), aestype]
    }
    # scale_discrete_manual sends warning every time it overwrite something
    # so this line need to be silent
    suppressMessages(
      newPlotObject <- newPlotObject +
        ggplot2::scale_discrete_manual(
          aesthetics = aestype,
          name = legend$title,
          breaks = orderedName[orderedVisibility],
          labels = orderedLabel[orderedVisibility],
          values = aesValues
        )
    )
  }
  return(newPlotObject)
}

#' @title getLegendCaption
#' @param plotObject `ggplot` graphical object
#' @return A data.frame corresponding to legend caption
#' @description
#' Get the legend caption
#' @export
#' @import ggplot2
getLegendCaption <- function(plotObject) {
  validateIsOfType(plotObject, "ggplot")

  # Clone the properties object to prevent spreading of modifications
  # to the plotObject itself
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  return(newPlotObject$plotConfiguration$legend$caption)
}

#' @title setCaptionLabel
#' @param plotObject `ggplot` graphical object
#' @param label new labels of the legend caption
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the legend caption labels
#' @export
#' @import ggplot2
setCaptionLabels <- function(plotObject, label, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(label, c("character", "numeric"))

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  labelExpression <- .getCoreSetCaptionProperty("label")
  eval(labelExpression)

  return(newPlotObject)
}

#' @title setCaptionVisibility
#' @param plotObject `ggplot` graphical object
#' @param visibility logical visibility of the legend caption labels
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the visibility of the legend caption labels
#' @export
#' @import ggplot2
setCaptionVisibility <- function(plotObject, visibility, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsLogical(visibility)

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  visibilityExpression <- .getCoreSetCaptionProperty("visibility")
  eval(visibilityExpression)

  return(newPlotObject)
}

#' @title setCaptionOrder
#' @param plotObject `ggplot` graphical object
#' @param order numeric order of the legend caption labels
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the order of the legend caption labels
#' @export
#' @import ggplot2
setCaptionOrder <- function(plotObject, order, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(order, "numeric")

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  orderExpression <- .getCoreSetCaptionProperty("order")
  eval(orderExpression)

  return(newPlotObject)
}

#' @title setCaptionColor
#' @param plotObject `ggplot` graphical object
#' @param color colors of the data in plot and legend caption
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the colors of the data in plot and legend caption
#' @export
#' @import ggplot2
setCaptionColor <- function(plotObject, color, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(color, c("character", "numeric"))

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  colorExpression <- .getCoreSetCaptionProperty("color")
  eval(colorExpression)

  return(newPlotObject)
}

#' @title setCaptionShape
#' @param plotObject `ggplot` graphical object
#' @param shape shapes of the data in plot and legend caption
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the shapes of the data in plot and legend caption
#' @export
#' @import ggplot2
setCaptionShape <- function(plotObject, shape, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(shape, c("character", "numeric"))

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  shapeExpression <- .getCoreSetCaptionProperty("shape")
  eval(shapeExpression)

  return(newPlotObject)
}

#' @title setCaptionSize
#' @param plotObject `ggplot` graphical object
#' @param size sizes of the data in plot and legend caption
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the sizes of the data in plot and legend caption
#' @export
#' @import ggplot2
setCaptionSize <- function(plotObject, size, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(size, c("character", "numeric"))

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  sizeExpression <- .getCoreSetCaptionProperty("size")
  eval(sizeExpression)

  return(newPlotObject)
}

#' @title setCaptionLinetype
#' @param plotObject `ggplot` graphical object
#' @param linetype linetypes of the data in plot and legend caption
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the linetypes of the data in plot and legend caption
#' @export
#' @import ggplot2
setCaptionLinetype <- function(plotObject, linetype, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(linetype, c("character", "numeric"))

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  linetypeExpression <- .getCoreSetCaptionProperty("linetype")
  eval(linetypeExpression)

  return(newPlotObject)
}

#' @title setCaptionFill
#' @param plotObject `ggplot` graphical object
#' @param fill fills of the data in plot and legend caption
#' @param name reference names in caption$name identifying the legend caption lines
#' @return A `ggplot` graphical object
#' @description
#' Set the fills of the data in plot and legend caption
#' @export
#' @import ggplot2
setCaptionFill <- function(plotObject, fill, name = NULL) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(fill, c("character", "numeric"))

  # .getCoreSetCaptionProperty is common for all the caption properties
  # prevent duplication of code everywhere
  fillExpression <- .getCoreSetCaptionProperty("fill")
  eval(fillExpression)

  return(newPlotObject)
}

# LegendPositions needed to be defined before to tlfEnv$defaultLegendPosition
# It was consequently moved from utilities-legend to tlf-env

#' @title LegendTypes
#' @import ospsuite.utils
#' @export
#' @description
#' List of all available legend types
#' @family enum helpers
LegendTypes <- enum(c(
  "color",
  "shape",
  "linetype",
  "fill",
  "size"
))

#' @title CaptionProperties
#' @import ospsuite.utils
#' @export
#' @description
#' List of all Caption Properties
#' @keywords internal
CaptionProperties <- enum(c(
  "name",
  "label",
  "visibility",
  "order",
  "color",
  "shape",
  "linetype",
  "size",
  "fill"
))

#' @title .createPlotLegendPosition
#' @description Create legend position directly usable by `ggplot2`
#' @param position Position of legend as defined in enum `LegendPositions`
#' @return A list with fields `xPosition`, `xJustification`, `yPosition`, `yJustification`
#' @keywords internal
.createPlotLegendPosition <- function(position) {
  validateIsIncluded(position, LegendPositions)

  listOfLegendPositions <- list(
    none = list(xPosition = "none", xJustification = NULL, yPosition = NULL, yJustification = NULL),
    insideTop = list(xPosition = 0.5, xJustification = 0.5, yPosition = 0.975, yJustification = 1),
    insideTopLeft = list(xPosition = 0.025, xJustification = 0, yPosition = 0.975, yJustification = 1),
    insideLeft = list(xPosition = 0.025, xJustification = 0, yPosition = 0.5, yJustification = 0.5),
    insideBottomLeft = list(xPosition = 0.025, xJustification = 0, yPosition = 0.025, yJustification = 0),
    insideBottom = list(xPosition = 0.5, xJustification = 0.5, yPosition = 0.025, yJustification = 0),
    insideBottomRight = list(xPosition = 0.975, xJustification = 1, yPosition = 0.025, yJustification = 0),
    insideRight = list(xPosition = 0.975, xJustification = 1, yPosition = 0.5, yJustification = 0.5),
    insideTopRight = list(xPosition = 0.975, xJustification = 1, yPosition = 0.975, yJustification = 1),
    outsideTop = list(xPosition = NULL, xJustification = 0.5, yPosition = "top", yJustification = NULL),
    outsideTopLeft = list(xPosition = NULL, xJustification = 0, yPosition = "top", yJustification = 0.5),
    outsideLeft = list(xPosition = "left", xJustification = NULL, yPosition = NULL, yJustification = NULL),
    outsideBottomLeft = list(xPosition = NULL, xJustification = 0, yPosition = "bottom", yJustification = 0.5),
    outsideBottom = list(xPosition = NULL, xJustification = 0.5, yPosition = "bottom", yJustification = NULL),
    outsideBottomRight = list(xPosition = NULL, xJustification = 1, yPosition = "bottom", yJustification = 0.5),
    outsideRight = list(xPosition = "right", xJustification = NULL, yPosition = NULL, yJustification = NULL),
    outsideTopRight = list(xPosition = NULL, xJustification = 1, yPosition = "top", yJustification = 0.5)
  )

  legendPosition <- listOfLegendPositions[[position]]
  return(legendPosition)
}

#' @title .mergeLegend
#' @description merge legend caption with existing legend caption
#' @param plotObject ggplot object
#' @param newLabels labels of caption to merge
#' @param aestheticSelections `ThemeAestheticSelections` object
#' @keywords internal
.mergeLegend <- function(plotObject, newLabels, aestheticSelections) {
  validateIsOfType(plotObject, "ggplot")
  validateIsOfType(aestheticSelections, "ThemeAestheticSelections")

  oldCaption <- plotObject$plotConfiguration$legend$caption
  oldCaptionLength <- nrow(oldCaption)
  newCaptionLength <- length(newLabels)

  # Associate new values of aesthetics based on theme aesthetic selections
  newCaptionExpression <- parse(text = paste0(
    names(AestheticProperties), " <- .getAestheticValues(n = newCaptionLength,
    selectionKey = aestheticSelections$", names(AestheticProperties), ",
    position = oldCaptionLength,
    aesthetic = '", names(AestheticProperties), "')"
  ))
  eval(newCaptionExpression)

  # Create the new caption
  newCaption <- data.frame(
    name = newLabels, label = newLabels,
    visibility = rep(TRUE, newCaptionLength),
    order = seq(oldCaptionLength + 1, oldCaptionLength + newCaptionLength),
    color = color, shape = shape, size = size, linetype = linetype, fill = fill,
    stringsAsFactors = FALSE
  )

  if (oldCaptionLength == 0) {
    plotObject <- setLegend(plotObject, caption = newCaption)
    return(plotObject)
  }

  # Resolve conflicts between old and new legend caption for merging
  # Fill must remain as defined by old (prevent overwriting addRibbon)
  for (oldCaptionName in oldCaption$name) {
    oldSelectedCaptions <- oldCaption$name %in% oldCaptionName
    newSelectedCaptions <- newCaption$name %in% oldCaptionName
    newCaption$fill[newSelectedCaptions] <- oldCaption$fill[oldSelectedCaptions]
    newCaption$order[newSelectedCaptions] <- oldCaption$order[oldSelectedCaptions]
  }
  # Remove duplicate old captions for merging
  oldCaption <- oldCaption[!(oldCaption$name %in% newCaption$name), ]
  mergeCaption <- rbind.data.frame(oldCaption, newCaption, stringsAsFactors = FALSE)
  plotObject <- setLegend(plotObject, caption = mergeCaption)
  return(plotObject)
}

.getCoreSetCaptionProperty <- function(property) {
  c(
    parse(text = 'validateIsOfType(plotObject, "ggplot")'),
    parse(text = "newCaption <- getLegendCaption(plotObject)"),
    parse(text = paste0(property, "Indices <- seq(1, length(", property, "))")),
    parse(text = paste0(
      "if(!is.null(name)){",
      "validateIsIncluded(name, newCaption$name) \n",
      "validateIsSameLength(", property, ", name) \n",
      property, "Indices <- match(name, newCaption$name)}"
    )),
    parse(text = paste0("newCaption$", property, "[", property, "Indices] <- ", property, " \n")),
    parse(text = "newPlotObject <- setLegendCaption(plotObject, newCaption)")
  )
}


.getDefaultCaptionFor <- function(plotName) {
  as.data.frame(tlfEnv$currentTheme$defaultCaption[[plotName]], stringsAsFactors = FALSE)
}
