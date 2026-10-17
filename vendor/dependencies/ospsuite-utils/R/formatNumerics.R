#' @title formatNumerics
#' @description Render numeric values of an `object` as character using the specified format:
#' - If `object` is a data.frame or a list, `formatNumerics` applies on each of its fields
#' - If `object` is of type character or integer, `formatNumerics` renders the values as is
#' - If `object` is of type numeric, `formatNumerics` applies the defined format
#'
#' @param object An R object such as a list, a data.frame, character or numeric values.
#' @param digits Number of decimal digits to render
#' @param scientific Logical value defining if scientific writing is rendered
#'
#' @return Numeric values are rendered as character values. If `object` is a
#'   data.frame or a list, a data.frame or list is returned with numeric values
#'   rendered as character values.
#'
#' @examples
#' # Format array of numeric values
#' formatNumerics(log(c(12, 15, 0.3)), digits = 1, scientific = TRUE)
#'
#' # Format a data.frame
#' x <- data.frame(parameter = c("a", "b", "c"), value = c(1, 110.4, 6.666))
#' formatNumerics(x, digits = 2, scientific = FALSE)
#' @export
formatNumerics <- function(
  object,
  digits = ospsuiteUtilsEnv$formatNumericsDigits,
  scientific = FALSE
) {
  validateIsInteger(digits)
  validateIsLogical(scientific)

  # Return integer as is before they are assumed as numeric
  if (is.integer(object)) {
    return(as.character(object))
  }

  # Method for numeric values
  if (is.numeric(object)) {
    # Scientific writing: using isTRUE because logical NA can provoke crash in `if` loop
    if (isTRUE(scientific)) {
      return(sprintf(paste0("%.", digits, "e"), object))
    }
    # Decimal writing
    return(sprintf(paste0("%.", digits, "f"), object))
  }

  # Method for data.frame or list: update each of its fields
  if (isOfType(object, c("list", "data.frame"))) {
    for (field in seq_along(object)) {
      object[[field]] <- formatNumerics(object[[field]], digits, scientific)
    }
  }

  # For other types such as character return values as is
  return(object)
}
