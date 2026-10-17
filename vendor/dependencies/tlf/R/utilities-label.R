#' @title setPlotLabels
#' @description Set labels properties on a ggplot object
#' @param plotObject A `ggplot` object
#' @param title A character value or `Label` object
#' @param subtitle A character value or `Label` object
#' @param xlabel A character value or `Label` object
#' @param ylabel A character value or `Label` object
#' @param caption A character value or `Label` object
#' @return A `ggplot` object
#' @export
#' @examples
#' # Set labels of a scatter plot
#' p <- addScatter(x = c(1, 2, 1, 2, 3), y = c(5, 0, 2, 3, 4))
#'
#' setPlotLabels(p, xlabel = "new x label", ylabel = "new y label")
#'
#' # Set labels using Label object
#' setPlotLabels(p, ylabel = Label$new(text = "red y label", color = "red"))
#'
setPlotLabels <- function(plotObject,
                          title = NULL,
                          subtitle = NULL,
                          xlabel = NULL,
                          ylabel = NULL,
                          caption = NULL) {
  validateIsOfType(plotObject, "ggplot")
  # Inputs will undergo the same code, so parse/eval
  # parse/eval of inputs prevent copy paste of code
  inputs <- c("title", "subtitle", "xlabel", "ylabel", "caption")
  validateExpressions <- parse(text = paste0("validateIsOfType(", inputs, ', c("Label", "character"), nullAllowed =TRUE)'))
  eval(validateExpressions)

  # Clone plotConfiguration into a new plot object
  # Prevents update of R6 class being spread to plotObject
  newPlotObject <- plotObject
  newPlotObject$plotConfiguration <- plotObject$plotConfiguration$clone(deep = TRUE)

  # R6 class not cloned will spread modifications into newPlotObject$plotConfiguration$yAxis
  labels <- newPlotObject$plotConfiguration$labels
  char2LabExpressions <- parse(text = paste0(
    "if(isOfType(", inputs, ', "character")){',
    inputs, " <- asLabel(", inputs, ", font = labels$", inputs, "$font)}"
  ))
  eval(char2LabExpressions)
  eval(.parseVariableToObject("labels", inputs, keepIfNull = TRUE))
  newPlotObject <- labels$updatePlot(newPlotObject)
  return(newPlotObject)
}

#' @title asLabel
#' @param text A character value
#' @param font A `Font` object defining the font properties of the Label
#' @return A `Label` object
#' @description
#' Set a character string into a `Label` object associating font properties to input
#' If text is already a `Label` object, `asLabel` can be used to update its font properties
#' @export
#' @examples
#' title <- "Title of Plot"
#' title <- asLabel(title)
asLabel <- function(text = "", font = NULL) {
  # Check input
  validateIsOfType(text, c("Label", "character"), nullAllowed = TRUE)
  validateIsOfType(font, Font, nullAllowed = TRUE)

  # Ensure Label class
  if (!isOfType(text, "Label")) {
    text <- Label$new(text)
  }

  # If font is not null, set font properties of Label
  text$font <- font %||% text$font

  return(text)
}

#' @title getLabelWithUnit
#' @description
#' Get label with its unit within square brackets when available
#' @param label text of axis label
#' @param unit Character value corresponding to unit of `label`
#' @return
#' `label [unit]` or `label` depending if `unit` is `NULL` or `""`
#' @export
#' @examples
#' getLabelWithUnit("Time", "min")
#'
#' getLabelWithUnit("Label without unit")
#'
getLabelWithUnit <- function(label, unit = NULL) {
  if (is.null(unit) || is.na(unit) || unit == "") {
    return(label)
  } else {
    return(paste0(label, " [", unit, "]"))
  }
}

#' Sanitize Label Text
#' @description
#' ggtext does not allow certain characters that can be converted to html
#' tags but that are not supported. This function removes this forbidden
#' characters.
#'
#' @param text a character string
#'
#' @return a sanitized character string
#'
.sanitizeLabel <- function(text) {
  forbiddenCharacters <- c("`")
  if (isOfType(text, "character", nullAllowed = FALSE)) {
    text <- stringr::str_remove_all(text, paste(forbiddenCharacters, sep = "|"))
  }
  return(text)
}
