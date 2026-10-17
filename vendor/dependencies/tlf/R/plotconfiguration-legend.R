#' @title LegendConfiguration
#' @description R6 class defining the legend configuration of a `ggplot` object
#' @export
#' @family PlotConfiguration classes
LegendConfiguration <- R6::R6Class(
  "LegendConfiguration",
  public = list(
    #' @description Create a new `LegendConfiguration` object
    #' @param position position of the legend as defined by enum `LegendPositions`
    #' @param caption data.frame containing the properties of the legend caption
    #' @param title character or `Label` object defining the title of the legend. A value of `NULL` removes the title.
    #' @param font `Font` object defining the font of the legend caption
    #' @param background `BackgroundElement` object defining the background of the legend
    #' @return A new `LegendConfiguration` object
    initialize = function(position = NULL,
                          caption = NULL,
                          title = NULL,
                          font = NULL,
                          background = NULL) {
      validateIsIncluded(position, LegendPositions, nullAllowed = TRUE)
      validateIsOfType(title, c("character", "Label"), nullAllowed = TRUE)
      validateIsOfType(font, "Font", nullAllowed = TRUE)
      validateIsOfType(background, "BackgroundElement", nullAllowed = TRUE)

      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)
      private$.position <- position %||% currentTheme$background$legendPosition
      private$.font <- font %||% currentTheme$fonts$legend
      private$.background <- background %||% currentTheme$background$legend

      # Title properties
      if (!isOfType(title, "Label")) {
        title <- asLabel(
          text = title %||% currentTheme$background$legendTitle,
          font = currentTheme$fonts$legendTitle
        )
      }
      private$.title <- title

      private$.caption <- caption %||% data.frame()
    },

    #' @description Update legend configuration on a `ggplot` object
    #' @param plotObject `ggplot` object
    #' @return A `ggplot` object with updated axis properties
    updatePlot = function(plotObject) {
      validateIsOfType(plotObject, "ggplot")
      # Update legend background, font and title font
      plotObject <- plotObject +
        ggplot2::theme(
          legend.background = private$.background$createPlotElement(),
          legend.text = private$.font$createPlotTextFont(),
          # symbol background same as legend background
          legend.key = private$.background$createPlotElement(linetype = Linetypes$blank),
          # Ensure legend keys are wide enough to display dashed or dotted lines
          legend.key.width = grid::unit(2, "lines")
        )
      # Update legend title for all aesthetic properties to prevent unwanted split of legends
      updateLegendTitleExpression <- parse(text = paste0(
        "plotObject <- plotObject + ggplot2::guides(",
        names(AestheticProperties), " = guide_legend(",
        "title = private$.title$text,",
        "title.theme = private$.title$createPlotTextFont())",
        ")"
      ))
      eval(updateLegendTitleExpression)

      # Update legend position and alignment
      legendPosition <- .createPlotLegendPosition(private$.position)


      if (is.numeric(c(legendPosition$xPosition, legendPosition$yPosition))) {
        plotObject <- plotObject + ggplot2::theme(
          legend.position = "inside",
          legend.position.inside = c(legendPosition$xPosition, legendPosition$yPosition),
          legend.justification = c(legendPosition$xJustification, legendPosition$yJustification),
          legend.direction = "vertical"
        )
      } else if (is.character(c(legendPosition$xPosition, legendPosition$yPosition))) {
        plotObject <- plotObject + ggplot2::theme(
          legend.position = c(legendPosition$xPosition, legendPosition$yPosition),
          legend.justification = c(legendPosition$xJustification, legendPosition$yJustification),
          legend.direction = "vertical"
        )
      }

      # TO DO: update caption properties before returning plot
      return(plotObject)
    }
  ),
  active = list(
    #' @field caption of legend defined as data.frame with caption properties
    caption = function(value) {
      if (missing(value)) {
        return(private$.caption)
      }
      validateIsOfType(value, c("data.frame", "list"))
      private$.caption <- value
      return(invisible())
    },
    #' @field position of legend as defined in Enum `LegendPositions`
    position = function(value) {
      if (missing(value)) {
        return(private$.position)
      }
      validateIsIncluded(value, LegendPositions)
      private$.position <- value
      return(invisible())
    },
    #' @field font `Font` object defining the font of the legend
    font = function(value) {
      if (missing(value)) {
        return(private$.font)
      }
      validateIsOfType(value, "Font", nullAllowed = TRUE)
      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)
      private$.font <- value %||% currentTheme$fonts$legend
      return(invisible())
    },

    #' @field background `Background` object defining the background of the legend
    background = function(value) {
      if (missing(value)) {
        return(private$.background)
      }
      validateIsOfType(value, "BackgroundElement", nullAllowed = TRUE)
      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)
      private$.background <- value %||% currentTheme$background$legend
      return(invisible())
    },
    #' @field title character defining title of the legend
    title = function(value) {
      if (missing(value)) {
        return(private$.title)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      if (isOfType(value, "Label")) {
        private$.title <- asLabel(value)
      }
      private$.title <- asLabel(value, font = private$.title$font)
      return(invisible())
    }
  ),
  private = list(
    .position = NULL,
    .title = NULL,
    .font = NULL,
    .background = NULL,
    .caption = NULL
  )
)
