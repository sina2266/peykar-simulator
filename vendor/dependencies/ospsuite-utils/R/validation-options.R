#' Validate Common Spec Parameters
#'
#' @keywords internal
#' @noRd
.validateSpecParams <- function(nullAllowed, naAllowed, expectedLength) {
  validateIsLogical(nullAllowed)
  validateIsLogical(naAllowed)
  if (!is.null(expectedLength)) {
    validateIsInteger(expectedLength)
    if (expectedLength < 1) {
      stop(messages$errorExpectedLengthPositive(), call. = FALSE)
    }
  }
}

#' Validate Min/Max Range Parameters
#'
#' @keywords internal
#' @noRd
.validateMinMax <- function(min, max, type) {
  # Validate min parameter
  if (!is.null(min)) {
    if (anyNA(min)) {
      stop(messages$errorWrongType("min", "NA", type), call. = FALSE)
    }
    if (type == "integer") {
      validateIsInteger(min)
    } else if (type == "numeric") {
      validateIsNumeric(min)
    }
  }

  # Validate max parameter
  if (!is.null(max)) {
    if (anyNA(max)) {
      stop(messages$errorWrongType("max", "NA", type), call. = FALSE)
    }
    if (type == "integer") {
      validateIsInteger(max)
    } else if (type == "numeric") {
      validateIsNumeric(max)
    }
  }

  if (!is.null(min) && !is.null(max) && min > max) {
    stop(messages$errorMinMaxInvalid(min, max), call. = FALSE)
  }
}

#' Create Integer Option Specification
#'
#' @param min Minimum allowed value. Defaults to `-Inf`.
#' @param max Maximum allowed value. Defaults to `Inf`.
#' @param nullAllowed Logical flag indicating whether `NULL` is permitted.
#'   Defaults to `FALSE`.
#' @param naAllowed Logical flag indicating whether `NA` values are permitted.
#'   Defaults to `FALSE`.
#' @param expectedLength Expected length of the option value. Use `NULL` for any
#'   length, `1` for scalar (default), or a positive integer for specific
#'   length.
#'
#' @return An S3 object of class `optionSpec_integer` and `optionSpec`.
#'
#' @export
integerOption <- function(
  min = -Inf,
  max = Inf,
  nullAllowed = FALSE,
  naAllowed = FALSE,
  expectedLength = 1
) {
  .validateSpecParams(nullAllowed, naAllowed, expectedLength)

  # Handle explicit NULL or NA by replacing with -Inf/Inf
  if (is.null(min) || (length(min) == 1 && is.na(min))) {
    min <- -Inf
  }
  if (is.null(max) || (length(max) == 1 && is.na(max))) {
    max <- Inf
  }

  .validateMinMax(min, max, "integer")

  # For integer type, only set valueRange if both are finite (to avoid type coercion)
  valueRange <- if (is.finite(min) && is.finite(max)) {
    c(as.integer(min), as.integer(max))
  } else if (is.finite(min)) {
    c(as.integer(min), .Machine$integer.max)
  } else if (is.finite(max)) {
    c(-.Machine$integer.max, as.integer(max))
  } else {
    NULL
  }

  spec <- list(
    type = "integer",
    valueRange = valueRange,
    nullAllowed = nullAllowed,
    naAllowed = naAllowed,
    expectedLength = expectedLength
  )

  class(spec) <- c("optionSpec_integer", "optionSpec")
  spec
}

#' Create Numeric Option Specification
#'
#' @inheritParams integerOption
#'
#' @return An S3 object of class `optionSpec_numeric` and `optionSpec`.
#'
#' @export
numericOption <- function(
  min = -Inf,
  max = Inf,
  nullAllowed = FALSE,
  naAllowed = FALSE,
  expectedLength = 1
) {
  .validateSpecParams(nullAllowed, naAllowed, expectedLength)

  # Handle explicit NULL or NA by replacing with -Inf/Inf
  if (is.null(min) || (length(min) == 1 && is.na(min))) {
    min <- -Inf
  }
  if (is.null(max) || (length(max) == 1 && is.na(max))) {
    max <- Inf
  }

  .validateMinMax(min, max, "numeric")

  spec <- list(
    type = "numeric",
    valueRange = c(min, max),
    nullAllowed = nullAllowed,
    naAllowed = naAllowed,
    expectedLength = expectedLength
  )

  class(spec) <- c("optionSpec_numeric", "optionSpec")
  spec
}

#' Create Character Option Specification
#'
#' @param allowedValues Vector of permitted values. Defaults to `NULL` (any
#'   value allowed).
#' @inheritParams integerOption
#'
#' @return An S3 object of class `optionSpec_character` and `optionSpec`.
#'
#' @export
characterOption <- function(
  allowedValues = NULL,
  nullAllowed = FALSE,
  naAllowed = FALSE,
  expectedLength = 1
) {
  .validateSpecParams(nullAllowed, naAllowed, expectedLength)

  if (!is.null(allowedValues)) {
    validateIsCharacter(allowedValues)
    if (length(allowedValues) == 0) {
      stop(messages$errorAllowedValuesEmpty())
    }
    if (anyNA(allowedValues)) {
      stop(messages$errorAllowedValuesEmpty(
        "allowedValues cannot contain NA. Use naAllowed = TRUE instead."
      ))
    }
  }

  spec <- list(
    type = "character",
    allowedValues = allowedValues,
    nullAllowed = nullAllowed,
    naAllowed = naAllowed,
    expectedLength = expectedLength
  )

  class(spec) <- c("optionSpec_character", "optionSpec")
  spec
}

#' Create Logical Option Specification
#'
#' @inheritParams integerOption
#'
#' @return An S3 object of class `optionSpec_logical` and `optionSpec`.
#'
#' @export
logicalOption <- function(
  nullAllowed = FALSE,
  naAllowed = FALSE,
  expectedLength = 1
) {
  .validateSpecParams(nullAllowed, naAllowed, expectedLength)

  spec <- list(
    type = "logical",
    nullAllowed = nullAllowed,
    naAllowed = naAllowed,
    expectedLength = expectedLength
  )

  class(spec) <- c("optionSpec_logical", "optionSpec")
  spec
}

#' Normalize Spec to optionSpec Object
#'
#' Converts a legacy list-based spec to an `optionSpec` using the typed
#' constructors. If `spec` is already an `optionSpec`, it is returned unchanged.
#' This preserves backward compatibility with the old specification format.
#'
#' @keywords internal
#' @noRd
.normalizeSpec <- function(spec, optionName = NULL) {
  if (inherits(spec, "optionSpec")) {
    return(spec)
  }

  # Convert old list format to optionSpec
  if (!is.list(spec)) {
    stop(messages$errorSpecNotList(optionName), call. = FALSE)
  }

  if (is.null(spec$type)) {
    stop(messages$errorSpecMissingType(optionName), call. = FALSE)
  }

  # Build constructor args from old spec format
  args <- list(
    nullAllowed = spec$nullAllowed %||% FALSE,
    naAllowed = spec$naAllowed %||% FALSE,
    expectedLength = spec$expectedLength %||% 1
  )

  # Type-specific parameters
  if (spec$type %in% c("integer", "numeric")) {
    if (!is.null(spec$valueRange)) {
      args$min <- spec$valueRange[1]
      args$max <- spec$valueRange[2]
    }
  }

  if (spec$type == "character") {
    args$allowedValues <- spec$allowedValues
  }

  # Call constructor
  constructorFn <- switch(
    spec$type,
    integer = integerOption,
    numeric = numericOption,
    character = characterOption,
    logical = logicalOption,
    stop(messages$errorInvalidSpecType(spec$type, optionName), call. = FALSE)
  )

  do.call(constructorFn, args)
}

#' Internal S3 generic for validating values against specs
#'
#' @keywords internal
#' @noRd
.validateValue <- function(value, spec, name) {
  UseMethod(".validateValue", spec)
}

#' Default method: validates common constraints and delegates to validateVector
#'
#' @keywords internal
#' @noRd
#' @exportS3Method
.validateValue.optionSpec <- function(value, spec, name) {
  if (!is.null(spec$expectedLength) && !is.null(value)) {
    if (length(value) != spec$expectedLength) {
      stop(
        messages$errorWrongLength(value, spec$expectedLength, name),
        call. = FALSE
      )
    }
  }

  validateVector(
    x = value,
    type = spec$type,
    valueRange = spec$valueRange,
    allowedValues = spec$allowedValues,
    nullAllowed = spec$nullAllowed,
    naAllowed = spec$naAllowed
  )

  return()
}

#' Integer-specific method: auto-converts numeric to integer when possible
#'
#' @keywords internal
#' @noRd
#' @exportS3Method
.validateValue.optionSpec_integer <- function(value, spec, name) {
  if (!is.null(value) && isOfType(value, "numeric") && !is.integer(value)) {
    if (isTRUE(all.equal(value, as.integer(value)))) {
      value <- as.integer(value)
    }
  }

  NextMethod()
}


#' Validate Options Against Specifications
#'
#' Validates a list of options against specified validation rules. Supports both
#' modern spec constructors (`integerOption()`, etc.) and legacy list format for
#' backward compatibility.
#'
#' @param options A list of options to validate.
#' @param validOptions A list specifying validation rules for each option. Each
#'   entry should either be:
#'   - A spec object created with `integerOption()`, `characterOption()`, etc.
#'   - A list with fields: `type`, `valueRange`, `allowedValues`, `nullAllowed`,
#'   `naAllowed`
#'
#' @details Each entry in `validOptions` is validated against the matching value
#'   from `options`. Spec objects created with constructors (e.g.,
#'   `integerOption()`) are recommended because they express intent clearly and
#'   work well with IDEs. For backward compatibility, legacy list-based specs
#'   are still accepted and are automatically normalized to `optionSpec` before
#'   validation.
#'
#' @return Returns `NULL` invisibly if all validations pass. Stops with detailed
#'   error message listing all failures if any validation fails.
#'
#' @examples
#' validOptions <- list(
#'   maxIterations = integerOption(min = 1L, max = 10000L),
#'   method = characterOption(allowedValues = c("a", "b")),
#'   threshold = numericOption(min = 0, max = 1, nullAllowed = TRUE)
#' )
#'
#' options <- list(maxIterations = 100L, method = "a", threshold = 0.05)
#' validateIsOption(options, validOptions)
#'
#' @export
validateIsOption <- function(options, validOptions) {
  validateIsOfType(options, "list")
  validateIsOfType(validOptions, "list")

  validOptions <- Map(.normalizeSpec, validOptions, names(validOptions))

  errors <- list()
  for (name in names(validOptions)) {
    result <- tryCatch(
      {
        .validateValue(options[[name]], validOptions[[name]], name)
        TRUE
      },
      error = function(e) sub("^[^`]*`\\: ", "", e$message)
    )

    if (!isTRUE(result)) {
      errors[[name]] <- result
    }
  }

  if (length(errors) > 0) {
    stop(
      paste(
        messages$errorOptionValidationFailed(),
        paste(names(errors), ":", unlist(errors), collapse = "\n"),
        sep = "\n"
      )
    )
  }

  return()
}
