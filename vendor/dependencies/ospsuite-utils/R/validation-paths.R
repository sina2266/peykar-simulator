#' Check if path is absolute
#'
#' @description
#'
#' Relative paths will be detected based on the presence of wildcard
#' character(*) in the path specification.
#'
#' @param path A valid file path name.
#'
#' @return
#'
#' - `isPathAbsolute()` returns `TRUE` if path is absolute (no wildcard); `FALSE` otherwise.
#'
#' - `validateIsPathAbsolute()` returns `NULL` if path is absolute. Otherwise, error is signaled.
#'
#' @examples
#' # check if path is absolute
#' isPathAbsolute("Organism|path") # TRUE
#' isPathAbsolute("Organism|*path") # FALSE
#'
#' # validation: no error if path is absolute
#' validateIsPathAbsolute("Organism|path")
#'
#' # validation: error otherwise
#' # validateIsPathAbsolute("Organism|*path")
#' @export
isPathAbsolute <- function(path) {
  wildcardChar <- "*"
  pathChars <- unlist(strsplit(path, ""), use.names = FALSE)

  !any(pathChars == wildcardChar)
}

#' @rdname isPathAbsolute
#' @export
validateIsPathAbsolute <- function(path) {
  if (isPathAbsolute(path)) {
    return()
  }

  stop(messages$errorEntityPathNotAbsolute(path))
}

#' @rdname isPathAbsolute
#' @export
validatePathIsAbsolute <- validateIsPathAbsolute
