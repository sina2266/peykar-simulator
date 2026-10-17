#' @title Value conditional on equality
#'
#' @description
#'
#' Short-key checking if arguments 1 and 2 are equal,
#' output argument 3 if equal, or output argument 4 otherwise.
#'
#' @param x argument 1
#' @param y argument 2
#' @param outputIfEqual argument 3
#' @param outputIfNotEqual argument 4
#'
#' @examples
#'
#' ifEqual(1, 1, "x", "y") # "x"
#' ifEqual(1, 2, "x", "y") # "y"
#'
#' @export
ifEqual <- function(x, y, outputIfEqual, outputIfNotEqual = NULL) {
  if (x == y) {
    return(outputIfEqual)
  }

  return(outputIfNotEqual)
}

#' @title Value conditional on inclusion
#'
#' @description
#'
#' Shortkey checking if arguments 1 is included in 2,
#' output argument 3 if included, or output argument 4 otherwise.
#'
#' @inheritParams ifEqual
#' @param outputIfIncluded argument 3
#' @param outputIfNotIncluded argument 4
#'
#' @examples
#'
#' ifIncluded("a", c("a", "b"), 1, 2) # 1
#' ifIncluded("x", c("a", "b"), 1, 2) # 2

#'
#' @export
ifIncluded <- function(x, y, outputIfIncluded, outputIfNotIncluded = NULL) {
  if (isIncluded(x, y)) {
    return(outputIfIncluded)
  }

  return(outputIfNotIncluded)
}

#' @title Value conditional on `NULL`
#'
#' @description
#'
#' Short-key checking if argument 1 is not `NULL`, output the argument 2 if not
#' null, or output argument 3 otherwise.
#'
#' @param condition argument 1
#' @param outputIfNotNull argument 2
#' @param outputIfNull argument 3
#'
#' @return
#' `outputIfNotNull` if condition is not `NULL`, `outputIfNull` otherwise.
#'
#' @description
#' Check if condition is not `NULL`, if so output `outputIfNotNull`,
#' otherwise, output `outputIfNull`.
#'
#' @examples
#'
#' ifNotNull(NULL, "x")
#' ifNotNull(NULL, "x", "y")
#' ifNotNull(1 < 2, "x", "y")
#'
#' @export
ifNotNull <- function(condition, outputIfNotNull, outputIfNull = NULL) {
  if (!is.null(condition)) {
    return(outputIfNotNull)
  }

  return(outputIfNull)
}
