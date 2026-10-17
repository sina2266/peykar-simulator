#' @title Label
#' @description  R6 class defining `text` and `font` of labels
#' @export
Label <- R6::R6Class(
  "Label",
  public = list(
    #' @description Create a new `Label` object.
    #' @param text character text of the label
    #' @param font `Font` object defining the font of the label
    #' @param size numeric defining the size of the label
    #' @param color character defining the color of the label
    #' @param fontFamily character defining the font family of the label
    #' @param fontFace character defining the font face of the label as defined in helper enum `FontFaces`.
    #' @param angle numeric defining the angle of the label.
    #' @param align character defining the alignment of the label as defined in helper enum `Alignments`.
    #' @param maxWidth numeric that will be converted to a ggplot2::unit object (in "pt" unit) defining the maximum width of text box.
    #' @param margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
    #' @return A new `Label` object
    initialize = function(text = "",
                          font = NULL,
                          color = NULL,
                          size = NULL,
                          fontFace = NULL,
                          fontFamily = NULL,
                          angle = NULL,
                          align = NULL,
                          maxWidth = NULL,
                          margin = NULL) {
      validateIsNumeric(as.numeric(size, angle), nullAllowed = TRUE)
      validateIsString(c(color, fontFamily), nullAllowed = TRUE)
      validateIsOfType(font, "Font", nullAllowed = TRUE)
      validateIsIncluded(fontFace, FontFaces, nullAllowed = TRUE)
      validateIsIncluded(align, Alignments, nullAllowed = TRUE)


      self$text <- .sanitizeLabel(text)

      self$font <- font %||% Font$new()
      # If font properties are explicitely written, they will overwrite the properties of input Font
      eval(.parseVariableToObject("self$font", c("size", "color", "fontFace", "fontFamily", "angle", "align", "maxWidth", "margin"), keepIfNull = TRUE))
    },

    #' @description Create a `ggtext::element_textbox` directly convertible by `ggplot2::theme()`.
    #' @param size numeric defining the size of the label
    #' @param color character defining the color of the label
    #' @param fontFamily character defining the font family of the label
    #' @param fontFace character defining the font face of the label as defined in helper enum `FontFaces`.
    #' @param angle numeric defining the angle of the label.
    #' @param align character defining the alignment of the label as defined in helper enum `Alignments`.
    #' @param maxWidth numeric that will be converted to a ggplot2::unit object (in "pt" unit) defining the maximum width of text box.
    #' @param margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
    #' @return An `element_text` or `element_blank`object.
    createPlotTextBoxFont = function(color = NULL,
                                     size = NULL,
                                     fontFace = NULL,
                                     fontFamily = NULL,
                                     angle = NULL,
                                     align = NULL,
                                     maxWidth = NULL,
                                     margin = NULL) {
      if (isEmpty(self$text)) {
        return(ggplot2::element_blank())
      }

      return(self$font$createPlotTextBoxFont(
        color = color,
        size = size,
        fontFace = fontFace,
        fontFamily = fontFamily,
        angle = angle,
        align = align,
        maxWidth = maxWidth,
        margin = margin
      ))
    },

    #' @description Create a `ggplot2::element_text()` directly convertible by `ggplot2::theme()`.
    #' @param size numeric defining the size of the label
    #' @param color character defining the color of the label
    #' @param fontFamily character defining the font family of the label
    #' @param fontFace character defining the font face of the label as defined in helper enum `FontFaces`.
    #' @param angle numeric defining the angle of the label.
    #' @param align character defining the alignment of the label as defined in helper enum `Alignments`.
    #' @param margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
    #' @return An `element_text` or `element_blank`object.
    createPlotTextFont = function(color = NULL,
                                  size = NULL,
                                  fontFace = NULL,
                                  fontFamily = NULL,
                                  angle = NULL,
                                  align = NULL,
                                  margin = NULL) {
      if (isEmpty(self$text)) {
        return(ggplot2::element_blank())
      }

      return(self$font$createPlotTextFont(
        color = color,
        size = size,
        fontFace = fontFace,
        fontFamily = fontFamily,
        angle = angle,
        align = align,
        margin = margin
      ))
    }
  ),
  active = list(
    #' @field text character text of the label
    text = function(value) {
      if (missing(value)) {
        return(private$.text)
      }
      validateIsString(value, nullAllowed = TRUE)
      private$.text <- value
      return(invisible())
    },
    #' @field font `Font` object
    font = function(value) {
      if (missing(value)) {
        return(private$.font)
      }
      validateIsOfType(value, "Font", nullAllowed = TRUE)
      private$.font <- value %||% Font$new()
      # Ensures that size is numeric
      private$.font$size <- as.numeric(private$.font$size)
      return(invisible())
    }
  ),
  private = list(
    .text = NULL,
    .font = NULL
  )
)
