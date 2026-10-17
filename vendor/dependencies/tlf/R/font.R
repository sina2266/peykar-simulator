#' @title Font
#' @description  R6 class defining font properties
#' @field size numeric defining the size of font
#' @field color character defining the color of font
#' @field fontFamily character defining the family of font
#' @field fontFace character defining the font face as defined in helper enum `FontFaces`.
#' @field angle numeric defining the angle of font
#' @field align character defining the alignment of font as defined in helper enum `Alignments`.
#' @field maxWidth numeric that will be converted to a ggplot2::unit object (in "pt" unit) defining the maximum width of text box.
#' @field margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
#' @export
Font <- R6::R6Class(
  "Font",
  public = list(
    size = 12,
    color = "black",
    fontFamily = "",
    fontFace = "plain",
    angle = 0,
    align = "center",
    maxWidth = NULL,
    margin = NULL,
    #' @description Create a new `Font` object.
    #' Default font properties are defined directly in the object field,
    #' so `NULL` input is allowed will lead to default properties.
    #' @param color character defining the color of font.
    #' @param size numeric defining the size of font.
    #' @param fontFamily character defining the family of font.
    #' @param fontFace character defining the font face as defined in helper enum `FontFaces`.
    #' @param angle numeric defining the angle of font.
    #' @param align character defining the alignment of font as defined in helper enum `Alignments`.
    #' @param maxWidth numeric that will be converted to a ggplot2::unit object (in "pt" unit) defining the maximum width of text box.
    #' @param margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
    #' @return A new `Font` object
    initialize = function(
      color = NULL,
      size = NULL,
      fontFamily = NULL,
      fontFace = NULL,
      angle = NULL,
      align = NULL,
      maxWidth = NULL,
      margin = NULL
    ) {
      validateIsString(c(color, fontFamily), nullAllowed = TRUE)
      validateIsNumeric(c(size, angle), nullAllowed = TRUE)
      validateIsIncluded(fontFace, FontFaces, nullAllowed = TRUE)
      validateIsIncluded(align, Alignments, nullAllowed = TRUE)
      eval(.parseVariableToObject(
        "self",
        c(
          "size",
          "color",
          "fontFace",
          "fontFamily",
          "angle",
          "align",
          "maxWidth"
        ),
        keepIfNull = TRUE
      ))
    },

    #' @description Create a `ggplot2::element_text` directly convertible by `ggplot2::theme`.
    #' @param size numeric defining the size of font
    #' @param color character defining the color of font
    #' @param fontFamily character defining the family of font
    #' @param fontFace character defining the font face as defined in helper enum `FontFaces`.
    #' @param angle numeric defining the angle of font.
    #' @param align character defining the alignment of font as defined in helper enum `Alignments`.
    #' @param margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
    #' @return An `element_text` object.
    createPlotTextFont = function(
      size = NULL,
      color = NULL,
      fontFamily = NULL,
      fontFace = NULL,
      angle = NULL,
      align = NULL,
      margin = NULL
    ) {
      margin <- margin %||% self$margin
      if (!is.null(margin)) {
        # Ensure margin is a vector of length 4 (top, right, bottom, left)
        if (length(margin) == 1) {
          margin <- rep(margin, 4)
        } else if (length(margin) == 2) {
          margin <- rep(margin, 2)
        } else if (length(margin) == 3) {
          margin <- c(margin, margin[2])
        }
        margin <- ggplot2::margin(
          t = margin[1],
          r = margin[2],
          b = margin[3],
          l = margin[4],
          unit = "pt"
        )
      }

      ggplot2::element_text(
        colour = color %||% self$color,
        size = size %||% self$size,
        face = fontFace %||% self$fontFace,
        # Use font family only if available in Windows font database database
        family = .checkPlotFontFamily(fontFamily %||% self$fontFamily),
        angle = angle %||% self$angle,
        vjust = 0.5,
        hjust = switch(
          align %||% self$align,
          "left" = 0,
          "center" = 0.5,
          "right" = 1
        ),
        margin = margin
      )
    },
    #' @description Create a `ggplot2::element_text` directly convertible by `ggplot2::theme`.
    #' @param size numeric defining the size of font
    #' @param color character defining the color of font
    #' @param fontFamily character defining the family of font
    #' @param fontFace character defining the font face as defined in helper enum `FontFaces`.
    #' @param angle numeric defining the angle of font.
    #' @param align character defining the alignment of font as defined in helper enum `Alignments`.
    #' @param maxWidth numeric that will be converted to a ggplot2::unit object (in "pt" unit) defining the maximum width of text box.
    #' @param margin a numeric vector of length 4 defining the size of the area (in pt) around the text in the followin order: top, right, bottom, left.
    #' @return An `ggtext::element_textbox` object.
    createPlotTextBoxFont = function(
      size = NULL,
      color = NULL,
      fontFamily = NULL,
      fontFace = NULL,
      angle = NULL,
      align = NULL,
      maxWidth = NULL,
      margin = NULL
    ) {
      maxWidth <- maxWidth %||% self$maxWidth
      if (!is.null(maxWidth)) {
        maxWidth <- ggplot2::unit(maxWidth, "pt")
      }
      margin <- margin %||% self$margin
      if (!is.null(margin)) {
        # Ensure margin is a vector of length 4 (top, right, bottom, left)
        if (length(margin) == 1) {
          margin <- rep(margin, 4)
        } else if (length(margin) == 2) {
          margin <- rep(margin, 2)
        } else if (length(margin) == 3) {
          margin <- c(margin, margin[2])
        }
        margin <- ggplot2::margin(
          t = margin[1],
          r = margin[2],
          b = margin[3],
          l = margin[4],
          unit = "pt"
        )
      }

      ggtext::element_textbox_simple(
        colour = color %||% self$color,
        size = size %||% self$size,
        face = fontFace %||% self$fontFace,
        # Use font family only if available in Windows font database database
        family = .checkPlotFontFamily(fontFamily %||% self$fontFamily),
        orientation = .convertAngleToOrientation(angle %||% self$angle),
        valign = 0.5,
        halign = switch(
          align %||% self$align,
          "left" = 0,
          "center" = 0.5,
          "right" = 1
        ),
        maxwidth = maxWidth,
        margin = margin
      )
    }
  )
)

#' @title .checkPlotFontFamily
#' @description Check if font family is available in Windows font database.
#' Use function `grDevices::windowsFonts()` to get the list of font families available.
#' @param fontFamily character defining the family of font
#' @return Name of font family if available in Windows font database
#' `NULL` otherwise
#' @keywords internal
.checkPlotFontFamily <- function(fontFamily) {
  # If showtext is installed, use its default font or user-defined font
  if (requireNamespace("showtext", quietly = TRUE)) {
    if (isIncluded(fontFamily, sysfonts::font_families())) {
      return(fontFamily)
    }
    return("NotoSans")
  }
  if (isEmpty(.Platform$OS.type)) {
    return(NULL)
  }
  if (.Platform$OS.type != "windows") {
    return(NULL)
  }
  if (isIncluded(fontFamily, names(grDevices::windowsFonts()))) {
    return(fontFamily)
  }
  return(NULL)
}

.convertAngleToOrientation <- function(angle) {
  # use modulo 360 to in case minus angles were provided
  return(
    switch(
      as.character(angle %% 360),
      "0" = "upright",
      "90" = "left-rotated",
      "180" = "right-rotated",
      "270" = "inverted"
    )
  )
}
