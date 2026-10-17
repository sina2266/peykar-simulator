# To prevent copy/paste, expressions are used in the sequel
plotNames <- c("DDIRatio", "", "TimeProfile", "ObsVsPred", "ResVsPred", "Histogram", "PKRatio", "BoxWhisker", "Tornado")

# Epression is "<plotname>plotConfiguration <- <PlotName>PlotConfiguration$new()"
createPlotConfigurations <- parse(text = paste0(tolower(plotNames), "plotConfiguration <- ", plotNames, "PlotConfiguration$new()"))
eval(createPlotConfigurations)

# Epression is "expect_false(<plotname>plotConfiguration$defaultExpand)"
# Use -1 to remove first element ie DDI Ratio from the evaluation
checkExpandIsFalse <- parse(text = paste0("expect_false(", tolower(plotNames[-1]), "plotConfiguration$defaultExpand)"))

test_that("Default expand is true for DDI ratio only", {
  eval(checkExpandIsFalse)
  expect_true(ddiratioplotConfiguration$defaultExpand)
})


test_that("Default x and y scales is log for ddi ratio", {
  expect_equal(ddiratioplotConfiguration$defaultXScale, Scaling$log)
  expect_equal(ddiratioplotConfiguration$defaultYScale, Scaling$log)
})

test_that("Default x scale is linear and y scale is log for pk ratio", {
  expect_equal(pkratioplotConfiguration$defaultXScale, Scaling$lin)
  expect_equal(pkratioplotConfiguration$defaultYScale, Scaling$log)
})

test_that("Default x scale is linear and y scale is discrete for tornado plot", {
  expect_equal(tornadoplotConfiguration$defaultXScale, Scaling$lin)
  expect_equal(tornadoplotConfiguration$defaultYScale, Scaling$discrete)
})

test_that("Default x scale is discrete and y scale is linear for box-whisker plot", {
  expect_equal(boxwhiskerplotConfiguration$defaultXScale, Scaling$discrete)
  expect_equal(boxwhiskerplotConfiguration$defaultYScale, Scaling$lin)
})

checkXScaleIsLinear <- parse(text = paste0("expect_equal(", tolower(plotNames[2:6]), "plotConfiguration$defaultXScale, Scaling$lin)"))
checkYScaleIsLinear <- parse(text = paste0("expect_equal(", tolower(plotNames[2:6]), "plotConfiguration$defaultYScale, Scaling$lin)"))
test_that("Default scales are linear for other plots", {
  eval(checkXScaleIsLinear)
  eval(checkYScaleIsLinear)
})
