#' Export a ggplot object to a file
#'
#' This function exports a ggplot object to a specified file with customizable options.
#'
#' @details
#' The height of the plot is calculated if it is not provided by the user. The calculation takes into account:
#' - The aspect ratio of the plot, which is derived from the theme settings.
#' - The number of rows and columns in the plot layout.
#' - The dimensions of plot components such as axes, legends, and margins.
#' The function ensures that the height is adjusted to maintain the correct aspect ratio based on the specified width.
#'
#' Options available for plot export with default values:
#' - `ospsuite.plots.exportWidth`: Width of the exported plot (default = 16).
#' - `ospsuite.plots.exportUnits`: Units of the exported plot (default = "cm").
#' - `ospsuite.plots.exportDevice`: File format of the exported plot (default = "png").
#' - `ospsuite.plots.exportDpi`: Resolution of the exported plot (default = 300).
#'
#' For more details and examples see the vignettes:
#' * \code{vignette("ospsuite.plots", package = "ospsuite.plots")}
#'
#' @param plotObject A ggplot object to be exported.
#' @param filepath A character string specifying the directory to save the plot.
#' @param filename A character string specifying the name of the file (without path).
#' @param width A numeric value specifying the width of the plot. If NULL, the default option is used.
#' @param height A numeric value specifying the height of the plot. If NULL, it is calculated based on the plot dimensions.
#' @param device Export device, if NULL (default) the device set by ospsuite.plots.exportDevice is used.
#' @param ... Additional arguments passed to `ggsave`.
#'
#' @return NULL, the function saves the plot to the specified file.
#'
#' @examples
#' \dontrun{
#' # Basic usage
#' p <- ggplot(mtcars, aes(x = wt, y = mpg)) +
#'   geom_point()
#' exportPlot(
#'   plotObject = p,
#'   filepath = tempdir(),
#'   filename = "my_plot.png"
#' )
#'
#' # Export with custom dimensions and device
#' exportPlot(
#'   plotObject = p,
#'   filepath = "./output",
#'   filename = "scatter_plot",
#'   width = 12,
#'   height = 8,
#'   device = "pdf"
#' )
#'
#' # Export with special characters in filename (will be cleaned)
#' exportPlot(
#'   plotObject = p,
#'   filepath = tempdir(),
#'   filename = "concentration in µg/L: results"
#' )
#' }
#' @export
exportPlot <- function(
  plotObject,
  filepath,
  filename,
  width = NULL,
  height = NULL,
  device = NULL,
  ...
) {
  if ("CombinedPlot" %in% class(plotObject)) plotObject <- plotObject$combined()
  filename <- validateFilename(filename = filename, device = device)
  validateInputsExportPlot(
    plotObject,
    filepath,
    filename,
    width = width,
    height = height,
    device = device
  )
  

  if (is.null(width))
    width <- getOspsuite.plots.option(optionKey = OptionKeys$exportWidth)

  if (is.null(height)) {
    dimensions <- calculatePlotDimensions(plotObject, width)
    width <- dimensions$width
    height <- dimensions$height
  }
  ggsave(
    filename = file.path(filepath, filename),
    plot = plotObject,
    width = width,
    height = height,
    dpi = getOspsuite.plots.option(optionKey = OptionKeys$exportDpi),
    units = getOspsuite.plots.option(optionKey = OptionKeys$exportUnits),
    ...
  )
}


#' Validate inputs for exporting a plot
#'
#' @param plotObject A ggplot object to be exported.
#' @param filepath A character string specifying the directory to save the plot.
#' @param filename A character string specifying the name of the file (without path).
#' @param width A numeric value specifying the width of the plot.
#' @param height A numeric value specifying the height of the plot.
#' @param device A character with the device to use
#'
#' @keywords internal
validateInputsExportPlot <- function(
  plotObject,
  filepath,
  filename,
  width,
  height,
  device
) {
  checkmate::assertClass(plotObject, "ggplot", null.ok = FALSE)
  checkmate::assertCharacter(filename, null.ok = FALSE)
  checkmate::assertCharacter(filepath, null.ok = FALSE)
  checkmate::assertCharacter(device, null.ok = TRUE)
  checkmate::assertDouble(width, null.ok = TRUE)
  checkmate::assertDouble(height, null.ok = TRUE)

  if (!dir.exists(filepath)) {
    dir.create(filepath, recursive = TRUE)
  }

  return(invisible())
}
#' Calculate plot dimensions based on the plot object and specified width
#'
#' @param plotObject A ggplot object.
#' @param width A numeric value specifying the width of the plot.
#'
#' @return A list containing the calculated width, height, heightOffset, and widthOffset of the plot.
#'
#' @keywords internal
calculatePlotDimensions <- function(plotObject, width) {
  # Base the resolved theme on the OSPSuite theme rather than the global
  # ggplot2 state, then overlay the plot's own theme settings.
  themeOfPlot <- utils::modifyList(theme_osp(), plotObject$theme)
  exportunits <- getOspsuite.plots.option(optionKey = OptionKeys$exportUnits)

  aspect.ratio <- themeOfPlot$aspect.ratio
  if (is.null(aspect.ratio)) aspect.ratio <- 1

  pb <- ggplot_build(plotObject)
  nCol <- max(pb$layout$layout$COL)
  nRow <- max(pb$layout$layout$ROW)
  nPanel <- length(pb$layout$layout$PANEL)

  plotDim <- getPlotDimensions(
    plotObject = plotObject,
    exportunits = exportunits,
    nCol = nCol,
    nRow = nRow,
    nPanel
  )

  # check if legend adds to height or to width,
  # if legend is numeric, it is assumed the legend is within the panel
  if (plotDim$legendHeight > 0) {
    legendPosition <- themeOfPlot$legend.position
    legendAddsToHeight <- as.numeric(any(
      grepl(pattern = "Top", x = legendPosition, ignore.case = TRUE),
      grepl(pattern = "Bottom", x = legendPosition, ignore.case = TRUE)
    ))
    legendAddsToWidth <- as.numeric(any(
      grepl(pattern = "Left", x = legendPosition, ignore.case = TRUE),
      grepl(pattern = "Right", x = legendPosition, ignore.case = TRUE)
    ))
  } else {
    legendAddsToHeight <- 0
    legendAddsToWidth <- 0
  }

  if (!is.null(themeOfPlot$plot.margin)) {
    plotMargins <- grid::convertUnit(
      themeOfPlot$plot.margin,
      unitTo = exportunits,
      valueOnly = TRUE
    )
  } else {
    plotMargins <- c(0, 0, 0, 0)
  }

  # scale height with aspect ratio taking into account axis width and heights
  heightOffset <- plotDim$axisHeight +
    plotDim$legendHeight * legendAddsToHeight +
    sum(plotMargins[c(1, 3)])
  widthOffset <- plotDim$axisWidth +
    sum(plotMargins[c(2, 4)]) +
    plotDim$legendWidth * legendAddsToWidth

  backgroundWidth <- plotDim$background

  height <- nRow /
    nCol *
    (backgroundWidth - widthOffset) *
    aspect.ratio +
    heightOffset

  # get scale factor
  scf <- width / backgroundWidth

  # Update width if top/bottom legend is too wide (add 5% to legend width to ensure all the entry content are displayed)
  scf <- max(1, (1.05 * plotDim$legendWidth * legendAddsToHeight) / width) * scf

  return(list(
    width = backgroundWidth * scf,
    height = height * scf,
    heightOffset = heightOffset * scf,
    widthOffset = widthOffset * scf
  ))
}
#' Get dimensions of plot components
#'
#' @param plotObject A ggplot object.
#' @param exportunits Units of the exported figure.
#' @param nCol Number of columns for panel plots.
#' @param nRow Number of rows for panel plots.
#' @param nPanel Number of panels for panel plots.
#'
#' @return A list with dimensions of plot components.
#' @keywords internal
getPlotDimensions <- function(plotObject, exportunits, nCol, nRow, nPanel) {
  plot <- cowplot::as_gtable(plotObject)
  grobNames <- cowplot::plot_component_names(plot)
  grobs <- cowplot::plot_components(plot)

  .getGrobDimForPattern <- function(patterns, selectedDim, n = 1) {
    gIndexVector <- c()
    for (pattern in patterns) {
      gIndexVector <- c(gIndexVector, grep(pattern, grobNames))
    }

    values <- c()
    for (gIndex in gIndexVector) {
      if (selectedDim %in% names(grobs[[gIndex]])) {
        values <- c(
          values,
          as.numeric(grid::convertUnit(
            grobs[[gIndex]][[selectedDim]],
            exportunits
          ))
        )
      }
    }
    if (length(values) == 0) {
      return(0)
    }

    if (pattern %in% c("axis-t", "axis-b", "axis-r", "axis-l")) {
      gropDim <- ceiling(length(values) / n) * mean(values)
    } else if (pattern %in% c("strip-t", "strip-b", "strip-r", "strip-l")) {
      gropDim <- n * mean(values)
    } else {
      gropDim <- sum(values)
    }

    return(gropDim)
  }

  return(list(
    axisHeight = .getGrobDimForPattern(
      patterns = c("axis-t", "axis-b"),
      selectedDim = "height",
      n = nCol
    ) +
      .getGrobDimForPattern(
        patterns = c("xlab-t", "xlab-b"),
        selectedDim = "heights"
      ) +
      .getGrobDimForPattern(
        patterns = c("strip-t", "strip-b"),
        selectedDim = "heights",
        n = nRow
      ),
    axisWidth = .getGrobDimForPattern(
      patterns = c("axis-r", "axis-l"),
      selectedDim = "width",
      n = nRow
    ) +
      .getGrobDimForPattern(
        patterns = c("ylab-l", "ylab-r"),
        selectedDim = "widths"
      ) +
      .getGrobDimForPattern(
        patterns = c("strip-r", "strip-l"),
        selectedDim = "widths",
        n = nCol
      ),
    legendWidth = .getGrobDimForPattern(
      patterns = c("guide-box"),
      selectedDim = "widths"
    ),
    legendHeight = .getGrobDimForPattern(
      patterns = c("guide-box"),
      selectedDim = "heights"
    ),
    background = .getGrobDimForPattern(
      patterns = c("background"),
      selectedDim = "width"
    )
  ))
}
#' Replace special letters in file names
#'
#' @param filename Name of the file to validate.
#' @param device A character with the device to use
#'
#' @keywords internal
#' @return File name without special letters.
validateFilename <- function(filename, device) {
  # Validate input parameters
  checkmate::assertCharacter(filename, len = 1, null.ok = FALSE)
  checkmate::assertCharacter(device, len = 1, null.ok = TRUE)

  # if  option is set overwrite file extension.
  if (is.null(device)) {
    device <- getOspsuite.plots.option(optionKey = OptionKeys$exportDevice)
  }

  # Validate that device is a supported format
  supportedDevices <- c("png", "pdf", "jpeg", "jpg", "tiff", "svg", "eps")
  if (!device %in% supportedDevices) {
    warning(messages$warningDeviceMayNotBeSupported(device))
  }

  filename <- fs::path_ext_set(filename, device)
  initialFilename <- filename

  # replace µ by u
  filename <- iconv(filename, from = "UTF-8", to = "UTF-8")
  filename <- gsub("\u00B5", "u", filename, fixed = TRUE)

  # Replace forbidden characters with the replacement character
  forbiddenChars <- c(":", "*", "?", "<", ">", "|", "\\", "/")
  replacementChar <- "_"
  for (char in forbiddenChars) {
    filename <- gsub(char, replacementChar, filename, fixed = TRUE)
  }
  if (filename != initialFilename) {
    warning(messages$warningUpdatedFilename(filename, initialFilename))
  }
  return(filename)
}
