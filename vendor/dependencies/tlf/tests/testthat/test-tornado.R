# Create a data.frames for the plot tests
tornadoPopData <- data.frame(
  sensitivity = c(c(1, 2, 3, -1, -2, -3), c(1, 2, 3, -1, -2, -3) - 0.2),
  path = rep(rep(c("liver", "kidney"), each = 3), 2),
  quantile = rep(rep(c("5th", "50th", "95th"), 2), 2),
  population = rep(c("Adults", "Peds"), each = 6)
)

tornadoMeanData <- tornadoPopData[tornadoPopData$population %in% "Adults" &
  tornadoPopData$quantile %in% "50th", ]

test_that("TornadoDataMapping features", {
  dataMapping0 <- TornadoDataMapping$new(x = "sensitivity", y = "path")
  dataMappingWithOption <- TornadoDataMapping$new(x = "sensitivity", y = "path", sorted = FALSE)

  dataMappingColor <- TornadoDataMapping$new(x = "sensitivity", y = "path", color = "path")

  dataMappingShape <- TornadoDataMapping$new(
    x = "sensitivity", y = "path",
    color = "population", shape = "quantile"
  )

  expect_equal(dataMapping0$x, "sensitivity")
  expect_equal(dataMapping0$y, "path")
  expect_true(dataMapping0$sorted)

  expect_equal(dataMappingWithOption$x, "sensitivity")
  expect_equal(dataMappingWithOption$y, "path")
  expect_false(dataMappingWithOption$sorted)

  expect_equal(dataMappingColor$x, "sensitivity")
  expect_equal(dataMappingColor$y, "path")
  # For tornado, if not specified, fill, shape and color properties are linked
  # to get directly a correct plot
  expect_equal(dataMappingColor$groupMapping$color$group, "path")
  expect_equal(dataMappingColor$groupMapping$fill$group, "path")
  expect_equal(dataMappingColor$groupMapping$shape$group, "path")

  expect_equal(dataMappingShape$x, "sensitivity")
  expect_equal(dataMappingShape$y, "path")
  # Since it is specified, shape variable is different here
  expect_equal(dataMappingShape$groupMapping$color$group, "population")
  expect_equal(dataMappingShape$groupMapping$fill$group, "population")
  expect_equal(dataMappingShape$groupMapping$shape$group, "quantile")
})

test_that("Regular tornado plots with their options work properly", {
  # Direct Tornado Plot
  defaultPlot <- plotTornado(x = tornadoMeanData$sensitivity, y = tornadoMeanData$path)
  # Options/Features
  unsortedPlot <- plotTornado(x = tornadoMeanData$sensitivity, y = tornadoMeanData$path, sorted = FALSE)
  colorPalettePlot <- plotTornado(x = tornadoMeanData$sensitivity, y = tornadoMeanData$path, colorPalette = "Dark2")
  pointPlot <- plotTornado(x = tornadoMeanData$sensitivity, y = tornadoMeanData$path, bar = FALSE)

  # Higher level with dataMapping used in population sensitivity plots
  # bar is an input option that can be directly provided to plotConfig input
  popSensitivityPlot <- plotTornado(
    data = tornadoPopData,
    bar = FALSE,
    dataMapping = TornadoDataMapping$new(
      x = "sensitivity",
      y = "path",
      color = "quantile",
      shape = "population"
    )
  )

  expect_s3_class(defaultPlot, "ggplot")
  expect_s3_class(unsortedPlot, "ggplot")
  expect_s3_class(colorPalettePlot, "ggplot")
  expect_s3_class(pointPlot, "ggplot")
  expect_s3_class(popSensitivityPlot, "ggplot")

  expect_s3_class(defaultPlot$plotConfiguration, "TornadoPlotConfiguration")
  expect_s3_class(unsortedPlot$plotConfiguration, "TornadoPlotConfiguration")
  expect_s3_class(colorPalettePlot$plotConfiguration, "TornadoPlotConfiguration")
  expect_s3_class(pointPlot$plotConfiguration, "TornadoPlotConfiguration")
  expect_s3_class(popSensitivityPlot$plotConfiguration, "TornadoPlotConfiguration")

  expect_equal(colorPalettePlot$plotConfiguration$colorPalette, "Dark2")
  expect_false(pointPlot$plotConfiguration$bar)
  expect_false(popSensitivityPlot$plotConfiguration$bar)
})
