A <- data.frame(
  col1 = c(1, 2, 3),
  col2 = c(4, 5, 6),
  col3 = c(7, 8, 9)
)

B <- data.frame(
  col1 = c(1, 2, 3),
  col2 = c(4, 5, 6),
  col3 = c(7, 8, 9),
  col4 = c(7, 8, 9)
)

test_that("Checks of type 'is' work properly", {
  # Output is logical
  expect_type(isSameLength(A, A), "logical")
  expect_type(isOfLength(A, 3), "logical")
  expect_type(isOfType(A, "data.frame"), "logical")
  expect_type(isIncluded("col3", names(A)), "logical")

  # Output is TRUE
  expect_true(isSameLength(A, A))
  expect_true(isOfLength(A, 3))
  expect_true(isOfType(A, "data.frame"))
  expect_true(isIncluded("col3", names(A)))

  # Output is FALSE
  expect_false(isSameLength(A, B))
  expect_false(isOfLength(A, 5))
  expect_false(isOfType(A, "character"))
  expect_false(isIncluded("col4", names(A)))
})

test_that("Checks method of type 'validate' work properly", {
  # NULL when checks succeed
  expect_null(validateIsSameLength(A, A))
  expect_null(validateIsOfLength(A, 3))
  expect_null(validateIsOfType(A, "data.frame"))
  expect_null(validateIsIncluded("col3", names(A)))

  errorMessageIsSameLength <- "Arguments \"A, B\" must have the same length, but they don't!"
  errorMessageIsOfLength <- "Object should be of length 5, but is of length 3 instead."
  errorMessageIsOfType <- 'argument "A" is of type <data.frame>, but expected <character>!'
  errorMessageIsIncluded <- '1 value \\("col4"\\) is not included in parent values: col1, col2, and col3\\.'

  # Error when checks fail
  expect_error(validateIsSameLength(A, B), errorMessageIsSameLength)
  expect_error(validateIsOfLength(A, 5), errorMessageIsOfLength)
  expect_error(validateIsOfType(A, "character"), errorMessageIsOfType)
  expect_error(validateIsIncluded("col4", names(A)), errorMessageIsIncluded)
})
