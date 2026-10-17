# Load data of comprehensive examples
load(testthat::test_path("pkRatioDataExample.RData"))

test_that("plotPKRatio() function works properly", {
  pkrp <- plotPKRatio(
    data = data.frame(
      x = c(1, 2, 10, 20, 100),
      y = c(1, 3, 4, 0.5, 0.2)
    )
  )

  expect_s3_class(pkrp, "ggplot")

  expect_error(plotPKRatio(), 'argument "data" is missing, with no default')
  expect_error(
    plotPKRatio(data = pkRatioData, dataMapping = list(x = "Age", y = "Ratio")),
    'argument "dataMapping" is of type <list>, but expected <PKRatioDataMapping>!'
  )
  expect_error(
    plotPKRatio(
      data = pkRatioData,
      plotConfiguration = list(title = "PK Ratio Plot")
    ),
    'argument "plotConfiguration" is of type <list>, but expected <PKRatioPlotConfiguration>!'
  )
})

test_that("PK Ratio default settings work", {
  pkRatioMapping <- PKRatioDataMapping$new()
  pkRatioConfig <- PKRatioPlotConfiguration$new()
  pkRatioConfigAsCalledInPlot <- PKRatioPlotConfiguration$new(
    data = pkRatioData[, c("Age", "Ratio")],
    metaData = pkRatioMetaData
  )

  expect_null(pkRatioMapping$x)
  expect_null(pkRatioMapping$y)
  expect_equal(
    pkRatioMapping$lines,
    list(
      pkRatio1 = 1,
      pkRatio2 = c(1.5, 1 / 1.5),
      pkRatio3 = c(2, 1 / 2)
    )
  )

  expect_s3_class(pkRatioConfig$labels$title, "Label")
  expect_s3_class(pkRatioConfig$labels$subtitle, "Label")
  expect_s3_class(pkRatioConfig$labels$xlabel, "Label")
  expect_s3_class(pkRatioConfig$labels$ylabel, "Label")
  expect_s3_class(pkRatioConfig$background$watermark, "Label")

  expect_null(pkRatioConfig$labels$title$text)
  expect_null(pkRatioConfig$labels$subtitle$text)

  expect_null(pkRatioConfigAsCalledInPlot$labels$title$text)
  expect_null(pkRatioConfigAsCalledInPlot$labels$subtitle$text)
  expect_equal(pkRatioConfigAsCalledInPlot$labels$xlabel$text, "Age [yrs]")
  expect_equal(pkRatioConfigAsCalledInPlot$labels$ylabel$text, "Ratio")
})

test_that("PK Ratio typical test works", {
  # Data mapping:
  # x = Age, y = Ratio, color = Gender, shape = c(Dose, Compound)
  pkRatioMap <- PKRatioDataMapping$new(
    x = "Age",
    y = "Ratio",
    color = "Gender",
    shape = c("Dose", "Compound")
  )

  # Use default configuration
  pkrp <- plotPKRatio(
    data = pkRatioData,
    metaData = pkRatioMetaData,
    dataMapping = pkRatioMap
  )

  vdiffr::expect_doppelganger(title = "PKRatioPlot", fig = pkrp)
})
