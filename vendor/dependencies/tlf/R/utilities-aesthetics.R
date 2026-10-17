.asPlotShape <- function(shapes) {
  ggplotShapes <- NULL
  for (shape in shapes) {
    ggplotShape <- as.character(shape)
    if (isIncluded(ggplotShape, names(Shapes))) {
      ggplotShape <- Shapes[[shape]]
    }
    ggplotShapes <- c(ggplotShapes, ggplotShape)
  }
  return(ggplotShapes)
}

#' @title .getAestheticValues
#' @description Get aesthetic values (e.g color, shape, linetype) based on a selected strategy
#' @param n integer defining size of returned aesthetic vector
#' @param selectionKey value of aesthetic to be returned or key function from enum `AestheticSelectionKeys`
#' @param position integer defining the current position in the aesthetic map
#' @param aesthetic name of aesthetic property as defined in enum `AestheticProperties`
#' @return Vector of aesthetics
#' @keywords internal
.getAestheticValues <- function(n, selectionKey = NA, position = 0, aesthetic = "color") {
  validateIsIncluded(aesthetic, AestheticProperties)
  # Load aesthetics from current `Theme` object
  map <- tlfEnv$currentTheme$aestheticMaps[[aesthetic]]
  if (isIncluded(aesthetic, AestheticProperties$shape)) {
    map <- .asPlotShape(map)
  }
  # next is necessary as `next` otherwise R crashes
  if (isIncluded(selectionKey, AestheticSelectionKeys$`next`)) {
    return(.getNextAestheticValues(n, position, map))
  }
  if (isIncluded(selectionKey, AestheticSelectionKeys$same)) {
    return(.getSameAestheticValues(n, position, map))
  }
  if (isIncluded(selectionKey, AestheticSelectionKeys$first)) {
    return(.getFirstAestheticValues(n, map))
  }
  if (isIncluded(selectionKey, AestheticSelectionKeys$reset)) {
    return(.getResetAestheticvalues(n, map))
  }
  # TO DO: Add wrappers for validation
  # which could translate some selection keys for ggplot 2
  if (isIncluded(aesthetic, AestheticProperties$shape)) {
    return(.asPlotShape(.getNextAestheticValues(n, position = position, map = selectionKey)))
  }
  if (isIncluded(aesthetic, c(AestheticProperties$size, AestheticProperties$alpha))) {
    return(as.numeric(.getNextAestheticValues(n, position = position, map = selectionKey)))
  }
  # If selection key is a specific value/set of values, it becomes the map
  return(.getNextAestheticValues(n, position = position, map = selectionKey))
}

#' @title .getNextAestheticValues
#' @description Get the next aesthetic values (e.g color, shape, linetype) from an aesthetic map defined in `Theme`
#' @param n integer defining size of returned aesthetic vector
#' @param position integer defining at which position to look for aesthetic vector
#' @param map Aesthetic (e.g. color, shape, linetype) map from `Theme` object.
#' @return Vector of aesthetics
#' @keywords internal
.getNextAestheticValues <- function(n, position = 0, map) {
  # Get the map indices of aesthetic values to be output
  aesPositions <- seq(position + 1, position + n)
  if (n <= 0) {
    return(NULL)
  }

  # In case the map is not long enough to get all the indices
  # the indices have to go start back from 1 using modelu function
  mapSize <- length(map)
  aesPositions <- ((aesPositions - 1) %% mapSize) + 1
  return(map[aesPositions])
}

#' @title .getSameAestheticValues
#' @description Get the same aesthetic values (e.g color, shape, linetype) from an aesthetic map defined in `Theme`
#' @param n integer defining size of returned aesthetic vector
#' @param position integer defining at which position to look for aesthetic vector
#' @param map Aesthetic (e.g. color, shape, linetype) map from `Theme` object.
#' @return Vector of aesthetics
#' @keywords internal
.getSameAestheticValues <- function(n, position = 0, map) {
  # In case the map is not long enough to get the indices in position
  # the indices have to go start back from 1 using modelu function
  mapSize <- length(map)
  aesPosition <- (position %% mapSize) + 1
  if (n <= 0) {
    return(NULL)
  }

  # Get the map indices of aesthetic values to be output
  aesPositions <- rep(aesPosition, n)
  return(map[aesPositions])
}

#' @title .getResetAestheticvalues
#' @description Get the aesthetic values (e.g color, shape, linetype) from an aesthetic map defined in `Theme`.
#' Reset the value every time it is used.
#' @param n integer defining size of returned aesthetic vector
#' @param map Aesthetic (e.g. color, shape, linetype) map from `Theme` object.
#' @return Vector of aesthetics
#' @keywords internal
.getResetAestheticvalues <- function(n, map) {
  return(.getNextAestheticValues(n, position = 0, map))
}

#' @title .getFirstAestheticValues
#' @description Get the next aesthetic values (e.g color, shape, linetype) from an aesthetic map defined in `Theme`
#' @param n integer defining size of returned aesthetic vector
#' @param position integer defining at which position to look for aesthetic vector
#' @param map Aesthetic (e.g. color, shape, linetype) map from `Theme` object.
#' @return Vector of aesthetics
#' @keywords internal
.getFirstAestheticValues <- function(n, map) {
  return(.getSameAestheticValues(n, position = 0, map))
}


#' @title .getAesPropertyColumnNameFromLabels
#' @description Get the column names of the variables mapped to aesthetic properties
#' @param mapLabels List of mapped label names passed to `ggplot2::aes`
#' @param propertyNames Names of aesthetic property (e.g. `"color"`, `"shape"`...)
#' @return A list of variable names
#' @keywords internal
.getAesPropertyColumnNameFromLabels <- function(mapLabels, propertyNames) {
  variableNames <- lapply(
    propertyNames,
    function(propertyName) {
      # For aes_string to work with special characters, column names are wrapped with "`"
      return(gsub("`", "", mapLabels[[propertyName]]))
    }
  )
  names(variableNames) <- propertyNames
  return(variableNames)
}

#' @title .getAesPropertyLengthFromLabels
#' @description Get the names of the variables mapped to aesthetic properties
#' @param data A data.frame with labels mapped to properties and obtained from a `DataMapping` object
#' @param columnNames List of mapped column names of `data` obtained
#' @param propertyNames Names of aesthetic property (e.g. `"color"`, `"shape"`...)
#' @return A list of variable names
#' @keywords internal
.getAesPropertyLengthFromLabels <- function(data, columnNames, propertyNames) {
  variableLengths <- lapply(
    propertyNames,
    function(propertyName) {
      return(length(unique(data[, columnNames[[propertyName]]])))
    }
  )
  names(variableLengths) <- propertyNames
  return(variableLengths)
}

#' @title .getColorNamesForFirstAesValues
#' @description Get the first found value of map color for another map property
#' @param data A data.frame with labels mapped to properties and obtained from a `DataMapping` object
#' @param columnNames List of mapped column names of `data` obtained
#' @param propertyName Name of aesthetic property (e.g. `"shape"`...)
#' @return Selected levels of `data[,columnNames$color]`
#' @keywords internal
.getColorNamesForFirstAesValues <- function(data, columnNames, propertyName) {
  colorNames <- sapply(
    levels(data[, columnNames[[propertyName]]]),
    function(propertyLevel) {
      head(data[which(data[, columnNames[[propertyName]]] %in% propertyLevel), columnNames$color], 1)
    }
  )
  return(colorNames)
}


#' @title .updateAesProperties
#' @description Updates the aesthetic properties of `plotObject`
#' @param plotObject A `ggplot` object
#' @param plotConfigurationProperty `PlotConfiguration` property name included in .
#' `"points"`, `"lines"`, `"ribbons"` or `"errorbars"`
#' @param propertyNames Names of aesthetic property (e.g. `"color"`, `"shape"`...)
#' @param data A data.frame with labels mapped to properties and obtained from a `DataMapping` object
#' @param mapLabels List of mapped label names passed to `ggplot2::aes`
#' @return A `ggplot` object
#' @keywords internal
#' @import ggplot2
.updateAesProperties <- function(plotObject, plotConfigurationProperty, propertyNames, data, mapLabels) {
  propertyVariables <- .getAesPropertyColumnNameFromLabels(mapLabels, propertyNames)
  propertyLengths <- .getAesPropertyLengthFromLabels(data, propertyVariables, propertyNames)
  for (propertyName in propertyNames) {
    # For match.fun to work, ggplot2 namespace is required
    ggplotScaleFunction <- match.fun(paste0("scale_", propertyName, "_manual"))
    # Use suppress messages to remove warning of overwriting property scale
    suppressMessages(
      plotObject <- plotObject +
        ggplotScaleFunction(
          values = .getAestheticValues(
            n = propertyLengths[[propertyName]],
            selectionKey = plotObject$plotConfiguration[[plotConfigurationProperty]][[propertyName]],
            aesthetic = propertyName
          )
        )
    )
    # remove the legend of aesthetic if default unmapped aesthetic
    if (isIncluded(propertyVariables[[propertyName]], "legendLabels")) {
      # Dynamic code is needed here for selecting the correct input parameter of function guides
      eval(parse(text = paste0(
        "plotObject <- plotObject + ggplot2::guides(", propertyName, " = 'none')"
      )))
    }
  }
  return(plotObject)
}

#' @title .getAestheticValuesFromConfiguration
#' @description Get list of values for requested aesthetic property
#' @param n integer defining size of returned aesthetic vector
#' @param position integer defining the current position in the aesthetic map
#' @param plotObject A `ggplot` object
#' @param plotConfigurationProperty `PlotConfiguration` property name included in .
#' `"points"`, `"lines"`, `"ribbons"` or `"errorbars"`
#' @param propertyNames Names of aesthetic property (e.g. `"color"`, `"shape"`...)
#' @return A list of values for requested aesthetic property
#' @keywords internal
.getAestheticValuesFromConfiguration <- function(n = 1, position = 0, plotConfigurationProperty, propertyNames) {
  aestheticValues <- lapply(
    propertyNames,
    function(propertyName) {
      .getAestheticValues(n = n, selectionKey = plotConfigurationProperty[[propertyName]], position = position, aesthetic = propertyName)
    }
  )
  names(aestheticValues) <- propertyNames
  return(aestheticValues)
}
