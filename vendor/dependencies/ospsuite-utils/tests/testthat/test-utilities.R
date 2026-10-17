test_that("Checks if `toList()` works properly", {
  expect_type(toList(list("a" = 1, "b" = 2)), "list")
  expect_type(toList(c("a" = 1, "b" = 2)), "list")
  expect_type(toList(c("a" = 1, "b" = 2, c("c" = 3))), "list")
})

test_that("Checks if `flattenList()` works properly", {
  expect_error(
    flattenList(array(1:3, c(2, 4)), type = "character"),
    messages$errorOnlyVectorAllowed()
  )

  expect_equal(
    flattenList(list(1, 2, 3, NA), type = "numeric"),
    c(1, 2, 3, NA)
  )

  expect_equal(
    flattenList(list(TRUE, FALSE, NA), type = "integer"),
    c(1L, 0L, NA)
  )
})

test_that("Checks if `toMissingOfType()` converts special constants", {
  expect_error(toMissingOfType(NA, type = "xyz"), "Incorrect type entered.")
  expect_equal(toMissingOfType(NA, type = "real"), NA_real_)
  expect_equal(toMissingOfType(NULL, type = "integer"), NA_integer_)
  expect_equal(toMissingOfType(Inf, type = "character"), NA_character_)
  expect_equal(toMissingOfType(NaN, type = "logical"), NA)
})

test_that("Checks if `toMissingOfType()` doesn't convert other objects", {
  expect_equal(toMissingOfType("NA", type = "real"), "NA")
  expect_equal(toMissingOfType(0, type = "integer"), 0)
  expect_equal(toMissingOfType(TRUE, type = "character"), TRUE)
  expect_equal(toMissingOfType(1L, type = "logical"), 1L)
})

test_that("Correct behavior of logSafe", {
  defEpsilon <- getOSPSuiteUtilsSetting("LOG_SAFE_EPSILON")
  inputVector <- c(NA, 1, 5, 0, -1, defEpsilon)

  expectedOutput <- c(
    NA_real_,
    log(1),
    log(5),
    log(defEpsilon),
    log(defEpsilon),
    log(defEpsilon)
  )
  expect_equal(logSafe(inputVector), expectedOutput)

  # Test for log10
  expectedOutput <- c(
    NA_real_,
    log10(1),
    log10(5),
    log10(defEpsilon),
    log10(defEpsilon),
    log10(defEpsilon)
  )
  expect_equal(logSafe(inputVector, base = 10), expectedOutput)
})

test_that("Correct behavior of foldSafe", {
  defEpsilon <- getOSPSuiteUtilsSetting("LOG_SAFE_EPSILON")
  inputX <- c(NA, 1, 5, 0, -1, defEpsilon, 5)
  inputY <- c(1, -1, NA, 0, -1, defEpsilon, 2)

  expectedOutput <- c(
    NA_real_,
    1 / defEpsilon,
    NA_real_,
    defEpsilon / defEpsilon,
    defEpsilon / defEpsilon,
    defEpsilon / defEpsilon,
    5 / 2
  )
  expect_equal(foldSafe(inputX, inputY), expectedOutput)
})
