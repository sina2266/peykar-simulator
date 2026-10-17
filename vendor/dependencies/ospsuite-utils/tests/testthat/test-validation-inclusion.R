# dataframes and other objects useful for all tests in this file
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

# isIncluded ----------------------------------

test_that("isIncluded returns `TRUE` when base type values are included", {
  expect_true(isIncluded("col3", names(A)))
  expect_true(isIncluded(2, 2))
  expect_true(isIncluded("x", list("w", "x", 1, 2)))
  expect_true(isIncluded(c("x", "y"), c("a", "y", "b", "x")))
  expect_true(isIncluded(list("x", "y"), list("a", "b", "x", "y")))
  expect_true(isIncluded(a, list(x, y, z)))
  expect_true(isIncluded(a, c(x, y, z)))
})

test_that("isIncluded returns `TRUE` when compound type values are included", {
  skip_if_not(getRversion() > "4.1") # for factors

  expect_true(isIncluded(as.factor("a"), c("a", "b")))
  expect_true(isIncluded(as.factor("a"), list("a", "b")))
  expect_true(isIncluded(c("a", "b"), as.factor(c("a", "b"))))
  expect_true(isIncluded(list("a", "b"), as.factor(c("a", "b"))))
  expect_true(isIncluded(
    as.Date("1970-02-01"),
    c(as.Date("1970-02-01"), as.Date("1980-12-21"))
  ))
  # Disabling this test as inclusion test does not work with Dat in a list:
  # > as.Date("1970-02-01") == as.Date("1970-02-01")
  # [1] TRUE
  # > as.Date("1970-02-01") %in% list(as.Date("1970-02-01"))
  # [1] FALSE
  # > "1970-02-01" %in% list("1970-02-01")
  # [1] TRUE
  # expect_true(isIncluded(as.Date("1970-02-01"), list(as.Date("1970-02-01"), as.Date("1980-12-21"))))
})

test_that("isIncluded returns `FALSE` when base type values are not included", {
  expect_false(isIncluded("col4", names(A)))
  expect_false(isIncluded(1, 2))
  expect_false(isIncluded("x", c("w", "a", "y")))
  expect_false(isIncluded(c("x", "y"), c("a", "b", "x")))
  expect_false(isIncluded(list("x", "y"), list("a", "b", "x")))
  expect_false(isIncluded(a, list(y, z)))
  expect_false(isIncluded(a, c(y, z)))
  expect_false(isIncluded(NULL))
  expect_false(isIncluded(character()))
})

test_that("isIncluded returns `FALSE` when compound type values are not included", {
  skip_if_not(getRversion() > "4.1") # for factors

  expect_false(isIncluded(as.factor("a"), c("d", "b")))
  expect_false(isIncluded(as.factor("a"), list("c", "b")))
  expect_false(isIncluded(c("a", "b"), as.factor(c("d", "b"))))
  expect_false(isIncluded(list("a", "b"), as.factor(c("c", "b"))))
  expect_false(isIncluded(
    as.Date("1970-02-01"),
    c(as.Date("1972-02-01"), as.Date("1980-12-21"))
  ))
  expect_false(isIncluded(
    as.Date("1970-02-01"),
    list(as.Date("1980-02-01"), as.Date("1980-12-21"))
  ))
})

test_that("isIncluded doesn't accept objects as arguments", {
  Person <- R6::R6Class(
    "Person",
    list(
      name = NULL,
      initialize = function(name) self$name <- name
    )
  )

  Jack <- Person$new(name = "Jack")
  Jill <- Person$new(name = "Jill")

  expect_error(isIncluded(Jack, Jill))
  expect_error(isIncluded(c(Jack), list(Jack, Jill)))
})

test_that("isIncluded doesn't accept environments as arguments", {
  e1 <- new.env()
  e2 <- new.env()

  expect_error(isIncluded(e1, e2))
  expect_error(isIncluded(c(e1), c(e2)))
  expect_error(isIncluded(c(e1), list(e2)))
})

# validateIsIncluded ----------------------------------

test_that("validateIsIncluded returns `NULL` when value is included", {
  expect_null(validateIsIncluded("col3", names(A)))
  expect_null(validateIsIncluded(NULL, NULL, nullAllowed = TRUE))
})

test_that("validateIsIncluded produces expected error message when value not included", {
  expect_error(
    validateIsIncluded("col4", names(A)),
    "(1 value).*(col4).*(is not included in parent values).*(col1).*(col2).*(col3)"
  )
})
