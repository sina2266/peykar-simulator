#' @title Generates Histograms
#' @description
#' Produces histograms with optional distribution fit.
#'
#' For more details and examples see the vignettes:
#' * \code{vignette("Histogram Plots", package = "ospsuite.plots")}
#' * \code{vignette("ospsuite.plots", package = "ospsuite.plots")}
#' * \code{vignette("Goodness of fit", package = "ospsuite.plots")}
#'
#' @inheritParams plotTimeProfile
#' @inheritParams plotYVsX
#' @param plotAsFrequency A `logical` indicating if the histogram displays frequency on the y-axis.
#' @param asBarPlot A `logical` indicating if `geom_histogram` should be used (for continuous data) or `geom_bar` (for categorical data).
#'    If TRUE, the variables `distribution`, `meanFunction`, `xScale`, and `xScaleArgs` are ignored.
#' @param geomHistAttributes A `list` of arguments passed to `ggplot2::geom_histogram` (or `geom_bar` if `asBarPlot` = TRUE).
#' @param distribution Name of the distribution to fit. Available distributions are those in the `stats` package (see ?stats::distributions):
#'   `norm`, `lnorm`, `weibull`, `gamma`, etc. Use `"none"` for no fit (default).
#'   Shortcuts: `"normal"` (same as `"norm"`), `"lognormal"` (same as `"lnorm"`).
#' @param meanFunction Function selection for the display of a vertical line. Options: `'none'`, `'mean'`, `'geomean'`, `'median'`, `'auto'` (default).
#'   `'auto'` selects `'mean'` for normal distribution, `'geomean'` for lognormal, `'median'` for other distributions, and `'none'` when no distribution fit.
#' @param residualScale Deprecated. Retained for backward compatibility only.
#'   Non-`NULL` values trigger a warning and have no effect.
#'
#' @return A `ggplot` object.
#' @export
#' @family plot functions
plotHistogram <- function(
  data,
  mapping,
  metaData = NULL,
  asBarPlot = NULL,
  geomHistAttributes = getDefaultGeomAttributes("Hist"),
  plotAsFrequency = FALSE,
  xScale = AxisScales$linear,
  xScaleArgs = list(),
  yScale = AxisScales$linear,
  yScaleArgs = list(),
  distribution = "none",
  meanFunction = "auto",
  residualScale = NULL
) {
  if (!is.null(residualScale)) {
    warning(messages$warningResidualScaleDeprecated())
  }
  #----- Validation and formatting of input arguments
  checkmate::assertList(metaData, types = "list", null.ok = TRUE)

  checkmate::assertFlag(plotAsFrequency)
  if (plotAsFrequency & "y" %in% names(mapping)) {
    warning(messages$warningPlotAsFrequencyOverwritesY())
  }

  checkmate::assertList(geomHistAttributes, null.ok = FALSE, min.len = 0)

  if (!is.null(xScale))
    xScale <- match.arg(xScale, c(AxisScales$linear, AxisScales$log))
  if (!is.null(yScale))
    yScale <- match.arg(yScale, c(AxisScales$linear, AxisScales$log))

  checkmate::assertList(xScaleArgs, null.ok = FALSE, min.len = 0)
  checkmate::assertList(yScaleArgs, null.ok = FALSE, min.len = 0)

  #-  map Data
  mappedData <- MappedData$new(
    data = data,
    mapping = mapping,
    groupAesthetics = "fill",
    xScale = xScale,
    yScale = yScale
  )
  mappedData$addMetaData(metaData)

  if (is.null(asBarPlot)) {
    asBarPlot <- !(mappedData$columnClasses[["x"]] %in% c("numeric"))
  }
  if (asBarPlot) {
    geomHistAttributes$bins <- NULL
  }

  plotHelper <- plotHelperHistogram$new(
    xScale = xScale,
    plotAsFrequency = plotAsFrequency,
    asBarPlot = asBarPlot,
    geomHistAttributes = geomHistAttributes,
    distribution = distribution,
    meanFunction = meanFunction
  )

  #-  initialize plot
  plotObject <- initializePlot(
    mappedData
  )

  if (plotAsFrequency) {
    plotObject <-
      plotObject + labs(y = "Relative Frequency")
  }

  # adds histogram
  plotObject <- plotObject +
    do.call(
      what = ifelse(
        plotHelper$asBarPlot,
        ggplot2::geom_bar,
        ggplot2::geom_histogram
      ),
      args = c(
        list(
          mapping = plotHelper$getHistMapping,
          na.rm = TRUE
        ),
        geomHistAttributes
      )
    )

  # add x-axis before distribution fit
  if (!plotHelper$asBarPlot) {
    plotObject <- addXScale(
      plotObject = plotObject,
      xScale = xScale,
      xScaleArgs = xScaleArgs
    )
  }
  plotObject <- addYScale(
    plotObject = plotObject,
    yScale = yScale,
    yScaleArgs = yScaleArgs
  )

  # distribution fit or mean will overwrite y label, save it to reset to current value
  ylabel <- plotObject$labels$y

  # OSP default line width, applied to the line-based overlays below so they
  # match the styling of the main data lines without relying on global state.
  lineWidth <- getDefaultGeomAttributes("Line")$linewidth

  # 2 - Distribution fit

  if (plotHelper$distribution != "none") {
    distrMapping <- plotHelper$getDistrMapping(mappedData, plotObject)

    plotObject <-
      plotObject +
      stat_theodensity(
        mapping = distrMapping,
        distri = plotHelper$distribution,
        inherit.aes = !plotHelper$isStacked,
        linewidth = lineWidth
      )
  }

  # 3 - Mean as vertical line

  if (!is.null(plotHelper$scaledMeanFun)) {
    meanMapping <- plotHelper$getMeanMapping(mappedData)

    plotObject <-
      plotObject +
      stat_summary(
        mapping = meanMapping,
        fun = plotHelper$scaledMeanFun,
        geom = "vline",
        orientation = "y",
        inherit.aes = !plotHelper$isStacked,
        linewidth = lineWidth
      )
  }

  # reset ylabel
  plotObject <- plotObject +
    labs(y = ylabel)

  return(plotObject)
}

# plotHelper -----------

#' @title class to support `plotHistogram`
#' @description  R6 class container for functions and properties used in `plotHistogram`
#' @keywords internal
plotHelperHistogram <- R6::R6Class(
  # nolint
  "plotHelperHistogram",
  public = list(
    #' @field plotAsFrequency `boolean` to decide if data should be displayed as frequency
    plotAsFrequency = NULL,
    #' @field distribution `character` distribution to fit
    distribution = NULL,
    #' @field scaledMeanFun `function` to display mean
    scaledMeanFun = NULL,
    #' @field isStacked `boolean` if True, frequency is displayed a per total, otherwise per group
    isStacked = FALSE,
    #' @field asBarPlot `boolean` indicates plot switches to bar plot
    asBarPlot = FALSE,

    #' @param xScale scale of x -axis
    #' @param plotAsFrequency A `boolean` to switch display of y to frequency
    #' @param asBarPlot A `boolean` to switch from geom_histogram to geom_bar
    #' @param geomHistAttributes attribute for plotting the histogram
    #' @param distribution  distribution to fit the data
    #' @param meanFunction  function to display mean
    #'
    #' @description Create a new `plotHelperHistogram` object.
    #' @return A new `plotHelperHistogram` object
    initialize = function(
      xScale,
      plotAsFrequency,
      asBarPlot,
      geomHistAttributes,
      distribution,
      meanFunction
    ) {
      self$plotAsFrequency <- plotAsFrequency

      self$distribution <- private$validateDistribution(distribution)

      self$scaledMeanFun <- private$selectMeanFunction(
        meanFunction = meanFunction,
        xScale = xScale
      )
      # stack means a difference in calculating relative frequency
      self$isStacked <- (is.character(geomHistAttributes$position) &&
        geomHistAttributes$position == "stack")

      # data is categorical
      self$asBarPlot <- asBarPlot
      if (self$asBarPlot) {
        if (self$distribution != "none") {
          warning(messages$warningCannotFitDistributionCategorical())
          self$distribution <- "none"
        }
        if (!is.null(self$scaledMeanFun)) {
          warning(messages$warningCannotCalculateMeanCategorical())
          self$scaledMeanFun <- NULL
        }
      }

      return(invisible(self))
    },
    #' @description Generates mapping for distribution.
    #' @param mappedData mapped data of Histogram
    #' @param plotObject ggplot object
    #' @return mapping
    getDistrMapping = function(mappedData, plotObject) {
      binWidth <- private$getBinWidthforPlot(plotObject = plotObject)

      # add mapping for distribution
      if (self$plotAsFrequency) {
        newMapping <- aes(y = after_stat(density))
      } else {
        newMapping <- aes(y = after_stat(count * binWidth))
      }
      newMapping$x <- mappedData$mapping$x
      if (!self$isStacked) {
        newMapping$colour <- mappedData$mapping$fill
      }
      return(newMapping)
    },
    #' @description Generates mapping for display of mean.
    #' @param mappedData mapped data of Histogram
    #' @return mapping
    getMeanMapping = function(mappedData) {
      meanMapping <- aes(xintercept = after_stat(x), y = 0)
      meanMapping$x <- mappedData$mapping$x
      if (!self$isStacked) {
        meanMapping$colour <- mappedData$mapping$fill
      }
      return(meanMapping)
    }
  ),
  active = list(
    #' @field getHistMapping generates mapping for histogram
    getHistMapping = function() {
      newMapping <- aes()
      if (self$plotAsFrequency) {
        if (self$isStacked) {
          newMapping <- aes(
            y = after_stat(count) / sum(count) / after_stat(width)
          )
        } else {
          newMapping <- aes(y = after_stat(density))
        }
      }
      return(newMapping)
    }
  ),
  private = list(
    # check if variable is a valid distribution
    validateDistribution = function(distribution) {
      checkmate::assertCharacter(distribution, null.ok = FALSE)
      if (!(distribution %in% c("none", "normal", "lognormal"))) {
        nsenv <- asNamespace("stats")
        tmp <- tryCatch(
          {
            get(paste0("d", distribution), envir = nsenv)
          }, # nolint
          error = function(cond) {
            stop(messages$errorInvalidDistribution(distribution))
          } # nolint
        )
      } else if (distribution == "normal") {
        distribution <- "norm"
      } else if (distribution == "lognormal") {
        distribution <- "lnorm"
      }

      return(distribution)
    },
    # extract bin width and bin border of ggplot object
    getBinWidthforPlot = function(plotObject) {
      plotBuild <- ggplot_build(plotObject)

      binwidth <- c()
      for (iLayer in seq_along(plotBuild$data)) {
        if (
          all(
            c("xmin", "xmax", "count", "y") %in% names(plotBuild$data[[iLayer]])
          )
        ) {
          binwidth <- unique(diff(plotBuild$data[[iLayer]]$x))
          binwidth <- binwidth[binwidth > 0]
        }
      }

      if (length(binwidth) == 0) {
        stop(messages$errorBinWidthNoBinwidthFound())
      }
      if (
        abs(
          diff(range(binwidth, na.rm = TRUE)) / mean(binwidth, na.rm = TRUE)
        ) >
          1e-5
      ) {
        stop(messages$errorBinWidthBinsNotUnique())
      }

      binwidth <- mean(binwidth, na.rm = TRUE)

      return(binwidth)
    },
    # check if input is valid and returns function for vertical line
    selectMeanFunction = function(meanFunction, xScale) {
      checkmate::assertChoice(
        meanFunction,
        choices = c("none", "mean", "geomean", "median", "auto"),
        null.ok = FALSE
      )
      if (meanFunction == "auto") {
        meanFunction <- switch(
          self$distribution,
          "norm" = "mean",
          "lnorm" = "geomean",
          "none" = "none",
          "median"
        )
      }
      if (meanFunction == "none") {
        meanFun <- NULL
      } else {
        meanFun <- switch(
          meanFunction,
          "mean" = function(x) {
            mean(x, na.rm = TRUE)
          },
          "geomean" = function(x) {
            exp(mean(log(x), na.rm = TRUE))
          },
          "median" = function(x) {
            median(x, na.rm = TRUE)
          }
        )
      }

      return(meanFun)
    }
  )
)
