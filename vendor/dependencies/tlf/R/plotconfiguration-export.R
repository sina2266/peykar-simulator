#' @title ExportConfiguration
#' @description R6 class defining properties for saving a `ggplot` object
#' @field name character defining the name of the file to be saved (without extension)
#' @field path Path of the directory to save plot to: path and filename are
#' combined to create the fully qualified file name. Defaults to the working directory.
#' @field format character defining the format of the file to be saved
#' @field width numeric values defining the width in `units` of the plot dimensions after saving
#' @field height numeric values defining the height in `units` of the plot dimensions after saving
#' @field units character defining the unit of the saving dimension
#' @field dpi (dots per inch) numeric value defining plot resolution
#' @export
#' @import ggplot2
#' @family PlotConfiguration classes
ExportConfiguration <- R6::R6Class(
  "ExportConfiguration",
  public = list(
    name = NULL,
    path = NULL,
    format = NULL,
    width = NULL,
    height = NULL,
    units = NULL,
    dpi = NULL,

    #' @description Create a new `ExportConfiguration` object
    #' @param name character defining the name of the file to be saved (without extension)
    #' @param path Path of the directory to save plot to: path and filename are
    #' combined to create the fully qualified file name. Defaults to the working directory.
    #' @param format character defining the format of the file to be saved.
    #' @param width numeric values defining the width in `units` of the plot dimensions after saving
    #' @param height numeric values defining the height in `units` of the plot dimensions after saving
    #' @param units character defining the unit of the saving dimension
    #' @param dpi numeric value defining plot resolution (dots per inch)
    #' @return A new `ExportConfiguration` object
    initialize = function(path = NULL,
                          name = NULL,
                          format = NULL,
                          width = NULL,
                          height = NULL,
                          units = NULL,
                          dpi = NULL) {
      validateIsString(name, nullAllowed = TRUE)
      validateIsString(format, nullAllowed = TRUE)
      validateIsIncluded(units, c("cm", "in", "mm", "px"), nullAllowed = TRUE)
      validateIsNumeric(width, nullAllowed = TRUE)
      validateIsNumeric(height, nullAllowed = TRUE)
      validateIsNumeric(dpi, nullAllowed = TRUE)

      inputs <- c("path", "name", "format", "width", "height", "units", "dpi")
      associateExpressions <- parse(text = paste0("self$", inputs, " <- ", inputs, " %||% tlfEnv$defaultExportParameters$", inputs))
      eval(associateExpressions)
    },

    #' @description Print properties of export configuration
    #' @return Export configuration properties
    print = function() {
      exportDimensions <- c(
        paste0("Format: ", self$format),
        paste0("Width: ", self$width, " ", self$units),
        paste0("Height: ", self$height, " ", self$units),
        paste0("Resolution: ", self$dpi, " dots per inch")
      )
      cat(exportDimensions, sep = "\n")
      return(invisible(exportDimensions))
    },

    #' @description Print the default exported file name from the export configuration
    #' @return Default file name
    getFileName = function() {
      paste0(self$name, ".", self$format)
    },

    #' @description Save/Export a plot
    #' @param plotObject A `ggplot` object
    #' @param fileName character file name of the exported plot
    #' @return The file name of the exported plot
    savePlot = function(plotObject, fileName = NULL) {
      validateIsOfType(plotObject, "ggplot")
      validateIsString(fileName, nullAllowed = TRUE)
      fileName <- fileName %||% self$getFileName()

      # If unit is in pixels, convert dimensions to inches to keep compatibility with older versions of ggplot2
      self$convertPixels()

      # When exporting a plot using tlf
      # Match point size dpi to file dpi
      if (requireNamespace("showtext", quietly = TRUE)) {
        currentDPI <- showtext::showtext_opts()$dpi
        showtext::showtext_opts(dpi = self$dpi)
      }
      tryCatch(
        {
          ggplot2::ggsave(
            path = self$path,
            filename = fileName,
            plot = plotObject,
            width = self$width,
            height = self$height,
            units = self$units,
            dpi = self$dpi,
            create.dir = TRUE
          )
        },
        error = function(e) {
          stop(e)
        },
        finally = {
          # Then, revert to current dpi
          if (requireNamespace("showtext", quietly = TRUE)) {
            showtext::showtext_opts(dpi = currentDPI)
          }
        }
      )
      # Return the file name in case users needs it
      # Use invisible to prevent writing the file name in the console every time a plot is exported
      return(invisible(fileName))
    },

    #' @description If unit is in pixels, convert all export dimensions to inches to keep compatibility with older versions of ggplot2
    convertPixels = function() {
      if (!isIncluded(self$units, "px")) {
        return(invisible())
      }
      unitConversionFactor <- grDevices::dev.size("in") / grDevices::dev.size("px")
      self$units <- "in"
      self$width <- self$width * unitConversionFactor[1]
      self$height <- self$height * unitConversionFactor[2]
      return(invisible())
    }
  )
)
