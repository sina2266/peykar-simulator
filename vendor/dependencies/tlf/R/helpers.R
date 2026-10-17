#' @title isBetween
#' @description Assess if `x` is between `left` and `right` bounds.
#' Shortcut for `x >= left & x <= right` if `strict=FALSE` (default).
#' Shortcut for `x > left & x < right` if `strict=TRUE`.
#' @param x Numeric values to assess
#' @param left Numeric value(s) used as lower bound
#' @param right Numeric value(s) used as upper bound
#' @param strict Logical value defining if `x` is strictly between `left` and `right`.
#' Default value is `FALSE`.
#' @return Logical values
#' @export
#' @examples
#' isBetween(1:12, 7, 9)
#'
#' x <- rnorm(1e2)
#' x[isBetween(x, -1, 1)]
#'
#' isBetween(x, cos(x) + 1, cos(x) - 1)
isBetween <- function(x, left, right, strict = FALSE) {
  if (strict) {
    return(x > left & x < right)
  }
  return(x >= left & x <= right)
}

#' @title getSymmetricLimits
#' @description Get symmetric limits from a set of values
#' @param values numeric values
#' @return An array of 2 symmetric values equally distant from 0
#' @export
#' @examples
#' getSymmetricLimits(seq(-3, 8))
#'
getSymmetricLimits <- function(values) {
  validateIsNumeric(values, nullAllowed = TRUE)
  # Remove Inf and NA values
  values <- values[!is.infinite(values)]
  values <- values[!is.na(values)]
  if (isEmpty(values)) {
    return(NULL)
  }
  return(c(-max(abs(values)), max(abs(values))))
}


#' @title getSameLimits
#' @description Get same limits from multiple sets of values
#' @param ... numeric values
#' @return An array of 2 values
#' @export
#' @examples
#' getSameLimits(seq(-3, 8), seq(-12, 4))
#'
getSameLimits <- function(...) {
  values <- c(...)
  validateIsNumeric(values, nullAllowed = TRUE)
  # Remove Inf and NA values
  values <- values[!is.infinite(values)]
  values <- values[!is.na(values)]
  if (isEmpty(values)) {
    return(NULL)
  }
  return(c(min(values), max(values)))
}
