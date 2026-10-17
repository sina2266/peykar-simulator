#' Validate external pointer
#'
#' @param extPtr external pointer to a .NET
#'
#' @keywords internal
#' @noRd
.validateIsExtPtr <- function(extPtr) {
  if (!inherits(extPtr, "externalptr")) {
    stop(messages$errorNoExternalPointer)
  }
}
