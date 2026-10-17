#' @note helpers.R
#' Bank of helper functions to source that make the code of the shiny app lighter and easier

#' @title listOfAvailablePlots
#' @description list of available atom and molecule plots available in tlf
listOfAvailablePlots <- c(
  "initializePlot",
  "addScatter",
  "addLine",
  "addRibbon",
  "addErrorbar",
  "plotPKRatio",
  "plotDDIRatio",
  "plotBoxWhisker",
  "plotHistogram",
  "plotTimeProfile",
  "plotTornado",
  "plotObsVsPred",
  "plotResVsPred"
)

#' @title parseFieldToObject
#' @description Create expressions to evaluate of type: `objectName$fieldName <- fieldName`
#' or `objectName$fieldName <- fieldName %||% objectName$fieldName` depending on option ``
#' @param fieldName Names of the field/variable to associate to the object `objectName`
#' @param objectName Name of the object on which fields `fieldName` are associated
#' @param nullCondition logical to use object field in case of null condition
#' @return Expressions to eval
parseFieldToObject <- function(fieldName, objectName, nullCondition = FALSE) {
  if (nullCondition) {
    parse(text = paste0(objectName, "$", fieldName, " <- ", fieldName, " %||% ", objectName, "$", fieldName))
  }
  parse(text = paste0(objectName, "$", fieldName, " <- ", fieldName))
}

#' @title parseFieldFromObject
#' @description Create expressions `fieldName <- objectName$fieldName` to evaluate
#' @param fieldName Names of the field/variable to associate to the object `object`
#' @param objectName Name of the object on which fields `fieldName` are associated
#' @return Expressions to eval
parseFieldFromObject <- function(fieldName, objectName) {
  parse(text = paste0(fieldName, " <- ", objectName, "$", fieldName))
}

#' @title %||%
#' @description get argument if not null, otherwise get other argument
`%||%` <- function(lhs, rhs) {
  if (!is.null(lhs)) {
    lhs
  } else {
    rhs
  }
}
#' @title isOfLength
#' @description check if input has a certain length
isOfLength <- function(object, nbElements) {
  return(length(object) == nbElements)
}
#' @title isOfType
#' @description check if input has a certain class
isOfType <- function(object, type) {
  if (is.null(object)) {
    return(FALSE)
  }
  any(class(object) %in% type)
}
#' @title isIncluded
#' @description check if input is included in a list
isIncluded <- function(values, parentValues) {
  if (is.null(values)) {
    return(FALSE)
  }
  if (length(values) == 0) {
    return(FALSE)
  }
  return(as.logical(min(values %in% parentValues)))
}

#' @title tlfInput
#' @description translate shiny input into tlf argument
#' replacing "none", "" and NA by NULL
tlfInput <- function(value) {
  if (isIncluded(value, c("none", ""))) {
    return()
  }
  if (isIncluded(value, NA)) {
    return()
  }
  return(value)
}

#' @title labelPanel
#' @description centralize all UIs for labels
labelPanel <- function(displayName, labelID = tolower(displayName), includeText = TRUE) {
  if (includeText) {
    return(tabPanel(
      displayName,
      uiOutput(paste0(labelID, "Text")),
      uiOutput(paste0(labelID, "Color")),
      uiOutput(paste0(labelID, "Size")),
      uiOutput(paste0(labelID, "Angle")),
      uiOutput(paste0(labelID, "Align")),
      uiOutput(paste0(labelID, "Face")),
      uiOutput(paste0(labelID, "Family"))
    ))
  }
  tabPanel(
    displayName,
    uiOutput(paste0(labelID, "Color")),
    uiOutput(paste0(labelID, "Size")),
    uiOutput(paste0(labelID, "Angle")),
    uiOutput(paste0(labelID, "Align")),
    uiOutput(paste0(labelID, "Face")),
    uiOutput(paste0(labelID, "Family"))
  )
}

#' @title backgroundPanel
#' @description centralize all UIs for background elements
backgroundPanel <- function(displayName, backgroundID = tolower(displayName), includeFill = TRUE) {
  if (includeFill) {
    return(tabPanel(
      displayName,
      uiOutput(paste0(backgroundID, "Fill")),
      uiOutput(paste0(backgroundID, "Color")),
      uiOutput(paste0(backgroundID, "Size")),
      uiOutput(paste0(backgroundID, "Linetype"))
    ))
  }
  tabPanel(
    displayName,
    uiOutput(paste0(backgroundID, "Color")),
    uiOutput(paste0(backgroundID, "Size")),
    uiOutput(paste0(backgroundID, "Linetype"))
  )
}
