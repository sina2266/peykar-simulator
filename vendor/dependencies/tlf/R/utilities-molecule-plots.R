#' @title .addScatterLayer
#' @description Add scatter points layer of a molecule plot
#' @param plotObject A `ggplot` object
#' @param data A data.frame with labels mapped to properties and obtained from a `DataMapping` object
#' @param mapLabels List of mapped label names passed to `ggplot2::aes`
#' @return A `ggplot` object
#' @keywords internal
.addScatterLayer <- function(plotObject, data, mapLabels) {
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = 0,
    plotConfigurationProperty = plotObject$plotConfiguration$points,
    propertyNames = c("size", "alpha")
  )

  plotObject <- plotObject +
    geomTLFPoint(
      data = data,
      mapping = ggplot2::aes(
        x = .data[[mapLabels$x]],
        y = .data[[mapLabels$y]],
        color = .data[[mapLabels$color]],
        shape = .data[[mapLabels$shape]],
        alpha = if (mapLabels$lloq != "legendLabels") {
          switch(plotObject$plotConfiguration$lloqDirection,
            "horizontal" = (.data[[mapLabels$y]] < .data[[mapLabels$lloq]]),
            "vertical" = (.data[[mapLabels$x]] < .data[[mapLabels$lloq]]),
            "both" = ((.data[[mapLabels$y]] < .data[[mapLabels$lloq]]) | (.data[[mapLabels$x]] < .data[[mapLabels$lloq]]))
          )
        } else {
          as.factor(aestheticValues$alpha)
        }
      ),
      size = aestheticValues$size,
      na.rm = TRUE,
      show.legend = TRUE
    ) +
    scale_alpha_manual(
      values = if (mapLabels$lloq != "legendLabels") {
        c(aestheticValues$alpha, tlfEnv$DefaultAlphaRatio * aestheticValues$alpha)
      } else {
        aestheticValues$alpha
      },
      breaks = NULL
    )
  return(plotObject)
}

#' @title .addErrorbarLayer
#' @description Add errorbar layer of a molecule plot
#' @param plotObject A `ggplot` object
#' @param data A data.frame with labels mapped to properties and obtained from a `DataMapping` object
#' @param mapLabels List of mapped label names passed to `ggplot2::aes_string`
#' @return A `ggplot` object
#' @keywords internal
.addErrorbarLayer <- function(plotObject, data, mapLabels, direction = "vertical") {
  validateEnumValue(direction, Directions)
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = 0,
    plotConfigurationProperty = plotObject$plotConfiguration$errorbars,
    propertyNames = c("size", "linetype", "alpha")
  )


  # If xmin/xmax or ymin/ymax are equal to x or y,  do not plot error bar caps
  data[[mapLabels$xmin]][data[[mapLabels$xmin]] == data[[mapLabels$x]] | is.na(data[[mapLabels$xmin]])] <- NA_real_
  data[[mapLabels$xmax]][data[[mapLabels$xmax]] == data[[mapLabels$x]] | is.na(data[[mapLabels$xmax]])] <- NA_real_
  data[[mapLabels$ymin]][data[[mapLabels$ymin]] == data[[mapLabels$y]] | is.na(data[[mapLabels$ymin]])] <- NA_real_
  data[[mapLabels$ymax]][data[[mapLabels$ymax]] == data[[mapLabels$y]] | is.na(data[[mapLabels$ymax]])] <- NA_real_


  plotObject <- switch(direction,
    "vertical" = {
      plotObject <-
        plotObject +
        ggplot2::geom_linerange(
          data = data,
          mapping = ggplot2::aes(
            x = .data[[mapLabels$x]],
            # If lower value is negative and plot is log scaled,
            # Upper bar will still be plotted
            ymin = .data[[mapLabels$ymin]],
            ymax = .data[[mapLabels$y]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          linewidth = aestheticValues$size,
          linetype = aestheticValues$linetype,
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        ) +
        ggplot2::geom_linerange(
          data = data,
          mapping = ggplot2::aes(
            x = .data[[mapLabels$x]],
            # If lower value is negative and plot is log scaled,
            # Upper bar will still be plotted
            ymin = .data[[mapLabels$y]],
            ymax = .data[[mapLabels$ymax]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          linewidth = aestheticValues$size,
          linetype = aestheticValues$linetype,
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        )

      # Add upper cap to error bar
      # If lower value is negative and plot is log scaled,
      # Upper bar cap will still be plotted{
      plotObject <- plotObject +
        ggplot2::geom_point(
          data = data,
          mapping = ggplot2::aes(
            x = .data[[mapLabels$x]],
            y = .data[[mapLabels$ymin]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          size = tlfEnv$defaultErrorbarCapSize,
          shape = "_",
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        )


      # Add lower cap to error bar
      # If lower value is negative and plot is log scaled,
      # Upper bar cap will still be plotted
      plotObject <- plotObject +
        ggplot2::geom_point(
          data = data,
          mapping = ggplot2::aes(
            x = .data[[mapLabels$x]],
            y = .data[[mapLabels$ymax]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          size = tlfEnv$defaultErrorbarCapSize,
          shape = "_",
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        )
      return(plotObject)
    },
    "horizontal" = {
      plotObject <-
        plotObject +
        ggplot2::geom_linerange(
          data = data,
          mapping = ggplot2::aes(
            # If lower value is negative and plot is log scaled,
            # Upper bar will still be plotted
            xmin = .data[[mapLabels$xmin]],
            xmax = .data[[mapLabels$x]],
            y = .data[[mapLabels$y]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          linewidth = aestheticValues$size,
          linetype = aestheticValues$linetype,
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        ) +
        ggplot2::geom_linerange(
          data = data,
          mapping = ggplot2::aes(
            # If lower value is negative and plot is log scaled,
            # Upper bar will still be plotted
            xmin = .data[[mapLabels$x]],
            xmax = .data[[mapLabels$xmax]],
            y = .data[[mapLabels$y]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          linewidth = aestheticValues$size,
          linetype = aestheticValues$linetype,
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        )

      # Add lower cap to error bar
      # If lower value is negative and plot is log scaled,
      # Upper bar cap will still be plotted
      plotObject <-
        plotObject +
        ggplot2::geom_point(
          data = data,
          mapping = ggplot2::aes(
            x = .data[[mapLabels$xmin]],
            y = .data[[mapLabels$y]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          size = tlfEnv$defaultErrorbarCapSize,
          shape = "|",
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        )


      # Add upper cap to error bar
      # If lower value is negative and plot is log scaled,
      # Upper bar cap will still be plotted
      plotObject <-
        plotObject +
        ggplot2::geom_point(
          data = data,
          mapping = ggplot2::aes(
            x = .data[[mapLabels$xmax]],
            y = .data[[mapLabels$y]],
            color = .data[[mapLabels$color]],
            group = .data[[mapLabels$shape]]
          ),
          size = tlfEnv$defaultErrorbarCapSize,
          shape = "|",
          alpha = aestheticValues$alpha,
          na.rm = TRUE,
          show.legend = FALSE
        )

      return(plotObject)
    }
  )
}

#' @title .addLineLayer
#' @description Add line layer of a molecule plot
#' @param plotObject A `ggplot` object
#' @param type one of "horizontal", "vertical" or "diagonal"
#' Note that for "diagonal", geom_abline is used.
#' `value` of intercept is taken as is for linear scale but corresponds to the log of `value` for log scale.
#'  For instance, intercept = c(-1, 0, 1) with log scale actually means that the line will go through c(0.1, 1, 10)
#'  because c(-1, 0, 1) = log10(c(0.1, 1, 10)).
#' @param value value of xintercept or yintercept
#' @param position line position for aesthetic properties
#' @return A `ggplot` object
#' @keywords internal
.addLineLayer <- function(plotObject, type, value, position) {
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = position,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = c("color", "linetype", "size", "alpha")
  )

  plotObject <- plotObject + switch(type,
    "horizontal" = ggplot2::geom_hline(
      yintercept = value,
      color = aestheticValues$color,
      linetype = aestheticValues$linetype,
      alpha = aestheticValues$alpha,
      linewidth = aestheticValues$size
    ),
    "vertical" = ggplot2::geom_vline(
      xintercept = value,
      color = aestheticValues$color,
      linetype = aestheticValues$linetype,
      alpha = aestheticValues$alpha,
      linewidth = aestheticValues$size
    ),
    "diagonal" = ggplot2::geom_abline(
      slope = 1,
      intercept = value,
      linetype = aestheticValues$linetype,
      color = aestheticValues$color,
      alpha = aestheticValues$alpha,
      linewidth = aestheticValues$size
    ),
    "obsvspredDiagonal" = ggplot2::geom_abline(
      data = data.frame(
        value = value,
        position = as.character(position),
        slope = 1
      ),
      aes(
        intercept = .data$value,
        linetype = .data$position,
        slope = .data$slope
      ),
      color = aestheticValues$color,
      alpha = aestheticValues$alpha,
      linewidth = aestheticValues$size,
      key_glyph = plotObject$plotConfiguration$foldLineslegendType
    ),
    "ddiHorizontal" = ggplot2::geom_abline(
      slope = 0,
      intercept = value,
      color = aestheticValues$color,
      linetype = aestheticValues$linetype,
      alpha = aestheticValues$alpha,
      linewidth = aestheticValues$size
    )
  )
  return(plotObject)
}

#' @title getDualAxisPlot
#' @description Check if dual Y Axis is needed
#' @param leftPlotObject A `ggplot` object with left y-axis
#' @param rightPlotObject A `ggplot` object with right y-axis
#' @return A `ggplot` object with dual y-axis
#' @export
getDualAxisPlot <- function(leftPlotObject, rightPlotObject) {
  rlang::check_installed("cowplot", reason = "to use dual axis plots")
  # Only one legend shall be kept to prevent text not aligned and on top of plot axes text
  # For most cases, right plot legend is kept as is while left plot legend is removed
  # If left side legend, left plot legend is kept as is while right plot legend is removed
  legendPosition <- getLegendPosition(leftPlotObject)
  if (isIncluded(legendPosition, LegendPositions$outsideLeft)) {
    rightPlotObject <- setLegendPosition(rightPlotObject, position = LegendPositions$none)
  } else {
    leftPlotObject <- setLegendPosition(leftPlotObject, position = LegendPositions$none)
  }
  # Set same X-Axis between plots
  leftScale <- ggplot2::layer_scales(leftPlotObject)
  rightScale <- ggplot2::layer_scales(rightPlotObject)
  mergeXRange <- range(leftScale$x$range$range, rightScale$x$range$range)

  leftPlotObject <- setXAxis(leftPlotObject, axisLimits = mergeXRange)
  rightPlotObject <- setXAxis(rightPlotObject, axisLimits = mergeXRange)

  # Transformed right plot to be compatible with left plot
  rightPlotObject <- rightPlotObject +
    ggplot2::theme(
      # Update right axis properties
      axis.text.y.right = rightPlotObject$plotConfiguration$y2Axis$font$createPlotTextFont(),
      axis.title.y.right = rightPlotObject$plotConfiguration$labels$y2label$createPlotTextBoxFont(),
      axis.line.y.right = rightPlotObject$plotConfiguration$background$y2Axis$createPlotElement(),
      panel.grid.major.y = rightPlotObject$plotConfiguration$background$y2Grid$createPlotElement(),
      panel.grid.minor.y = rightPlotObject$plotConfiguration$background$y2Grid$createPlotElement(
        size = as.numeric(rightPlotObject$plotConfiguration$background$y2Grid$size) / 2
      ),
      # Remove all other background properties
      plot.background = ggplot2::element_blank(),
      panel.background = ggplot2::element_blank(),
      axis.line.x = ggplot2::element_blank(),
      axis.line.y.left = ggplot2::element_blank(),
      panel.grid.major.x = ggplot2::element_blank(),
      panel.grid.minor.x = ggplot2::element_blank()
    )
  rightPlotObject <- setPlotLabels(
    rightPlotObject,
    ylabel = rightPlotObject$plotConfiguration$labels$y2label
  )
  rightPlotObject <- setY2Axis(rightPlotObject)
  leftPlotObject <- setYAxis(leftPlotObject)

  # Fixes issue #522, define null device
  # allowing cowplot to handle font database
  cowplot::set_null_device(leftPlotObject$plotConfiguration$export$format)
  alignedPlots <- cowplot::align_plots(
    leftPlotObject,
    rightPlotObject,
    align = "hv",
    axis = "tblr"
  )

  mergedPlotObject <- cowplot::ggdraw(alignedPlots[[1]]) +
    cowplot::draw_plot(alignedPlots[[2]])
  # In case of additional updates, clone plotConfiguration
  mergedPlotObject$plotConfiguration <- leftPlotObject$plotConfiguration$clone(deep = TRUE)

  return(mergedPlotObject)
}

#' @title .addLLOQLayer
#' @description Add a line layer representing the Lower Limit Of Quantification (LLOQ)
#' @param plotObject A `ggplot` object
#' @param data A data.frame with labels mapped to properties and obtained from a `DataMapping` object
#' @param mapLabels List of mapped label names passed to `ggplot2::aes_string`
#' @param direction Whether to draw LLOQ lines for x (vertical), y (horizontal) or x and y (both).
#' @return A `ggplot` object
#' @keywords internal
.addLLOQLayer <- function(plotObject, data, mapLabels, direction) {
  aestheticValues <- .getAestheticValuesFromConfiguration(
    n = 1,
    position = 0,
    plotConfigurationProperty = plotObject$plotConfiguration$lines,
    propertyNames = c("color", "linetype", "size", "alpha")
  )

  validateEnumValue(direction, enum = Directions)

  plotObject <- switch(direction,
    "horizontal" = plotObject + ggplot2::geom_hline(
      data = data,
      mapping = ggplot2::aes(
        yintercept = .data[[mapLabels$lloq]],
        color = .data[[mapLabels$color]]
      ),
      linetype = tlfEnv$defaultLLOQLinetype,
      linewidth = aestheticValues$size,
      alpha = aestheticValues$alpha,
      na.rm = TRUE,
      show.legend = FALSE
    ),
    "vertical" = plotObject + ggplot2::geom_vline(
      data = data,
      mapping = ggplot2::aes(
        xintercept = .data[[mapLabels$lloq]],
        color = .data[[mapLabels$color]]
      ),
      linetype = tlfEnv$defaultLLOQLinetype,
      linewidth = aestheticValues$size,
      alpha = aestheticValues$alpha,
      na.rm = TRUE,
      show.legend = FALSE
    ),
    "both" = .addLLOQLayer(
      .addLLOQLayer(plotObject,
        data,
        mapLabels,
        direction = "vertical"
      ),
      data, mapLabels,
      direction = "horizontal"
    )
  )

  return(plotObject)
}

#' @title .applyColorPalette
#' @description Apply a color palette to a ggplot object
#' @param plotObject A `ggplot` object
#' @param colorPalette A color palette name.
#' See enum `ColorPalettes` to get available color palettes.
#' @return A `ggplot` object
#' @keywords internal
.applyColorPalette <- function(plotObject, colorPalette = NULL) {
  if (isEmpty(colorPalette)) {
    return(plotObject)
  }
  if (isIncluded(colorPalette, .ViridisPalettes)) {
    suppressMessages(
      plotObject <- plotObject +
        ggplot2::scale_fill_viridis_d(
          option = colorPalette,
          aesthetics = c("color", "fill")
        )
    )
    return(plotObject)
  }
  # For unknown color palettes, ggplot2 throw a warning and default to Greens color palette
  # Use tryCatch to use a better default (Set1) and suggest a colorPalette from enum ColorPalette
  tryCatch(
    {
      # Silence ggplot2 message from using 'scale_fill_brewer':
      # 'Adding another scale for fill, which will replace the existing scale'
      suppressMessages(
        plotObject <- plotObject +
          ggplot2::scale_fill_brewer(
            palette = colorPalette,
            aesthetics = c("color", "fill")
          )
      )
    },
    warning = function(w) {
      warning(messages$unknownColorPalette(colorPalette), call. = FALSE)
      plotObject$plotConfiguration$colorPalette <- ColorPalettes$Set1
      suppressMessages(
        plotObject <- plotObject +
          ggplot2::scale_fill_brewer(
            palette = ColorPalettes$Set1,
            aesthetics = c("color", "fill")
          )
      )
    }
  )
  return(plotObject)
}
