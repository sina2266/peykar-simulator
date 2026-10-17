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

# isSameLength ------------------------------------

test_that("isSameLength returns `TRUE` when objects have same lengths", {
  expect_true(isSameLength(A, A))
  expect_true(isSameLength(c(1, 2), c("x", "y"), c(TRUE, FALSE)))
  expect_true(isSameLength(list(1, 2), list("x", "y"), list(TRUE, FALSE)))
  expect_true(isSameLength(character(), integer()))
})

test_that("isSameLength returns `FALSE` when objects have different lengths", {
  expect_false(isSameLength(A, B))
  expect_false(isSameLength(c(1, 2), c("x"), c(TRUE, FALSE)))
  expect_false(isSameLength(list(1, 2), list("x", "y"), list(FALSE)))
})

# validateIsSameLength ------------------------------------

test_that("validateIsSameLength returns `NULL` when objects have same lengths", {
  expect_null(validateIsSameLength(A, A))
})

test_that("validateIsSameLength produces errors when objects do not have same lengths", {
  errorMessageIsSameLength <- "Arguments 'A, B' must have the same length, but they don't!"
  expect_error(
    validateIsSameLength(A, B),
    "(Arguments).*(A, B).*(must have the same length, but they don't)"
  )
})
