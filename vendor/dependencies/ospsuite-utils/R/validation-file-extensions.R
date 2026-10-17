#' Validate if the provided path has required extension
#'
#' @param file A name of the file or full path.
#' @param extension A required extension of the file.
#'
#' @return
#'
#' `isFileExtension()` returns `TRUE` if the file name (or full path) includes
#' the extension.
#'
#' If validations are successful, `validateIsFileExtension()` returns `NULL`.
#' Otherwise, error is signaled.
#'
#' @examples
#'
#' isFileExtension("enum.R", "R") # TRUE
#' isFileExtension("enum.R", "pkml") # FALSE
#'
#' validateIsFileExtension("enum.R", "R") # NULL
#' # validateIsFileExtension("enum.R", "pkml") # error
#'
#' @export
isFileExtension <- function(file, extension) {
  extension <- c(extension)
  file_ext <- .fileExtension(file)
  file_ext %in% extension
}

#' @rdname isFileExtension
#' @export
validateIsFileExtension <- function(file, extension) {
  if (isFileExtension(file, extension)) {
    return(NULL)
  }

  stop(messages$errorWrongFileExtension(.fileExtension(file), extension))
}
