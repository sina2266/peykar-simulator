#' @title CumulativeTimeProfileDataMapping
#' @description  R6 class for mapping `x`, `y`, `GroupMapping` variables to `data`
#' @export
#' @family DataMapping classes
CumulativeTimeProfileDataMapping <- R6::R6Class(
  "CumulativeTimeProfileDataMapping",
  inherit = XYGDataMapping
)
