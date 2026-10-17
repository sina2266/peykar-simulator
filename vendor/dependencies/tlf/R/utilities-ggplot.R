#' @title GeomTLFPoint
#' @description
#' Define a Geom using `ggplot2::ggproto()` and based on GeomPoint.
#' The Geom internally uses `textGrob` instead of `pointsGrob` so that fonts leverage for drawing shapes.
#' The `grid` and `scales` packages are supposed to be required by `ggplot2`.
#' So there should not be any issue as installing `ggplot2` should install those 2 packages.
#' @keywords internal
#'
GeomTLFPoint <- ggplot2::ggproto(
  "GeomTLFPoint",
  GeomPoint,
  # This will correspond to the default property displayed in legend
  # if property not used in data mapping
  # this replaces displayed "19" by a colored square
  default_aes = ggplot2::aes(
    shape = "\u2588", colour = "black", size = 1.5, fill = NA,
    alpha = NA, stroke = 0.5
  ),
  draw_panel = function(data, panel_params, coord) {
    coords <- coord$transform(data, panel_params)
    # Replace grid::pointsGrob from geom_point accounting for font family
    grid::textGrob(
      # If shape is included in plot dictionary, use it
      label = .asPlotShape(coords$shape),
      x = coords$x, y = coords$y,
      default.units = "native",
      gp = grid::gpar(
        col = scales::alpha(coords$colour %||% "black", coords$alpha),
        fill = scales::alpha(coords$fill %||% "black", coords$alpha),
        fontsize = coords$size * ggplot2::.pt,
        fontfamily = .selectFontFamily()
      )
    )
  },
  draw_key = function(data, params, size) {
    if (is.null(data$shape)) {
      data$shape <- Shapes$blank
    }
    # NULL means the default stroke size, and NA means no stroke.
    stroke_size <- data$stroke %||% 0.5
    stroke_size[is.na(stroke_size)] <- 0

    # Replace grid::pointsGrob from geom_point accounting for font family
    grid::textGrob(
      # If shape is included in plot dictionary, use it in legend
      # Prevents having "circle" instead of its shape displayed in the legend
      label = .asPlotShape(data$shape),
      x = 0.5, y = 0.5,
      # Code copied from ggplot2 except for font family
      gp = grid::gpar(
        col = scales::alpha(data$colour %||% "black", data$alpha),
        fill = scales::alpha(data$fill %||% "black", data$alpha),
        fontsize = (data$size %||% 1.5) * .pt + stroke_size * .stroke / 2,
        lwd = stroke_size * .stroke / 2,
        fontfamily = .selectFontFamily()
      )
    )
  }
)

#' @title geomTLFPoint
#' @description
#' geom similar to `geom_point()` but that leverage fonts to draw its shapes
#' @param mapping mapping from `ggplot2` package as provided by `aes()`
#' @param data data.frame
#' @param stat stat name from `ggplot2`
#' @param position position name from `ggplot2`
#' @param show.legend = NA,
#' @param na.rm a logical value indicating
#' @param inherit.aes a logical value indicating if aesthetics are inherited
#' @param ... other arguments.
#' @export
#'
geomTLFPoint <- function(mapping = NULL, data = NULL, stat = "identity",
                         position = "identity", na.rm = FALSE, show.legend = NA,
                         inherit.aes = TRUE, ...) {
  ggplot2::layer(
    geom = GeomTLFPoint, mapping = mapping, data = data, stat = stat,
    position = position, show.legend = show.legend, inherit.aes = inherit.aes,
    params = list(na.rm = na.rm, ...)
  )
}

#' @title .selectFontFamily
#' @description
#' Select appropriate font family based on font and `showtext` package availability
#' @param fontfamily default font family
#' @keywords internal
#'
.selectFontFamily <- function(fontfamily = "sans") {
  if (!requireNamespace("showtext", quietly = TRUE)) {
    return(fontfamily)
  }
  # sysfonts is required by showtext
  # thus, installing showtext also installs sysfonts
  # when loading the tlf package,
  # the Symbola font family is added to the sysfont font families
  # however in some environments such as devtools::check(),
  # it seems that the family is removed from that list
  # consequently, this line perform a last check
  # of font availability before displaying the grob
  if (!isIncluded("Symbola", sysfonts::font_families())) {
    return(fontfamily)
  }
  return("Symbola")
}
