#' Assess if a character vector is UTF-8 encoded.
#'
#' @param text A character vector
#' @return
#' A logical assessing whether there is non UTF-8 encoded characters in `text`
#'
#' @examples
#'
#' isUTF8("Hello, world!") # TRUE
#' isUTF8("\xb5g/L") # FALSE
#'
#' @export
isUTF8 <- function(text) {
  stringi::stri_enc_isutf8(paste(text, collapse = "\n"))
}

#' Assess if a file is UTF-8 encoded.
#'
#' @param file A name of the file or full path.
#' @return
#' A logical assessing whether there is non UTF-8 encoded characters in `file`
#'
#' @examples
#'
#' writeLines(c("Hello, world!"), "utf.txt")
#' writeLines(c("Hello, world!", "\xb5g/L"), "non-utf.txt")
#'
#' isFileUTF8("utf.txt") # TRUE
#' isFileUTF8("non-utf.txt") # FALSE
#'
#' \dontshow{
#' unlink("utf.txt")
#' unlink("non-utf.txt")
#' }
#'
#' @export
isFileUTF8 <- function(file) {
  isUTF8(readLines(file))
}

#' Validate if a file is UTF-8 encoded.
#'
#' @param file A name of the file or full path.
#'
#' @return
#'
#' If validations are successful, `validateIsFileUTF8()` returns `NULL`.
#' Otherwise, error is signaled.
#'
#' @examples
#'
#' writeLines(c("Hello, world!"), "utf.txt")
#' writeLines(c("Hello, world!", "\xb5g/L"), "non-utf.txt")
#'
#' validateIsFileUTF8("utf.txt") # NULL
#' \dontrun{
#' validateIsFileUTF8("non-utf.txt") # Error
#' }
#'
#' \dontshow{
#' unlink("utf.txt")
#' unlink("non-utf.txt")
#' }
#'
#' @export
validateIsFileUTF8 <- function(file) {
  if (isFileUTF8(file)) {
    return(NULL)
  }
  stop(messages$errorFileNotUTF8(file))
}
