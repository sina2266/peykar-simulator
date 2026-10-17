data <- data.frame("Ratio" = c(seq(0.1, 5, 0.1), NA), "OtherRatio" = c(seq(0.2, 9.8, 0.2), NA, NA))
testMapping <- PKRatioDataMapping$new(y = "OtherRatio")

test_that("outputs a data.frame", {
  expect_s3_class(getPKRatioMeasure(data), "data.frame")
})

test_that("default works", {
  expectedDefaultResult <- data.frame(
    "Number" = c(50, 8, 16),
    "Ratio" = c(NA, 8 / 50, 16 / 50),
    row.names = c("Points Total", "Points within 1.5-fold", "Points within 2-fold")
  )
  expect_equal(getPKRatioMeasure(data$Ratio), expectedDefaultResult)
})

test_that("works when ratio limits are changed", {
  expectedResultLimit <- data.frame(
    "Number" = c(50, 26),
    "Ratio" = c(NA, 0.52),
    row.names = c("Points Total", "Points within 3-fold")
  )
  expect_equal(getPKRatioMeasure(data$Ratio, dataMapping = NULL, ratioLimits = 3), expectedResultLimit)
})

test_that("dataMapping input works", {
  expectedResultMapping <- data.frame(
    "Number" = c(49, 4, 8),
    "Ratio" = c(NA, 4 / 49, 8 / 49),
    row.names = c("Points Total", "Points within 1.5-fold", "Points within 2-fold")
  )
  expect_equal(getPKRatioMeasure(data, dataMapping = testMapping), expectedResultMapping)
})
