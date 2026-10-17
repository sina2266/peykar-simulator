#' Validate that a vector has only unique values
#'
#' @param values An array of values
#' @param na.rm Logical to decide if missing values should be removed from the
#'   duplicate checking. Note that duplicate `NA` values are flagged if
#'   `na.rm=FALSE`.
#'
#' @return
#'
#' - `hasOnlyDistinctValues` returns `TRUE` if all values are unique.
#'
#' - `validateHasOnlyDistinctValues()` returns `NULL` if only unique values
#' present, otherwise produces error.
#'
#' @examples
#'
#' hasOnlyDistinctValues(c("x", "y"))
#' hasOnlyDistinctValues(c("x", "y", "x"))
#' hasOnlyDistinctValues(c("x", NA, "y", NA), na.rm = FALSE)
#' hasOnlyDistinctValues(c("x", NA, "y", NA), na.rm = TRUE)
#'
#' validateHasOnlyDistinctValues(c("x", "y")) # NULL
#' # validateHasOnlyDistinctValues(c("x", "y", "x")) # error
#'
#' @export
hasOnlyDistinctValues <- function(values, na.rm = TRUE) {
  if (na.rm) {
    values <- values[!is.na(values)]
  }

  return(!any(duplicated(values)))
}


#' @rdname hasOnlyDistinctValues
#' @export
validateHasOnlyDistinctValues <- function(values, na.rm = TRUE) {
  if (hasOnlyDistinctValues(values)) {
    return()
  }

  stop(messages$errorDuplicatedValues())
}
