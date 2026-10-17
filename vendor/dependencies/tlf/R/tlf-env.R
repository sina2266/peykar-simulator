#' @title tlfEnv
#' @description
#' Environment that holds various global variables and settings for the tlf library.
#' It is not exported and should not be directly manipulated by other packages.
#' @field packageName Name of the package.
#' This will be used to retrieve information on the package at run time.
#' @field defaultTagPosition Default tag position of subplots.
#' @field defaultHorizontalJustification Default horizontal justification for plot labels.
#' @field defaultVerticalJustificationDefault horizontal vertical for plot labels.
#' @field defaultExportParameters
#' Default properties for exporting/saving plots. Use `setDefaultExportParameters()` to change its values.
#' @field defaultAggregation
#' Default aggregation binning, functions and labels for aggregating data.
#' Use `setDefaultAggregationBins()`, `setDefaultAggregationFunctions()`
#' and `setDefaultAggregationLabels()` to change its values.
#' @field logTicks
#' Default tick values when using log ticks. Use `setDefaultLogTicks()` to change its values.
#' @field lnTicks Default tick values when using ln ticks
#' @field logMinorTicks Default minor tick values when using log ticks
#' @field defaultErrorbarCapSize
#' Default cap size displayed for errorbars. Use `setDefaultErrorbarCapSize()` to change its values.
#' Currently `defaultErrorbarCapSize=0`, which hides the caps.
#' @field defaultLLOQLinetype
#' LLOQ lines are of dotted type in the default settings. Use `setDefaultLLOQLinetype()` to change its values.
#' @field DefaultAlphaRatio
#' Points bellow LLOQ have a transparency level at `.618 * base alpha level`.
#' The `.618` value (1/golden ratio) ensures that both levels are always visible and
#' distinguishable even if the user set a low value for default alpha.
#' Use `setDefaultAlphaRatio()` to change its values.
#' @field maxCharacterWidth
#' Maximum character width before wrap for axis ticks labels and legend
#' Use `setDefaultMaxCharacterWidth()` to change its values.
#' @field currentTheme
#' Current `Theme` object used by plots providing their default plot configuration values.
#' Use `useTheme()` to change its values.
#' @keywords internal
tlfEnv <- new.env(parent = emptyenv())
tlfEnv$packageName <- "tlf"
tlfEnv$defaultTagPosition <- TagPositions$topLeft
tlfEnv$defaultHorizontalJustification <- HorizontalJustification$left
tlfEnv$defaultVerticalJustification <- VerticalJustification$bottom
tlfEnv$defaultExportParameters <- list(
  format = "png",
  width = 16,
  height = 9,
  units = "cm",
  dpi = 300,
  name = "figure"
)
tlfEnv$defaultAggregation <- list(
  functions = list(
    y = median,
    ymin = function(x) {
      as.numeric(stats::quantile(x, probs = 5 / 100))
    },
    ymax = function(x) {
      as.numeric(stats::quantile(x, probs = 95 / 100))
    }
  ),
  labels = list(
    y = "median",
    range = "[5th-95th] percentiles"
  ),
  bins = 10
)
tlfEnv$logTicks <- 10^seq(-10, 10)
tlfEnv$lnTicks <- exp(seq(-10, 10))
tlfEnv$logMinorTicks <- rep(seq(1, 9), 21) * rep(10^seq(-10, 10), each = 9)
tlfEnv$defaultErrorbarCapSize <- 2
tlfEnv$defaultLLOQLinetype <- "dotted"
tlfEnv$DefaultAlphaRatio <- 0.618
tlfEnv$maxCharacterWidth <- 50
