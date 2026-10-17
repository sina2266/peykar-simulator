#' @title PlotConfiguration
#' @description R6 class defining the configuration of a `ggplot` object
#' @field export R6 class `ExportConfiguration` defining properties for saving/exporting plot
#' @field defaultXScale Default xAxis scale value when creating a `PlotConfiguration` object
#' @field defaultYScale Default yAxis scale value when creating a `PlotConfiguration` object
#' @field defaultExpand Default expand value when creating a `PlotConfiguration` object
#' @field defaultSymmetricAxes Default option setting symmetric xAxis and/or yAxis limits when creating a `PlotConfiguration` object
#' @family PlotConfiguration classes
#' @references For examples, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/plot-configuration.html>
#' @export
PlotConfiguration <- R6::R6Class(
  "PlotConfiguration",
  public = list(
    export = NULL,
    # Caution, helper enum Scaling doesn't work here
    # (even using @include to collate and define the variable beforehand)
    defaultXScale = "lin",
    defaultYScale = "lin",
    defaultExpand = FALSE,
    defaultSymmetricAxes = FALSE,

    #' @description Create a new `PlotConfiguration` object
    #' @param title character or `Label` object defining plot title
    #' @param subtitle character or `Label` object defining plot subtitle
    #' @param xlabel character or `Label` object defining plot xlabel
    #' @param ylabel character or `Label` object defining plot ylabel
    #' @param caption character or `Label` object defining plot caption
    #' @param legend `LegendConfiguration` object defining legend properties
    #' @param legendTitle character or `Label` object defining legend title
    #' @param legendPosition character defining legend position.
    #' Use Enum `LegendPositions` to get a list of available to legend positions.
    #' @param xAxis `XAxisConfiguration` object defining x-axis properties
    #' @param xScale name of X-axis scale. Use enum `Scaling` to access predefined scales.
    #' @param xValuesLimits numeric vector of length 2 defining x values limits
    #' @param xAxisLimits numeric vector of length 2 defining x-axis limits
    #' @param xLimits `r lifecycle::badge("deprecated")`. Replaced by xAxisLimits argument.
    #' @param yAxis `YAxisConfiguration` object defining y-axis properties
    #' @param yScale name of y-axis scale. Use enum `Scaling` to access predefined scales.
    #' @param yValuesLimits numeric vector of length 2 defining x values limits
    #' @param yAxisLimits numeric vector of length 2 defining x-axis limits
    #' @param yLimits `r lifecycle::badge("deprecated")`. Replaced by yAxisLimits argument.
    #' @param background `BackgroundConfiguration` object defining background properties
    #' @param plotArea `BackgroundElement` object defining properties of plot area
    #' @param panelArea `BackgroundElement` object defining properties of panel area
    #' @param xGrid `LineElement` object defining properties of x-grid background
    #' @param yGrid `LineElement` object defining properties of y-grid background
    #' @param watermark character or `Label` object defining watermark
    #' @param export R6 class `ExportConfiguration` defining properties for saving/exporting plot
    #' @param name character defining the name of the file to be saved (without extension)
    #' @param format character defining the format of the file to be saved.
    #' @param width numeric values defining the width in `units` of the plot dimensions after saving
    #' @param height numeric values defining the height in `units` of the plot dimensions after saving
    #' @param units character defining the unit of the saving dimension
    #' @param dpi numeric value defining plot resolution (dots per inch)
    #' @param data data.frame used by `.smartMapping`
    #' @param metaData list of information on `data`
    #' @param dataMapping R6 class or subclass `XYDataMapping`
    #' @param lines `ThemeAestheticSelections` object or list defining how lines are plotted
    #' @param points `ThemeAestheticSelections` object or list defining how points are plotted
    #' @param ribbons `ThemeAestheticSelections` object or list defining how ribbons are plotted
    #' @param errorbars `ThemeAestheticSelections` object or list defining how errorbars are plotted
    #' @return A new `PlotConfiguration` object
    initialize = function( # Label configuration
                          title = NULL,
                          subtitle = NULL,
                          xlabel = NULL,
                          ylabel = NULL,
                          caption = NULL,
                          # Legend Configuration
                          legend = NULL,
                          legendTitle = NULL,
                          legendPosition = NULL,
                          # X-Axis configuration
                          xAxis = NULL,
                          xScale = NULL,
                          xValuesLimits = NULL,
                          xAxisLimits = NULL,
                          xLimits = lifecycle::deprecated(),
                          # Y-Axis configuration
                          yAxis = NULL,
                          yScale = NULL,
                          yValuesLimits = NULL,
                          yAxisLimits = NULL,
                          yLimits = lifecycle::deprecated(),
                          # Background configuration
                          background = NULL,
                          plotArea = NULL,
                          panelArea = NULL,
                          xGrid = NULL,
                          yGrid = NULL,
                          watermark = NULL,
                          # configuration of how objects are plotted
                          lines = NULL,
                          points = NULL,
                          ribbons = NULL,
                          errorbars = NULL,
                          # Export configuration
                          export = NULL,
                          name = NULL,
                          format = NULL,
                          width = NULL,
                          height = NULL,
                          units = NULL,
                          dpi = NULL,
                          # Smart configuration using metaData
                          data = NULL,
                          metaData = NULL,
                          dataMapping = NULL) {
      if (lifecycle::is_present(xLimits)) {
        lifecycle::deprecate_warn("1.5.0", "PlotConfiguration(xLimits)", "PlotConfiguration(xAxisLimits)")
        xAxisLimits <- yLimits
      }

      if (lifecycle::is_present(yLimits)) {
        lifecycle::deprecate_warn("1.5.0", "PlotConfiguration(yLimits)", "PlotConfiguration(yAxisLimits)")
        yAxisLimits <- yLimits
      }

      # Label configuration
      # validation of the input is done within the creation of the object
      private$.labels <- LabelConfiguration$new(
        title = title,
        subtitle = subtitle,
        xlabel = xlabel,
        ylabel = ylabel,
        caption = caption
      )

      # Smart configuration if xlabel and ylabel
      # 1) If xlabel and ylabel provided: use as is.
      # 2) Else, if data, metaData (and optionally dataMapping) provided: use dimension [unit] from metaData
      # 3) Else, if data (and optionally dataMapping) provided: use variable names from data
      if (!is.null(data)) {
        dataMapping <- dataMapping %||% XYGDataMapping$new(data = data)
      }
      private$.labels$xlabel <- asLabel(xlabel %||%
        .dataMappingLabel(dataMapping$x, metaData) %||%
        dataMapping$x %||%
        private$.labels$xlabel$text, font = private$.labels$xlabel$font)
      private$.labels$ylabel <- asLabel(ylabel %||%
        .dataMappingLabel(dataMapping$y, metaData) %||%
        dataMapping$y %||%
        .dataMappingLabel(dataMapping$ymax, metaData) %||%
        dataMapping$ymax %||%
        private$.labels$ylabel, font = private$.labels$ylabel$font)

      # Legend Configuration, overwrite some properties only if they are defined
      validateIsOfType(legend, "LegendConfiguration", nullAllowed = TRUE)
      private$.legend <- legend %||% LegendConfiguration$new()
      private$.legend$title <- legendTitle %||% private$.legend$title
      validateIsIncluded(legendPosition, LegendPositions, nullAllowed = TRUE)
      private$.legend$position <- legendPosition %||% private$.legend$position

      # X-Axis Configuration, overwrite some properties only if they are defined
      validateIsOfType(xAxis, "XAxisConfiguration", nullAllowed = TRUE)
      private$.xAxis <- xAxis %||% XAxisConfiguration$new(
        scale = self$defaultXScale,
        expand = self$defaultExpand
      )
      private$.xAxis$valuesLimits <- xValuesLimits %||% private$.xAxis$valuesLimits
      private$.xAxis$axisLimits <- xAxisLimits %||% private$.xAxis$axisLimits
      private$.xAxis$scale <- xScale %||% private$.xAxis$scale

      # Y-Axis configuration, overwrite some properties only if they are defined
      validateIsOfType(yAxis, "YAxisConfiguration", nullAllowed = TRUE)
      private$.yAxis <- yAxis %||% YAxisConfiguration$new(
        scale = self$defaultYScale,
        expand = self$defaultExpand
      )
      private$.yAxis$valuesLimits <- yValuesLimits %||% private$.yAxis$limits
      private$.yAxis$axisLimits <- yAxisLimits %||% private$.yAxis$axisLimits
      private$.yAxis$scale <- yScale %||% private$.yAxis$scale

      # Background configuration, overwrite some properties only if they are defined
      validateIsOfType(background, "BackgroundConfiguration", nullAllowed = TRUE)
      validateIsOfType(plotArea, "BackgroundElement", nullAllowed = TRUE)
      validateIsOfType(panelArea, "BackgroundElement", nullAllowed = TRUE)
      validateIsOfType(xGrid, "LineElement", nullAllowed = TRUE)
      validateIsOfType(yGrid, "LineElement", nullAllowed = TRUE)
      validateIsOfType(watermark, c("character", "Label"), nullAllowed = TRUE)

      private$.background <- background %||% BackgroundConfiguration$new()
      private$.background$plot <- plotArea %||% private$.background$plot
      private$.background$panel <- panelArea %||% private$.background$panel
      private$.background$xGrid <- xGrid %||% private$.background$xGrid
      private$.background$yGrid <- yGrid %||% private$.background$yGrid
      private$.background$watermark <- watermark %||% private$.background$watermark

      # Define how to plot points, lines, ribbons and errorbars of the plot
      private$.lines <- lines %||% .getThemePropertyFor(plotConfiguration = self, propertyName = "lines")
      private$.ribbons <- ribbons %||% .getThemePropertyFor(plotConfiguration = self, propertyName = "ribbons")
      private$.points <- points %||% .getThemePropertyFor(plotConfiguration = self, propertyName = "points")
      private$.errorbars <- errorbars %||% .getThemePropertyFor(plotConfiguration = self, propertyName = "errorbars")

      # Define export configuration, overwrite properties only if they are defined
      validateIsOfType(export, "ExportConfiguration", nullAllowed = TRUE)
      self$export <- export %||% ExportConfiguration$new()
      self$export$name <- name %||% self$export$name
      self$export$format <- format %||% self$export$format
      self$export$width <- width %||% self$export$width
      self$export$height <- height %||% self$export$height
      self$export$units <- units %||% self$export$units
      self$export$dpi <- dpi %||% self$export$dpi
    }
  ),
  active = list(
    #' @field labels `LabelConfiguration` object defining properties of labels
    labels = function(value) {
      if (missing(value)) {
        return(private$.labels)
      }
      validateIsOfType(value, "LabelConfiguration", nullAllowed = TRUE)
      private$.labels <- value %||% private$.labels
      return(invisible())
    },
    #' @field legend `LegendConfiguration` object defining properties of legend
    legend = function(value) {
      if (missing(value)) {
        return(private$.legend)
      }
      validateIsOfType(value, "LegendConfiguration", nullAllowed = TRUE)
      private$.legend <- value %||% private$.legend
      return(invisible())
    },
    #' @field xAxis `XAxisConfiguration` object defining properties of x-axis
    xAxis = function(value) {
      if (missing(value)) {
        return(private$.xAxis)
      }
      validateIsOfType(value, "XAxisConfiguration", nullAllowed = TRUE)
      private$.xAxis <- value %||% private$.xAxis
      return(invisible())
    },
    #' @field yAxis `YAxisConfiguration` object defining properties of x-axis
    yAxis = function(value) {
      if (missing(value)) {
        return(private$.yAxis)
      }
      validateIsOfType(value, "YAxisConfiguration", nullAllowed = TRUE)
      private$.yAxis <- value %||% private$.yAxis
      return(invisible())
    },
    #' @field background `BackgroundConfiguration` object defining properties of x-axis
    background = function(value) {
      if (missing(value)) {
        return(private$.background)
      }
      validateIsOfType(value, "BackgroundConfiguration", nullAllowed = TRUE)
      private$.background <- value %||% private$.background
      return(invisible())
    },
    #' @field lines `ThemeAestheticSelections` defining properties of lines
    lines = function(value) {
      if (missing(value)) {
        return(private$.lines)
      }
      validateIsOfType(value, "ThemeAestheticSelections", nullAllowed = TRUE)
      private$.lines <- value %||% private$.lines
      return(invisible())
    },
    #' @field ribbons `ThemeAestheticSelections` defining properties of ribbons
    ribbons = function(value) {
      if (missing(value)) {
        return(private$.ribbons)
      }
      validateIsOfType(value, "ThemeAestheticSelections", nullAllowed = TRUE)
      private$.ribbons <- value %||% private$.ribbons
      return(invisible())
    },
    #' @field points `ThemeAestheticSelections` defining properties of points
    points = function(value) {
      if (missing(value)) {
        return(private$.points)
      }
      validateIsOfType(value, "ThemeAestheticSelections", nullAllowed = TRUE)
      private$.points <- value %||% private$.points
      return(invisible())
    },
    #' @field errorbars `ThemeAestheticSelections` defining properties of error bars
    errorbars = function(value) {
      if (missing(value)) {
        return(private$.errorbars)
      }
      validateIsOfType(value, "ThemeAestheticSelections", nullAllowed = TRUE)
      private$.errorbars <- value %||% private$.errorbars
      return(invisible())
    }
  ),
  private = list(
    .labels = NULL,
    .legend = NULL,
    .xAxis = NULL,
    .yAxis = NULL,
    .background = NULL,
    .lines = NULL,
    .ribbons = NULL,
    .points = NULL,
    .errorbars = NULL
  )
)
