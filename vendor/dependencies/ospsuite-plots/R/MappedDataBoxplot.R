# Boxplots ----------------
#' @title MappedDataBoxplot
#' @description R6 class for mapping variable to data for boxplot visualizations.
#' This class extends MappedData to provide specialized mapping functionality for box-and-whisker plots,
#' including handling of discrete and continuous x-axis scales and automatic grouping logic.
#' @examples
#' \dontrun{
#' # Create boxplot mapping with continuous x variable
#' boxplotData <- MappedDataBoxplot$new(
#'   data = myDataFrame,
#'   mapping = aes(x = dose, y = concentration),
#'   xScale = "linear"
#' )
#'
#' # Create boxplot mapping with categorical x variable
#' boxplotData <- MappedDataBoxplot$new(
#'   data = myDataFrame,
#'   mapping = aes(x = treatment_group, y = response),
#'   xScale = "discrete"
#' )
#' }
#' @export
#' @family MappedData classes
MappedDataBoxplot <- R6::R6Class( # nolint
  "MappedDataBoxplot",
  inherit = MappedData,
  public = list(
    #' @field xScale scale of x axis
    xScale = NULL,
    #' @field xScaleArgs arguments for scale of x axis
    xScaleArgs = NULL,
    #' @field hasXmapping boolean, if TRUE x is mapped
    hasXmapping = NULL,
    #'
    #' @param data data.frame used for mapping
    #' @param mapping list of aesthetic mappings
    #' @param groupAesthetics vector of aesthetics, which are used for columns mapped with aesthetic `groupby`
    #' @param direction direction of plot either "x" or "y"
    #' @param isObserved A `boolean` if TRUE mappings mdv, lloq, error and error_relative are evaluated
    #' @param xScale scale of x-axis either 'linear' or 'log'
    #' @param yScale scale of y-axis either 'linear' or 'log'
    #' @param xlimits limits for x-axis (may be NULL)
    #' @param ylimits limits for y-axis (may be NULL)
    #'
    #' @description Create a new `MappedDataBoxplot` object
    #'
    #' @return `MappedDataBoxplot` class object
    initialize = function(data,
                          mapping,
                          groupAesthetics = NULL,
                          direction = "y",
                          isObserved = TRUE,
                          xlimits = NULL,
                          ylimits = NULL,
                          xScale = AxisScales$linear,
                          yScale = AxisScales$linear) {
      super$initialize(
        data = data,
        mapping = mapping,
        groupAesthetics = groupAesthetics,
        direction = direction,
        isObserved = isObserved,
        xlimits = xlimits,
        ylimits = ylimits,
        xScale = xScale,
        yScale = yScale
      )
      # check if one dimensional in x
      self$hasXmapping <- ("x" %in% names(self$mapping))
    },
    #' @description Use Metadata to adjust binning of x-axis, and group aesthetic.
    #' @param originalmapping mapping provided by user
    #' @param xScale either 'linear','log', 'discrete' or 'auto' (default) auto select linear for continuous data and discrete for categorical data
    #' @param xScaleArgs list of arguments passed to `ggplot2::scale_x_continuous()`, `ggplot2::scale_x_log10()` or
    #'    `ggplot2::scale_x_discrete()`
    #'
    #' @return adjusted `MappedDataBoxplot` class object
    doAdjustmentsWithMetaData = function(originalmapping,
                                         xScale,
                                         xScaleArgs) {
      if (is.null(self$columnClasses[["x"]])) {
        warning(messages$warningNoMetaDataForXAxis())
        return(invisible(self))
      }
      # Validate input mapping structure
      checkmate::assertList(originalmapping, null.ok = FALSE)
      checkmate::assertCharacter(xScale, len = 1, null.ok = FALSE)
      checkmate::assertList(xScaleArgs, null.ok = TRUE)

      # Adjust group aesthetic based on x variable type and mapping requirements
      private$adjustGroupMapping(originalmapping = originalmapping)
      # Determine and validate appropriate x-axis scale based on data type
      private$checkXScale(xScale = xScale, xScaleArgs)

      return(invisible(self))
    }
  ),
  ## active -------
  #' @field boxwhiskerMapping mapping for box whisker plot
  active = list(
    boxwhiskerMapping = function() {
      # Return different mapping based on whether x variable is mapped
      if (self$hasXmapping) {
        # When x is mapped, use existing aesthetics (x variable drives grouping)
        return(aes())
      } else {
        # When no x mapping, create a single category for all data points
        return(aes(x = " "))
      }
    }
  ),
  private = list(
    checkXScale = function(xScale, xScaleArgs) {
      if (self$hasXmapping) {
        if (self$columnClasses[["x"]] == "factor") {
          if (xScale %in% c(AxisScales$linear, AxisScales$log)) {
            stop(messages$errorContinuousXScaleNotPossibleForFactors(AxisScales$discrete))
          }
          xScale <- AxisScales$discrete
        } else {
          if (self$columnClasses[["x"]] == "numeric") {
            if (xScale == AxisScales$discrete) {
              stop(messages$errorDiscreteXScaleNotPossibleForContinuous(AxisScales$linear, AxisScales$log))
            }
            if (xScale == "auto") {
              xScale <- AxisScales$linear
            }
          } else {
            xScale <- AxisScales$discrete
          }
        }
      } else {
        xScale <- AxisScales$discrete
      }

      self$xScale <- xScale

      return(invisible(NULL))
    },
    adjustGroupMapping = function(originalmapping) {
      if (self$hasXmapping) {
        # non numeric should  not have a group mapping,
        # so delete it if not explicitly set by user
        if (self$columnClasses[["x"]] != "numeric" &
          private$aestheticExists("group") &
          !("group" %in% names(originalmapping))) {
          self$mapping[["group"]] <- NULL
        }
        # numeric should have a group mapping
        if (self$columnClasses[["x"]] == "numeric" &
          !private$aestheticExists("group")) {
          newMapping <- list()
          newMapping[["group"]] <- self$mapping$x
          private$addOverwriteAes(newMapping)
        }
      }

      return(invisible(self))
    }
  )
)
