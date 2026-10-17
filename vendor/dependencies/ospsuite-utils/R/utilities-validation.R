#' @keywords internal
.inheritType <- function(x, type, nullAllowed = FALSE) {
  if (is.null(x) && nullAllowed) {
    return(TRUE)
  }

  inherits(x, type)
}

#' @keywords internal
.isBaseType <- function(x) {
  baseTypes <- c("character", "logical", "integer", "double")

  if (any(baseTypes == typeof(x))) {
    return(TRUE)
  }

  return(FALSE)
}

#' @keywords internal
.typeNamesFrom <- function(type) {
  type <- c(type)
  sapply(type, function(t) ifelse(is.character(t), t, t$classname))
}

#' @keywords internal
.getCallingFunctionName <- function() {
  for (call in sys.calls()) {
    fn <- call[[1]]
    if (is.name(fn)) {
      return(as.character(fn))
    }
    if (
      is.call(fn) &&
        length(fn) == 3L &&
        (identical(fn[[1L]], as.name("::")) ||
          identical(fn[[1L]], as.name(":::")))
    ) {
      return(as.character(fn[[3L]]))
    }
  }
  deparse(sys.calls()[[1L]][[1L]])[1L]
}

#' @keywords internal
.fileExtension <- function(file) {
  # if file has no extension, return empty string
  if (!grepl("\\.", basename(file)) || grepl("\\.$", basename(file))) {
    return("")
  }

  ex <- strsplit(basename(file), split = "\\.")[[1]]

  return(utils::tail(ex, 1))
}
