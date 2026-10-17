#' Initialize Plot
#'
#' Initialize a `ggplot` object with a watermark and set its labels by metaData.
#'
#' @param mappedData A `MappedData` object.
#' @param setMapping A boolean indicating if TRUE (default) mapping is passed to ggplot; otherwise, mapping will be used only to create labels.
#'
#' @return A `ggplot` object.
#' @export
initializePlot <- function(mappedData = NULL, setMapping = TRUE) {
  # Validation
  checkmate::assertClass(mappedData, classes = "MappedData", null.ok = TRUE)
  checkmate::assertFlag(setMapping)

  mappingToSet <- aes()
  if (setMapping && !is.null(mappedData)) {
    mappingToSet <- mappedData$mapping
  }
  plotObject <- ggplotWithWatermark(
    data = mappedData$dataForPlot,
    mapping = mappingToSet
  ) +
    theme_osp()

  # Tag the plot so ggplot_build.osp_ggplot() applies the OSP discrete
  # color/fill (and shape) scales per plot, without mutating global state.
  class(plotObject) <- unique(c("osp_ggplot", class(plotObject)))

  # add labels
  plotObject <- addLabels(plotObject, mappedData)

  return(plotObject)
}
#' Add Layer
#'
#' Add a layer to a `ggplot` object.
#'
#' @param mappedData A `MappedData` object.
#' @param plotObject A `ggplot` object on which to add the plot layer.
#' @param geomAttributes Arguments passed on to the ggplot2 geom layer.
#' @param geom A character string used to select appropriate aesthetics.
#' @param layerToCall A function representing the ggplot2 geom layer.
#'
#' @return The updated `ggplot` object.
#' @keywords internal
addLayer <- function(
  mappedData,
  geomAttributes,
  geom,
  plotObject,
  layerToCall
) {
  # Validate input attributes
  checkmate::assertClass(mappedData, "MappedData")
  checkmate::assertClass(plotObject, "gg")
  checkmate::assertList(geomAttributes, null.ok = TRUE)
  checkmate::assertCharacter(geom, len = 1)
  checkmate::assertFunction(layerToCall)

  filteredMapping <- mappedData$getAestheticsForGeom(
    geom = geom,
    geomAttributes = geomAttributes
  )

  # Always use geom_point_osp for point geoms
  if (geom == "point") {
    layerToCall <- geom_point_osp
  }

  if (!is.null(filteredMapping)) {
    plotObject <- plotObject +
      do.call(
        what = layerToCall,
        args = c(
          list(
            data = mappedData$dataForPlot,
            mapping = filteredMapping
          ),
          utils::modifyList(
            x = list(na.rm = TRUE),
            val = geomAttributes
          )
        )
      )
  }

  if (geom == "point" & mappedData$hasLLOQMatch) {
    plotObject <- plotObject +
      scale_alpha_manual(
        values = getOspsuite.plots.option(
          optionKey = OptionKeys$lloqAlphaVector
        )
      ) +
      guides(alpha = "none")
  }

  return(plotObject)
}
#' Add LLOQ Layer with LLOQ Lines
#'
#' Add a layer for LLOQ lines to a `ggplot` object.
#'
#' @param plotObject A `ggplot` object on which to add the plot layer.
#' @param mappedData A `MappedData` object with LLOQ data.
#' @param layerToCall A function representing the ggplot2 geom layer.
#' @param useLinetypeAsAttribute A boolean indicating whether to set the line type
#' as an attribute (TRUE) or not (FALSE); if TRUE, no legend is created.
#' @param geomLLOQAttributes Additional attributes for the LLOQ layer.
#'
#' @return The updated `ggplot` object.
#' @export
addLLOQLayer <-
  function(
    plotObject,
    mappedData,
    layerToCall,
    useLinetypeAsAttribute,
    geomLLOQAttributes
  ) {
    # Early return if no LLOQ data is present
    if (!mappedData$hasLLOQMatch) {
      return(plotObject)
    }

    # Configure LLOQ line appearance based on legend preference
    if (useLinetypeAsAttribute) {
      # When using linetype as attribute: no legend entry, direct styling
      geomLLOQAttributes <- utils::modifyList(
        list(
          linetype = getOspsuite.plots.option(
            optionKey = OptionKeys$lloqLineType
          )
        ),
        geomLLOQAttributes
      )
    }

    filteredMapping <- mappedData$getAestheticsForGeom(
      geom = "hvline",
      geomAttributes = geomLLOQAttributes
    )

    if (!useLinetypeAsAttribute) {
      # When not using linetype as attribute: create legend entry with "LLOQ" label
      filteredMapping <-
        structure(
          utils::modifyList(
            filteredMapping,
            aes(linetype = "LLOQ") # Maps to legend with "LLOQ" label
          ),
          class = "uneval"
        )
    }

    plotObject <- plotObject +
      do.call(
        what = layerToCall,
        args = c(
          list(
            data = mappedData$dataForPlot,
            mapping = filteredMapping
          ),
          utils::modifyList(
            # Default to the OSP line width so the LLOQ line matches the other
            # data lines without relying on global geom defaults; a user-supplied
            # `linewidth` in geomLLOQAttributes still overrides it.
            x = list(
              na.rm = TRUE,
              linewidth = getDefaultGeomAttributes("Line")$linewidth
            ),
            val = geomLLOQAttributes
          )
        )
      )

    if (!useLinetypeAsAttribute) {
      # Add manual scale for legend: maps "LLOQ" label to specified line type
      plotObject <- plotObject +
        scale_linetype_manual(
          values = c(
            LLOQ = getOspsuite.plots.option(optionKey = OptionKeys$lloqLineType)
          ),
          guide = guide_legend(
            title = NULL,
            order = 10,
          )
        )
    }

    return(plotObject)
  }
#' Add X and Y Scale
#'
#' Add X and Y scales to a `ggplot` object.
#'
#' @param plotObject A `ggplot` object on which to add the scale.
#' @param xScale The x-axis scale type. Available is 'linear', 'log', 'discrete'
#' @param xScaleArgs A list of arguments for the x-axis scale.
#' @param yScale The y-axis scale type. Available is 'linear', 'log'
#' @param yScaleArgs A list of arguments for the y-axis scale.
#' @param secAxis Secondary axis arguments for scale_y functions.
#'
#' @return The updated `ggplot` object.
#' @export
addXYScale <- function(
  plotObject,
  xScale = NULL,
  xScaleArgs = list(),
  yScale = NULL,
  yScaleArgs = list(),
  secAxis = waiver()
) {
  if (!is.null(xScale))
    xScale <- match.arg(
      xScale,
      c(AxisScales$linear, AxisScales$log, AxisScales$discrete)
    )
  if (!is.null(yScale))
    yScale <- match.arg(yScale, c(AxisScales$linear, AxisScales$log))

  if (!is.null(xScale)) {
    plotObject <- addXScale(
      plotObject,
      xScale = xScale,
      xScaleArgs = xScaleArgs
    )
  }

  if (!is.null(yScale)) {
    plotObject <- addYScale(
      plotObject,
      yScale = yScale,
      yScaleArgs = yScaleArgs,
      secAxis = secAxis
    )
  }

  return(plotObject)
}
#' Build scale args for continuous axes
#'
#' Shared helper that applies log-scale settings and sets the default
#' out-of-bounds handler to `scales::oob_keep` (zoom-style behaviour).
#' Users can override `oob` by including it in the `scaleArgs` list.
#'
#' @param scaleArgs list of arguments to be passed to a `scale_*_continuous` function
#' @param isLog logical; `TRUE` when the axis uses a log10 transform
#'
#' @return updated `scaleArgs` list
#' @keywords internal
#' @noRd
.buildContinuousScaleArgs <- function(scaleArgs, isLog) {
  if (isLog) {
    scaleArgs[["transform"]] <- "log10"
    if (is.null(scaleArgs$guide)) {
      scaleArgs[["guide"]] <- "axis_logticks"
    }
  }

  # Default to zoom-style out-of-bounds handling unless user overrides
  if (is.null(scaleArgs$oob)) {
    scaleArgs[["oob"]] <- scales::oob_keep
  }

  return(scaleArgs)
}
#' add X-scale
#'
#' @inheritParams addXYScale
#'
#' @return The updated `ggplot` object
#' @export
addXScale <- function(plotObject, xScale, xScaleArgs = list()) {
  xScale <- match.arg(
    xScale,
    c(AxisScales$linear, AxisScales$log, AxisScales$discrete)
  )

  if (xScale == AxisScales$discrete) {
    scaleFunction <- scale_x_discrete
  } else {
    scaleFunction <- scale_x_continuous
    xScaleArgs <- .buildContinuousScaleArgs(
      xScaleArgs,
      isLog = xScale == AxisScales$log
    )
  }

  plotObject <- plotObject +
    do.call(
      what = scaleFunction,
      args = xScaleArgs
    )

  return(plotObject)
}
#' add y-scale
#'
#' @inheritParams addXYScale
#'
#' @return The updated `ggplot` object
#' @export
addYScale <- function(
  plotObject,
  yScale,
  yScaleArgs = list(),
  secAxis = waiver()
) {
  yScale <- match.arg(yScale, c(AxisScales$linear, AxisScales$log))

  yScaleArgs <- .buildContinuousScaleArgs(
    yScaleArgs,
    isLog = yScale == AxisScales$log
  )

  plotObject <- plotObject +
    do.call(
      what = scale_y_continuous,
      args = c(
        yScaleArgs,
        list(sec.axis = secAxis)
      )
    )

  return(plotObject)
}
