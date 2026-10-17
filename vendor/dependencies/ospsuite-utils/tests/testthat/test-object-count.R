test_that("objectCount returns correct count for atomic vectors", {
  x <- c(1, 2, 3)
  expect_equal(objectCount(x), 3L)

  names(x) <- c("a", "b", "c")
  expect_equal(objectCount(x), 3L)

  attr(x, "rando") <- "random"
  expect_equal(objectCount(x), 3L)

  expect_equal(objectCount(character()), 0L)
})

test_that("objectCount returns correct count for lists", {
  x <- list("a", "b")
  expect_equal(objectCount(x), 2L)

  names(x) <- c("a", "b")
  expect_equal(objectCount(x), 2L)

  attr(x, "rando") <- "random"
  expect_equal(objectCount(x), 2L)

  expect_equal(objectCount(list()), 0L)
})

test_that("objectCount returns correct count for data frames", {
  expect_equal(objectCount(mtcars), 1L)
  expect_equal(objectCount(data.frame()), 1L)
})

test_that("objectCount returns correct count for R6 objects", {
  myPrintable <- R6::R6Class(
    "myPrintable",
    inherit = Printable,
    public = list(
      x = NULL,
      y = NULL,
      print = function() {
        private$printClass()
        private$printLine("x", self$x)
        private$printLine("y", self$y)
        invisible(self)
      }
    )
  )

  expect_warning(x <- myPrintable$new())

  expect_equal(objectCount(x), 1L)
  expect_equal(objectCount(list(x, x)), 2L)
})

test_that("objectCount returns correct count for environments, model objects, etc.", {
  e1 <- new.env(parent = baseenv())
  e2 <- new.env(parent = e1)
  assign("a", 3, envir = e1)
  assign("b", 4, envir = e1)

  expect_equal(objectCount(e1), 1L)
  expect_equal(objectCount(list(e1, e2)), 2L)

  mod <- lm(wt ~ mpg, mtcars)
  expect_equal(objectCount(mod), 1L)

  x <- as.POSIXlt(ISOdatetime(2020, 1, 1, 0, 0, 1:3))
  expect_equal(objectCount(x), 1L)
})
