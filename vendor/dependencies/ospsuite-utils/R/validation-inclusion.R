#' Check if a vector of values is included in another vector of values
#'
#' @param values A vector of values.
#' @param parentValues A vector of values where `values` are checked for
#'   inclusion.
#' @inheritParams isOfType
#'
#' @return
#'
#' - `isIncluded()` returns `TRUE` if the value or **all** `values` (if it's a
#' vector) are present in the `parentValues`; `FALSE` otherwise.
#'
#' - `validateIsIncluded()` returns `NULL` if child value is included in parent
#' value set, otherwise error is signaled.
#'
#' @examples
#' # check if a column is present in dataframe
#' A <- data.frame(
#'   col1 = c(1, 2, 3),
#'   col2 = c(4, 5, 6),
#'   col3 = c(7, 8, 9)
#' )
#' isIncluded("col3", names(A)) # TRUE
#'
#' # check if single element is present in a vector (atomic or non-atomic)
#' isIncluded("x", list("w", "x", 1, 2)) # TRUE
#' isIncluded("x", c("w", "a", "y")) # FALSE
#'
#' # check if **all** values (if it's a vector) are contained in parent values
#' isIncluded(c("x", "y"), c("a", "y", "b", "x")) # TRUE
#' isIncluded(list("x", 1), list("a", "b", "x", 1)) # TRUE
#' isIncluded(c("x", "y"), c("a", "b", "x")) # FALSE
#' isIncluded(list("x", 1), list("a", "b", "x")) # FALSE
#'
#' # corresponding validation
#' validateIsIncluded("col3", names(A)) # NULL
#' # validateIsIncluded("col6", names(A)) # error
#' @export
isIncluded <- function(values, parentValues) {
  values <- c(values)

  hasObject <- any(mapply(function(x) !.isBaseType(x), values))

  if (hasObject) {
    stop("Only vectors of base object types are allowed.", call. = FALSE)
  }

  if (is.null(values) || length(values) == 0) {
    return(FALSE)
  }

  as.logical(min(values %in% parentValues))
}

#' @rdname isIncluded
#' @export
validateIsIncluded <- function(values, parentValues, nullAllowed = FALSE) {
  if (nullAllowed && is.null(values)) {
    return()
  }

  if (isIncluded(values, parentValues)) {
    return()
  }

  stop(messages$errorNotIncluded(values, parentValues))
}
