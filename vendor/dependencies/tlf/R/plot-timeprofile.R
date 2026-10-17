#' @title plotTimeProfile
#' @description
#' Producing Time Profile plots
#'
#' @inheritParams addScatter
#' @param dataMapping
#' A `TimeProfileDataMapping` object mapping `x`, `y`, `ymin`, `ymax` and aesthetic groups to their variable names of `data`.
#' @param observedData A data.frame to use for plot.
#' Unlike `data`, meant for simulated data, plotted as lines and ribbons;
#' `observedData` is plotted as scatter points and errorbars.
#' @param observedDataMapping
#' An `ObservedDataMapping` object mapping `x`, `y`, `ymin`, `ymax` and aesthetic groups to their variable names of `observedData`.
#' @param plotConfiguration
#' An optional `TimeProfilePlotConfiguration` object defining labels, grid, background and watermark.
#' @return A `ggplot` object
#'
#' @export
#' @family molecule plots
#' @examples
#' # Produce a Time profile plot with observed and simulated data
#' obsData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#' simTime <- seq(1, 10, 0.1)
#' simData <- data.frame(
#'   x = simTime,
#'   y = 10 * exp(-simTime),
#'   ymin = 8 * exp(-simTime),
#'   ymax = 12 * exp(-simTime)
#' )
#'
#' plotTimeProfile(
#'   data = simData,
#'   observedData = obsData,
#'   dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax"),
#'   observedDataMapping = ObservedDataMapping$new(x = "x", y = "y")
#' )
plotTimeProfile <- function(data = NULL,
                            metaData = NULL,
                            dataMapping = NULL,
                            observedData = NULL,
                            observedDataMapping = NULL,
                            plotConfiguration = NULL,
                            plotObject = NULL) {
  #----- Validation and formatting of input arguments -----
  if (all(isEmpty(data), isEmpty(observedData))) {
    stop("At least 'data' or 'observedData' is required.")
  }
  # If no observed data, plotTimeProfile = plotSimulatedTimeProfile
  if (isEmpty(observedData)) {
    return(plotSimulatedTimeProfile(
      data = data,
      metaData = metaData,
      dataMapping = dataMapping,
      plotConfiguration = plotConfiguration,
      plotObject = plotObject
    ))
  }
  # If no data, plotTimeProfile = plotObservedTimeProfile
  if (isEmpty(data)) {
    return(plotObservedTimeProfile(
      data = observedData,
      metaData = metaData,
      dataMapping = observedDataMapping,
      plotConfiguration = plotConfiguration,
      plotObject = plotObject
    ))
  }

  # In the sequel, both data and observed data are defined
  validateIsOfType(data, "data.frame")
  validateIsOfType(observedData, "data.frame")

  dataMapping <- .setDataMapping(dataMapping, TimeProfileDataMapping, data)
  observedDataMapping <- .setDataMapping(observedDataMapping, ObservedDataMapping, observedData)

  plotConfiguration <- .setPlotConfiguration(
    plotConfiguration, TimeProfilePlotConfiguration,
    data, metaData, dataMapping
  )

  plotObject <- .setPlotObject(plotObject, plotConfiguration)

  requireDualAxis <- any(
    dataMapping$requireDualAxis(data),
    observedDataMapping$requireDualAxis(observedData)
  )

  if (!requireDualAxis) {
    plotObject <- .plotTimeProfileCore(
      data = data,
      metaData = metaData,
      dataMapping = dataMapping,
      observedData = observedData,
      observedDataMapping = observedDataMapping,
      plotConfiguration = plotConfiguration,
      plotObject = plotObject
    )
    return(plotObject)
  }

  leftPlotObject <- .plotTimeProfileCore(
    data = dataMapping$getLeftAxis(data),
    metaData = metaData,
    dataMapping = dataMapping,
    observedData = observedDataMapping$getLeftAxis(observedData),
    observedDataMapping = observedDataMapping,
    plotConfiguration = plotConfiguration,
    plotObject = plotObject
  )
  rightPlotObject <- .plotTimeProfileCore(
    data = dataMapping$getRightAxis(data),
    metaData = metaData,
    dataMapping = dataMapping,
    observedData = observedDataMapping$getRightAxis(observedData),
    observedDataMapping = observedDataMapping,
    plotConfiguration = plotConfiguration,
    plotObject = plotObject
  )
  plotObject <- getDualAxisPlot(leftPlotObject, rightPlotObject)
  return(plotObject)
}


#' @title .plotTimeProfileCore
#' @description Producing Core of Time Profile plots
#' @inheritParams plotTimeProfile
#' @return A `ggplot` object
#' @keywords internal
.plotTimeProfileCore <- function(data = NULL,
                                 metaData = NULL,
                                 dataMapping = NULL,
                                 observedData = NULL,
                                 observedDataMapping = NULL,
                                 plotConfiguration = NULL,
                                 plotObject = NULL) {
  mapData <- dataMapping$checkMapData(data)
  mapLabels <- .getAesStringMapping(dataMapping)

  mapObservedData <- observedDataMapping$checkMapData(observedData)
  observedMapLabels <- .getAesStringMapping(observedDataMapping)

  #----- Build layers of molecule plot -----
  #--- Simulated data ---
  # 1- if available, add LLOQ line
  if (!isEmpty(mapObservedData$lloq)) {
    plotObject <- .addLLOQLayer(
      plotObject,
      data = mapObservedData,
      mapLabels = observedMapLabels,
      direction = plotConfiguration$lloqDirection
    )
  }

  # 2- If available, add ribbons for population time profiles
  if (!any(isEmpty(dataMapping$ymin), isEmpty(dataMapping$ymax))) {
    aestheticValues <- .getAestheticValuesFromConfiguration(
      n = 1,
      position = 0,
      plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
      propertyNames = c("alpha")
    )
    plotObject <- plotObject +
      ggplot2::geom_ribbon(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          ymin = .data[[mapLabels$ymin]],
          ymax = .data[[mapLabels$ymax]],
          fill = .data[[mapLabels$fill]],
          group = .data[[mapLabels$linetype]]
        ),
        alpha = aestheticValues$alpha,
        na.rm = TRUE,
        show.legend = TRUE
      )
  }
  # 3- If available, add simulated time profile
  if (!isEmpty(dataMapping$y)) {
    aestheticValues <- .getAestheticValuesFromConfiguration(
      n = 1,
      position = 0,
      plotConfigurationProperty = plotObject$plotConfiguration$lines,
      propertyNames = c("alpha", "size")
    )
    plotObject <- plotObject +
      ggplot2::geom_path(
        data = mapData,
        mapping = ggplot2::aes(
          x = .data[[mapLabels$x]],
          y = .data[[mapLabels$y]],
          color = .data[[mapLabels$color]],
          linetype = .data[[mapLabels$linetype]]
        ),
        linewidth = aestheticValues$size,
        alpha = aestheticValues$alpha,
        na.rm = TRUE,
        show.legend = TRUE,
      )
  }
  # 4- If available, add error bars
  if (!any(isEmpty(observedDataMapping$ymin), isEmpty(observedDataMapping$ymax))) {
    plotObject <- .addErrorbarLayer(
      plotObject,
      data = mapObservedData,
      mapLabels = observedMapLabels
    )
  }
  # 5 - Add observed scatter points
  plotObject <- .addScatterLayer(
    plotObject,
    data = mapObservedData,
    mapLabels = observedMapLabels
  )
  #----- Update properties using ggplot2::scale functions -----
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "ribbons",
    propertyNames = "fill",
    data = mapData,
    mapLabels = mapLabels
  )
  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "lines",
    propertyNames = "linetype",
    data = mapData,
    mapLabels = mapLabels
  )

  # Get aesthetic information of simulated data for creating a legend caption data.frame
  simColumnNames <- .getAesPropertyColumnNameFromLabels(
    mapLabels,
    c("color", "fill", "linetype")
  )
  simAesLengths <- .getAesPropertyLengthFromLabels(
    data = mapData,
    simColumnNames,
    c("color", "fill", "linetype")
  )

  simFillValues <- .getAestheticValuesFromConfiguration(
    n = simAesLengths$fill,
    plotConfigurationProperty = plotObject$plotConfiguration$ribbons,
    propertyNames = "fill"
  )$fill
  simLinetypeValues <- .getAestheticValuesFromConfiguration(
    n = simAesLengths$linetype,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = "linetype"
  )$linetype
  simColorValues <- .getAestheticValuesFromConfiguration(
    n = simAesLengths$color,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = "color"
  )$color

  plotObject <- .updateAesProperties(
    plotObject,
    plotConfigurationProperty = "points",
    propertyNames = "shape",
    data = mapObservedData,
    mapLabels = observedMapLabels
  )

  obsColumnNames <- .getAesPropertyColumnNameFromLabels(
    observedMapLabels,
    c("color", "shape")
  )
  obsAesLengths <- .getAesPropertyLengthFromLabels(
    data = mapObservedData,
    obsColumnNames,
    c("color", "shape")
  )

  obsShapeValues <- .getAestheticValuesFromConfiguration(
    n = obsAesLengths$shape,
    plotConfigurationProperty = plotObject$plotConfiguration$points,
    propertyNames = "shape"
  )$shape
  obsColorValues <- .getAestheticValuesFromConfiguration(
    n = obsAesLengths$color,
    plotConfigurationProperty = plotObject$plotConfiguration$points,
    propertyNames = "color"
  )$color


  # The final color vector needs a length of totalLength to prevent scale_color_manual to crash
  colorBreaks <- levels(as.factor(c(mapData[, simColumnNames$color], mapObservedData[, obsColumnNames$color])))
  totalColorLength <- length(colorBreaks)

  # colorValues are selected colors for simulated (and shared observed data) and then colors for remaining observed data
  # the function ".getAestheticValues" selects these values as defined in the plotConfiguration object
  finalColorValues <- c(
    .getAestheticValues(
      n = simAesLengths$color,
      selectionKey = plotConfiguration$lines$color,
      aesthetic = "color"
    ),
    # By using position = simAesLengths$color,
    # the function will start selecting the colors that come after the colors selected for lines
    # this aims at preventing a reset of the colors and a need for manual update of the user
    .getAestheticValues(
      n = totalColorLength - simAesLengths$color,
      selectionKey = plotConfiguration$points$color,
      position = simAesLengths$color,
      aesthetic = "color"
    )
  )

  # Export the legend captions so the user can update legend keys order
  plotObject$plotConfiguration$legend$caption <- list(
    color = data.frame(
      names = colorBreaks,
      labels = colorBreaks,
      values = finalColorValues,
      stringsAsFactors = FALSE
    ),
    fill = data.frame(
      names = levels(as.factor(mapData[, simColumnNames$fill])),
      labels = levels(as.factor(mapData[, simColumnNames$fill])),
      colorNames = .getColorNamesForFirstAesValues(mapData, simColumnNames, "fill"),
      values = simFillValues %||% NA,
      stringsAsFactors = FALSE
    ),
    linetype = data.frame(
      names = levels(as.factor(mapData[, simColumnNames$linetype])),
      labels = levels(as.factor(mapData[, simColumnNames$linetype])),
      colorNames = .getColorNamesForFirstAesValues(mapData, simColumnNames, "linetype"),
      values = simLinetypeValues %||% "blank",
      stringsAsFactors = FALSE
    ),
    shape = data.frame(
      names = levels(as.factor(mapObservedData[, obsColumnNames$shape])),
      labels = levels(as.factor(mapObservedData[, obsColumnNames$shape])),
      colorNames = .getColorNamesForFirstAesValues(mapObservedData, obsColumnNames, "shape"),
      values = obsShapeValues %||% NA,
      stringsAsFactors = FALSE
    )
  )

  plotObject <- updateTimeProfileLegend(
    plotObject = plotObject,
    caption = plotObject$plotConfiguration$legend$caption
  )

  # remove the legend of aesthetic if default unmapped aesthetic
  if (isIncluded(simColumnNames$color, "legendLabels") & isIncluded(obsColumnNames$color, "legendLabels")) {
    plotObject <- plotObject + ggplot2::guides(color = "none")
  }
  plotObject <- .updateAxes(plotObject)
  return(plotObject)
}



#' @title updateTimeProfileLegend
#' @description Update time profile legend caption
#' @param plotObject A ggplot object
#' @param caption A data.frame as obtained from `getLegendCaption` to use for updating a plot legend.
#' @return A `ggplot` object
#' @export
updateTimeProfileLegend <- function(plotObject, caption) {
  # Apply ggplot2 scale functions only to defined/mapped aesthetics
  # values for undefined aesthetics are set to blank symbol, linetype, etc.
  blankValues <- list(
    shape = " ",
    linetype = "blank",
    fill = NA
  )
  scaleFunction <- list(
    shape = ggplot2::scale_shape_manual,
    linetype = ggplot2::scale_linetype_manual,
    fill = ggplot2::scale_fill_manual
  )

  unDefinedNames <- list()
  for (property in c("shape", "linetype", "fill")) {
    # Use suppressMessages to prevent ggplot2 to
    # throw message that scale was updated
    suppressMessages(
      plotObject <- plotObject +
        scaleFunction[[property]](
          breaks = caption[[property]]$names,
          labels = caption[[property]]$labels,
          # Even if values and breaks have same length
          # scale_<>_manual requires at least as many values as it counts factors
          # Thus, add a default number of 50 blank values to prevent this potential crash
          # Only values matching breaks are actually used
          values = c(caption[[property]]$values, rep(blankValues[[property]], 50))
        )
    )
    # Get legend names that are not scaled but in final legend
    unDefinedNames[[property]] <- setdiff(
      caption$color$names,
      caption[[property]]$colorNames
    )
    # Fill in the caption guide for shape, linetype and fill
    caption[[property]] <- rbind.data.frame(
      caption[[property]],
      data.frame(
        names = unDefinedNames[[property]],
        labels = unDefinedNames[[property]],
        colorNames = unDefinedNames[[property]],
        values = rep(blankValues[[property]], length(unDefinedNames[[property]]))
      ),
      stringsAsFactors = FALSE
    )
    # Keep only names and order provided by color legend
    captionOrder <- sapply(
      caption$color$names,
      function(colorName) {
        head(which(caption[[property]]$colorNames == colorName), 1)
      }
    )
    caption[[property]] <- caption[[property]][captionOrder, ]
  }

  # Update color scale and use color caption
  # as reference for displaying final legend
  suppressMessages(
    plotObject <- plotObject +
      ggplot2::scale_color_manual(
        breaks = caption$color$names,
        labels = caption$color$labels,
        # Even if values and breaks have same length
        # scale_<>_manual requires at least as many values as it counts factors
        # Thus, add a default number of 50 blank values to prevent this potential crash
        # Only values matching breaks are actually used
        values = c(caption$color$values, rep(NA, 50))
      ) +
      ggplot2::guides(
        fill = "none", shape = "none", linetype = "none",
        color = ggplot2::guide_legend(
          title = plotObject$plotConfiguration$legend$title$text,
          title.theme = plotObject$plotConfiguration$legend$title$createPlotTextFont(),
          override.aes = list(
            shape = caption$shape$values,
            fill = caption$fill$values,
            linetype = caption$linetype$values
          )
        )
      )
  )
  plotObject$plotConfiguration$legend$caption <- caption
  return(plotObject)
}
