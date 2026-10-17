isLogTicksIncludedInLimits <- tlf:::.isLogTicksIncludedInLimits

test_that("isLogTicksIncludedInLimits checks work as expected", {
  expect_true(isLogTicksIncludedInLimits(limits = 1, scale = Scaling$log))
  expect_true(isLogTicksIncludedInLimits(limits = 1, scale = Scaling$ln))
  expect_true(isLogTicksIncludedInLimits(limits = 10, scale = Scaling$log))
  expect_true(isLogTicksIncludedInLimits(limits = exp(1), scale = Scaling$ln))

  expect_true(isLogTicksIncludedInLimits(
    limits = c(5, 15),
    scale = Scaling$log
  ))
  expect_true(isLogTicksIncludedInLimits(limits = c(5, 15), scale = Scaling$ln))

  expect_false(isLogTicksIncludedInLimits(
    limits = c(32, 33),
    scale = Scaling$log
  ))
  expect_false(isLogTicksIncludedInLimits(
    limits = c(32, 33),
    scale = Scaling$ln
  ))
})


test_that("A plot with log ticks do not crash when isLogTicksIncludedInLimits is false", {
  testPlot <- addLine(x = c(31, 32), y = c(31, 32))
  # ggplot2 4.0.0 produces "NaNs produced" warnings, 3.5.2 doesn't
  # Just ensure the functions don't crash
  expect_no_error(print(setXAxis(testPlot, scale = Scaling$log)))
  expect_no_error(print(setYAxis(testPlot, scale = Scaling$log)))
  expect_no_error(print(setXAxis(testPlot, scale = Scaling$ln)))
  expect_no_error(print(setYAxis(testPlot, scale = Scaling$ln)))

  # If a classical plot is used, an error would be obtained as in the example below
  # testPlot <- ggplot(
  # data.frame(x=c(31,32),y=c(31,32)),
  # aes(x=x,y=y)) +
  # geom_point() + scale_y_log10() + annotation_logticks()
  # expect_error(print(testPlot))
})
