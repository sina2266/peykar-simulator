#' Check if the provided object has expected length
#'
#' @param object An object or a list of objects
#' @param nbElements number of elements that are supposed in object
#'
#' @return
#'
#' - `isOfLength()` returns `TRUE` if the object or all objects inside the list
#' have `nbElements`.
#'
#' - For `validateIsOfLength()`, if validations are successful, `NULL` is
#' returned. Otherwise, error is signaled.
#'
#' @note
#' Only the first level of the given list is considered.
#'
#' @examples
#'
#' df <- data.frame(x = c(1, 2, 3))
#'
#' isOfLength(df, 1) # TRUE
#' isOfLength(df, 3) # FALSE
#'
#' validateIsOfLength(list(1, 2), 2L) # NULL
#' # validateIsOfLength(c("3", "4"), 3L) # error
#' @export
isOfLength <- function(object, nbElements) {
  return(length(object) == nbElements)
}


#' @rdname isOfLength
#' @export
validateIsOfLength <- function(object, nbElements) {
  if (isOfLength(object, nbElements)) {
    return()
  }

  stop(messages$errorWrongLength(object, nbElements))
}
