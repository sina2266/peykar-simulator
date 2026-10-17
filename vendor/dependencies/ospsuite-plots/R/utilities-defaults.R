# Default Theme -------------

#' @title OSPSuite plot theme
#' @description A `ggplot2` theme with OSPSuite-specific styling, based on
#'   [ggplot2::theme_bw()]. Apply it to a single plot with `plot + theme_osp()`
#'   without altering the global `ggplot2` state.
#'
#' @param base_size base font size, given in pts (passed on to [ggplot2::theme_bw()]).
#' @param ... further arguments passed on to [ggplot2::theme_bw()]
#'   (for example `base_family`).
#'
#' @return a complete `ggplot2` theme object.
#' @examples
#' library(ggplot2)
#' ggplot(mtcars, aes(x = wt, y = mpg)) +
#'   geom_point() +
#'   theme_osp()
#'
#' @export
#' @family setDefault functions
theme_osp <- function(base_size = 11, ...) {
  # Following ggplot2's guidance for theme functions: build on a complete
  # theme with `%+replace%` and fully specify every element that is touched,
  # since `%+replace%` discards unspecified element properties (unlike `+`).
  halfLine <- base_size / 2

  theme_bw(base_size = base_size, ...) %+replace%
    theme(
      legend.position = "right",
      legend.direction = "vertical",
      legend.justification = 0.5,
      plot.title = element_text(
        size = rel(1.2),
        hjust = 0.5,
        vjust = 0.5,
        margin = margin(b = halfLine)
      ),
      plot.subtitle = element_text(
        hjust = 0.5,
        vjust = 0.5,
        margin = margin(b = halfLine)
      ),
      panel.grid.minor = element_blank(),
      strip.background = element_rect(color = NA, fill = NA),
      complete = TRUE
    )
}


#' @title set the default theme
#'
#' @description
#' `r lifecycle::badge("deprecated")`
#'
#' set the OSPSuite theme as the global `ggplot2` theme for the whole session
#' via [ggplot2::theme_set()].
#'
#' `ospsuite.plots` plot functions now apply [theme_osp()] per plot, so this
#' global mutation is no longer needed for them. Use `plot + theme_osp()` to
#' style an individual (non-`ospsuite.plots`) plot instead.
#'
#' @return invisibly return the previous theme so you can easily save it, then later restore it.
#' @examples
#' \dontrun{
#' oldTheme <- setDefaultTheme()
#'
#' # Create a plot with the new theme
#' p <- ggplot(mtcars, aes(x = wt, y = mpg)) +
#'   geom_point()
#' print(p)
#'
#' # Restore previous theme
#' resetDefaultTheme(oldTheme)
#' }
#'
#' @export
#' @family setDefault functions
setDefaultTheme <- function() {
  lifecycle::deprecate_soft(
    when = "1.3.0",
    what = "setDefaultTheme()",
    details = paste(
      "ospsuite.plots plots are now themed per plot.",
      "Use `plot + theme_osp()` to theme an individual plot."
    )
  )
  return(invisible(ggplot2::theme_set(theme_osp())))
}


#' reset the default theme
#'
#' wrapper for `ggplot2::theme_set(oldTheme)`
#'
#' @param oldTheme theme to set
#'
#' @export
#'
#' @family setDefault functions
resetDefaultTheme <- function(oldTheme) {
  ggplot2::theme_set(oldTheme)
}


# Default ColorMap Discrete -------------

#' @title Color maps
#'
#' @description
#'
#' List with some color maps for `Theme` object.
#'
#' The `ospDefault` color map is based on colors in `default_igv` qualitative
#' color palette from `{ggsci}` package.
#'
#' @family setDefault functions
#' @export
colorMaps <- list(
  # nolint: object_name_linter
  default = c("#0078D7", "#D83B01", "#107C10", "#A80000", "#002050", "#B4009E"),
  grays = paste("gray", seq(0, 100, 10), sep = ""),
  prism = c(
    "#FF0000",
    "#FF7F00",
    "#FFFF00",
    "#00FF00",
    "#0000FF",
    "#4B0082",
    "#8F00FF"
  ),
  blot = c("blue", "magenta", "cyan", "green", "yellow", "red"),
  temperature = c(
    "#262A76",
    "#234990",
    "#2F8AC3",
    "#26B0D2",
    "#FFC1CB",
    "#EB559",
    "#AE3535",
    "8E1F20"
  ),
  ospDefault = c(
    "#5050FFFF",
    "#CE3D32FF",
    "#749B58FF",
    "#F0E685FF",
    "#466983FF",
    "#BA6338FF",
    "#5DB1DDFF",
    "#802268FF",
    "#6BD76BFF",
    "#D595A7FF",
    "#924822FF",
    "#837B8DFF",
    "#C75127FF",
    "#D58F5CFF",
    "#7A65A5FF",
    "#E4AF69FF",
    "#3B1B53FF",
    "#CDDEB7FF",
    "#612A79FF",
    "#AE1F63FF",
    "#E7C76FFF",
    "#5A655EFF",
    "#CC9900FF",
    "#99CC00FF",
    "#A9A9A9FF",
    "#CC9900FF",
    "#99CC00FF",
    "#33CC00FF",
    "#00CC33FF",
    "#00CC99FF",
    "#0099CCFF",
    "#0A47FFFF",
    "#4775FFFF",
    "#FFC20AFF",
    "#FFD147FF",
    "#990033FF",
    "#991A00FF",
    "#996600FF",
    "#809900FF",
    "#339900FF",
    "#00991AFF",
    "#009966FF",
    "#008099FF",
    "#003399FF",
    "#1A0099FF",
    "#660099FF",
    "#990080FF",
    "#D60047FF",
    "#FF1463FF",
    "#00D68FFF",
    "#14FFB1FF"
  )
)


# Per-plot color scales -------------

#' @title OSP discrete color and fill scales
#'
#' @description
#' Discrete `ggplot2` scales that apply the OSPSuite color palette per plot,
#' without mutating global `ggplot2` state. Add them to a plot with
#' `plot + scale_colour_osp()` or `plot + scale_fill_osp()`. All
#' `ospsuite.plots` plot functions apply these automatically (unless a color
#' or fill scale is already present), so explicit use is only needed to style
#' unrelated plots or to override a different scale.
#'
#' The palette reproduces the previous global behavior of [setDefaults()]:
#' `colorMaps$default` (6 colors) is used when there are at most 6 groups,
#' otherwise `colorMaps$ospDefault` (50 colors).
#'
#' @param ... further arguments passed on to [ggplot2::discrete_scale()].
#'
#' @return a discrete `ggplot2` scale.
#' @examples
#' library(ggplot2)
#' df <- data.frame(x = 1:3, y = 1:3, group = c("A", "B", "C"))
#' ggplot(df, aes(x, y, color = group)) +
#'   geom_point() +
#'   scale_colour_osp()
#'
#' @export
#' @rdname scale_osp
#' @family scales
# nolint start: object_name_linter
scale_colour_osp <- function(...) {
  ggplot2::discrete_scale(
    aesthetics = "colour",
    palette = .ospColorPalette,
    ...
  )
}

#' @export
#' @rdname scale_osp
scale_color_osp <- scale_colour_osp

#' @export
#' @rdname scale_osp
scale_fill_osp <- function(...) {
  ggplot2::discrete_scale(
    aesthetics = "fill",
    palette = .ospColorPalette,
    ...
  )
}
# nolint end

#' OSP color palette function
#'
#' Reproduces the stacked-palette selection of the previous global
#' `ggplot2.discrete.*` options: `colorMaps$default` for up to 6 groups,
#' otherwise `colorMaps$ospDefault`.
#'
#' @param n number of colors needed
#' @return character vector of colors of length `n`
#' @keywords internal
.ospColorPalette <- function(n) {
  fewColors <- colorMaps$default
  manyColors <- colorMaps$ospDefault

  pal <- if (n <= length(fewColors)) fewColors else manyColors

  if (n > length(pal)) {
    warning(
      "Number of groups (",
      n,
      ") exceeds available colors (",
      length(pal),
      "). ",
      "Colors will be recycled.",
      call. = FALSE
    )
    pal <- rep(pal, length.out = n)
  }

  pal[seq_len(n)]
}


#' @param colorMapList list of color-maps to be set
#'
#' @title set the default color-map for discrete colors
#' @description Sets default color mappings for discrete color and fill aesthetics in ggplot2.
#'   Each color map should be a vector of valid color values (hex codes, color names, etc.).
#'
#' @return list with color-maps previously set
#' @examples
#' \dontrun{
#' # Set custom color maps
#' customColors <- list(
#'   c("#FF0000", "#00FF00", "#0000FF"), # RGB colors
#'   c("red", "green", "blue") # Named colors
#' )
#' oldColors <- setDefaultColorMapDistinct(customColors)
#'
#' # Use default OSP color maps
#' setDefaultColorMapDistinct()
#'
#' # Reset to previous colors
#' resetDefaultColorMapDistinct(oldColors)
#' }
#'
#' @family setDefault functions
#' @export
#'
setDefaultColorMapDistinct <- function(colorMapList = NULL) {
  # Convert character vector to a list if it's not NULL
  if (!is.null(colorMapList) && is.character(colorMapList)) {
    colorMapList <- list(colorMapList)
  }
  # Validate colorMapList structure
  validateColors <- function(colorVector, varName) {
    for (color in colorVector) {
      if (any(is.na(grDevices::col2rgb(color, alpha = FALSE)))) {
        stop(messages$errorInvalidColor(color, varName))
      }
    }
  }
  if (!is.null(colorMapList)) {
    checkmate::assertList(colorMapList, min.len = 1)
    # Validate that each element is a character vector of colors
    for (i in seq_along(colorMapList)) {
      checkmate::assertCharacter(
        colorMapList[[i]],
        min.len = 1,
        .var.name = paste0("colorMapList[[", i, "]]")
      )
      # Validate colors using the helper function
      validateColors(colorMapList[[i]], paste0("colorMapList[[", i, "]]"))
    }
  }

  if (is.null(colorMapList)) {
    colorMapList <- list(
      colorMaps[["default"]],
      colorMaps[["ospDefault"]]
    )
  }

  optionNames <- c(
    "ggplot2.discrete.colour",
    "ggplot2.discrete.fill"
  )

  # Capture previous values, preserving the names even when an option is unset
  # (a plain `list[[name]] <- NULL` would drop the name); the names are needed
  # for a correct round-trip through resetDefaultColorMapDistinct().
  oldColorOptions <- stats::setNames(
    lapply(optionNames, getOption),
    optionNames
  )

  newColorOptions <- stats::setNames(
    rep(list(colorMapList), length(optionNames)),
    optionNames
  )

  options(newColorOptions)

  return(invisible(oldColorOptions))
}


#' @title reset the default color map for discrete colors
#'
#' @param oldColorMaps list of color maps previously set
#'
#' @export
#' @family setDefault functions
#'
resetDefaultColorMapDistinct <- function(oldColorMaps) {
  checkmate::assertList(oldColorMaps, names = "named")

  options(ggplot2.discrete.colour = oldColorMaps[["ggplot2.discrete.colour"]])
  options(ggplot2.discrete.fill = oldColorMaps[["ggplot2.discrete.fill"]])
  options(ggplot2.ordinal.colour = oldColorMaps[["ggplot2.ordinal.colour"]])
  options(ggplot2.ordinal.fill = oldColorMaps[["ggplot2.ordinal.fill"]])

  return(invisible(NULL))
}


# Options ----------------

#' get list of default options
#'
#' @family setDefault functions
#' @return names list with default options
#' @export
#'
getDefaultOptions <- function() {
  # Single sources of truth shared by the per-geom attribute defaults below.
  # `defaultAlpha` is seeded from the live `ospsuite.plots.alpha` option (with a
  # literal fallback to avoid recursion, since this function also defines that
  # option), so `options(ospsuite.plots.alpha = ...)` flows into the geom
  # attribute defaults the next time a plot function reads them. These defaults
  # are applied per layer, so OSP plots get the OSP look without relying on the
  # global `update_geom_defaults()` mutation done by `setDefaults()`.
  defaultAlpha <- getOption("ospsuite.plots.alpha", default = 0.5)
  defaultLinewidth <- 1.0

  optionList <- list(
    # watermark
    ospsuite.plots.watermarkEnabled = TRUE,
    ospsuite.plots.watermarkLabel = "preliminary analysis",
    ospsuite.plots.watermarkFormat = list(
      x = 0.5,
      y = 0.5,
      color = "lightgrey",
      angle = 30,
      fontsize = 12,
      alpha = 0.7
    ),
    # geom attributes
    ospsuite.plots.geomLineAttributes = list(linewidth = defaultLinewidth),
    ospsuite.plots.geomRibbonAttributes = list(
      color = NA,
      alpha = defaultAlpha
    ),
    ospsuite.plots.geomPointAttributes = list(),
    ospsuite.plots.geomErrorbarAttributes = list(width = 2),
    ospsuite.plots.geomLLOQAttributes = list(),
    ospsuite.plots.geomComparisonLineAttributes = list(
      linetype = "dashed",
      linewidth = defaultLinewidth
    ),
    ospsuite.plots.geomGuestLineAttributes = list(
      linetype = "dashed",
      linewidth = defaultLinewidth
    ),
    ospsuite.plots.geomBoxplotAttributes = list(
      position = position_dodge(width = 1),
      color = "black",
      alpha = defaultAlpha
    ),
    ospsuite.plots.geomHistAttributes = list(
      bins = 10,
      position = ggplot2::position_nudge(),
      color = "black",
      alpha = defaultAlpha
    ),
    # default alpha
    ospsuite.plots.alpha = defaultAlpha,
    # alpha of LLOQ values
    ospsuite.plots.lloqAlphaVector = c("TRUE" = 0.3, "FALSE" = 1),
    # linetype of LLOQ
    ospsuite.plots.lloqLineType = "dashed",
    # percentiles
    ospsuite.plots.percentiles = c(0.05, 0.25, 0.5, 0.75, 0.95),
    # default percentiles (subset of percentiles used as default in plot functions)
    ospsuite.plots.defaultPercentiles = c(0.05, 0.5, 0.95),
    # used for plot export
    ospsuite.plots.exportWidth = 16,
    ospsuite.plots.exportUnits = "cm",
    ospsuite.plots.exportDevice = "png",
    ospsuite.plots.exportDpi = 300
  )

  return(optionList)
}

# Geoms ----------

#' get the defaults for the geom attributes used as defaults in plot functions
#' see \code{vignette("ospsuite.plots", package = "ospsuite.plots")} how to change defaults
#'
#' @param geom  type of geom to return attributes
#'
#' @family setDefault functions
#' @return list with default attributes
#' @export
#'
getDefaultGeomAttributes <- function(geom) {
  choices <- names(getDefaultOptions())
  choices <- gsub(
    "Attributes",
    "",
    gsub(
      "ospsuite.plots.geom",
      "",
      choices[grep("ospsuite.plots.geom", choices)]
    )
  )
  checkmate::assertChoice(geom, choices = choices)

  optionName <- paste0("ospsuite.plots.geom", geom, "Attributes")

  defaults <- getOption(optionName, default = getDefaultOptions()[[optionName]])

  return(defaults)
}

#' returns an option value for a option defined by the package OSPSuite.plots
#'
#'
#' @param optionKey identifier of option
#'
#' @return option value
#' @export
#' @family setDefault functions
#'
#' @examples
#' \dontrun{
#' getOspsuite.plots.option(optionKey = OptionKeys$watermarkEnabled)
#' }
# fmt: skip
getOspsuite.plots.option <- function(optionKey) { # nolint: object_name_linter
  checkmate::assert_choice(optionKey, choices = names(OptionKeys))

  return(getOption(
    paste0("ospsuite.plots.", optionKey),
    getDefaultOptions()[[paste0("ospsuite.plots.", optionKey)]]
  ))
}


#' Set OSPSuite plots option with a given key and value.
#'
#' @param optionKey The key for the option.
#' @param value The value for the option.
#'
#' @return NULL
#' @export
#' @family setDefault functions
#'
#' @examples
#' \dontrun{
#' setOspsuite.plots.option(optionKey = OptionKeys$watermarkEnabled, value = TRUE)
#' }
# fmt: skip
setOspsuite.plots.option <- function(optionKey, value) { # nolint: object_name_linter
  checkmate::assert_choice(optionKey, choices = names(OptionKeys))

  if (is.null(value)) {
    eval(parse(
      text = paste0(
        "options(ospsuite.plots.",
        optionKey,
        " = NULL)"
      )
    ))
  } else {
    newOption <- list()
    newOption[[paste0("ospsuite.plots.", optionKey)]] <- value
    options(newOption)
  }

  return(invisible(NULL))
}

# Wrapper for all defaults  ----------------------

#' sets the defaults for the OSPSuite.plots package
#'
#' @description
#' `r lifecycle::badge("deprecated")`
#'
#' Mutates global `ggplot2` state (theme, geom defaults and discrete color
#' options) for the whole session.
#'
#' `ospsuite.plots` plot functions now apply the full OSPSuite styling per plot,
#' so this is no longer needed for them. To style individual (non-`ospsuite.plots`)
#' plots, compose the per-plot constructors instead:
#' `plot + theme_osp() + scale_colour_osp() + scale_fill_osp()`.
#'
#' for detailed information see
#' \code{vignette("ospsuite.plots", package = "ospsuite.plots")}
#'
#'
#' @param colorMapList list of color maps
#' @param defaultOptions list of options
#'
#' @return list of old settings which can be used to reset defaults with `resetDefaults()`
#'
#' @family setDefault functions
#' @export
setDefaults <- function(
  defaultOptions = list(),
  colorMapList = NULL
) {
  lifecycle::deprecate_soft(
    when = "1.3.0",
    what = "setDefaults()",
    details = paste(
      "ospsuite.plots plots are now styled per plot.",
      "Use `plot + theme_osp() + scale_colour_osp() + scale_fill_osp()`",
      "to style an individual plot."
    )
  )
  checkmate::assertList(colorMapList, null.ok = TRUE)
  checkmate::assertList(defaultOptions, null.ok = TRUE)

  # initialize return value
  oldDefaults <- list()

  # append default options with user input
  defaultOptions <- utils::modifyList(
    getDefaultOptions(),
    defaultOptions
  )

  # options
  oldDefaults$options <- lapply(names(getDefaultOptions()), getOption)
  names(oldDefaults$options) <- names(getDefaultOptions())

  options(defaultOptions)

  defaultAlpha <- getOption(
    "ospsuite.plots.alpha",
    getDefaultOptions()[["ospsuite.plots.alpha"]]
  )

  # use the same default line width as the per-plot geom attributes
  defaultLinewidth <- getDefaultGeomAttributes("Line")$linewidth

  # get old settings of defaults for geoms
  nsenv <- asNamespace("ggplot2")

  oldDefaults[["geomPoint"]] <- get("GeomPoint", envir = nsenv)$default_aes
  oldDefaults[["geomBoxplot"]] <- get("GeomBoxplot", envir = nsenv)$default_aes
  oldDefaults[["geomBar"]] <- get("GeomBar", envir = nsenv)$default_aes
  oldDefaults[["geomRibbon"]] <- get("GeomRibbon", envir = nsenv)$default_aes
  oldDefaults[["geomLine"]] <- get("GeomLine", envir = nsenv)$default_aes

  # set theme, color and shapes
  # (call theme_set() directly rather than the deprecated setDefaultTheme()
  # wrapper, so setDefaults() only emits its own deprecation warning)
  oldDefaults[["theme"]] <- ggplot2::theme_set(theme_osp())
  oldDefaults[["colorMaps"]] <- setDefaultColorMapDistinct(
    colorMapList = colorMapList
  )

  # Set geom_point to use shape 1 (open circle) for backwards compatibility
  # when raw geom_point() is used. Our custom shape names from ospShapeNames
  # only work with geom_point_osp.
  update_geom_defaults(
    "point",
    list(shape = 1)
  )

  # set geoms
  update_geom_defaults(
    "boxplot",
    list(
      fill = getOption("ggplot2.discrete.fill")[[1]][[1]],
      alpha = defaultAlpha
    )
  )
  update_geom_defaults(
    "bar",
    list(
      fill = getOption("ggplot2.discrete.fill")[[1]][[1]],
      alpha = defaultAlpha,
      color = "black"
    )
  )
  update_geom_defaults(
    "ribbon",
    list(
      fill = getOption("ggplot2.discrete.fill")[[1]][[1]],
      alpha = defaultAlpha
    )
  )
  update_geom_defaults(
    "line",
    list(
      linewidth = defaultLinewidth,
      fill = getOption("ggplot2.discrete.fill")[[1]][[1]]
    )
  )

  return(invisible(oldDefaults))
}


#' restore to previously stored settings
#'
#' @param oldDefaults `list`with previously stored settings
#'
#' @family setDefault functions
#' @export
#'
resetDefaults <- function(oldDefaults) {
  ggplot2::theme_set(oldDefaults$theme)

  options(oldDefaults$options)

  if (!is.null(oldDefaults$geomBar)) {
    update_geom_defaults("bar", oldDefaults$geomBar)
  }
  if (!is.null(oldDefaults$geomBoxplot)) {
    update_geom_defaults("boxplot", oldDefaults$geomBoxplot)
  }
  if (!is.null(oldDefaults$geomPoint)) {
    update_geom_defaults("point", oldDefaults$geomPoint)
  }
  if (!is.null(oldDefaults$geomRibbon)) {
    update_geom_defaults("ribbon", oldDefaults$geomRibbon)
  }
  if (!is.null(oldDefaults$geomLine)) {
    update_geom_defaults("line", oldDefaults$geomLine)
  }

  resetDefaultColorMapDistinct(oldColorMaps = oldDefaults$colorMaps)
}
