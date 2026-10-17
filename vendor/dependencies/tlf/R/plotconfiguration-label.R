#' @title LabelConfiguration
#' @description R6 class defining the configuration of the labels of a `ggplot` object
#' @export
#' @family PlotConfiguration classes
LabelConfiguration <- R6::R6Class(
  "LabelConfiguration",
  public = list(
    #' @description Create a new `LabelConfiguration` object
    #' @param title character or `Label` object defining title
    #' @param subtitle character or `Label` object defining subtitle
    #' @param xlabel character or `Label` object defining xlabel
    #' @param ylabel character or `Label` object defining ylabel
    #' @param caption character or `Label` object defining caption
    #' @return A new `LabelConfiguration` object
    initialize = function(title = NULL,
                          subtitle = NULL,
                          xlabel = NULL,
                          ylabel = NULL,
                          caption = NULL) {
      # y2label not available for all plots but time profile
      y2label <- NULL
      inputs <- c("title", "subtitle", "xlabel", "ylabel", "caption", "y2label")

      labels <- list(
        "title" = title,
        "subtitle" = subtitle,
        "xlabel" = xlabel,
        "ylabel" = ylabel,
        "caption" = caption,
        "y2label" = y2label
      )

      # Check label type is either a character or `Label` object
      lapply(labels, function(x) {
        validateIsOfType(x, c("Label", "character"), nullAllowed = TRUE)
      })

      if (isOfType(labels$ylabel, "Label")) {
        if (labels$ylabel$font$angle %% 360 %in% c(0, 180)) {
          labels$ylabel$font$maxWidth <- unit(100, "pt")
        }
        if (labels$ylabel$font$angle %% 360 %in% c(90, 270)) {
          labels$ylabel$font$maxWidth <- NULL
        }
      }

      if (isOfType(labels$xlabel, "Label")) {
        if (labels$xlabel$font$angle %% 360 %in% c(0, 180)) {
          labels$xlabel$font$maxWidth <- NULL
        }
        if (labels$xlabel$font$angle %% 360 %in% c(90, 270)) {
          labels$xlabel$font$maxWidth <- unit(100, "pt")
        }
      }

      currentTheme <- tlfEnv$currentTheme$clone(deep = TRUE)

      for (labelName in names(labels)) {
        label <- labels[[labelName]]
        if (!isOfType(label, "Label")) {
          label <- asLabel(text = label, font = currentTheme$fonts[[labelName]])
        }
        # Check chosen angle is available
        # use modulo 360 to in case minus angles were provided
        label$font$angle <- .checkIsInAvailableAngles(label$font$angle %% 360)
        private[[paste0(".", labelName)]] <- asLabel(label)
      }
    },

    #' @description Update labels of a `ggplot` object and their properties
    #' @param plotObject a `ggplot` object
    #' @return A `ggplot` object
    updatePlot = function(plotObject) {
      validateIsOfType(plotObject, "ggplot")
      # Update titles and axes labels
      plotObject <- plotObject + ggplot2::labs(
        title = private$.title$text,
        subtitle = private$.subtitle$text,
        x = private$.xlabel$text,
        y = private$.ylabel$text,
        caption = private$.caption$text
      )

      plotObject <- plotObject +
        ggplot2::theme(
          plot.title = private$.title$createPlotTextBoxFont(),
          plot.subtitle = private$.subtitle$createPlotTextBoxFont(),
          axis.title.x = private$.xlabel$createPlotTextBoxFont(),
          axis.title.y = private$.ylabel$createPlotTextBoxFont(),
          axis.title.y.right = private$.y2label$createPlotTextBoxFont(),
          plot.caption = private$.caption$createPlotTextBoxFont()
        )
      return(plotObject)
    }
  ),
  active = list(
    #' @field title `Label` object defining the title of the plot
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
    },
    #' @field subtitle `Label` object defining the subtitle of the plot
    subtitle = function(value) {
      if (missing(value)) {
        return(private$.subtitle)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      if (isOfType(value, "Label")) {
        private$.subtitle <- asLabel(value)
      }
      private$.subtitle <- asLabel(value, font = private$.subtitle$font)
      return(invisible())
    },
    #' @field xlabel `Label` object defining the xlabel of the plot
    xlabel = function(value) {
      if (missing(value)) {
        return(private$.xlabel)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      if (isOfType(value, "Label")) {
        private$.xlabel <- asLabel(value)
      }
      private$.xlabel <- asLabel(value, font = private$.xlabel$font)
      return(invisible())
    },
    #' @field ylabel `Label` object defining the ylabel of the plot
    ylabel = function(value) {
      if (missing(value)) {
        return(private$.ylabel)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      if (isOfType(value, "Label")) {
        private$.ylabel <- asLabel(value)
      }
      private$.ylabel <- asLabel(value, font = private$.ylabel$font)
      return(invisible())
    },
    #' @field caption `Label` object defining the caption of the plot
    caption = function(value) {
      if (missing(value)) {
        return(private$.caption)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      if (isOfType(value, "Label")) {
        private$.caption <- asLabel(value)
      }
      private$.caption <- asLabel(value, font = private$.caption$font)
      return(invisible())
    },
    #' @field y2label `Label` object defining the y2label of the plot
    y2label = function(value) {
      if (missing(value)) {
        return(private$.y2label)
      }
      validateIsOfType(value, c("character", "Label"), nullAllowed = TRUE)
      if (isOfType(value, "Label")) {
        private$.y2label <- asLabel(value)
      }
      private$.y2label <- asLabel(value, font = private$.y2label$font)
      return(invisible())
    }
  ),
  private = list(
    .title = NULL,
    .subtitle = NULL,
    .xlabel = NULL,
    .ylabel = NULL,
    .caption = NULL,
    .y2label = NULL
  )
)
