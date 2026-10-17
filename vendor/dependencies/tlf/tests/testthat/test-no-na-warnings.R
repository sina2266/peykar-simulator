# Test data that include a NA value
testData <- data.frame(
  x = c(1, 2, 3),
  y = c(1, 2, NA)
)

# To prevent copy/paste, expressions are used in the sequel
plotNames <- c("TimeProfile", "ObsVsPred", "ResVsPred", "PKRatio", "DDIRatio")

# Expression is "<plotname>DataMapping <- <PlotName>DataMapping$new()"
createDataMapping <- parse(
  text = paste0(
    tolower(plotNames),
    "DataMapping <- ",
    plotNames,
    "DataMapping$new(x='x',y='y')"
  )
)
eval(createDataMapping)

# createPlot expression is "plot<PlotName>(data=testData, dataMapping=<plotname>DataMapping)"
createPlot <- paste0(
  "plot",
  plotNames,
  "(data=testData, dataMapping = ",
  tolower(plotNames),
  "DataMapping)"
)
ggplotExpression <- parse(
  text = paste0("expect_s3_class(", createPlot, ', "ggplot")')
)
warningExpression <- parse(text = paste0("expect_silent(", createPlot, ")"))


test_that("Plot is produced as a ggplot object", {
  expect_s3_class(addScatter(data = testData), "ggplot")
  expect_s3_class(addLine(data = testData), "ggplot")
  eval(ggplotExpression)
})

test_that("No warnings were produced when creating/printing the plot", {
  expect_silent(addScatter(data = testData))
  expect_silent(addLine(data = testData))
  eval(warningExpression)
})
