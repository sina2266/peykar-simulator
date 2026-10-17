test_that("Checks if `ifNotNull()` works as expected", {
  expect_null(ifNotNull(NULL, "x"))

  expect_equal(ifNotNull(NULL, "x", "y"), "y")
  expect_equal(ifNotNull(1 < 2, "x", "y"), "x")
})

test_that("Checks if `ifEqual()` works as expected", {
  expect_equal(ifEqual(1, 1, "x", "y"), "x")
  expect_equal(ifEqual(1, 2, "x", "y"), "y")
})

test_that("Checks if `ifIncluded()` works as expected", {
  expect_equal(ifIncluded("a", c("a", "b"), 1, 2), 1)
  expect_equal(ifIncluded("x", c("a", "b"), 1, 2), 2)
})
