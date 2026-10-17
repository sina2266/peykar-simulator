#' Validate if objects are of same length
#'
#' @param ... Objects to compare.
#'
#' @return
#'
#' - `isSameLength()` returns `TRUE` if all objects have same lengths.
#'
#' - For `validateIsSameLength()`, if validations are successful, `NULL` is
#' returned. Otherwise, error is signaled.
#'
#' @examples
#' # compare length of only 2 objects
#' isSameLength(mtcars, ToothGrowth) # FALSE
#' isSameLength(cars, BOD) # TRUE
#'
#' # or more number of objects
#' isSameLength(c(1, 2), c(TRUE, FALSE), c("x", "y")) # TRUE
#' isSameLength(list(1, 2), list(TRUE, FALSE), list("x")) # FALSE
#'
#' # validation
#' validateIsSameLength(list(1, 2), c("3", "4")) # NULL
#' # validateIsSameLength(list(1, 2), c("3", "4"), c(FALSE)) # error
#'
#' @export
isSameLength <- function(...) {
  args <- list(...)
  nrOfLengths <- length(unique(lengths(args)))

  return(nrOfLengths == 1)
}


#' @rdname isSameLength
#' @export
validateIsSameLength <- function(...) {
  if (isSameLength(...)) {
    return()
  }

  # Name of the arguments
  argnames <- sys.call()
  arguments <- paste(lapply(argnames[-1], as.character), collapse = ", ")

  stop(messages$errorDifferentLength(arguments))
}
