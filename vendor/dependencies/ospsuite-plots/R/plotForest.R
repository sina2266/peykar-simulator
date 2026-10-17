#' Create a Forest Plot
#'
#' This function generates a forest plot with optional faceting and a corresponding table.
#'
#' @param plotData A data.table containing the data to be plotted. Must include columns specified in `yFacetColumns`, `xFacetColumn`, `tableColumns`, and others.
#' @param mapping A ggplot2 mapping object, typically created with `ggplot2::aes()`, to specify how variables in the data are mapped to visual properties.
#' @param xLabel A string representing the label for the x-axis.
#' @param yFacetColumns A character vector of column names used for faceting on the y-axis. Can be NULL or of length up to 2.
#' @param xFacetColumn A character string specifying the column name for the x-axis facet. Must be of length 1 or NULL.
#' @param xScale A character string indicating the scale type for the x-axis. Options are "linear" or "log".
#' @param xScaleArgs A list of additional arguments for customizing the x-axis scale.
#' @param groupAesthetics A character vector specifying aesthetics for grouping (e.g., color, fill, shape).
#' @param tableColumns A character vector of column names to be included in the table.
#' @param tableLabels A character vector of labels corresponding to `tableColumns`.
#' @param labelWrapWidth A numeric value specifying the width for label wrapping in facets.
#' @param digitsToRound An integer specifying the number of digits to round in the table.
#' @param digitsToShow An integer specifying the number of digits to display in the table.
#' @param withTable A logical flag indicating whether to include the table in the output. Defaults to TRUE if `xFacetColumn` is not NULL.
#' @param geomPointAttributes A list of attributes for the point geometry in the plot.
#' @param geomErrorbarAttributes A list of attributes for the error bar geometry in the plot.
#' @param facetScales A character string indicating the scales used for facets. Options are "free_y" or "free".
#'
#' @return A combined plot object containing the forest plot and the table (if applicable).
#' @export
#' @family plot functions
plotForest <- function(
  plotData,
  mapping = aes(
    y = y, # nolint
    x = x, # nolint
    groupby = dataType # nolint
  ),
  xLabel,
  yFacetColumns = NULL,
  xFacetColumn = NULL,
  xScale = c("linear", "log"),
  xScaleArgs = list(), # nolint
  groupAesthetics = c("color", "fill", "shape"),
  tableColumns = c("yValues", "yErrorValues"),
  tableLabels = c("M", "Variance"),
  labelWrapWidth = 10,
  digitsToRound = 2,
  digitsToShow = 2,
  withTable = is.null(xFacetColumn),
  geomPointAttributes = getDefaultGeomAttributes("Point"),
  geomErrorbarAttributes = getDefaultGeomAttributes("Errorbar"),
  facetScales = c("free_y", "free")
) {
  # avoid warning for global variable
  x <- y <- dataType <- NULL

  # Input checks
  checkmate::assertDataTable(plotData)
  xScale <- match.arg(xScale)
  checkmate::assertCharacter(xFacetColumn, null.ok = TRUE, len = 1)
  checkmate::assertCharacter(yFacetColumns, null.ok = TRUE, max.len = 2)
  checkmate::assertCharacter(tableColumns)
  checkmate::assertCharacter(tableLabels, len = length(tableColumns))
  checkmate::assertNames(
    names(plotData),
    must.include = c(yFacetColumns, xFacetColumn, tableColumns)
  )
  checkmate::assertIntegerish(digitsToRound, lower = 0, len = 1)
  checkmate::assertIntegerish(digitsToShow, lower = 0, len = 1)
  checkmate::assertString(xLabel)
  checkmate::assertNumeric(labelWrapWidth, lower = 0)
  checkmate::assertFlag(withTable)
  facetScales <- match.arg(facetScales)

  plotObject <-
    createPlotObject(
      plotData = plotData,
      mapping = mapping,
      groupAesthetics = groupAesthetics,
      xScale = xScale,
      xScaleArgs = xScaleArgs,
      xLabel = xLabel,
      yFacetColumns = yFacetColumns,
      xFacetColumn = xFacetColumn,
      labelWrapWidth = labelWrapWidth,
      geomPointAttributes = geomPointAttributes,
      geomErrorbarAttributes = geomErrorbarAttributes,
      facetScales = facetScales
    )

  combinedPlot <- CombinedPlot$new(plotObject = plotObject)

  # Create table data and object only if xFacetColumn is NULL
  if (is.null(xFacetColumn) & withTable) {
    tableData <- createTableData(
      plotData = plotData,
      tableColumns = tableColumns,
      tableLabels = tableLabels
    )
    combinedPlot$tableObject <- createTableObject(
      tableData = tableData,
      mapping = mapping,
      digitsToRound = digitsToRound,
      digitsToShow = digitsToShow,
      yFacetColumns = yFacetColumns
    )
  } else {
    if (!is.null(xFacetColumn) & withTable) {
      warning(messages$warningTablesOnlyWithoutFaceting())
    }
  }

  return(combinedPlot)
}
#' Create the Plot Object
#'
#' This function generates the main plot object for the forest plot.
#'
#' @param plotData A data.table containing the data to be plotted.
#' @param mapping A ggplot mapping object.
#' @param xScale A character string indicating the scale type for the x-axis.
#' @param xScaleArgs A list of additional arguments for customizing the x-axis scale.
#' @param xLabel A string representing the label for the x-axis.
#' @param yFacetColumns A character vector of column names used for faceting on the y-axis.
#' @param xFacetColumn A character string specifying the column name for the x-axis facet.
#' @param labelWrapWidth A numeric value specifying the width for label wrapping in facets.
#' @param groupAesthetics A character vector specifying aesthetics for grouping.
#' @param geomPointAttributes A list of attributes for the point geometry.
#' @param geomErrorbarAttributes A list of attributes for the error bar geometry.
#' @param facetScales A character string indicating the scales used for facets. Defaults to "free_y".
#'
#' @return A ggplot object representing the main plot.
#' @keywords internal
createPlotObject <- function(
  plotData,
  mapping,
  xScale,
  xScaleArgs, # nolint
  xLabel,
  groupAesthetics,
  yFacetColumns,
  xFacetColumn,
  labelWrapWidth,
  geomPointAttributes,
  geomErrorbarAttributes,
  facetScales = "free_y"
) {
  # Generate the facet formula if yFacetColumns are provided
  facetFormula <- if (!is.null(yFacetColumns) && length(yFacetColumns) > 0) {
    if (!is.null(xFacetColumn) && length(xFacetColumn) > 0) {
      stats::as.formula(paste(
        paste(yFacetColumns, collapse = " + "),
        "~",
        xFacetColumn
      ))
    } else {
      stats::as.formula(paste(paste(yFacetColumns, collapse = " + "), "~."))
    }
  } else {
    NULL
  }

  mappedData <- MappedData$new(
    data = plotData,
    mapping = mapping,
    direction = "x",
    yScale = "linear",
    xScale = xScale,
    groupAesthetics = groupAesthetics
  )
  plotObject <- initializePlot(mappedData)
  # Points
  plotObject <- addLayer(
    mappedData = mappedData,
    geom = "point",
    geomAttributes = utils::modifyList(
      list(position = position_dodge(width = 1)),
      geomPointAttributes
    ),
    plotObject = plotObject,
    layerToCall = geom_point
  )
  # Bar
  if (
    "xmin" %in%
      names(mappedData$mapping) &
      "xmax" %in% names(mappedData$mapping)
  ) {
    plotObject <- addLayer(
      mappedData = mappedData,
      geom = "errorbar",
      geomAttributes = utils::modifyList(
        list(position = position_dodge(width = 1), orientation = "x"),
        geomErrorbarAttributes
      ),
      plotObject = plotObject,
      layerToCall = geom_errorbar_osp
    )
  }
  plotObject <- plotObject +
    (if (!is.null(facetFormula)) {
      ggh4x::facet_nested(
        facetFormula,
        switch = "y",
        scales = facetScales,
        space = "free_y",
        labeller = label_wrap_gen(width = labelWrapWidth),
        nest_line = element_line(color = "black"),
        resect = unit(1, units = "lines")
      )
    }) +
    labs(y = "", x = xLabel) +
    theme(
      panel.spacing = unit(0, "lines"),
      panel.border = element_rect(colour = "grey"),
      axis.line = element_line(color = "black"),
      strip.text.y.left = element_text(angle = 0, hjust = 0.5, vjust = 0.5),
      strip.background.y = element_rect(fill = NA, color = NA),
      strip.text.x = element_text(hjust = 0, vjust = 1, angle = 0),
      strip.background.x = element_rect(fill = NA, color = NA),
      strip.placement = "outside"
    )

  plotObject <- addXScale(
    plotObject = plotObject,
    xScale = xScale,
    xScaleArgs = xScaleArgs
  )

  return(plotObject)
}
#' Create Table Data
#'
#' This function prepares the data for the table.
#'
#' @param plotData A data.table containing the data to be plotted in the table.
#' @param tableColumns A character vector of column names to be included in the table.
#' @param tableLabels A character vector of labels corresponding to `tableColumns`.
#'
#' @return A data.table containing the prepared table data.
#'
#' @keywords internal
createTableData <- function(plotData, tableColumns, tableLabels) {
  # initialize variable to avoid messages
  .value <- .valueType <- NULL # nolint

  tableData <- melt(
    plotData,
    measure.vars = tableColumns,
    variable.name = ".valueType",
    value.name = ".value"
  )
  tableData <- tableData[!is.na(.value)]

  tableData$.valueType <- factor(
    tableData$.valueType,
    levels = tableColumns,
    labels = tableLabels,
    ordered = TRUE
  )

  return(tableData)
}
#' Create the Table Object
#'
#' This function generates the table plot object.
#'
#' @param tableData A data.table containing the data to be plotted in the table.
#' @param mapping A ggplot mapping object.
#' @param digitsToRound An integer specifying the number of digits to round in the table.
#' @param digitsToShow An integer specifying the number of digits to display in the table.
#' @param yFacetColumns A character vector of column names used for faceting on the y-axis.
#'
#' @return A ggplot object representing the table.
#' @keywords internal
createTableObject <- function(
  tableData,
  mapping,
  digitsToRound,
  digitsToShow,
  yFacetColumns
) {
  # initialize variable to avoid messages
  .value <- .valueType <- NULL # nolint

  nTypes <- tryCatch(
    {
      length(unique(rlang::eval_tidy(
        expr = rlang::get_expr(mapping$groupby),
        data = tableData,
        env = rlang::get_env(mapping$groupby)
      )))
    },
    error = function(cond) {
      1
    }
  )

  facetFormula <- if (nTypes > 1) {
    stats::as.formula(paste(
      paste(yFacetColumns, collapse = " + "),
      "~",
      rlang::get_expr(mapping$groupby)
    ))
  } else {
    stats::as.formula(paste(paste(yFacetColumns, collapse = " + "), "~."))
  }

  mapping <- utils::modifyList(
    mapping["y"],
    aes(
      x = .valueType,
      label = sprintf(
        paste0("%.", digitsToShow, "f"),
        round(.value, digitsToRound)
      )
    )
  )

  mappedData <- MappedData$new(
    data = tableData,
    mapping = mapping,
    yScale = "linear",
    xScale = "linear",
    groupAesthetics = c()
  )
  tableObject <- initializePlot(mappedData) +
    geom_text(size = 3) +
    (if (!is.null(facetFormula)) {
      facet_grid(
        facetFormula,
        scales = "free",
        space = "free",
        switch = "x",
        drop = TRUE
      )
    }) +
    scale_x_discrete(position = "top") +
    labs(y = "", x = "") +
    theme(
      panel.spacing = unit(0, "lines"),
      panel.border = element_rect(colour = "grey"),
      panel.grid = element_blank(),
      axis.text.x = element_text(),
      axis.text.y = element_blank(),
      axis.title.y = element_blank(),
      axis.ticks.y = element_blank(),
      strip.text.y = element_blank(),
      strip.placement = "outside"
    )

  return(tableObject)
}
#' Combine Plot and Table
#'
#' This function combines the plot and the table into a single output.
#'
#' @param plotObject A ggplot object representing the main plot.
#' @param tableObject A ggplot object representing the table.
#' @param relWidths A numeric vector of length 2 specifying the relative widths of the plot and table.
#'
#' @return A combined ggplot object containing both the plot and the table.
#' @keywords internal
combinePlots <- function(plotObject, tableObject, relWidths) {
  # Check the current legend position from the plot's own theme, falling back
  # to the OSPSuite theme rather than the global ggplot2 state.
  legendPosition <- plotObject$theme$legend.position %||%
    theme_osp()$legend.position
  if (identical(legendPosition, "right")) {
    # Change legend position to top if table is included
    plotObject <- plotObject + theme(legend.position = "top")
  }

  cowplot::plot_grid(
    plotObject,
    tableObject,
    nrow = 1,
    axis = "tb",
    align = "h",
    rel_widths = relWidths
  )
}
