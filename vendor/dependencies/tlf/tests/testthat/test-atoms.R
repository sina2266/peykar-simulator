Sys.setenv(LANG = "en")

test_that("Enum 'Atoms' includes all the atoms", {
  expect_setequal(
    as.character(AtomPlots),
    c("initializePlot", "addScatter", "addLine", "addRibbon", "addErrorbar")
  )
})

test_that("Regular atom plots provide ggplot objects that includes a PlotConfiguration object", {
  emptyPlot <- initializePlot()
  scatterPlot <- addScatter(x = c(1, 2, 3), y = c(1, 2, 3))
  linePlot <- addLine(x = c(1, 2, 3), y = c(1, 2, 3))
  ribbonPlot <- addRibbon(x = c(1, 2, 3), ymin = c(1, 2, 3), ymax = c(3, 4, 5))
  errorbarPlot <- addErrorbar(x = c(1, 2, 3), ymin = c(1, 2, 3), ymax = c(3, 4, 5))

  expect_s3_class(emptyPlot, "ggplot")
  expect_s3_class(scatterPlot, "ggplot")
  expect_s3_class(linePlot, "ggplot")
  expect_s3_class(ribbonPlot, "ggplot")
  expect_s3_class(errorbarPlot, "ggplot")

  expect_s3_class(emptyPlot$plotConfiguration, "PlotConfiguration")
  expect_s3_class(scatterPlot$plotConfiguration, "PlotConfiguration")
  expect_s3_class(linePlot$plotConfiguration, "PlotConfiguration")
  expect_s3_class(ribbonPlot$plotConfiguration, "PlotConfiguration")
  expect_s3_class(errorbarPlot$plotConfiguration, "PlotConfiguration")
})

test_that("A PlotConfiguration input is correctlty used to create the plot when specified", {
  testPlotConfiguration <- PlotConfiguration$new(
    xlabel = "test X",
    ylabel = "test Y",
    watermark = "test watermark"
  )
  emptyPlot <- initializePlot(plotConfiguration = testPlotConfiguration)
  scatterPlot <- addScatter(x = c(1, 2, 3), y = c(1, 2, 3), plotConfiguration = testPlotConfiguration)
  linePlot <- addLine(x = c(1, 2, 3), y = c(1, 2, 3), plotConfiguration = testPlotConfiguration)
  ribbonPlot <- addRibbon(x = c(1, 2, 3), ymin = c(1, 2, 3), ymax = c(3, 4, 5), plotConfiguration = testPlotConfiguration)
  errorbarPlot <- addErrorbar(x = c(1, 2, 3), ymin = c(1, 2, 3), ymax = c(3, 4, 5), plotConfiguration = testPlotConfiguration)

  expect_error(initializePlot(plotConfiguration = "testPlotConfiguration"))
  expect_error(addScatter(x = c(1, 2, 3), y = c(1, 2, 3), plotConfiguration = "testPlotConfiguration"))
  expect_error(addLine(x = c(1, 2, 3), y = c(1, 2, 3), plotConfiguration = "testPlotConfiguration"))
  expect_error(addRibbon(x = c(1, 2, 3), ymin = c(1, 2, 3), ymax = c(3, 4, 5), plotConfiguration = "testPlotConfiguration"))
  expect_error(addErrorbar(x = c(1, 2, 3), ymin = c(1, 2, 3), ymax = c(3, 4, 5), plotConfiguration = "testPlotConfiguration"))

  # Same Labels
  expect_equal(emptyPlot$plotConfiguration$labels, testPlotConfiguration$labels)
  expect_equal(scatterPlot$plotConfiguration$labels, testPlotConfiguration$labels)
  expect_equal(linePlot$plotConfiguration$labels, testPlotConfiguration$labels)
  expect_equal(ribbonPlot$plotConfiguration$labels, testPlotConfiguration$labels)
  expect_equal(errorbarPlot$plotConfiguration$labels, testPlotConfiguration$labels)

  # Same Background
  expect_equal(emptyPlot$plotConfiguration$background, testPlotConfiguration$background)
  expect_equal(scatterPlot$plotConfiguration$background, testPlotConfiguration$background)
  expect_equal(linePlot$plotConfiguration$background, testPlotConfiguration$background)
  expect_equal(ribbonPlot$plotConfiguration$background, testPlotConfiguration$background)
  expect_equal(errorbarPlot$plotConfiguration$background, testPlotConfiguration$background)

  # Same xAxis
  expect_equal(emptyPlot$plotConfiguration$xAxis, testPlotConfiguration$xAxis)
  expect_equal(scatterPlot$plotConfiguration$xAxis, testPlotConfiguration$xAxis)
  expect_equal(linePlot$plotConfiguration$xAxis, testPlotConfiguration$xAxis)
  expect_equal(ribbonPlot$plotConfiguration$xAxis, testPlotConfiguration$xAxis)
  expect_equal(errorbarPlot$plotConfiguration$xAxis, testPlotConfiguration$xAxis)

  # Same yAxis
  expect_equal(emptyPlot$plotConfiguration$yAxis, testPlotConfiguration$yAxis)
  expect_equal(scatterPlot$plotConfiguration$yAxis, testPlotConfiguration$yAxis)
  expect_equal(linePlot$plotConfiguration$yAxis, testPlotConfiguration$yAxis)
  expect_equal(ribbonPlot$plotConfiguration$yAxis, testPlotConfiguration$yAxis)
  expect_equal(errorbarPlot$plotConfiguration$yAxis, testPlotConfiguration$yAxis)
})
