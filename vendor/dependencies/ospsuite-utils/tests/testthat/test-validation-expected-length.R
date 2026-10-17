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

# isOfLength --------------------------------------

test_that("isOfLength returns `TRUE` when correct length is specified", {
  expect_true(isOfLength(A, 3L))
  expect_true(isOfLength(c(1, 2), 2L))
  expect_true(isOfLength(list("1", 2, 4.5, TRUE), 4L))
  expect_true(isOfLength(character(), 0L))
})

test_that("isOfLength returns `FALSE` when incorrect length is specified", {
  expect_false(isOfLength(A, 5))
  expect_false(isOfLength(c(1, 2), 3L))
  expect_false(isOfLength(list("1", 2, 4.5, TRUE), 3L))
  expect_false(isOfLength(character(), 1L))
})

# validateIsOfLength ------------------------------------

test_that("validateIsOfLength returns `NULL` when objects have expected length", {
  expect_null(validateIsOfLength(A, 3))
})

test_that("validateIsOfLength produces error when objects do not have expected length", {
  errorMessageIsOfLength <- "(Object).*(should be of length).*(5).*(but is of length).*(3)"
  expect_error(validateIsOfLength(A, 5), errorMessageIsOfLength)
})
