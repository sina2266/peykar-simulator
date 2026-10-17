#' Check if the provided object is of certain type
#'
#' @param object An object or an atomic vector or a list of objects.
#' @param type A single string or a vector of string representation or class of
#'   the type that should be checked for.
#' @param nullAllowed Boolean flag if `NULL` is accepted for the `object`. If
#'   `TRUE`, `NULL` always returns `TRUE`, otherwise `NULL` returns `FALSE`.
#'   Default is `FALSE`.
#'
#' @return
#' `TRUE` if the object or all objects inside the list are of the given type.
#'
#' @note
#' Only the first level of the given list is considered.
#'
#' @examples
#' # checking type of a single object
#' df <- data.frame(x = c(1, 2, 3))
#' isOfType(df, "data.frame")
#' @export
isOfType <- function(object, type, nullAllowed = FALSE) {
  if (!is.logical(nullAllowed)) {
    stop(
      messages$errorWrongType("nullAllowed", typeof(nullAllowed), "logical"),
      call. = FALSE
    )
  }

  if (is.null(object)) {
    return(nullAllowed)
  }

  type <- .typeNamesFrom(type)

  if (.inheritType(object, type, nullAllowed)) {
    return(TRUE)
  }

  object <- c(object)

  all(sapply(object, .inheritType, type, nullAllowed))
}

#' Check if the provided object is of certain type. If not, stop with an error.
#'
#' @return
#'
#' `NULL` if the entered object is of expected type, otherwise produces error.
#' Also accepts `NULL` as an input if `nullAllowed` argument is set to `TRUE`.
#'
#' @inheritParams isOfType
#'
#' @examples
#' A <- data.frame(
#'   col1 = c(1, 2, 3),
#'   col2 = c(4, 5, 6),
#'   col3 = c(7, 8, 9)
#' )
#'
#' validateIsOfType(A, "data.frame")
#' validateIsInteger(5)
#' validateIsNumeric(1.2)
#' validateIsCharacter("x")
#' validateIsLogical(TRUE)
#' @export
validateIsOfType <- function(object, type, nullAllowed = FALSE) {
  # special case for integer to ensure that we call the special method
  if (length(type) == 1 && type[1] == "integer") {
    return(validateIsInteger(object, nullAllowed = nullAllowed))
  }

  if (isOfType(object, type, nullAllowed)) {
    return()
  }

  # Name of the variable in the calling function
  objectName <- deparse(substitute(object))
  objectTypes <- .typeNamesFrom(type)

  # There might be no call stack available if called from terminal
  callStack <- as.character(sys.call(-1)[[1]])

  # Object name is one frame further for functions such as ValidateIsNumeric
  if (
    (length(callStack) > 0) && any(grepl(pattern = "validateIs", x = callStack))
  ) {
    objectName <- deparse(substitute(object, sys.frame(-1)))
  }

  stop(messages$errorWrongType(objectName, class(object)[1], objectTypes))
}

#' @rdname validateIsOfType
#' @export
validateIsCharacter <- function(object, nullAllowed = FALSE) {
  validateIsOfType(object, "character", nullAllowed)
}

#' @rdname validateIsOfType
#' @export
validateIsString <- validateIsCharacter

#' @rdname validateIsOfType
#' @export
validateIsNumeric <- function(object, nullAllowed = FALSE) {
  # Only NA values. It is numeric
  if (all(is.na(object)) && !any(is.null(object))) {
    return()
  }

  validateIsOfType(object, c("numeric", "integer"), nullAllowed)
}

#' @rdname validateIsOfType
#' @export
validateIsInteger <- function(object, nullAllowed = FALSE) {
  if (nullAllowed && is.null(object)) {
    return()
  }

  if (is.list(object)) {
    object <- unlist(object, use.names = FALSE)
  }

  # if it's an actual integer (e.g. 5L)
  if (is.integer(object)) {
    return()
  }

  # making sure we check for numeric values before calling floor
  # e.g. `5` is numeric but can be considered integer for our purposes
  if (is.numeric(object) && all(floor(object) == object, na.rm = TRUE)) {
    return()
  }

  # Name of the variable in the calling function
  objectName <- deparse(substitute(object))
  objectTypes <- "integer"

  stop(messages$errorWrongType(objectName, class(object)[1], objectTypes))
}

#' @rdname validateIsOfType
#' @export
validateIsLogical <- function(object, nullAllowed = FALSE) {
  validateIsOfType(object, "logical", nullAllowed)
}
