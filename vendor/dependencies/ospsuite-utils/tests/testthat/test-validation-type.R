# dataframes useful for all tests in this file
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

# isOfType ------------------------------

test_that("isOfType doesn't work when `nullAllowed` argument is not logical", {
  expect_error(
    isOfType(NULL, nullAllowed = "a"),
    "(argument).*(nullAllowed).*(is of type).*(character).*(but expected).*(logical)"
  )

  expect_error(
    isOfType(NULL, nullAllowed = 1),
    "(argument).*(nullAllowed).*(is of type).*(double).*(but expected).*(logical)"
  )

  expect_error(
    isOfType(NULL, nullAllowed = 0L),
    "(argument).*(nullAllowed).*(is of type).*(integer).*(but expected).*(logical)"
  )
})

test_that("isOfType returns `TRUE` when values are of expected type", {
  expect_true(isOfType(A, "data.frame"))
  expect_true(isOfType(list(A, B), "data.frame"))
  expect_true(isOfType(c(1, "x"), c("numeric", "character")))
  expect_true(isOfType(logical(), "logical"))
  expect_true(isOfType(NULL, nullAllowed = TRUE))

  Person <- R6::R6Class(
    "Person",
    list(
      name = NULL,
      initialize = function(name) self$name <- name
    )
  )

  Jack <- Person$new(name = "Jack")
  Jill <- Person$new(name = "Jill")
  tmp <- list(Jack, NULL, Jill)

  expect_true(isOfType(tmp, Person, nullAllowed = TRUE))
})

test_that("isOfType returns `FALSE` when values are not of expected type", {
  expect_false(isOfType(A, "character"))
})

# validateIsOfType ------------------------------

test_that("validateIsOfType returns `NULL` when type is unexpected", {
  expect_null(validateIsOfType(object = 2, type = "integer"))
  expect_null(validateIsOfType(object = 2L, type = "integer"))
  expect_null(validateIsOfType(A, "data.frame"))
})

test_that("validateIsOfType throws an error when type is unexpected", {
  expect_error(validateIsOfType(object = 2.5, type = "integer"))
  expect_error(validateIsOfType(A, data.frame))
})

test_that("validateIsOfType throws expected error message when type is unexpected", {
  errorMessageIsOfType <- "(argument).*(A).*(is of type).*(data.frame).*(but expected).*(character)"
  expect_error(validateIsOfType(A, "character"), errorMessageIsOfType)
})

# validateIsInteger ------------------------------

test_that("validateIsInteger returns `NULL` for integers", {
  expect_null(validateIsInteger(5))
  expect_null(validateIsInteger(5L))
  expect_null(validateIsInteger(c(1L, 5)))
  expect_null(validateIsInteger(c(1L, 5L)))
  expect_null(validateIsInteger(list(1L, 5)))
  expect_null(validateIsInteger(list(1L, 5L)))
  expect_null(validateIsInteger(NA_integer_))
  expect_null(validateIsInteger(NULL, nullAllowed = TRUE))
})

test_that("validateIsInteger produces errors if not integers", {
  expect_error(validateIsInteger(c(1.5, 5)))
  expect_error(validateIsInteger(list(1.5, 5)))
  expect_error(validateIsInteger(2.4))
  expect_error(validateIsInteger("2"))
  expect_error(validateIsInteger(TRUE))
  expect_error(validateIsInteger(NA_character_))
})

test_that("validateIsInteger produces expected error message when validating that a string is an integer", {
  expect_error(
    validateIsInteger("s"),
    "(argument).*(s).*(is of type).*(character).*(but expected).*(integer)"
  )
})

test_that("validateIsInteger produces errors if a factor is provided", {
  df <- data.frame(numCol = c(1, 2, 3))
  df$numCol <- as.factor(df$numCol)

  expect_error(
    validateIsInteger(df$numCol),
    "(argument).*(df\\$numCol).*(is of type).*(factor).*(but expected).*(integer)"
  )
})

# validateIsCharacter ------------------------------

test_that("validateIsCharacter returns `NULL` for characters", {
  expect_null(validateIsCharacter(c("x", "y")))
  expect_null(validateIsCharacter(list("x", "y")))
})

test_that("validateIsCharacter produces errors if not character", {
  a <- 4L
  expect_error(validateIsCharacter(a))
})

# validateIsNumeric ------------------------------

test_that("validateIsNumeric returns `NULL` for numeric", {
  expect_null(validateIsNumeric(c(1.2, 2.3)))
  expect_null(validateIsNumeric(list(1.2, 2.3)))
  expect_null(validateIsNumeric(NULL, nullAllowed = TRUE))
  expect_null(validateIsNumeric(c(NA, NULL)))
})

test_that("validateIsNumeric produces errors if not numeric", {
  expect_error(validateIsNumeric(c("1.2", "2.3")))
  expect_error(validateIsNumeric(list("1.2", "2.3")))
})

test_that("validateIsNumeric produces errors if a factor is provided", {
  skip_if_not(getRversion() > "4.1") # for factors

  df <- data.frame(numCol = c(1, 2, 3))
  df$numCol <- as.factor(df$numCol)

  expect_error(
    validateIsNumeric(df$numCol),
    "(argument).*(df\\$numCol).*(is of type).*(factor).*(but expected).*(numeric/integer)"
  )
})

# validateIsLogical ------------------------------

test_that("validateIsLogical returns `NULL` for logicals", {
  expect_null(validateIsLogical(TRUE))
  expect_null(validateIsLogical(c(TRUE, FALSE)))
  expect_null(validateIsLogical(list(TRUE, FALSE)))
})

test_that("validateIsLogical produces errors if not logical", {
  expect_error(validateIsLogical(c(1, 0)))
  expect_error(validateIsLogical(list(1, 0)))
})
