#' Make sure the object is a list
#'
#' @param object Object to be converted to a list.
#'
#' @return
#' If `is.list(object) == TRUE`, returns the `object`; otherwise, `list(object)`.
#'
#' @examples
#' toList(list("a" = 1, "b" = 2))
#' toList(c("a" = 1, "b" = 2))
#'
#' @export
toList <- function(object) {
  if (is.list(object)) {
    return(object)
  }

  return(list(object))
}

#' Flatten a list to an atomic vector of desired type
#'
#' @param x A list or an atomic vector. If the latter, no change will be made.
#' @param type Type of atomic vector to be returned.
#'
#' @details
#'
#' The `type` argument will decide which variant from `purrr::flatten()` family
#' is used to flatten the list.
#'
#' @examples
#'
#' flattenList(list(1, 2, 3, NA), type = "numeric")
#' flattenList(list(TRUE, FALSE, NA), type = "integer")
#'
#' @return An atomic vector of desired type.
#'
#' @export
flattenList <- function(x, type) {
  if (!is.null(dim(x))) {
    stop(messages$errorOnlyVectorAllowed())
  }

  if (is.list(x)) {
    x <- switch(
      type,
      "character" = purrr::list_c(x, ptype = "c"),
      "numeric" = ,
      "real" = ,
      "double" = purrr::list_c(x, ptype = 1.0),
      "integer" = purrr::list_c(x, ptype = 1),
      "logical" = purrr::list_c(x, ptype = TRUE),
      purrr::list_flatten(x)
    )
  }

  return(x)
}


#' Convert special constants to `NA` of desired type
#'
#' @details
#'
#' Special constants (`NULL`, `Inf`, `-Inf`, `NaN`,  `NA`) will be converted to
#' `NA` of desired type.
#'
#' This function is **not** vectorized, and therefore only scalar values should
#' be entered.
#'
#' @param x A single element.
#' @inheritParams flattenList
#'
#' @examples
#'
#' toMissingOfType(NA, type = "real")
#' toMissingOfType(NULL, type = "integer")
#'
#' @export
toMissingOfType <- function(x, type) {
  # all unexpected values will be converted to `NA` of a desired type
  if (is.null(x) || is.na(x) || is.nan(x) || is.infinite(x)) {
    x <- switch(
      type,
      "character" = NA_character_,
      "numeric" = ,
      "real" = ,
      "double" = NA_real_,
      "integer" = NA_integer_,
      "complex" = NA_complex_,
      "logical" = NA,
      stop("Incorrect type entered.")
    )
  }

  return(x)
}

#' Computes logarithm of a number or of a vector of numbers and handles zeros while
#' substituting all values below `epsilon` by `epsilon`.
#'
#' @param x A numeric or a vector of numerics.
#' @param base a positive or complex number: the base with respect to which logarithms are computed. Defaults to e = exp(1).
#' @param epsilon A very small number which is considered as threshold below which
#' all values are treated as `epsilon`. Allows computation of `log` close to 0.
#' Default value is `getOSPSuiteUtilsSetting("LOG_SAFE_EPSILON")`.
#'
#' @return `log(x, base = base)` for `x > epsilon`, or `log(epsilon, base = base)`,
#' or `NA_real_` for `NA` elements.
#' @export
#'
#' @examples
#' inputVector <- c(NA, 1, 5, 0, -1)
#' logSafe(inputVector)
logSafe <- function(
  x,
  base = exp(1),
  epsilon = ospsuiteUtilsEnv$LOG_SAFE_EPSILON
) {
  # Vectorized conversion of special constants to NA
  # Handle Inf, -Inf, NaN in a vectorized way (NA values are preserved)
  x[is.infinite(x) | is.nan(x)] <- NA_real_
  
  # Apply epsilon threshold: values strictly below epsilon become epsilon
  # NA values are preserved by the logical indexing
  x[!is.na(x) & x < epsilon] <- epsilon
  
  # Vectorized log calculation
  x <- log(x, base = base)
  
  return(x)
}

#' Safe fold calculation
#'
#' @description
#' Calculates `x / y` while substituting values below `epsilon` (for x and y) by `epsilon`.
#' `x` and `y` must be of the same length
#'
#'
#' @param x A numeric or a vector of numerics.
#' @param y A numeric or a vector of numerics.
#' @param epsilon A very small number which is considered as threshold below which
#' all values are treated as `epsilon`. Allows computation of fold changes for values close to 0.
#' Default value is `getOSPSuiteUtilsSetting("LOG_SAFE_EPSILON")`.
#'
#' @return A vector with `x / y`.
#' @export
#'
#' @examples
#' inputX <- c(NA, 1, 5, 0, -1)
#' inputY <- c(1, -1, NA, 0, -1)
#' folds <- foldSafe(inputX, inputY)
foldSafe <- function(x, y, epsilon = ospsuiteUtilsEnv$LOG_SAFE_EPSILON) {
  validateIsSameLength(x, y)
  # Vectorized threshold application with explicit NA handling
  # Use logical indexing which is faster than ifelse
  x_idx <- !is.na(x) & x <= epsilon
  y_idx <- !is.na(y) & y <= epsilon
  x[x_idx] <- epsilon
  y[y_idx] <- epsilon

  return(x / y)
}
