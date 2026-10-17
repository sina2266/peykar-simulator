#' Count number of objects
#'
#' @details
#'
#' Classes in R can roughly be distinguished as following:
#'
#' - Vector-style classes have the property that `length()` represents number of
#' elements (e.g., `factor`, `list`, etc.).
#'
#' - Record-style (or dataframe or scalar) classes, on the other hand, have
#' complex structures to represent a single thing (e.g., `data.frame`, `R6`,
#' `lm`, etc.).
#'
#' This function counts objects differently depending on the entered class:
#'
#' If the argument is a vector or a vector-style class, it will return the
#' output from `length()` function. Otherwise, it will returns `1`.
#'
#' For example,
#'
#' - `length(mtcars)` returns `11`, but `objectCount(mtcars)` will
#' return `1`, while
#' - `length(list(1, 2))` returns `2`, and `objectCount(list(1, 2))` will return
#' `2` as well.
#'
#' @param x An object (an atomic vector, a list, or instance(s) of a class).
#'
#' @examples
#'
#' # vectors or vector-style classes
#' objectCount(c(1, 2, 3)) # 3
#' objectCount(list("a", "b")) # 2
#' objectCount(list(iris, mtcars)) # 2
#'
#' # everything else
#' objectCount(mtcars) # 1
#' objectCount(lm(wt ~ mpg, mtcars)) # 1
#' objectCount(new.env()) # 1
#'
#' @return Integer representing the count of objects.
#'
#' @export
objectCount <- function(x) {
  # *WARNING*: Do not use `is.vector()` to check for a vector.
  # It returns `FALSE` if `x` is a vector but has attributes other than names.
  #
  # `is.object()` catches all base type objects, i.e. objects that don't have
  # `class` attribute.
  if (is.null(dim(x)) && !is.object(x) && !is.environment(x)) {
    return(length(x))
  }

  # For anything other than vector or vector-style class, count should be 1
  return(1L)
}
