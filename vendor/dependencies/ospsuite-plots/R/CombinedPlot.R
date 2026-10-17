#' @title CombinedPlot
#' @docType class
#' @description
#' This class represents a combined plot object that includes a plot and an optional table.
#' It provides methods to get and set the plot and table objects, as well as to print the combined output.
#' @examples
#' \dontrun{
#'
#' # Create a new CombinedPlot instance
#' combinedPlotInstance <- CombinedPlot$new(plotObject = myPlotObject, tableObject = myTableObject)
#'
#' # Print the combined plot and table
#' print(combinedPlotInstance)
#' # or simply
#' combinedPlotInstance
#' }
#' @export
CombinedPlot <- R6::R6Class(
  # nolint
  "CombinedPlot",
  cloneable = TRUE,
  public = list(
    #' @description Create a new `CombinedPlot` object.
    #' @param tableObject A ggplot object for the table.
    #' @param plotObject A ggplot object for the main plot.
    #' @return A new `CombinedPlot` object.
    initialize = function(plotObject = ggplot(), tableObject = NULL) {
      checkmate::assertClass(plotObject, "gg")
      checkmate::assertClass(tableObject, "gg", null.ok = TRUE)

      self$plotObject <- plotObject
      self$tableObject <- tableObject
      self$relWidths <- c(4, 1)
    },
    #' @description Combine the combined plot and table.
    #' This method combines the plot and table into a single output and displays it.
    #' @return A ggplot object representing the combined plot and table
    combined = function() {
      if (is.null(private$.tableObject)) {
        return(self$plotObject)
      }

      # Adjust legend position when table is present to optimize layout
      private$adjustLegendPosition()

      return(cowplot::plot_grid(
        self$plotObject,
        self$tableObject,
        nrow = 1,
        axis = "tb",
        align = "h",
        rel_widths = self$relWidths
      ))
    },
    #' @description Print the combined plot and table.
    #' This method overrides the default print function to display the combined output.
    #' @return Invisibly returns the combined ggplot object
    print = function() {
      combinedPlot <- self$combined()
      if (is.null(private$.tableObject)) {
        print(combinedPlot)
      } else if ("ggWatermark" %in% class(self$plotObject)) {
        # the combined plot has lost its watermarkClass
        print(addWatermark(combinedPlot))
      }
      invisible(combinedPlot)
    }
  ),
  active = list(
    #' @field plotObject A ggplot object representing the main plot.
    plotObject = function(value) {
      if (missing(value)) {
        return(private$.plotObject) # Get the plotObject
      } else {
        checkmate::assertClass(value, "gg")
        private$.plotObject <- value # Set the plotObject
      }
    },
    #' @field tableObject A ggplot object representing the table.
    tableObject = function(value) {
      if (missing(value)) {
        return(private$.tableObject) # Get the tableObject
      } else {
        checkmate::assertClass(value, "gg", null.ok = TRUE)
        private$.tableObject <- value # Set the tableObject
      }
    },
    #' @field relWidths  A numeric vector of length 2 specifying the relative widths of the plot and table.
    relWidths = function(value) {
      if (missing(value)) {
        return(private$.relWidths) # Get the relative width
      } else {
        checkmate::assertNumeric(value, lower = 0, len = 2)
        private$.relWidths <- value # Set the  relative width
      }
    }
  ),
  private = list(
    .plotObject = NULL,
    .tableObject = NULL,
    .relWidths = NULL,

    # Helper function to adjust legend position for better layout when table is present
    adjustLegendPosition = function() {
      # Get current legend position from the plot's own theme, falling back to
      # the OSPSuite theme rather than the global ggplot2 state.
      currentLegendPos <- private$.plotObject$theme$legend.position
      if (is.null(currentLegendPos)) {
        currentLegendPos <- theme_osp()$legend.position
      }

      # Move legend to top if currently on right side to accommodate table
      if (identical(currentLegendPos, "right")) {
        private$.plotObject <- private$.plotObject +
          theme(legend.position = "top")
      }
    }
  )
)
