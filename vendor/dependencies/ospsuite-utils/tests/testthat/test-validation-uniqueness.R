test_that("hasOnlyDistinctValues returns TRUE if values are distinct", {
  expect_true(hasOnlyDistinctValues(c("x", NA, "y")))
  expect_true(hasOnlyDistinctValues(list("x", NA, "y")))
})

test_that("hasOnlyDistinctValues returns FALSE if values are repeated", {
  expect_false(hasOnlyDistinctValues(c("x", NA, "y", "x")))
  expect_false(hasOnlyDistinctValues(list("x", NA, "y", "x")))
})

test_that("validateHasOnlyDistinctValues returns NULL with distinct values", {
  expect_null(validateHasOnlyDistinctValues(c("x", "y")))
  expect_null(validateHasOnlyDistinctValues(list("x", "y")))
})

test_that("validateHasOnlyDistinctValues errors with duplicated values", {
  expect_error(
    validateHasOnlyDistinctValues(c("x", "y", "x")),
    messages$errorDuplicatedValues()
  )

  expect_error(
    validateHasOnlyDistinctValues(list("x", "y", "x")),
    messages$errorDuplicatedValues()
  )
})
