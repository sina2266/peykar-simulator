test_that("formatNumerics returns character for integer, numeric or character objects", {
  # integer
  expect_type(formatNumerics(2L, digits = 1, scientific = TRUE), "character")
  # numeric
  expect_type(formatNumerics(2, digits = 1, scientific = TRUE), "character")
  # character
  expect_type(formatNumerics("2", digits = 1, scientific = TRUE), "character")
})


test_that("formatNumerics works as expected", {
  # integer and character as is
  expect_equal(formatNumerics(2L, digits = 1, scientific = TRUE), "2")
  expect_equal(formatNumerics("2", digits = 1, scientific = TRUE), "2")
  # numeric is formated
  expect_equal(formatNumerics(2, digits = 1, scientific = TRUE), "2.0e+00")

  # vector
  x <- formatNumerics(log(c(12, 15, 0.3)), digits = 1, scientific = TRUE)
  expect_equal(x, c("2.5e+00", "2.7e+00", "-1.2e+00"))

  y <- formatNumerics(c(12L, 15L, 3L))
  expect_equal(y, c("12", "15", "3"))

  # data.frame
  df <- data.frame(
    parameter = c("a", "b", "c"),
    value = c(1, 110.4, 6.666),
    stringsAsFactors = FALSE
  )
  expect_equal(
    formatNumerics(df, digits = 2, scientific = FALSE),
    structure(
      list(
        parameter = c("a", "b", "c"),
        value = c(
          "1.00",
          "110.40",
          "6.67"
        )
      ),
      row.names = c(NA, -3L),
      class = "data.frame"
    )
  )
})
