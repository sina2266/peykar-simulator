#' @title OSP Errorbar Layer
#' @description
#' A geom that renders error bars with cap `width` specified in **mm** units,
#' keeping it visually consistent with the `linewidth` aesthetic, which is
#' also expressed in **mm**. Unlike [ggplot2::geom_errorbar()],
#' the cap width is independent of the data coordinate range or axis scale.
#'
#' Vertical orientation (`aes(x, ymin, ymax)`) is used by default.
#' Pass `orientation = "x"` for horizontal error bars (`aes(y, xmin, xmax)`).
#'
#' @inheritParams ggplot2::geom_errorbar
#' @param width Width of the error bar caps in mm units. Default: `2`.
#' @param orientation Orientation of the layer. `"x"` produces horizontal
#'   error bars (range along the x-axis). Any other value, including `NA`
#'   (default) and `"y"`, produces vertical error bars (range along the
#'   y-axis).
#' @param na.rm Missing values in the range aesthetics are dropped (the
#'   affected cap is simply not drawn) for both `TRUE` and `FALSE`. No warning
#'   is emitted in either case. The argument is kept for consistency with the
#'   [ggplot2::geom_errorbar()] interface.
#' @return A ggplot2 layer that can be added to a plot.
#' @export
#' @family layers
#' @examples
#' library(ggplot2)
#' df <- data.frame(
#'   x    = 1:3,
#'   y    = c(1, 2, 3),
#'   ymin = c(0.5, 1.5, 2.5),
#'   ymax = c(1.5, 2.5, 3.5)
#' )
#' ggplot(df, aes(x, y, ymin = ymin, ymax = ymax)) +
#'   geom_errorbar_osp(width = 2, linewidth = 0.8)
# nolint start: object_name_linter
geom_errorbar_osp <- function(
  mapping = NULL,
  data = NULL,
  stat = "identity",
  position = "identity",
  ...,
  orientation = NA,
  width = 2,
  lineend = "butt",
  na.rm = FALSE,
  show.legend = NA,
  inherit.aes = TRUE
) {
  # nolint end
  checkmate::assertNumber(width, lower = 0, finite = TRUE)
  ggplot2::layer(
    geom = GeomErrorbarOsp,
    mapping = mapping,
    data = data,
    stat = stat,
    position = position,
    show.legend = show.legend,
    inherit.aes = inherit.aes,
    params = list(
      orientation = orientation,
      width = width,
      lineend = lineend,
      na.rm = na.rm,
      ...
    )
  )
}

# ggproto object ----

#' @title GeomErrorbarOsp
#' @description
#' ggproto object for OSP error bars with cap width in mm units.
#' Use [geom_errorbar_osp()] to add this geom to a ggplot.
#' @format NULL
#' @usage NULL
#' @export
#' @family layers
# nolint start: object_name_linter
GeomErrorbarOsp <- ggplot2::ggproto(
  # nolint end
  "GeomErrorbarOsp",
  ggplot2::Geom,
  required_aes = c("x|y", "ymin|xmin", "ymax|xmax"),
  default_aes = ggplot2::aes(
    colour = "black",
    linewidth = 0.5,
    linetype = 1,
    alpha = NA
  ),
  draw_key = ggplot2::draw_key_path,
  extra_params = c("na.rm", "orientation", "width", "lineend"),
  setup_data = function(data, params) {
    # Suppress rows where the bar has zero length to avoid orphan caps.
    # Both range columns are set to NA_real_ so the cap is drawn at an NA
    # coordinate, i.e. not rendered.
    # Respect orientation when provided so only the relevant axis is checked.
    orientation <- params$orientation %||% NA
    horizontal <- identical(orientation, "x")

    if (!horizontal && all(c("ymin", "ymax") %in% names(data))) {
      zeroRange <- !is.na(data$ymin) &
        !is.na(data$ymax) &
        data$ymin == data$ymax
      data$ymin[zeroRange] <- NA_real_
      data$ymax[zeroRange] <- NA_real_
    }
    if (horizontal && all(c("xmin", "xmax") %in% names(data))) {
      zeroRange <- !is.na(data$xmin) &
        !is.na(data$xmax) &
        data$xmin == data$xmax
      data$xmin[zeroRange] <- NA_real_
      data$xmax[zeroRange] <- NA_real_
    }
    return(data)
  },
  # nolint start: object_name_linter
  draw_panel = function(
    self,
    data,
    panel_params,
    coord,
    lineend = "butt",
    orientation = NA,
    width = 2
  ) {
    # nolint end
    if (nrow(data) == 0) return(grid::nullGrob())
    coords <- coord$transform(data, panel_params)
    if (nrow(coords) == 0) return(grid::nullGrob())

    alpha <- ifelse(is.na(coords$alpha), 1, coords$alpha)
    col <- scales::alpha(coords$colour, alpha)
    # linewidth aesthetic is in mm (ggplot2 >= 3.4); grid lwd is in pt
    lwd <- coords$linewidth * ggplot2::.pt
    lty <- coords$linetype

    halfWidthMm <- grid::unit(width / 2, "mm")
    gp <- grid::gpar(col = col, lwd = lwd, lty = lty, lineend = lineend)

    # orientation = "x" → horizontal bars; everything else ("y", NA) → vertical.
    horizontal <- identical(orientation, "x")

    # Pick the aesthetics for the chosen orientation: `along` runs the bar
    # (range), `across` positions it (centre) and is where the caps extend.
    if (horizontal) {
      along <- c("xmin", "xmax")
      across <- "y"
    } else {
      along <- c("ymin", "ymax")
      across <- "x"
    }
    if (any(lengths(coords[c(across, along)]) == 0)) {
      return(grid::nullGrob())
    }

    center <- grid::unit(coords[[across]], "npc")
    rangeMin <- grid::unit(coords[[along[1]]], "npc")
    rangeMax <- grid::unit(coords[[along[2]]], "npc")

    # A bar is the range segment plus a cap at each end. The orientation only
    # decides which grid axis the range and centre map to, so resolve it once:
    # for horizontal bars the range runs along x and caps extend in y, and for
    # vertical bars it is mirrored.
    segment <- if (horizontal) {
      \(rangeFrom, rangeTo, centerFrom, centerTo) {
        grid::segmentsGrob(
          x0 = rangeFrom,
          y0 = centerFrom,
          x1 = rangeTo,
          y1 = centerTo,
          gp = gp
        )
      }
    } else {
      \(rangeFrom, rangeTo, centerFrom, centerTo) {
        grid::segmentsGrob(
          x0 = centerFrom,
          y0 = rangeFrom,
          x1 = centerTo,
          y1 = rangeTo,
          gp = gp
        )
      }
    }

    grid::grobTree(
      segment(rangeMin, rangeMax, center, center),
      segment(rangeMin, rangeMin, center - halfWidthMm, center + halfWidthMm),
      segment(rangeMax, rangeMax, center - halfWidthMm, center + halfWidthMm)
    )
  }
)
