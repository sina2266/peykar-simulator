test_that("Background properties can be defined a priori", {
  plotArea <- BackgroundElement$new(color = "blue", linetype = "twodash", fill = "yellow")
  panelArea <- LineElement$new(color = "red", linetype = "dotdash", fill = "pink")
  backgroundPlotConfiguration <- PlotConfiguration$new(plotArea = plotArea, panelArea = panelArea)

  expect_equal(backgroundPlotConfiguration$background$plot, plotArea)
  expect_equal(backgroundPlotConfiguration$background$panel, panelArea)

  xGrid <- LineElement$new(color = "blue", linetype = "twodash")
  yGrid <- LineElement$new(color = "red", linetype = "dotdash")
  gridPlotConfiguration <- PlotConfiguration$new(xGrid = xGrid, yGrid = yGrid)

  expect_equal(gridPlotConfiguration$background$xGrid, xGrid)
  expect_equal(gridPlotConfiguration$background$yGrid, yGrid)
})

# Create a reference plot configuration
refPlotConfiguration <- PlotConfiguration$new()
for (property in AestheticProperties) {
  refPlotConfiguration$lines[[property]] <- "reference"
  refPlotConfiguration$ribbons[[property]] <- "reference"
  refPlotConfiguration$points[[property]] <- "reference"
  refPlotConfiguration$errorbars[[property]] <- "reference"
}

test_that("Theme properties are not updated when PlotConfiguration is updated", {
  for (property in AestheticProperties) {
    testPlotConfiguration <- PlotConfiguration$new()
    expect_false(testPlotConfiguration$lines[[property]] %in% "reference")
    expect_false(testPlotConfiguration$ribbons[[property]] %in% "reference")
    expect_false(testPlotConfiguration$points[[property]] %in% "reference")
    expect_false(testPlotConfiguration$errorbars[[property]] %in% "reference")
  }
})
