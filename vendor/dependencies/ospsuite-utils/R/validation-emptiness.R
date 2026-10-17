#' Validate if the provided object is empty
#'
#' @param object An object or an atomic vector or a list of objects.
#'
#' @return
#'
#' - `isEmpty()` returns `TRUE` if the object is empty; `FALSE` otherwise.
#'
#' - `validateIsNotEmpty()` returns `NULL` if validation is successful.
#' Otherwise, error is signaled.
#'
#' @examples
#' # empty list or data.frame
#' isEmpty(NULL)
#' isEmpty(numeric())
#' isEmpty(list())
#' isEmpty(data.frame())
#'
#' # accounts for filtering of arrays and data.frame
#' df <- data.frame(x = c(1, 2, 3), y = c(4, 5, 6))
#' isEmpty(df)
#' isEmpty(df$x[FALSE])
#' isEmpty(df[FALSE, ])
#'
#' # validation helper
#' validateIsNotEmpty(list(1, 2)) # NULL
#' # validateIsNotEmpty(NULL) # error
#' @export
isEmpty <- function(object) {
  if (is.data.frame(object)) {
    return(nrow(object) == 0)
  }

  return(isOfLength(object, 0))
}


#' @rdname isEmpty
#' @export
validateIsNotEmpty <- function(object) {
  if (!isEmpty(object)) {
    return()
  }

  objectName <- deparse(substitute(object))

  stop(messages$errorEmpty(objectName))
}


#' Validate that no empty string is present
#'
#' @param x A character string or a vector of character strings.
#'
#' @details
#'
#' If any of the following conditions are met, the input string is considered
#' empty:
#'
#' - if any `NA`s are present (e.g. `x = c("a", "abc", NA)`)
#' - if string is empty (e.g. `x = list("a", "abc", "")`)
#' - if length is 0 (e.g. `x = character()`)
#'
#' @return
#'
#' - `hasEmptyStrings()` returns `TRUE` if any of the strings are empty; `FALSE`
#' otherwise.
#'
#' - `validateHasOnlyNonEmptyStrings()` produces an error if empty string are
#' present. It returns `NULL` otherwise.
#'
#' @examples
#'
#' hasEmptyStrings(c("x", "y")) # FALSE
#' hasEmptyStrings(list("x", "y")) # FALSE
#' hasEmptyStrings("   abc   ") # FALSE
#' hasEmptyStrings(c("", "y")) # TRUE
#' hasEmptyStrings(list("", "y")) # TRUE
#' hasEmptyStrings(NA) # TRUE
#' hasEmptyStrings(character(0)) # TRUE
#' hasEmptyStrings(c(NA, "x", "y")) # TRUE
#'
#' validateHasOnlyNonEmptyStrings(c("x", "y")) # NULL
#' validateHasOnlyNonEmptyStrings(list("x", "y")) # NULL
#' validateHasOnlyNonEmptyStrings("   abc   ") # NULL
#' # validateHasOnlyNonEmptyStrings(c("", "y")) # error
#' # validateHasOnlyNonEmptyStrings(list("", "y")) # error
#' # validateHasOnlyNonEmptyStrings(NA) # error
#' # validateHasOnlyNonEmptyStrings(character(0)) # error
#' # validateHasOnlyNonEmptyStrings(c(NA, "x", "y")) # error
#'
#' @export
hasEmptyStrings <- function(x) {
  length(x) == 0L || any(is.na(x)) || any(nchar(x) == 0L)
}

#' @rdname hasEmptyStrings
#' @export
validateHasOnlyNonEmptyStrings <- function(x) {
  argName <- deparse(substitute(x))

  if (hasEmptyStrings(x)) {
    stop(messages$errorEmptyString(argName))
  }
}
