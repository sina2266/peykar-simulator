test_that("Check that plot configuration uses default values when undefined", {
  testPlotConfiguration <- PlotConfiguration$new()
  expect_equal(testPlotConfiguration$export$format, getTLFSettings("defaultExportParameters")$Value$format)
  expect_equal(testPlotConfiguration$export$name, getTLFSettings("defaultExportParameters")$Value$name)
  expect_equal(testPlotConfiguration$export$width, getTLFSettings("defaultExportParameters")$Value$width)
  expect_equal(testPlotConfiguration$export$height, getTLFSettings("defaultExportParameters")$Value$height)
  expect_equal(testPlotConfiguration$export$units, getTLFSettings("defaultExportParameters")$Value$units)
  expect_equal(testPlotConfiguration$export$dpi, getTLFSettings("defaultExportParameters")$Value$dpi)
})


test_that("Check that plot configuration uses values when defined", {
  testPlotConfiguration <- PlotConfiguration$new(
    name = "test",
    format = "pdf",
    width = 4,
    height = 3,
    units = "in",
    dpi = 72
  )

  expect_equal(testPlotConfiguration$export$format, "pdf")
  expect_equal(testPlotConfiguration$export$name, "test")
  expect_equal(testPlotConfiguration$export$width, 4)
  expect_equal(testPlotConfiguration$export$height, 3)
  expect_equal(testPlotConfiguration$export$units, "in")
  expect_equal(testPlotConfiguration$export$dpi, 72)
})


test_that("Check that export configuration convert pixels to inches for ggplot compatibility", {
  plotConfigurationPx <- PlotConfiguration$new(units = "px")
  plotConfigurationIn <- PlotConfiguration$new(units = "in")
  plotConfigurationInRef <- PlotConfiguration$new(units = "in")

  plotConfigurationPx$export$convertPixels()
  plotConfigurationIn$export$convertPixels()

  expect_equal(plotConfigurationPx$export$units, "in")
  expect_equal(plotConfigurationIn$export$units, "in")
  expect_equal(plotConfigurationInRef$export$width, plotConfigurationIn$export$width)
  expect_equal(plotConfigurationInRef$export$height, plotConfigurationIn$export$height)
})

test_that("Update of dimensions prevents shrinking", {
  initialPlotObject <- addScatter(x = c(1, 2), y = c(1, 2), caption = "test legend caption")
  # Ensure unit is not in px for test
  initialPlotObject <- setPlotExport(initialPlotObject, units = "in", width = 4, height = 3)

  # No legend, export dimensions stay the same
  nonePlotObject <- setLegendPosition(initialPlotObject, position = LegendPositions$none)
  updatedNonePlotObject <- updateExportDimensionsForLegend(nonePlotObject)
  expect_equal(nonePlotObject$plotConfiguration$export$width, updatedNonePlotObject$plotConfiguration$export$width)
  expect_equal(nonePlotObject$plotConfiguration$export$height, updatedNonePlotObject$plotConfiguration$export$height)

  # Legend on top, export dimensions  extend height
  topPlotObject <- setLegendPosition(initialPlotObject, position = LegendPositions$outsideTop)
  updatedTopPlotObject <- updateExportDimensionsForLegend(topPlotObject)
  expect_equal(topPlotObject$plotConfiguration$export$width, updatedTopPlotObject$plotConfiguration$export$width)
  expect_lt(topPlotObject$plotConfiguration$export$height, updatedTopPlotObject$plotConfiguration$export$height)

  # Legend on side, export dimensions  extend width
  rightPlotObject <- setLegendPosition(initialPlotObject, position = LegendPositions$outsideRight)
  updatedRightPlotObject <- updateExportDimensionsForLegend(rightPlotObject)
  expect_lt(rightPlotObject$plotConfiguration$export$width, updatedRightPlotObject$plotConfiguration$export$width)
  expect_equal(rightPlotObject$plotConfiguration$export$height, updatedRightPlotObject$plotConfiguration$export$height)
})

test_that("Plots are saved with appropriate names", {
  plotObject <- initializePlot()
  exportPlot(plotObject)
  expect_true(file.exists(plotObject$plotConfiguration$export$getFileName()))
  exportPlot(plotObject, fileName = "test-fileName.png")
  expect_true(file.exists("test-fileName.png"))
  exportPlot(plotObject, name = "test-name", format = "png")
  expect_true(file.exists("test-name.png"))
  # remove created files
  unlink(plotObject$plotConfiguration$export$getFileName(), recursive = TRUE)
  unlink("test-fileName.png", recursive = TRUE)
  unlink("test-name.png", recursive = TRUE)
})

test_that("Exporting plot configuration code leads to new plot configration with same fields", {
  plotConfigurationToExport <- PlotConfiguration$new()
  eval(parse(text = exportPlotConfigurationCode(plotConfigurationToExport, name = "exportedPlotConfiguration")))

  expect_equal(class(plotConfigurationToExport), class(exportedPlotConfiguration))
  expect_equal(plotConfigurationToExport, exportedPlotConfiguration)
})


test_that("Plot exports are correctly rendered", {
  skip_on_ci() # skiped on CI because png() device cannot be initialized

  plot <- addScatter(
    x = c(1, 2, 3), y = c(1, 2, 3),
    plotConfiguration = PlotConfiguration$new(
      title = "This is a title",
      subtitle = "This is a subtitle",
      xlabel = "This is the x label",
      ylabel = "This is the y label",
    )
  )

  tempFile <- tempfile(fileext = ".png")

  exportConfiguration <- ExportConfiguration$new(
    format = "PNG",
    path = ".",
    name = "test-export-obs-vs-pred",
    dpi = 300
  )

  exportConfiguration$savePlot(plot)

  exportConfiguration <- ExportConfiguration$new(
    format = "JPG",
    path = ".",
    name = "test-export-obs-vs-pred",
    dpi = 300
  )

  expect_snapshot_file(paste0(file.path(exportConfiguration$path, exportConfiguration$name), ".png"), "test-export-obs-vs-pred.png")
})
