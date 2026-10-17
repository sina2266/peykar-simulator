#' @title DDIRatioDataMapping
#' @description  R6 class for mapping `x`, `y`, `GroupMapping` and DDI ratio `lines` variables to `data`
#' @export
#' @family DataMapping classes
DDIRatioDataMapping <- R6::R6Class(
  "DDIRatioDataMapping",
  inherit = PKRatioDataMapping,
  public = list(
    #' @description Create a new `DDIRatioDataMapping` object
    #' @param deltaGuest
    #' Value of `delta` in [Guest et al.](https://dmd.aspetjournals.org/content/39/2/170) equation.
    #' Default value is 1.
    #' @param minRange Minimum range of x values for guest and ratio lines
    #' Default is \[0.01 - 100\]
    #' @param lines List of ratio limits to display as diagonal/horizontal lines
    #' @param residualsVsObserved Logical defining if calculated DDI data are as residuals vs observed or predicted vs observed
    #' @param ... parameters inherited from \code{PKRatioDataMapping}
    #' @return A new \code{DDIRatioDataMapping} object
    initialize = function(deltaGuest = NULL,
                          minRange = c(1e-2, 1e2),
                          lines = DefaultDataMappingValues$ddiRatio,
                          residualsVsObserved = FALSE,
                          ...) {
      super$initialize(...)
      validateIsNumeric(deltaGuest, nullAllowed = TRUE)
      .validateIsStrictlyPositive(minRange)
      validateIsLogical(residualsVsObserved)

      self$lines <- lines
      private$.minRange <- range(minRange)
      private$.deltaGuest <- deltaGuest %||% 1
      .checkIsBetween(private$.deltaGuest, 1, 2)
      private$.residualsVsObserved <- residualsVsObserved
    }
  ),
  active = list(
    #' @field deltaGuest
    #' Value of `delta` in [Guest et al.](https://dmd.aspetjournals.org/content/39/2/170) equation
    deltaGuest = function(value) {
      if (missing(value)) {
        return(private$.deltaGuest)
      }
      validateIsNumeric(value, nullAllowed = TRUE)
      private$.deltaGuest <- value %||% private$.deltaGuest
      .checkIsBetween(private$.deltaGuest, 1, 2)
      return(invisible())
    },
    #' @field minRange
    #' Minimum range of x values for guest and ratio lines
    minRange = function(value) {
      if (missing(value)) {
        return(private$.minRange)
      }
      validateIsNumeric(value, nullAllowed = TRUE)
      if (is.null(value)) {
        return(invisible())
      }
      .validateIsStrictlyPositive(value)
      # Range enforce a final array of min-max value
      private$.minRange <- range(value)
      return(invisible())
    },
    #' @field residualsVsObserved
    #' Logical defining if calculated DDI data are as residuals vs observed or predicted vs observed
    residualsVsObserved = function(value) {
      if (missing(value)) {
        return(private$.residualsVsObserved)
      }
      validateIsLogical(value, nullAllowed = TRUE)
      private$.residualsVsObserved <- value %||% private$.residualsVsObserved
      return(invisible())
    }
  ),
  private = list(
    .minRange = NULL,
    .deltaGuest = NULL,
    .residualsVsObserved = NULL
  )
)
