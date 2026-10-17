#' @title BackgroundConfiguration
#' @description R6 class defining the configuration of background
#' @export
#' @family PlotConfiguration classes
BackgroundConfiguration <- R6::R6Class(
  "BackgroundConfiguration",
  public = list(
    #' @description Create a new `BackgroundConfiguration` object
    #' @param watermark `Label` object defining properties of watermark
    #' @param plot `BackgroundElement` object defining outside plot background properties
    #' @param panel `BackgroundElement` object defining panel (inside of plot) background properties
    #' @param xAxis `LineElement` object defining properties of x-axis
    #' @param yAxis `LineElement` object defining properties of y-axis
    #' @param y2Axis `LineElement` object defining properties of right y-axis
    #' @param xGrid `LineElement` object defining properties of x-grid
    #' @param yGrid `LineElement` object defining properties of y-grid
    #' @param y2Grid `LineElement` object defining properties of right y-grid
    #' @return A new `BackgroundConfiguration` object
    initialize = function(watermark = NULL,
                          plot = NULL,
                          panel = NULL,
                          xAxis = NULL,
                          yAxis = NULL,
                          y2Axis = NULL,
                          xGrid = NULL,
                          yGrid = NULL,
                          y2Grid = NULL) {
      validateIsOfType(watermark, c("character", "Label"), nullAllowed = TRUE)
      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)
      watermark <- watermark %||% currentTheme$background$watermark
      # Enforce watermark as Label with value
      if (isOfType(watermark, "character")) {
        watermark <- asLabel(text = watermark, font = currentTheme$fonts$watermark)
      }
      private$.watermark <- watermark

      areaFieldNames <- c("plot", "panel")
      lineFieldNames <- c("xAxis", "yAxis", "y2Axis", "xGrid", "yGrid", "y2Grid")

      validateAreaExpression <- parse(text = paste0("validateIsOfType(", areaFieldNames, ", 'BackgroundElement', nullAllowed = TRUE)"))
      validateLineExpression <- parse(text = paste0("validateIsOfType(", lineFieldNames, ", 'LineElement', nullAllowed = TRUE)"))
      eval(validateAreaExpression)
      eval(validateLineExpression)

      setAreaExpression <- parse(text = paste0("self$", areaFieldNames, " <- ", areaFieldNames, " %||% currentTheme$background$", areaFieldNames))
      setLineExpression <- parse(text = paste0("self$", lineFieldNames, " <- ", lineFieldNames, " %||% currentTheme$background$", lineFieldNames))
      eval(setAreaExpression)
      eval(setLineExpression)
    },

    #' @description Update background a `ggplot` object from `BackgroundConfiguration` properties
    #' @param plotObject a `ggplot` object
    #' @return A `ggplot` object
    updatePlot = function(plotObject) {
      plotObject <- plotObject + ggplot2::theme(
        plot.background = private$.plot$createPlotElement(),
        panel.background = private$.panel$createPlotElement(),
        axis.line.x = private$.xAxis$createPlotElement(),
        axis.line.y = private$.yAxis$createPlotElement(),
        panel.grid.major.x = private$.xGrid$createPlotElement(),
        panel.grid.major.y = private$.yGrid$createPlotElement(),
        # Minor grid is same as Major grid but less thick
        panel.grid.minor.x = private$.xGrid$createPlotElement(size = as.numeric(private$.xGrid$size) / 2),
        panel.grid.minor.y = private$.yGrid$createPlotElement(size = as.numeric(private$.yGrid$size) / 2)
      )
      return(plotObject)
    }
  ),
  active = list(
    #' @field watermark `Label` object
    watermark = function(value) {
      if (missing(value)) {
        return(private$.watermark)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      value <- value %||% private$.watermark$text
      # Enforce watermark as Label with value
      if (isOfType(value, "character")) {
        value <- asLabel(text = value, font = private$.watermark$font)
      }
      private$.watermark <- value
      return(invisible())
    },
    #' @field plot `BackgroundElement` object
    plot = function(value) {
      if (missing(value)) {
        return(private$.plot)
      }
      validateIsOfType(value, "BackgroundElement", nullAllowed = TRUE)
      private$.plot <- value %||% private$.plot
    },
    #' @field panel `BackgroundElement` object
    panel = function(value) {
      if (missing(value)) {
        return(private$.panel)
      }
      validateIsOfType(value, "BackgroundElement", nullAllowed = TRUE)
      private$.panel <- value %||% private$.panel
    },
    #' @field xAxis `LineElement` object
    xAxis = function(value) {
      if (missing(value)) {
        return(private$.xAxis)
      }
      validateIsOfType(value, "LineElement", nullAllowed = TRUE)
      private$.xAxis <- value %||% private$.xAxis
    },
    #' @field yAxis `LineElement` object
    yAxis = function(value) {
      if (missing(value)) {
        return(private$.yAxis)
      }
      validateIsOfType(value, "LineElement", nullAllowed = TRUE)
      private$.yAxis <- value %||% private$.yAxis
    },
    #' @field y2Axis `LineElement` object
    y2Axis = function(value) {
      if (missing(value)) {
        return(private$.y2Axis)
      }
      validateIsOfType(value, "LineElement", nullAllowed = TRUE)
      private$.y2Axis <- value %||% private$.y2Axis
    },
    #' @field xGrid `LineElement` object
    xGrid = function(value) {
      if (missing(value)) {
        return(private$.xGrid)
      }
      validateIsOfType(value, "LineElement", nullAllowed = TRUE)
      private$.xGrid <- value %||% private$.xGrid
    },
    #' @field yGrid `LineElement` object
    yGrid = function(value) {
      if (missing(value)) {
        return(private$.yGrid)
      }
      validateIsOfType(value, "LineElement", nullAllowed = TRUE)
      private$.yGrid <- value %||% private$.yGrid
    },
    #' @field y2Grid `LineElement` object
    y2Grid = function(value) {
      if (missing(value)) {
        return(private$.y2Grid)
      }
      validateIsOfType(value, "LineElement", nullAllowed = TRUE)
      private$.y2Grid <- value %||% private$.y2Grid
    }
  ),
  private = list(
    .watermark = NULL,
    .plot = NULL,
    .panel = NULL,
    .xAxis = NULL,
    .yAxis = NULL,
    .y2Axis = NULL,
    .xGrid = NULL,
    .yGrid = NULL,
    .y2Grid = NULL
  )
)

#' @title BackgroundElement
#' @description  R6 class defining the properties of background elements
#' @field fill character defining the color filling of the background element
#' @field color character defining the color of the background element frame/line
#' @field size numeric defining the size of the background element frame/line
#' @field linetype character defining the size of the background element frame/line
#' @export
#' @family PlotConfiguration classes
BackgroundElement <- R6::R6Class(
  "BackgroundElement",
  public = list(
    fill = NULL,
    color = NULL,
    size = NULL,
    linetype = NULL,

    #' @description Create a new `BackgroundElement` object
    #' @param fill character color filling of the background element
    #' @param color character color of the frame of the background element
    #' @param size character size of the frame of the background element
    #' @param linetype character linetype of the frame of the background element
    #' @return A new `BackgroundElement` object
    initialize = function(fill = NULL,
                          color = NULL,
                          size = NULL,
                          linetype = NULL) {
      validateIsString(c(fill, color, linetype), nullAllowed = TRUE)
      validateIsNumeric(size, nullAllowed = TRUE)

      fieldNames <- c("fill", "color", "size", "linetype")
      setPropertiesExpression <- parse(text = paste0("self$", fieldNames, " <- ", fieldNames))
      eval(setPropertiesExpression)
    },

    #' @description Create a `ggplot2::element_rect`  directly usable by `ggplot2::theme`.
    #' @param fill character color filling of the background element
    #' @param color character color of the frame of the background element
    #' @param size character size of the frame of the background element
    #' @param linetype character linetype of the frame of the background element
    #' @return An `element_rect` object.
    createPlotElement = function(fill = NULL, color = NULL, size = NULL, linetype = NULL) {
      ggplot2::element_rect(
        fill = fill %||% self$fill,
        colour = color %||% self$color,
        linewidth = size %||% as.numeric(self$size),
        linetype = linetype %||% self$linetype
      )
    }
  )
)

#' @title LineElement
#' @description  R6 class defining the properties of background line elements
#' @export
#' @family PlotConfiguration classes
LineElement <- R6::R6Class(
  "LineElement",
  inherit = BackgroundElement,
  public = list(
    #' @description Create a `ggplot2::element_line` directly usable by `ggplot2::theme`.
    #' @param color character color of the frame of the background element
    #' @param size character size of the frame of the background element
    #' @param linetype character linetype of the frame of the background element
    #' @return An `element_line` object.
    createPlotElement = function(color = NULL, size = NULL, linetype = NULL) {
      ggplot2::element_line(
        colour = color %||% self$color,
        linewidth = size %||% as.numeric(self$size),
        linetype = linetype %||% self$linetype
      )
    }
  )
)
