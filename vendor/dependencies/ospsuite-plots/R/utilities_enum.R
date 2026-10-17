#' @title enumeration keys for OSPSuite.plots options
#' @export
OptionKeys <- enum(c( # nolint
  gsub("ospsuite.plots.", "", names(getDefaultOptions()))
))

#' @title enumeration keys for OSPSuite.plots scaling options for axis scalings
#' @export
AxisScales <- enum(c( # nolint
  linear = "linear",
  log = "log",
  discrete = "discrete"
))

#' @title enumeration keys for mode of Binning
#' @export
BINNINGMODE <- ospsuite.utils::enum(c( # nolint
  number = "Equal Frequency Binning",
  interval = "Equal Width Binning",
  breaks = "Custom Binning"
))
