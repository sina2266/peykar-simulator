# Exported data ----

#' @title OSP Shape Names
#' @description Character vector of all available OSP shape names.
#' @export
#' @family shapes
ospShapeNames <- c(
  "circle",
  "diamond",
  "triangle",
  "square",
  "invertedTriangle",
  "cross",
  "plus",
  "asterisk",
  "star",
  "pentagon",
  "hexagon",
  # Thin shapes
  "thinCross",
  "thinPlus",
  # Open shapes
  "circleOpen",
  "diamondOpen",
  "triangleOpen",
  "squareOpen",
  "invertedTriangleOpen",
  "starOpen",
  "pentagonOpen",
  "hexagonOpen",
  "blank"
)

#' @title Shapes
#' @description Named list of OSP shape names for backward compatibility.
#' Use `Shapes$circle` to get the shape name "circle".
#' @family shapes
#' @export
Shapes <- stats::setNames(as.list(ospShapeNames), ospShapeNames) # nolint: object_name_linter

# User-facing functions ----

#' @title OSP Point Layer
#' @description
#' A geom that renders OSP shapes using grid primitives.
#' Uses shape names from `ospShapeNames`. Automatically applies
#' `scale_shape_osp()` when added to a plot (unless a shape scale
#' is already present).
#'
#' @inheritParams ggplot2::geom_point
#' @param na.rm If `FALSE` (default), missing values are removed with a warning.
#'   If `TRUE`, missing values are silently removed.
#' @return A ggplot2 layer that can be added to a plot.
#' @export
#' @family layers
#' @examples
#' library(ggplot2)
#' df <- data.frame(x = 1:5, y = 1:5, shape = ospShapeNames[1:5])
#' ggplot(df, aes(x, y, shape = shape)) +
#'   geom_point_osp(size = 4) +
#'   scale_shape_osp_identity()
# nolint start: object_name_linter
geom_point_osp <- function(
  mapping = NULL,
  data = NULL,
  stat = "identity",
  position = "identity",
  ...,
  na.rm = FALSE,
  show.legend = NA,
  inherit.aes = TRUE
) {
  # nolint end
  layer <- ggplot2::layer(
    geom = GeomPointOsp,
    mapping = mapping,
    data = data,
    stat = stat,
    position = position,
    show.legend = show.legend,
    inherit.aes = inherit.aes,
    params = list(na.rm = na.rm, ...)
  )
  class(layer) <- c("geom_point_osp_layer", class(layer))
  layer
}

#' @export
#' @method ggplot_add geom_point_osp_layer
ggplot_add.geom_point_osp_layer <- function(object, plot, ...) {
  class(object) <- setdiff(class(object), "geom_point_osp_layer")
  plot <- plot + object
  class(plot) <- unique(c("osp_ggplot", class(plot)))
  plot
}

#' @export
#' @method ggplot_build osp_ggplot
ggplot_build.osp_ggplot <- function(plot, ...) {
  hasScale <- function(aesthetic) {
    any(vapply(
      plot$scales$scales,
      function(s) aesthetic %in% s$aesthetics,
      logical(1)
    ))
  }

  if (!hasScale("shape")) {
    plot <- plot + scale_shape_osp()
  }

  # Apply the OSP discrete color/fill scales per plot, but only when the
  # aesthetic is mapped to discrete data (a continuous mapping would error on
  # a discrete scale) and no scale for it is already present (so user-supplied
  # scales win without a "Scale already present" message).
  discreteAesthetics <- .usedDiscreteAesthetics(plot)
  if ("colour" %in% discreteAesthetics && !hasScale("colour")) {
    plot <- plot + scale_colour_osp()
  }
  if ("fill" %in% discreteAesthetics && !hasScale("fill")) {
    plot <- plot + scale_fill_osp()
  }

  class(plot) <- setdiff(class(plot), "osp_ggplot")
  ggplot2::ggplot_build(plot)
}

#' Determine which discrete color/fill aesthetics a plot uses
#'
#' Scans the plot-level and per-layer mappings for `colour`/`fill` and returns
#' those that resolve to discrete (factor/character/logical) data. Used to
#' decide whether the OSP discrete scales can be safely applied.
#'
#' @param plot a `ggplot` object
#' @return character vector, a subset of `c("colour", "fill")`
#' @keywords internal
.usedDiscreteAesthetics <- function(plot) {
  aesthetics <- c("colour", "fill")

  # Build (mapping, data) pairs: the plot-level mapping/data, plus one per
  # layer (each layer either inherits the plot mapping or carries its own,
  # and may carry its own data).
  pairs <- list(list(
    mapping = plot$mapping %||% aes(),
    data = plot$data
  ))
  for (layer in plot$layers) {
    mapping <- if (isTRUE(layer$inherit.aes)) {
      utils::modifyList(plot$mapping %||% aes(), layer$mapping %||% aes())
    } else {
      layer$mapping %||% aes()
    }
    data <- if (is.data.frame(layer$data)) layer$data else plot$data
    pairs[[length(pairs) + 1]] <- list(mapping = mapping, data = data)
  }

  isDiscreteMapping <- function(aesthetic) {
    for (pair in pairs) {
      # ggplot2 standardises "color" to "colour" in mappings.
      quoVar <- pair$mapping[[aesthetic]]
      if (is.null(quoVar) || !is.data.frame(pair$data)) {
        next
      }
      resolved <- tryCatch(
        rlang::eval_tidy(quoVar, data = pair$data),
        error = function(e) NULL
      )
      if (
        is.factor(resolved) ||
          is.character(resolved) ||
          is.logical(resolved)
      ) {
        return(TRUE)
      }
    }
    FALSE
  }

  aesthetics[vapply(aesthetics, isDiscreteMapping, logical(1))]
}

#' @title OSP Shape Scale
#' @description
#' Discrete shape scale that automatically assigns shapes from
#' `ospShapeNames` in order based on the number of factor levels.
#' Equivalent to `ggplot2::scale_shape()`.
#'
#' If there are more levels than available shapes, shapes are recycled
#' and a warning is issued.
#'
#' @param ... Passed to `ggplot2::discrete_scale`.
#' @return A ggplot2 scale that can be added to a plot.
#' @export
#' @family shapes
#' @examples
#' library(ggplot2)
#' df <- data.frame(x = 1:3, y = 1:3, group = c("A", "B", "C"))
#' ggplot(df, aes(x, y, shape = group)) +
#'   geom_point_osp(size = 4) +
#'   scale_shape_osp()
# nolint start: object_name_linter
scale_shape_osp <- function(...) {
  # nolint end
  ggplot2::discrete_scale(
    aesthetics = "shape",
    palette = .ospShapePalette,
    ...
  )
}

#' @title OSP Shape Manual Scale
#' @description
#' Manual shape scale for explicit mapping of factor levels to OSP shape names.
#' Equivalent to `ggplot2::scale_shape_manual()`.
#'
#' @param values Named character vector. Names are factor levels;
#'   values are entries from `ospShapeNames`.
#' @param ... Passed to `ggplot2::scale_shape_manual`.
#' @return A ggplot2 scale that can be added to a plot.
#' @export
#' @family shapes
#' @examples
#' library(ggplot2)
#' df <- data.frame(x = 1:3, y = 1:3, group = c("A", "B", "C"))
#' ggplot(df, aes(x, y, shape = group)) +
#'   geom_point_osp(size = 4) +
#'   scale_shape_osp_manual(values = c(A = "circle", B = "diamond", C = "star"))
# nolint start: object_name_linter
scale_shape_osp_manual <- function(values, ...) {
  # nolint end
  bad <- setdiff(values, ospShapeNames)
  if (length(bad) > 0) {
    stop(
      "Values not in ospShapeNames: ",
      paste(shQuote(bad), collapse = ", "),
      call. = FALSE
    )
  }
  ggplot2::scale_shape_manual(values = values, ...)
}

#' @title OSP Shape Identity Scale
#' @description
#' Identity scale for when data already contains OSP shape names.
#' Use this when your shape column contains values from `ospShapeNames`
#' directly (e.g., "circle", "diamond", "star").
#' Equivalent to `ggplot2::scale_shape_identity()`.
#'
#' @param guide Guide for the legend. Use `"legend"` to show a legend,
#'   or `"none"` to hide it.
#' @param ... Passed to `ggplot2::scale_shape_manual`.
#' @return A ggplot2 scale that can be added to a plot.
#' @export
#' @family shapes
#' @examples
#' library(ggplot2)
#' df <- data.frame(x = 1:3, y = 1:3, shape = c("circle", "diamond", "star"))
#' ggplot(df, aes(x, y, shape = shape)) +
#'   geom_point_osp(size = 4) +
#'   scale_shape_osp_identity(guide = "legend")
# nolint start: object_name_linter
scale_shape_osp_identity <- function(guide = "none", ...) {
  # nolint end
  values <- stats::setNames(ospShapeNames, ospShapeNames)
  ggplot2::scale_shape_manual(values = values, guide = guide, ...)
}

#' @title OSP Q-Q Stat
#' @description
#' A stat_qq that uses OSP shapes via `GeomPointOsp` instead of standard
#' `geom_point`. This ensures QQ plots have visual consistency with other
#' OSP plots.
#'
#' Unlike `ggplot2::stat_qq()`, this function does not expose a `geom` parameter
#' as it always uses `GeomPointOsp` for rendering.
#'
#' @inheritParams ggplot2::stat_qq
#' @return A ggplot2 layer that can be added to a plot.
#' @export
#' @family layers
# nolint start: object_name_linter
stat_qq_osp <- function(
  mapping = NULL,
  data = NULL,
  position = "identity",
  ...,
  distribution = stats::qnorm,
  dparams = list(),
  na.rm = FALSE,
  show.legend = NA,
  inherit.aes = TRUE
) {
  # nolint end
  layer <- ggplot2::layer(
    stat = ggplot2::StatQq,
    geom = GeomPointOsp,
    mapping = mapping,
    data = data,
    position = position,
    show.legend = show.legend,
    inherit.aes = inherit.aes,
    params = list(
      distribution = distribution,
      dparams = dparams,
      na.rm = na.rm,
      ...
    )
  )
  class(layer) <- c("geom_point_osp_layer", class(layer))
  layer
}

# ggproto object ----

#' @title GeomPointOsp
#' @description ggproto object for OSP point shapes.
#' @format NULL
#' @usage NULL
#' @export
#' @family layers
# nolint start: object_name_linter
GeomPointOsp <- ggplot2::ggproto(
  # nolint end

  "GeomPointOsp",
  ggplot2::Geom,
  required_aes = c("x", "y"),
  default_aes = ggplot2::aes(
    colour = "black",
    fill = NA,
    size = 1.5,
    stroke = 0.5,
    alpha = NA,
    shape = "circle"
  ),
  # nolint start: object_name_linter
  draw_panel = function(data, panel_params, coord) {
    # nolint end
    coords <- coord$transform(data, panel_params)

    # Warn once per unique unknown shape
    unknownShapes <- setdiff(unique(as.character(coords$shape)), ospShapeNames)
    if (length(unknownShapes) > 0) {
      warning(
        "Unknown shape(s): ",
        paste(shQuote(unknownShapes), collapse = ", "),
        ". Using 'circle' instead.",
        call. = FALSE
      )
    }

    grobs <- lapply(seq_len(nrow(coords)), function(i) {
      name <- .validateShapeName(coords$shape[i])
      .ospGrob(
        name = name,
        cx = grid::unit(coords$x[i], "native"),
        cy = grid::unit(coords$y[i], "native"),
        half = grid::unit(coords$size[i] * .kSizeMultiplier / 2, "mm"),
        fill = scales::alpha(coords$fill[i], coords$alpha[i] %||% 1),
        colour = scales::alpha(coords$colour[i], coords$alpha[i] %||% 1),
        stroke = coords$stroke[i],
        alpha = 1
      )
    })

    do.call(grid::grobTree, grobs)
  },
  draw_key = function(data, params, size) {
    name <- .validateShapeName(data$shape %||% "circle")
    pointSize <- data$size %||% 1.5
    keyAlpha <- if (is.na(data$alpha)) 1 else data$alpha %||% 1
    .ospGrob(
      name = name,
      cx = grid::unit(0.5, "npc"),
      cy = grid::unit(0.5, "npc"),
      half = grid::unit(pointSize * .kSizeMultiplier / 2, "mm"),
      fill = scales::alpha(data$fill %||% NA, keyAlpha),
      colour = scales::alpha(data$colour %||% "black", keyAlpha),
      stroke = data$stroke %||% 0.5,
      alpha = 1
    )
  }
)

# Internal helpers ----

# Calibration constants (visually matched against ggplot2::geom_point pch=21)
.kSizeMultiplier <- 0.92
.kStrokeLwdMultiplier <- 1.9

#' Validate shape name, falling back to "circle" for unknown shapes
#' @param name shape name to validate
#' @return valid shape name (original if known, "circle" otherwise)
#' @keywords internal
.validateShapeName <- function(name) {
  name <- as.character(name)
  if (!name %in% ospShapeNames) "circle" else name
}

#' OSP shape palette function
#' @param n Number of shapes needed
#' @return Character vector of shape names
#' @keywords internal
.ospShapePalette <- function(n) {
  visibleShapes <- setdiff(ospShapeNames, "blank")
  nShapes <- length(visibleShapes)
  if (n > nShapes) {
    warning(
      "Number of groups (",
      n,
      ") exceeds available shapes (",
      nShapes,
      "). ",
      "Shapes will be recycled.",
      call. = FALSE
    )
  }
  visibleShapes[((seq_len(n) - 1) %% nShapes) + 1]
}

#' Build grid grob for OSP shape
#' @param name shape name
#' @param cx center x (grid unit)
#' @param cy center y (grid unit)
#' @param half half-size (grid unit)
#' @param fill fill color
#' @param colour stroke color
#' @param stroke stroke width
#' @param alpha alpha value
#' @return grid grob
#' @keywords internal
.ospGrob <- function(name, cx, cy, half, fill, colour, stroke, alpha) {
  spec <- .ospShapeSpec[[name]]
  if (is.null(spec)) {
    return(grid::nullGrob())
  }

  # Convert stroke to lwd matching ggplot2's geom_point rendering
  strokeLwd <- stroke * .kStrokeLwdMultiplier

  # Apply shape-specific scaling factor
  shapeScale <- spec$scale %||% 1
  scaledHalf <- half * shapeScale

  switch(
    spec$kind,
    polygon = {
      fg <- if (isTRUE(spec$open)) fill else colour
      v <- .polyVertices(spec$n, spec$angle)
      yOff <- (spec$yOffset %||% 0) * scaledHalf
      grid::polygonGrob(
        x = cx + v$x * scaledHalf,
        y = cy + v$y * scaledHalf + yOff,
        gp = grid::gpar(fill = fg, col = colour, lwd = strokeLwd, alpha = alpha)
      )
    },
    star = {
      fg <- if (isTRUE(spec$open)) fill else colour
      v <- .starVertices(spec$points)
      grid::polygonGrob(
        x = cx + v$x * scaledHalf,
        y = cy + v$y * scaledHalf,
        gp = grid::gpar(fill = fg, col = colour, lwd = strokeLwd, alpha = alpha)
      )
    },
    stroke = {
      lwd <- if (isTRUE(spec$thick)) strokeLwd * 2.2 else strokeLwd
      gpStroke <- grid::gpar(
        col = colour,
        lwd = lwd,
        alpha = alpha,
        lineend = "butt"
      )
      switch(
        spec$glyph,
        plus = grid::segmentsGrob(
          x0 = grid::unit.c(cx - scaledHalf, cx),
          y0 = grid::unit.c(cy, cy - scaledHalf),
          x1 = grid::unit.c(cx + scaledHalf, cx),
          y1 = grid::unit.c(cy, cy + scaledHalf),
          gp = gpStroke
        ),
        cross = grid::segmentsGrob(
          x0 = grid::unit.c(cx - scaledHalf, cx - scaledHalf),
          y0 = grid::unit.c(cy - scaledHalf, cy + scaledHalf),
          x1 = grid::unit.c(cx + scaledHalf, cx + scaledHalf),
          y1 = grid::unit.c(cy + scaledHalf, cy - scaledHalf),
          gp = gpStroke
        ),
        asterisk = {
          theta <- c(pi / 2, pi / 2 + pi / 3, pi / 2 + 2 * pi / 3)
          grid::segmentsGrob(
            x0 = cx - cos(theta) * scaledHalf,
            y0 = cy - sin(theta) * scaledHalf,
            x1 = cx + cos(theta) * scaledHalf,
            y1 = cy + sin(theta) * scaledHalf,
            gp = gpStroke
          )
        },
        grid::nullGrob()
      )
    },
    blank = grid::nullGrob(),
    grid::nullGrob()
  )
}

# Low-level geometry and shape specifications ----

#' Regular n-gon vertices on unit circle
#' @param n number of vertices
#' @param angleDeg angle of first vertex in degrees
#' @return list with x and y coordinate vectors
#' @keywords internal
.polyVertices <- function(n, angleDeg = 0) {
  theta <- seq(0, 2 * pi, length.out = n + 1)[-(n + 1)] + angleDeg * pi / 180
  list(x = cos(theta), y = sin(theta))
}

#' Star vertices with alternating outer/inner radius
#' @param points number of star points
#' @param innerRatio ratio of inner to outer radius
#' @return list with x and y coordinate vectors
#' @keywords internal
.starVertices <- function(points = 5, innerRatio = 0.4) {
  n <- 2 * points
  theta <- seq(pi / 2, pi / 2 + 2 * pi, length.out = n + 1)[-(n + 1)]
  r <- rep(c(1, innerRatio), length.out = n)
  list(x = r * cos(theta), y = r * sin(theta))
}

#' Polygon area using shoelace formula
#' @param x x-coordinates of vertices
#' @param y y-coordinates of vertices
#' @return area of polygon
#' @keywords internal
.shoelaceArea <- function(x, y) {
  0.5 * abs(sum(x * c(y[-1], y[1]) - c(x[-1], x[1]) * y))
}

#' Compute area-based scale factor for a shape
#' @param spec shape specification from .ospShapeSpec
#' @param circleArea area of reference circle (default: 64-gon approximation)
#' @return scale factor to match circle area
#' @keywords internal
.computeAreaScale <- function(spec, circleArea = NULL) {
  if (is.null(circleArea)) {
    circleVerts <- .polyVertices(64, 0)
    circleArea <- .shoelaceArea(circleVerts$x, circleVerts$y)
  }

  if (spec$kind == "polygon") {
    v <- .polyVertices(spec$n, spec$angle)
    shapeArea <- .shoelaceArea(v$x, v$y)
  } else if (spec$kind == "star") {
    v <- .starVertices(spec$points)
    shapeArea <- .shoelaceArea(v$x, v$y)
  } else {
    return(1)
  }

  sqrt(circleArea / shapeArea)
}

# Manual adjustment factors for fine-tuning perceptual appearance.
# Applied on top of area-based scaling: final_scale = area_scale * adjustment.
# Set to 1.0 for pure area scaling, adjust empirically if needed.
# For stroke shapes (plus, cross, asterisk): base scale is 1.0 (bounding-based)
# since they have no fill area.
.manualScaleAdjustments <- c(
  circle = 1.00,
  diamond = 1.00,
  triangle = 0.90,
  square = 1.10,
  invertedTriangle = 0.90,
  cross = 0.95,
  plus = 1.10,
  asterisk = 1.00,
  star = 0.90,
  pentagon = 1.00,
  hexagon = 1.00
)

# Shape specifications for OSP shapes (base definitions without scale).
# Order matches ospShapeNames.
.ospShapeSpecBase <- list(
  # Filled shapes

  circle = list(kind = "polygon", n = 64, angle = 0, open = FALSE),
  diamond = list(kind = "polygon", n = 4, angle = 0, open = FALSE),
  triangle = list(
    kind = "polygon",
    n = 3,
    angle = 90,
    open = FALSE,
    yOffset = -0.22
  ),
  square = list(kind = "polygon", n = 4, angle = 45, open = FALSE),
  invertedTriangle = list(
    kind = "polygon",
    n = 3,
    angle = -90,
    open = FALSE,
    yOffset = 0.22
  ),
  cross = list(kind = "stroke", glyph = "cross", thick = TRUE),
  plus = list(kind = "stroke", glyph = "plus", thick = TRUE),
  asterisk = list(kind = "stroke", glyph = "asterisk", thick = TRUE),
  star = list(kind = "star", points = 5, open = FALSE),
  pentagon = list(kind = "polygon", n = 5, angle = 90, open = FALSE),
  hexagon = list(kind = "polygon", n = 6, angle = 90, open = FALSE),
  # Thin shapes
  thinCross = list(kind = "stroke", glyph = "cross", thick = FALSE),
  thinPlus = list(kind = "stroke", glyph = "plus", thick = FALSE),
  # Open shapes
  circleOpen = list(kind = "polygon", n = 64, angle = 0, open = TRUE),
  diamondOpen = list(kind = "polygon", n = 4, angle = 0, open = TRUE),
  triangleOpen = list(
    kind = "polygon",
    n = 3,
    angle = 90,
    open = TRUE,
    yOffset = -0.22
  ),
  squareOpen = list(kind = "polygon", n = 4, angle = 45, open = TRUE),
  invertedTriangleOpen = list(
    kind = "polygon",
    n = 3,
    angle = -90,
    open = TRUE,
    yOffset = 0.22
  ),
  starOpen = list(kind = "star", points = 5, open = TRUE),
  pentagonOpen = list(kind = "polygon", n = 5, angle = 90, open = TRUE),
  hexagonOpen = list(kind = "polygon", n = 6, angle = 90, open = TRUE),
  blank = list(kind = "blank")
)

# Map variant names to base shape names for adjustment lookup.
# Order matches ospShapeNames.
.shapeBaseNames <- c(
  # Filled shapes
  circle = "circle",
  diamond = "diamond",
  triangle = "triangle",
  square = "square",
  invertedTriangle = "invertedTriangle",
  cross = "cross",
  plus = "plus",
  asterisk = "asterisk",
  star = "star",
  pentagon = "pentagon",
  hexagon = "hexagon",
  # Thin shapes
  thinCross = "cross",
  thinPlus = "plus",
  # Open shapes
  circleOpen = "circle",
  diamondOpen = "diamond",
  triangleOpen = "triangle",
  squareOpen = "square",
  invertedTriangleOpen = "invertedTriangle",
  starOpen = "star",
  pentagonOpen = "pentagon",
  hexagonOpen = "hexagon",
  blank = "blank"
)

#' Build shape specs with computed area-based scaling
#' @return list of shape specifications with scale factors
#' @keywords internal
.buildOspShapeSpec <- function() {
  circleVerts <- .polyVertices(64, 0)
  circleArea <- .shoelaceArea(circleVerts$x, circleVerts$y)

  specs <- .ospShapeSpecBase
  for (name in names(specs)) {
    spec <- specs[[name]]

    # Compute area-based scale for polygon/star shapes
    # Stroke shapes (plus, cross, asterisk) use base scale of 1.0
    areaScale <- .computeAreaScale(spec, circleArea)

    # Apply manual adjustment
    baseName <- .shapeBaseNames[name]
    adjustment <- if (!is.na(baseName)) .manualScaleAdjustments[baseName] else
      NA
    if (is.na(adjustment)) adjustment <- 1

    specs[[name]]$scale <- areaScale * adjustment
  }

  specs
}

# Shape specifications with computed scale factors (built at load time)
.ospShapeSpec <- .buildOspShapeSpec()
