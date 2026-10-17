# Test CombinedPlot R6 class

test_that("CombinedPlot initialization works correctly", {
  skip_if_not_installed("ggplot2")
  skip_if_not_installed("R6")

  # Create test plot object
  testPlot <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  testTable <- ggplot2::ggplot() +
    ggplot2::geom_blank()

  # Test basic initialization
  combined <- CombinedPlot$new(plotObject = testPlot)
  expect_s3_class(combined, "CombinedPlot")
  expect_equal(combined$plotObject, testPlot)
  expect_null(combined$tableObject)
  expect_equal(combined$relWidths, c(4, 1))

  # Test initialization with table
  combinedWithTable <- CombinedPlot$new(plotObject = testPlot, tableObject = testTable)
  expect_equal(combinedWithTable$plotObject, testPlot)
  expect_equal(combinedWithTable$tableObject, testTable)

  # Test input validation
  expect_error(CombinedPlot$new(plotObject = "not_a_plot"))
  expect_error(CombinedPlot$new(plotObject = testPlot, tableObject = "not_a_dataframe"))
})

test_that("CombinedPlot active bindings work correctly", {
  skip_if_not_installed("ggplot2")

  testPlot <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  testPlot2 <- ggplot2::ggplot() +
    ggplot2::geom_point(data = data.frame(x = 1, y = 1), aes(x = x, y = y))
  testTable <- ggplot2::ggplot() +
    ggplot2::geom_blank()

  combined <- CombinedPlot$new(plotObject = testPlot)

  # Test plotObject getter and setter
  expect_equal(combined$plotObject, testPlot)
  combined$plotObject <- testPlot2
  expect_equal(combined$plotObject, testPlot2)

  # Test tableObject getter and setter
  expect_null(combined$tableObject)
  combined$tableObject <- testTable
  expect_equal(combined$tableObject, testTable)
  combined$tableObject <- NULL
  expect_null(combined$tableObject)

  # Test relWidths getter and setter
  expect_equal(combined$relWidths, c(4, 1))
  combined$relWidths <- c(3, 2)
  expect_equal(combined$relWidths, c(3, 2))

  # Test validation in setters
  expect_error(combined$plotObject <- "not_a_plot")
  expect_error(combined$tableObject <- "not_a_dataframe")
  expect_error(combined$relWidths <- c(1, 2, 3)) # wrong length
  expect_error(combined$relWidths <- c(-1, 2)) # negative value
})

test_that("CombinedPlot combined method works correctly", {
  skip_if_not_installed("ggplot2")
  skip_if_not_installed("cowplot")

  testPlot <- ggplot2::ggplot() +
    ggplot2::geom_blank()

  # Test without table returns plot directly
  combined <- CombinedPlot$new(plotObject = testPlot)
  result <- combined$combined()
  expect_equal(result, testPlot)

  # Test with table returns cowplot grid
  testTable <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  combined$tableObject <- testTable
  result <- combined$combined()
  expect_s3_class(result, "ggplot")
  # Should be different from original plot since it's now a combined plot
  expect_false(identical(result, testPlot))
})

test_that("CombinedPlot print method works", {
  skip_if_not_installed("ggplot2")

  testPlot <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  combined <- CombinedPlot$new(plotObject = testPlot)

  # Test that print method runs without errors
  expect_no_error(capture.output(combined$print()))

  # Test with table
  testTable <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  combined$tableObject <- testTable
  expect_no_error(combined$print())

  # Print method should return the combined plot invisibly
  result <- capture.output(returnedValue <- combined$print())
  expect_s3_class(returnedValue, "ggplot")
})

test_that("CombinedPlot legend adjustment works", {
  skip_if_not_installed("ggplot2")

  # Create plot with legend on right
  testData <- data.frame(x = 1:3, y = 1:3, color = factor(c("A", "B", "A")))
  testPlot <- ggplot2::ggplot(testData, ggplot2::aes(x = x, y = y, color = color)) +
    ggplot2::geom_point() +
    ggplot2::theme(legend.position = "right")

  combined <- CombinedPlot$new(plotObject = testPlot)
  testTable <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  combined$tableObject <- testTable

  # When combined with table, legend should be moved to top
  result <- combined$combined()
  expect_s3_class(result, "ggplot")

  # The original plot should have been modified to move legend to top
  # (this tests the private adjustLegendPosition method)
  expect_equal(combined$plotObject$theme$legend.position, "top")
})

test_that("CombinedPlot cloning works", {
  skip_if_not_installed("ggplot2")

  testPlot <- ggplot2::ggplot() +
    ggplot2::geom_blank()
  testTable <- ggplot2::ggplot() +
    ggplot2::geom_blank()

  original <- CombinedPlot$new(plotObject = testPlot, tableObject = testTable)
  cloned <- original$clone()

  expect_s3_class(cloned, "CombinedPlot")
  expect_equal(cloned$plotObject, original$plotObject)
  expect_equal(cloned$tableObject, original$tableObject)
  expect_equal(cloned$relWidths, original$relWidths)

  # Test deep cloning - changes to clone shouldn't affect original
  cloned$relWidths <- c(2, 3)
  expect_equal(original$relWidths, c(4, 1)) # should be unchanged
  expect_equal(cloned$relWidths, c(2, 3))
})
