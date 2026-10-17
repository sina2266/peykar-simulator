#' Check if `value` is in the given `enum`. If not, stops with an error.
#'
#' @param enum `enum` where the `value` should be contained.
#' @param value A value to search for in the `enum`.
#' @param nullAllowed If `TRUE`, `value` can be `NULL` and the test always
#'   passes. If `FALSE` (default), `NULL` is not accepted and the test fails.
#'
#' @examples
#' Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))
#' validateEnumValue(1, Symbol)
#' @export
validateEnumValue <- function(value, enum, nullAllowed = FALSE) {
  if (is.null(value)) {
    if (nullAllowed) {
      return()
    }

    stop(messages$errorEnumValueUndefined(value))
  }

  enumKey <- enumGetKey(enum, value)

  if (any(names(enum) == enumKey)) {
    return()
  }

  stop(messages$errorValueNotInEnum(enum, value, deparse(substitute(enum))))
}
