#' Validate Vector Against Specified Criteria
#'
#' Validates a vector `x` based on specified criteria, including type correctness, value range,
#' allowed values, and handling of `NULL` and `NA` values. If the vector fails any validation,
#' an informative error message is thrown.
#'
#' @param x Vector to validate.
#' @param type Expected type of elements in `x` ("numeric", "integer", "character",
#'   "factor", "logical", or "Date"). Type "double" is treated as "numeric".
#' @param valueRange Optional vector of length 2 specifying the range of allowed values
#' for `x`, applicable to "numeric", "integer", "character", and "Date" types.
#' @param allowedValues Optional vector specifying a set of allowed values for `x`.
#' @param nullAllowed Logical flag indicating whether `x` can be `NULL`. Defaults to `FALSE`.
#' @param naAllowed Logical flag indicating whether elements in `x` can be `NA`. Defaults to `FALSE`.
#'
#' @details
#' `validateVector` is the primary function for checking a vector against defined validation criteria.
#' It ensures that `x` meets the type, range, and allowed value conditions specified. For more detailed
#' validations related to the value range and allowed values, `validateVectorRange` and
#' `validateVectorValues` functions are utilized respectively.
#'
#' @return
#' Does not return a value explicitly but will stop with a descriptive error message if any of the
#' validations fail.
#'
#' @examples
#' validateVector(x = 1:5, type = "integer")
#' validateVector(x = c(1.2, 2.5), type = "numeric", valueRange = c(1, 3))
#' validateVector(x = c("a", "b"), type = "character", allowedValues = c("a", "b", "c"))
#' validateVector(
#'   x = as.Date("2020-01-01"), type = "Date",
#'   valueRange = as.Date(c("2020-01-01", "2020-12-31"))
#' )
#'
#' # Range validation examples
#' validateVectorRange(x = c(5, 10), type = "numeric", valueRange = c(1, 10))
#' validateVectorRange(x = c("a", "b"), type = "character", valueRange = c("a", "c"))
#' validateVectorRange(
#'   x = as.Date(c("2020-01-01")), type = "Date",
#'   valueRange = as.Date(c("2020-01-01", "2020-12-31"))
#' )
#' validateVectorRange(x = 1:3, type = "integer", valueRange = c(1L, 5L))
#'
#' # Allowed values validation examples
#' validateVectorValues(x = c("a", "b"), type = "character", allowedValues = c("a", "b", "c"))
#' validateVectorValues(x = c(2L, 4L), type = "integer", allowedValues = c(1L, 2L, 3L, 4L))
#' validateVectorValues(x = c(TRUE), type = "logical", allowedValues = c(TRUE, FALSE))
#'
#' @export
validateVector <- function(
  x,
  type = NULL,
  valueRange = NULL,
  allowedValues = NULL,
  nullAllowed = FALSE,
  naAllowed = FALSE
) {
  if (!is.logical(nullAllowed)) {
    stop(
      messages$errorWrongType("nullAllowed", typeof(nullAllowed), "logical"),
      call. = FALSE
    )
  }
  if (!is.logical(naAllowed)) {
    stop(
      messages$errorWrongType("naAllowed", typeof(naAllowed), "logical"),
      call. = FALSE
    )
  }

  if (is.null(type)) {
    stop(messages$errorMissingType(), call. = FALSE)
  }
  if (is.null(x)) {
    if (!nullAllowed) {
      stop(messages$errorWrongType("x", "NULL", "vector"))
    }
    return()
  }

  if (!naAllowed && any(is.na(x))) {
    stop(messages$errorNaNotAllowed(), call. = FALSE)
  }

  validTypes <- c(
    "numeric",
    "integer",
    "character",
    "factor",
    "logical",
    "Date"
  )
  type <- if (type == "double") "numeric" else type
  if (!type %in% validTypes) {
    stop(
      messages$errorTypeNotSupported("type", type, validTypes),
      call. = FALSE
    )
  }

  if (!isOfType(x, type, nullAllowed = FALSE)) {
    stop(messages$errorWrongType("x", class(x)[1], type))
  }

  validateVectorRange(x, type, valueRange)
  validateVectorValues(x, type, allowedValues, naAllowed)

  return()
}

#' @rdname validateVector
#' @export
validateVectorRange <- function(x, type, valueRange) {
  if (is.null(valueRange)) {
    return()
  }
  validRangeTypes <- c("numeric", "integer", "character", "Date")
  if (type %in% validRangeTypes) {
    if (!isOfType(valueRange, type)) {
      stop(
        messages$errorWrongType(
          "valueRange",
          class(valueRange)[1],
          type,
          "\n'valueRange' should match the specified 'type' parameter."
        ),
        call. = FALSE
      )
    }
    if (
      length(valueRange) != 2 ||
        valueRange[1] > valueRange[2] ||
        any(is.na(valueRange))
    ) {
      stop(messages$errorValueRange(valueRange), call. = FALSE)
    }
    if (any(x < valueRange[1] | x > valueRange[2], na.rm = TRUE)) {
      stop(messages$errorOutOfRange(valueRange), call. = FALSE)
    }
  } else {
    stop(messages$errorValueRangeType(valueRange, type), call. = FALSE)
  }

  return()
}

#' @rdname validateVector
#' @export
validateVectorValues <- function(
  x,
  type,
  allowedValues = NULL,
  naAllowed = FALSE
) {
  if (is.null(allowedValues)) {
    return()
  }
  if (!naAllowed) {
    allowedValues <- allowedValues[!is.na(allowedValues)]
  } else {
    allowedValues <- unique(c(allowedValues, NA))
  }

  if (!isOfType(allowedValues, type)) {
    stop(
      messages$errorWrongType("allowedValues", class(allowedValues)[1], type),
      call. = FALSE
    )
  }

  if (!all(x %in% allowedValues)) {
    stop(messages$errorValueNotAllowed(x, allowedValues))
  }

  return()
}
