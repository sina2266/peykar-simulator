# Test MappedDataTimeProfile class

test_that("MappedDataTimeProfile initialization works correctly", {
  skip_if_not_installed("ggplot2")
  skip_if_not_installed("R6")

  # Create test data
  testData <- data.frame(
    time = c(0, 1, 2, 4, 8),
    concentration = c(0, 10, 8, 6, 3),
    y2axis = c(FALSE, FALSE, FALSE, FALSE, FALSE)
  )

  testMapping <- ggplot2::aes(x = time, y = concentration)

  # Test basic initialization
  mappedData <- MappedDataTimeProfile$new(
    data = testData,
    mapping = testMapping
  )

  expect_s3_class(mappedData, "MappedDataTimeProfile")
  expect_s3_class(mappedData, "MappedData")
  expect_equal(mappedData$data, testData)
  expect_false(mappedData$requireDualAxis)
})

test_that("MappedDataTimeProfile handles secondary axis correctly", {
  skip_if_not_installed("ggplot2")

  # Create test data with y2axis mapping
  testData <- data.frame(
    time = c(0, 1, 2, 4, 8, 0, 1, 2, 4, 8),
    concentration = c(0, 10, 8, 6, 3, 0, 0.5, 0.4, 0.3, 0.15),
    y2axis = c(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE)
  )

  testMapping <- ggplot2::aes(x = time, y = concentration, y2axis = y2axis)

  mappedData <- MappedDataTimeProfile$new(
    data = testData,
    mapping = testMapping,
    scaleOfPrimaryAxis = "linear",
    scaleOfSecondaryAxis = "linear"
  )

  expect_true(mappedData$requireDualAxis)
  expect_true("y2" %in% names(mappedData$mapping))
})

test_that("MappedDataTimeProfile scaling validation works", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(
    time = c(0, 1, 2),
    concentration = c(1, 10, 8)
  )

  # Test valid scales
  expect_no_error({
    MappedDataTimeProfile$new(
      data = testData,
      mapping = ggplot2::aes(x = time, y = concentration),
      scaleOfPrimaryAxis = "linear"
    )
  })

  expect_no_error({
    MappedDataTimeProfile$new(
      data = testData,
      mapping = ggplot2::aes(x = time, y = concentration),
      scaleOfPrimaryAxis = "log"
    )
  })

  # Test invalid scales
  expect_error({
    MappedDataTimeProfile$new(
      data = testData,
      mapping = ggplot2::aes(x = time, y = concentration),
      scaleOfPrimaryAxis = "invalid_scale"
    )
  })
})

test_that("MappedDataTimeProfile scaleDataForSecondaryAxis works", {
  skip_if_not_installed("ggplot2")
  skip_if_not_installed("dplyr")

  # Create test data with secondary axis
  testData <- data.frame(
    time = c(0, 1, 2, 0, 1, 2),
    concentration = c(1, 10, 8, 0.1, 1, 0.8),
    y2axis = c(FALSE, FALSE, FALSE, TRUE, TRUE, TRUE)
  )

  mappedData <- MappedDataTimeProfile$new(
    data = testData,
    mapping = ggplot2::aes(x = time, y = concentration, y2axis = y2axis),
    scaleOfPrimaryAxis = "linear",
    scaleOfSecondaryAxis = "linear",
    ylimits = c(0, 10),
    y2limits = c(0, 1)
  )

  # Test scaling
  expect_no_error({
    mappedData$scaleDataForSecondaryAxis()
  })

  # Test that dataForPlot returns scaled data
  plotData <- mappedData$dataForPlot
  expect_s3_class(plotData, "data.frame")
  expect_equal(nrow(plotData), nrow(testData))
})

test_that("MappedDataTimeProfile handles different scale combinations", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(
    time = c(0, 1, 2, 0, 1, 2),
    concentration = c(1, 10, 8, 0.1, 1, 0.8),
    y2axis = c(FALSE, FALSE, FALSE, TRUE, TRUE, TRUE)
  )

  # Test linear-linear combination
  mappedDataLinLin <- MappedDataTimeProfile$new(
    data = testData,
    mapping = ggplot2::aes(x = time, y = concentration, y2axis = y2axis),
    scaleOfPrimaryAxis = "linear",
    scaleOfSecondaryAxis = "linear",
    ylimits = c(0, 10),
    y2limits = c(0, 1)
  )

  expect_no_error(mappedDataLinLin$scaleDataForSecondaryAxis())

  # Test log-linear combination (skip if data has values <= 0)
  testDataPositive <- data.frame(
    time = c(1, 2, 3, 1, 2, 3),
    concentration = c(1, 10, 8, 0.1, 1, 0.8),
    y2axis = c(FALSE, FALSE, FALSE, TRUE, TRUE, TRUE)
  )

  mappedDataLogLin <- MappedDataTimeProfile$new(
    data = testDataPositive,
    mapping = ggplot2::aes(x = time, y = concentration, y2axis = y2axis),
    scaleOfPrimaryAxis = "log",
    scaleOfSecondaryAxis = "linear",
    ylimits = c(0.1, 10),
    y2limits = c(0.1, 1)
  )

  expect_no_error(mappedDataLogLin$scaleDataForSecondaryAxis())
})

test_that("MappedDataTimeProfile active fields work correctly", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(
    time = c(0, 1, 2),
    concentration = c(1, 10, 8),
    group = c("A", "A", "B")
  )

  mappedData <- MappedDataTimeProfile$new(
    data = testData,
    mapping = ggplot2::aes(x = time, y = concentration, color = group),
    groupAesthetics = c("color")
  )

  # Test listOfGroups
  groups <- mappedData$listOfGroups
  expect_true(is.character(groups) || is.null(groups))

  # Test secAxis (should be waiver when no secondary axis)
  secAxis <- mappedData$secAxis
  expect_s3_class(secAxis, "waiver")

  # Test dataForPlot
  plotData <- mappedData$dataForPlot
  expect_s3_class(plotData, "data.frame")
  expect_equal(nrow(plotData), nrow(testData))
})

test_that("MappedDataTimeProfile handles limits correctly", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(
    time = c(0, 1, 2, 0, 1, 2),
    concentration = c(1, 10, 8, 0.1, 1, 0.8),
    y2axis = c(FALSE, FALSE, FALSE, TRUE, TRUE, TRUE)
  )

  # Test with explicit limits
  mappedData <- MappedDataTimeProfile$new(
    data = testData,
    mapping = ggplot2::aes(x = time, y = concentration, y2axis = y2axis),
    ylimits = c(0, 12),
    y2limits = c(0, 1.5)
  )

  expect_equal(mappedData$ylimits, c(0, 12))
  expect_equal(mappedData$y2limits, c(0, 1.5))

  # Test with NULL limits (should be calculated automatically)
  mappedDataAuto <- MappedDataTimeProfile$new(
    data = testData,
    mapping = ggplot2::aes(x = time, y = concentration, y2axis = y2axis)
  )

  expect_type(mappedDataAuto$ylimits, "double")
  expect_type(mappedDataAuto$y2limits, "double")
  expect_length(mappedDataAuto$ylimits, 2)
  expect_length(mappedDataAuto$y2limits, 2)
})

test_that("MappedDataTimeProfile input validation works", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(time = c(0, 1, 2), concentration = c(1, 10, 8), y2 = c(0, 1, 0))

  # Test y2limits validation
  expect_error({
    mappedData <- MappedDataTimeProfile$new(
      data = testData,
      mapping = ggplot2::aes(x = time, y = concentration, y2axis = as.logical(y2)),
      y2limits = c(10, 5) # Should be sorted
    )
  })

  expect_error({
    MappedDataTimeProfile$new(
      data = testData,
      mapping = ggplot2::aes(x = time, y = concentration, y2axis = as.logical(y2)),
      y2limits = c(1, 1) # Should be unique
    )
  })
})
