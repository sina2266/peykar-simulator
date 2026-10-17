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

x <- 1
y <- 2
z <- 3

a <- 1

# isEmpty ------------------------------

test_that("isEmpty returns `TRUE` when objects are empty", {
  expect_true(isEmpty(NULL))
  expect_true(isEmpty(data.frame()))
  expect_true(isEmpty(list()))
  expect_true(isEmpty(character()))
  expect_true(isEmpty(numeric()))
  expect_true(isEmpty(A[FALSE, ]))
})

test_that("isEmpty returns `FALSE` when objects are not empty", {
  expect_false(isEmpty(a))
  expect_false(isEmpty(A))
  expect_false(isEmpty(list("x", "y")))
  expect_false(isEmpty(""))
})

# validateIsNotEmpty ------------------------------

test_that("validateIsNotEmpty returns `NULL` when objects are not empty", {
  expect_null(validateIsNotEmpty(A))
  expect_null(validateIsNotEmpty(""))
})

test_that("validateIsNotEmpty produces error when objects are empty", {
  errorMessageIsEmpty <- "(argument).*(NULL).*(is).*(empty)"
  expect_error(validateIsNotEmpty(NULL), errorMessageIsEmpty)
})

# hasEmptyStrings ------------------------------

test_that("hasEmptyStrings returns `TRUE` when strings are empty", {
  expect_true(hasEmptyStrings(character(0)))
  expect_true(hasEmptyStrings(c("", "y")))
  expect_true(hasEmptyStrings(list("", "y")))
  expect_true(hasEmptyStrings(c("", NA)))
  expect_true(hasEmptyStrings(list("", NA)))
  expect_true(hasEmptyStrings(NA))
  expect_true(hasEmptyStrings(c(NA, "x", "y")))
  expect_true(hasEmptyStrings(list(NA, "x", "y")))
})

test_that("hasEmptyStrings returns `FALSE` when string are not empty", {
  expect_false(hasEmptyStrings("   abc   "))
  expect_false(hasEmptyStrings(c("x", "y")))
  expect_false(hasEmptyStrings(list("x", "y")))
})

test_that("validateHasOnlyNonEmptyStrings returns `NULL` when string are not empty", {
  expect_null(validateHasOnlyNonEmptyStrings("   abc   "))
  expect_null(validateHasOnlyNonEmptyStrings(c("x", "y")))
  expect_null(validateHasOnlyNonEmptyStrings(list("x", "y")))
})


test_that("validateHasOnlyNonEmptyStrings produces error when strings are empty", {
  expect_error(validateHasOnlyNonEmptyStrings(character(0)))
  expect_error(validateHasOnlyNonEmptyStrings(c("", "y")))
  expect_error(validateHasOnlyNonEmptyStrings(list("", "y")))
  expect_error(validateHasOnlyNonEmptyStrings(c("", NA)))
  expect_error(validateHasOnlyNonEmptyStrings(list("", NA)))
  expect_error(validateHasOnlyNonEmptyStrings(NA))
  expect_error(validateHasOnlyNonEmptyStrings(c(NA, "x", "y")))
  expect_error(validateHasOnlyNonEmptyStrings(list(NA, "x", "y")))
})

test_that("validateHasOnlyNonEmptyStrings produces expected error message", {
  myArgument <- list("", "y")
  expect_error(
    validateHasOnlyNonEmptyStrings(myArgument),
    "(argument).*(myArgument).*(has empty).*(strings)"
  )
})
