# Helper functions --------------------------------------------------------

test_that(".validateSpecParams() validates nullAllowed", {
  expect_error(
    .validateSpecParams("yes", FALSE, 1),
    regexp = messages$errorWrongType("nullAllowed", typeof("yes"), "logical"),
    fixed = TRUE
  )
  expect_silent(.validateSpecParams(TRUE, FALSE, 1))
})

test_that(".validateSpecParams() validates naAllowed", {
  expect_error(
    .validateSpecParams(FALSE, "no", 1),
    regexp = messages$errorWrongType("naAllowed", typeof("no"), "logical"),
    fixed = TRUE
  )
  expect_silent(.validateSpecParams(FALSE, TRUE, 1))
})

test_that(".validateSpecParams() validates expectedLength", {
  expect_error(
    .validateSpecParams(FALSE, FALSE, 0),
    regexp = messages$errorExpectedLengthPositive(),
    fixed = TRUE
  )
  expect_error(
    .validateSpecParams(FALSE, FALSE, 1.5),
    regexp = messages$errorWrongType(
      "expectedLength",
      class(1.5)[1],
      "integer"
    ),
    fixed = TRUE
  )
  expect_silent(.validateSpecParams(FALSE, FALSE, 1))
  expect_silent(.validateSpecParams(FALSE, FALSE, NULL))
})

test_that(".validateMinMax() handles NULL/NULL edge case", {
  expect_silent(.validateMinMax(NULL, NULL, "integer"))
  expect_silent(.validateMinMax(NULL, NULL, "numeric"))
})

test_that(".validateMinMax() rejects NA values", {
  expect_error(
    .validateMinMax(NA, NULL, "integer"),
    regexp = messages$errorWrongType("min", "NA", "integer"),
    fixed = TRUE
  )
  expect_error(
    .validateMinMax(NULL, NA, "integer"),
    regexp = messages$errorWrongType("max", "NA", "integer"),
    fixed = TRUE
  )
  expect_error(
    .validateMinMax(NA_integer_, NULL, "integer"),
    regexp = messages$errorWrongType("min", "NA", "integer"),
    fixed = TRUE
  )
  expect_error(
    .validateMinMax(NA_real_, NULL, "numeric"),
    regexp = messages$errorWrongType("min", "NA", "numeric"),
    fixed = TRUE
  )
  expect_error(
    .validateMinMax(NA, NA, "integer"),
    regexp = messages$errorWrongType("min", "NA", "integer"),
    fixed = TRUE
  )
})

test_that(".validateMinMax() validates min greater max", {
  expect_error(
    .validateMinMax(10L, 1L, "integer"),
    regexp = messages$errorMinMaxInvalid(10L, 1L),
    fixed = TRUE
  )
  expect_error(
    .validateMinMax(10.5, 1.5, "numeric"),
    regexp = messages$errorMinMaxInvalid(10.5, 1.5),
    fixed = TRUE
  )
  expect_silent(.validateMinMax(1L, 10L, "integer"))
  expect_silent(.validateMinMax(1L, 1L, "integer"))
})


# Spec constructors -------------------------------------------------------

test_that("integerOption() creates valid spec", {
  spec <- integerOption(min = 1L, max = 10L)
  expect_s3_class(spec, "optionSpec_integer")
  expect_s3_class(spec, "optionSpec")
  expect_equal(spec$type, "integer")
  expect_equal(spec$valueRange, c(1L, 10L))
  expect_equal(spec$nullAllowed, FALSE)
  expect_equal(spec$naAllowed, FALSE)
  expect_equal(spec$expectedLength, 1)
})

test_that("integerOption() validates inputs", {
  expect_error(
    integerOption(nullAllowed = "yes"),
    regexp = messages$errorWrongType("nullAllowed", typeof("yes"), "logical"),
    fixed = TRUE
  )
  expect_error(
    integerOption(expectedLength = 0),
    regexp = messages$errorExpectedLengthPositive(),
    fixed = TRUE
  )
  expect_error(
    integerOption(expectedLength = 1.5),
    regexp = messages$errorWrongType(
      "expectedLength",
      class(1.5)[1],
      "integer"
    ),
    fixed = TRUE
  )
  expect_error(
    integerOption(min = 10L, max = 1L),
    regexp = messages$errorMinMaxInvalid(10L, 1L),
    fixed = TRUE
  )
})

test_that("integerOption() supports one-sided ranges with -Inf/Inf defaults", {
  specMinOnly <- integerOption(min = 1L)
  expect_equal(specMinOnly$valueRange, c(1L, .Machine$integer.max))

  specMaxOnly <- integerOption(max = 100L)
  expect_equal(specMaxOnly$valueRange, c(-.Machine$integer.max, 100L))

  specNoRange <- integerOption()
  expect_null(specNoRange$valueRange)

  validOptions <- list(
    age = integerOption(min = 0L, max = 120L),
    count = integerOption(min = 1L)
  )

  options <- list(age = 25L, count = 10L)
  expect_silent(validateIsOption(options, validOptions))
})

test_that("integerOption() handles explicit NULL and NA for min/max", {
  specNullMin <- integerOption(min = NULL, max = 100L)
  expect_equal(specNullMin$valueRange, c(-.Machine$integer.max, 100L))

  specNullMax <- integerOption(min = 1L, max = NULL)
  expect_equal(specNullMax$valueRange, c(1L, .Machine$integer.max))

  specNaMin <- integerOption(min = NA, max = 100L)
  expect_equal(specNaMin$valueRange, c(-.Machine$integer.max, 100L))

  specNaMax <- integerOption(min = 1L, max = NA)
  expect_equal(specNaMax$valueRange, c(1L, .Machine$integer.max))
})

test_that("numericOption() handles explicit NULL and NA for min/max", {
  specNullMin <- numericOption(min = NULL, max = 1.0)
  expect_equal(specNullMin$valueRange, c(-Inf, 1.0))

  specNullMax <- numericOption(min = 0.0, max = NULL)
  expect_equal(specNullMax$valueRange, c(0.0, Inf))

  specNaMin <- numericOption(min = NA, max = 1.0)
  expect_equal(specNaMin$valueRange, c(-Inf, 1.0))

  specNaMax <- numericOption(min = 0.0, max = NA)
  expect_equal(specNaMax$valueRange, c(0.0, Inf))
})

test_that("numericOption() creates valid spec", {
  spec <- numericOption(min = 0, max = 1)
  expect_s3_class(spec, "optionSpec_numeric")
  expect_s3_class(spec, "optionSpec")
  expect_equal(spec$type, "numeric")
  expect_equal(spec$valueRange, c(0, 1))
})

test_that("characterOption() creates valid spec", {
  spec <- characterOption(allowedValues = c("a", "b"))
  expect_s3_class(spec, "optionSpec_character")
  expect_s3_class(spec, "optionSpec")
  expect_equal(spec$type, "character")
  expect_equal(spec$allowedValues, c("a", "b"))
})

test_that("characterOption() validates allowedValues", {
  expect_error(
    characterOption(allowedValues = 123),
    regexp = messages$errorWrongType(
      "allowedValues",
      class(123)[1],
      "character"
    ),
    fixed = TRUE
  )
  expect_error(
    characterOption(allowedValues = character(0)),
    regexp = messages$errorAllowedValuesEmpty(),
    fixed = TRUE
  )
})

test_that("characterOption() rejects NA in allowedValues", {
  expect_error(
    characterOption(allowedValues = c("a", "b", NA)),
    "allowedValues cannot contain NA"
  )
  expect_error(
    characterOption(allowedValues = c("a", "b", NA)),
    "naAllowed = TRUE"
  )
  expect_error(
    characterOption(allowedValues = c(NA_character_)),
    "NA"
  )
})

test_that("characterOption() correct usage with NA", {
  spec <- characterOption(allowedValues = c("a", "b"), naAllowed = TRUE)
  expect_equal(spec$allowedValues, c("a", "b"))
  expect_true(spec$naAllowed)

  spec2 <- characterOption(allowedValues = NULL, naAllowed = TRUE)
  expect_null(spec2$allowedValues)
  expect_true(spec2$naAllowed)
})

test_that("logicalOption() creates valid spec", {
  spec <- logicalOption()
  expect_s3_class(spec, "optionSpec_logical")
  expect_s3_class(spec, "optionSpec")
  expect_equal(spec$type, "logical")
  expect_equal(spec$nullAllowed, FALSE)
})


# Spec normalization ------------------------------------------------------

test_that(".normalizeSpec() passes through new optionSpec objects unchanged", {
  newSpec <- integerOption(min = 1L, max = 10L)
  normalized <- .normalizeSpec(newSpec)
  expect_identical(newSpec, normalized)
})

test_that(".normalizeSpec() converts old integer list format correctly", {
  oldSpec <- list(type = "integer", valueRange = c(1L, 10L))
  newSpec <- .normalizeSpec(oldSpec, "testOption")
  expect_s3_class(newSpec, "optionSpec_integer")
  expect_s3_class(newSpec, "optionSpec")
  expect_equal(newSpec$valueRange, c(1L, 10L))
  expect_equal(newSpec$expectedLength, 1)
})

test_that(".normalizeSpec() converts old numeric list format correctly", {
  oldSpec <- list(type = "numeric", valueRange = c(0, 1))
  newSpec <- .normalizeSpec(oldSpec)
  expect_s3_class(newSpec, "optionSpec_numeric")
  expect_equal(newSpec$valueRange, c(0, 1))
})

test_that(".normalizeSpec() converts old character list format correctly", {
  oldSpec <- list(type = "character", allowedValues = c("a", "b"))
  newSpec <- .normalizeSpec(oldSpec)
  expect_s3_class(newSpec, "optionSpec_character")
  expect_equal(newSpec$allowedValues, c("a", "b"))
})

test_that(".normalizeSpec() converts old logical list format correctly", {
  oldSpec <- list(type = "logical", nullAllowed = TRUE)
  newSpec <- .normalizeSpec(oldSpec)
  expect_s3_class(newSpec, "optionSpec_logical")
  expect_true(newSpec$nullAllowed)
})

test_that(".normalizeSpec() handles missing optional fields with defaults", {
  oldSpec <- list(type = "integer")
  newSpec <- .normalizeSpec(oldSpec)
  expect_equal(newSpec$nullAllowed, FALSE)
  expect_equal(newSpec$naAllowed, FALSE)
  expect_equal(newSpec$expectedLength, 1)
})

test_that(".normalizeSpec() errors when spec is not a list", {
  expect_error(
    .normalizeSpec("invalid", "testOption"),
    regexp = messages$errorSpecNotList("testOption"),
    fixed = TRUE
  )
  expect_error(
    .normalizeSpec(123),
    regexp = messages$errorSpecNotList(),
    fixed = TRUE
  )
})

test_that(".normalizeSpec() errors when type field is missing", {
  oldSpec <- list(valueRange = c(1L, 10L))
  expect_error(
    .normalizeSpec(oldSpec, "testOption"),
    regexp = messages$errorSpecMissingType("testOption"),
    fixed = TRUE
  )
})

test_that(".normalizeSpec() errors on invalid type", {
  oldSpec <- list(type = "invalid")
  expect_error(
    .normalizeSpec(oldSpec, "testOption"),
    regexp = messages$errorInvalidSpecType("invalid", "testOption"),
    fixed = TRUE
  )
})


# S3 validation methods ---------------------------------------------------

test_that(".validateValue.optionSpec() checks expectedLength", {
  spec <- integerOption(min = 1L, max = 10L, expectedLength = 1)
  expect_error(
    .validateValue(c(1L, 2L), spec, "test"),
    regexp = messages$errorWrongLength(c(1L, 2L), 1, "test"),
    fixed = TRUE
  )
  expect_silent(.validateValue(5L, spec, "test"))
})

test_that(".validateValue.optionSpec() validates with NULL expectedLength", {
  spec <- integerOption(min = 1L, max = 10L, expectedLength = NULL)
  expect_silent(.validateValue(c(1L, 2L, 3L), spec, "test"))
  expect_silent(.validateValue(5L, spec, "test"))
})

test_that(".validateValue.optionSpec_integer() auto-converts numeric silently", {
  spec <- integerOption(min = 1L, max = 10L)

  expect_silent(.validateValue(5.0, spec, "myOption"))
  expect_silent(.validateValue(5L, spec, "myOption"))
})

test_that(".validateValue.optionSpec_integer() rejects non-convertible numeric", {
  spec <- integerOption(min = 1L, max = 10L)

  # Should fail if numeric doesn't convert cleanly (5.5 can't be integer)
  expect_error(
    .validateValue(5.5, spec, "test"),
    regexp = messages$errorWrongType("x", class(5.5)[1], "integer"),
    fixed = TRUE
  )
})


# validateIsOption --------------------------------------------------------

validOptions <- list(
  optimizationMethod = list(
    type = "character",
    allowedValues = c("gradientDescent", "geneticAlgorithm")
  ),
  includeInteractions = list(
    type = "logical"
  ),
  maxIterations = list(
    type = "integer",
    valueRange = c(1L, 1e5L),
    nullAllowed = FALSE
  ),
  convergenceThreshold = list(
    type = "numeric",
    valueRange = c(0, 0.1),
    nullAllowed = TRUE
  )
)

defaultOptions <- list(
  optimizationMethod = "gradientDescent",
  includeInteractions = TRUE,
  maxIterations = 10000L,
  convergenceThreshold = 0.02
)

test_that("validateIsOption() handles valid options correctly", {
  expect_null(
    validateIsOption(defaultOptions, validOptions)
  )
})

test_that("validateIsOption() auto-converts numerics to integers with warning", {
  testOptions <- modifyList(defaultOptions, list(numCores = 1))
  expect_null(
    validateIsOption(testOptions, validOptions)
  )
})

test_that("validateIsOption() auto-converts numerics to integers silently", {
  testOptions <- modifyList(defaultOptions, list(maxIterations = 1000))
  expect_silent(validateIsOption(testOptions, validOptions))
})

test_that("validateIsOption() errors on non-list 'options'", {
  expect_error(
    validateIsOption(options = "a", validOptions = validOptions),
    regexp = messages$errorWrongType("object", "character", "list"),
    fixed = TRUE
  )
  expect_error(
    validateIsOption(options = NULL, validOptions = validOptions),
    regexp = messages$errorWrongType("object", "NULL", "list"),
    fixed = TRUE
  )
})

test_that("isValidOption() errors on non-list `validOptions`", {
  expect_error(
    validateIsOption(defaultOptions, validOptions = 1.2),
    regexp = messages$errorWrongType("object", "numeric", "list"),
    fixed = TRUE
  )
  expect_error(
    validateIsOption(options = defaultOptions, validOptions = NULL),
    regexp = messages$errorWrongType("object", "NULL", "list"),
    fixed = TRUE
  )
})

test_that("validateIsOption() errors when option values exceed expected length", {
  testOptions <- modifyList(
    defaultOptions,
    list(includeInteractions = c(TRUE, FALSE))
  )
  expect_error(
    validateIsOption(testOptions, validOptions),
    "(includeInteractions).*(should be of length).*(1)"
  )
})

test_that("validateIsOption() type mismatches trigger errors", {
  invalidOptions <- modifyList(defaultOptions, list(maxIterations = Inf))
  expect_warning(
    expect_error(
      validateIsOption(invalidOptions, validOptions),
      "(maxIterations).*(argument).*(x).*(is of type).*(numeric).*(but expected).*(integer)"
    )
  )
  invalidOptions <- modifyList(
    defaultOptions,
    list(includeInteractions = "notValid")
  )
  expect_error(
    validateIsOption(invalidOptions, validOptions),
    "(includeInteractions).*(argument).*(x).*(is of type).*(character).*(but expected).*(logical)"
  )
  invalidOptions <- modifyList(
    defaultOptions,
    list(convergenceThreshold = "low")
  )
  expect_error(
    validateIsOption(invalidOptions, validOptions),
    "(argument).*(x).*(is of type).*(character).*(but expected).*(numeric)"
  )
})

test_that("validateIsOption() errors when NA values are present and not allowed", {
  testOptions <- modifyList(defaultOptions, list(maxIterations = NA_integer_))
  expect_error(
    validateIsOption(testOptions, validOptions),
    "(maxIterations).*(NA).*(values are not allowed)"
  )
})

test_that("validateIsOption() flags out-of-range and invalid values", {
  invalidOptions <- modifyList(defaultOptions, list(maxIterations = 1e6L))
  expect_error(
    validateIsOption(invalidOptions, validOptions),
    "(maxIterations).*(Value\\(s\\) out of the allowed range)"
  )
  invalidOptions <- modifyList(defaultOptions, list(convergenceThreshold = 2))
  expect_error(
    validateIsOption(invalidOptions, validOptions),
    "(convergenceThreshold).*(Value\\(s\\) out of the allowed range)"
  )
  invalidOptions <- modifyList(
    defaultOptions,
    list(optimizationMethod = "none")
  )
  expect_error(
    validateIsOption(invalidOptions, validOptions),
    regexp = messages$errorValueNotAllowed(
      "none",
      c("gradientDescent", "geneticAlgorithm")
    ),
    fixed = TRUE
  )
})

test_that("validateIsOption() correctly handles NULL values per configuration", {
  invalidOptions <- modifyList(
    defaultOptions,
    list(maxIterations = NULL),
    keep.null = TRUE
  )
  expect_error(
    validateIsOption(invalidOptions, validOptions),
    "(maxIterations).*(argument).*(x).*(is of type).*(NULL).*(but expected).*(vector)"
  )
  invalidOptions <- modifyList(
    defaultOptions,
    list(convergenceThreshold = NULL)
  )
  expect_null(
    validateIsOption(invalidOptions, validOptions)
  )
})

test_that("validateIsOption() works with spec constructors", {
  modernValidOptions <- list(
    maxIterations = integerOption(min = 1L, max = 10000L),
    method = characterOption(
      allowedValues = c("gradientDescent", "geneticAlgorithm")
    )
  )

  modernOptions <- list(maxIterations = 100L, method = "gradientDescent")
  expect_null(validateIsOption(modernOptions, modernValidOptions))
})

test_that("validateIsOption() validates expectedLength", {
  validOptionsWithLength <- list(
    methods = characterOption(
      allowedValues = c("a", "b", "c"),
      expectedLength = 2
    )
  )

  optionsCorrectLength <- list(methods = c("a", "b"))
  expect_null(validateIsOption(optionsCorrectLength, validOptionsWithLength))

  optionsWrongLength <- list(methods = "a")
  expect_error(
    validateIsOption(optionsWrongLength, validOptionsWithLength),
    "(methods).*(should be of length).*(2)"
  )
})

test_that("validateIsOption() reports all validation failures at once", {
  validOptionsMultiple <- list(
    maxIterations = integerOption(min = 1L, max = 100L),
    method = characterOption(allowedValues = c("a", "b")),
    threshold = numericOption(min = 0, max = 1)
  )

  invalidOptionsMultiple <- list(
    maxIterations = 500L,
    method = "invalid",
    threshold = 1.5
  )

  err <- tryCatch(
    validateIsOption(invalidOptionsMultiple, validOptionsMultiple),
    error = function(e) e$message
  )

  expect_true(grepl("maxIterations", err))
  expect_true(grepl("method", err))
  expect_true(grepl("threshold", err))
  expect_true(grepl("validation failed", err, ignore.case = TRUE))
})

test_that("validateIsOption() handles mix of valid and invalid options", {
  validOptionsMixed <- list(
    maxIterations = integerOption(min = 1L, max = 100L),
    method = characterOption(allowedValues = c("a", "b")),
    threshold = numericOption(min = 0, max = 1)
  )

  optionsMixed <- list(
    maxIterations = 50L,
    method = "invalid",
    threshold = 1.5
  )

  err <- tryCatch(
    validateIsOption(optionsMixed, validOptionsMixed),
    error = function(e) e$message
  )

  expect_true(grepl("method", err))
  expect_true(grepl("threshold", err))
  expect_false(grepl("maxIterations.*:", err))
})


# Data frame column validation using validateIsOption() -------------------

test_that("validateIsOption() validates data frame columns", {
  df <- data.frame(
    age = c(25L, 30L, 35L),
    gender = c("M", "F", "M")
  )

  validOptions <- list(
    age = integerOption(min = 18L, max = 65L, expectedLength = nrow(df)),
    gender = characterOption(
      allowedValues = c("M", "F"),
      expectedLength = nrow(df)
    )
  )

  expect_silent(validateIsOption(as.list(df), validOptions))
})

test_that("validateIsOption() validates data frame columns with NA values", {
  df <- data.frame(
    age = c(32L, NA, 19L, 26L, 47L),
    bmi = c(22.4, NA, 19.5, 27.8, 32.4),
    gender = c("M", "F", "F", "M", "M"),
    smoker = c(TRUE, NA, FALSE, FALSE, FALSE)
  )

  validOptions <- list(
    age = integerOption(
      min = 0L,
      max = 120L,
      naAllowed = TRUE,
      expectedLength = nrow(df)
    ),
    bmi = numericOption(
      min = 10,
      max = 50,
      naAllowed = TRUE,
      expectedLength = nrow(df)
    ),
    gender = characterOption(
      allowedValues = c("M", "F"),
      expectedLength = nrow(df)
    ),
    smoker = logicalOption(naAllowed = TRUE, expectedLength = nrow(df))
  )

  expect_silent(validateIsOption(as.list(df), validOptions))
})

test_that("validateIsOption() detects wrong type in data frame columns", {
  df <- data.frame(
    age = c(23.2, 30.5, 35.1),
    gender = c("M", "F", "M")
  )

  validOptions <- list(
    age = integerOption(min = 18L, max = 65L, expectedLength = nrow(df)),
    gender = characterOption(
      allowedValues = c("M", "F"),
      expectedLength = nrow(df)
    )
  )

  expect_error(
    validateIsOption(as.list(df), validOptions),
    "(age).*(argument).*(x).*(is of type).*(numeric).*(but expected).*(integer)"
  )
})

test_that("validateIsOption() detects values outside allowed range in data frame columns", {
  df <- data.frame(
    age = c(25L, 30L, 200L),
    bmi = c(22.4, 27.8, 50.1)
  )

  validOptions <- list(
    age = integerOption(min = 0L, max = 120L, expectedLength = nrow(df)),
    bmi = numericOption(min = 10, max = 50, expectedLength = nrow(df))
  )

  err <- tryCatch(
    validateIsOption(as.list(df), validOptions),
    error = function(e) e$message
  )

  expect_true(grepl("age", err))
  expect_true(grepl("Value\\(s\\) out of the allowed range", err))
})

test_that("validateIsOption() detects values not in allowed values for data frame columns", {
  df <- data.frame(
    gender = c("M", "F", "Other"),
    status = c("active", "inactive", "pending")
  )

  validOptions <- list(
    gender = characterOption(
      allowedValues = c("M", "F"),
      expectedLength = nrow(df)
    ),
    status = characterOption(
      allowedValues = c("active", "inactive"),
      expectedLength = nrow(df)
    )
  )

  err <- tryCatch(
    validateIsOption(as.list(df), validOptions),
    error = function(e) e$message
  )

  expect_true(grepl("gender", err))
  expect_true(grepl(
    messages$errorValueNotAllowed(c("Other"), c("M", "F")),
    err,
    fixed = TRUE
  ))
})

test_that("validateIsOption() rejects NA when not allowed in data frame columns", {
  df <- data.frame(
    age = c(25L, NA, 35L),
    gender = c("M", "F", "M")
  )

  validOptions <- list(
    age = integerOption(
      min = 18L,
      max = 65L,
      naAllowed = FALSE,
      expectedLength = nrow(df)
    ),
    gender = characterOption(
      allowedValues = c("M", "F"),
      expectedLength = nrow(df)
    )
  )

  expect_error(
    validateIsOption(as.list(df), validOptions),
    "(age).*(NA).*(values are not allowed)"
  )
})
