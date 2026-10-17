#' Create a ggplot with an optional watermark
#'
#' This function creates a ggplot object and adds a watermark if the watermark option is enabled.
#' The watermark can be customized with various options such as position, angle, font size, color, and transparency.
#'
#' @param ... Arguments to be passed to `ggplot()`, such as data and aesthetics. This allows for flexibility in creating different types of plots.
#'
#' @return A ggplot object, which may have the class "ggWatermark" if the watermark is enabled. The object can be printed or further modified as needed.
#'
#' @details
#' If the watermark feature is enabled, the resulting ggplot object will include a watermark overlay when printed.
#' The watermark's properties are determined by options set in the Ospsuite plotting configuration.
#'
#' The following options can be used to customize the watermark:
#' - `watermarkLabel`: Text to be displayed as the watermark.
#' - `watermarkFormat`: A list with the following entries:
#'   - `x`: The x-coordinate for the watermark's position on the plot.
#'   - `y`: The y-coordinate for the watermark's position on the plot.
#'   - `angle`: The angle at which the watermark text is displayed (in degrees).
#'   - `fontsize`: The size of the font for the watermark text.
#'   - `color`: The color of the watermark text, specified in a valid color format (e.g., "red", "#FF0000").
#'   - `alpha`: The transparency level of the watermark text, ranging from 0 (completely transparent) to 1 (completely opaque).
#'
#' @examples
#' # Example usage with watermark enabled (watermark is enabled by default)
#' plotWithWatermark <- ggplotWithWatermark(data = mtcars, aes(x = wt, y = mpg)) +
#'   geom_point()
#' print(plotWithWatermark)
#'
#' # Example usage with watermark disabled
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkEnabled, value = FALSE)
#' plotWithoutWatermark <- ggplotWithWatermark(data = mtcars, aes(x = wt, y = mpg)) +
#'   geom_point()
#' print(plotWithoutWatermark)
#' # Reset options
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkEnabled, value = TRUE)
#'
#' # Example usage with customized watermark
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkLabel, value = "Custom Label")
#' watermarkFormat <- getOspsuite.plots.option(optionKey = OptionKeys$watermarkFormat)
#' watermarkFormat$color <- "red"
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkFormat, value = watermarkFormat)
#' plotWithCustomizedWatermark <- ggplotWithWatermark(data = mtcars, aes(x = wt, y = mpg)) +
#'   geom_point()
#' print(plotWithCustomizedWatermark)
#' # Reset options
#' setOspsuite.plots.option(
#'   optionKey = OptionKeys$watermarkFormat,
#'   value = getDefaultOptions()[[OptionKeys$watermarkFormat]]
#' )
#' setOspsuite.plots.option(
#'   optionKey = OptionKeys$watermarkLabel,
#'   value = getDefaultOptions()[[OptionKeys$watermarkLabel]]
#' )
#' @family watermark
#' @export
ggplotWithWatermark <- function(...) {
  plotObject <- ggplot(...)

  # Set new class to overwrite the print method for the ggplot object
  class(plotObject) <- c("ggWatermark", class(plotObject))

  return(plotObject)
}


#' Helper function to get watermark enabled status.
#' @keywords internal
.getWatermarkEnabled <- function() {
  watermarkStatus <- getOption("ospsuite.plots.watermarkEnabled", default = TRUE)
  checkmate::assert_flag(watermarkStatus)
  return(watermarkStatus)
}

#' Print method for ggWatermark objects
#'
#' This function customizes the printing of ggplot objects with the class "ggWatermark" by adding a watermark.
#'
#' @param x A ggWatermark object created by `ggplotWithWatermark()`.
#' @param ... Additional arguments to be passed to the print method, allowing for further customization of the output.
#' @return A ggplot object with a watermark drawn on it. The watermark is displayed according to the specified options.
#'
#' @family watermark
#' @export
print.ggWatermark <- function(x, ...) {
  watermarkEnabled <- .getWatermarkEnabled()

  if (watermarkEnabled) {
    # Add watermark overlay when watermark is enabled
    print(addWatermark(x))
  } else {
    # When watermark is disabled, use ggdraw to reset class back to ggplot2
    # This prevents infinite recursion in print method calls
    print(cowplot::ggdraw(x))
  }
}
#' Add a watermark to a ggplot object
#'
#' This function adds a customizable watermark to a ggplot object. The watermark can be configured with various options such as position, angle, font size, color, and transparency.
#'
#' @param plotObject A ggplot object to which the watermark will be added.
#'
#' @return A ggplot object with a watermark drawn on it. The watermark is displayed according to the specified options.
#'
#' @examples
#' \dontrun{
#' # Example usage (watermark enabled by default)
#' p <- ggplot(mtcars, aes(x = wt, y = mpg)) +
#'   geom_point()
#' p_with_watermark <- addWatermark(p)
#' print(p_with_watermark)
#'
#' # Example of customizing the watermark
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkLabel, value = "Custom Watermark")
#' watermarkFormat <- getOspsuite.plots.option(optionKey = OptionKeys$watermarkFormat)
#' watermarkFormat$x <- 0.5 # Centered horizontally
#' watermarkFormat$y <- 0.5 # Centered vertically
#' watermarkFormat$angle <- 45 # Rotated 45 degrees
#' watermarkFormat$fontsize <- 6 # Font size 6
#' watermarkFormat$color <- "blue" # Blue color
#' watermarkFormat$alpha <- 0.5 # 50% transparency
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkFormat, value = watermarkFormat)
#'
#' # Create plot with customized watermark
#' p_custom <- addWatermark(p)
#' print(p_custom)
#' }
#'
#' @export
#'
#' @family watermark
addWatermark <- function(plotObject) {
  # initialize variables to avoid check messages
  x <- y <- label <- NULL

  checkmate::assert_class(plotObject, classes = "gg")

  # Check if watermarkEnabled option is set and get its value
  watermarkEnabled <- .getWatermarkEnabled()

  # if watermark is not enabled return unchanged object
  if (!watermarkEnabled) {
    return(plotObject)
  }

  watermarkLabel <- getOspsuite.plots.option(optionKey = OptionKeys$watermarkLabel)
  watermarkOptions <- getOspsuite.plots.option(optionKey = OptionKeys$watermarkFormat)

  # Validate watermark options
  checkmate::assertCharacter(watermarkLabel, len = 1, null.ok = FALSE)
  checkmate::assertList(watermarkOptions, null.ok = FALSE)
  checkmate::assertNumber(watermarkOptions$x, lower = 0, upper = 1)
  checkmate::assertNumber(watermarkOptions$y, lower = 0, upper = 1)
  checkmate::assertNumber(watermarkOptions$angle, lower = 0, upper = 360)
  checkmate::assertNumber(watermarkOptions$fontsize, lower = 0)
  checkmate::assertCharacter(watermarkOptions$color, len = 1)
  checkmate::assertNumber(watermarkOptions$alpha, lower = 0, upper = 1)

  cowplot::ggdraw(plotObject) +
    geom_text(
      data = data.frame(
        x = watermarkOptions$x,
        y = watermarkOptions$y,
        label = watermarkLabel
      ),
      aes(x, y, label = label),
      hjust = 0.5,
      vjust = 0.5,
      angle = watermarkOptions$angle,
      size = watermarkOptions$fontsize,
      color = watermarkOptions$color,
      alpha = watermarkOptions$alpha,
      inherit.aes = FALSE
    )
}


#' Create plot function for ggWatermark.
#'
#' This method allows for the ggWatermark object to be plotted using the standard print method.
#'
#' @param x A ggWatermark object to be printed.
#' @param ... Additional arguments to be passed to the print method.
#'
#' @family watermark
#' @export
plot.ggWatermark <- function(x, ...) {
  print(x, ...)
}
