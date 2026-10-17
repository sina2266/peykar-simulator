# Test MappedDataBoxplot class

test_that("MappedDataBoxplot initialization works correctly", {
  skip_if_not_installed("ggplot2")
  skip_if_not_installed("R6")

  # Create test data
  testData <- data.frame(
    x = c(1, 2, 3, 1, 2, 3),
    y = c(10, 15, 12, 8, 14, 11),
    group = factor(c("A", "A", "A", "B", "B", "B"))
  )

  testMapping <- ggplot2::aes(x = x, y = y)

  # Test basic initialization
  mappedData <- MappedDataBoxplot$new(
    data = testData,
    mapping = testMapping
  )

  expect_s3_class(mappedData, "MappedDataBoxplot")
  expect_s3_class(mappedData, "MappedData")
  expect_equal(mappedData$data, testData)
  expect_true(mappedData$hasXmapping)
})

test_that("MappedDataBoxplot handles x mapping detection correctly", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(y = c(10, 15, 12))

  # Test without x mapping
  mappedDataNoX <- MappedDataBoxplot$new(
    data = testData,
    mapping = ggplot2::aes(y = y)
  )
  expect_false(mappedDataNoX$hasXmapping)

  # Test with x mapping
  testDataWithX <- data.frame(x = c(1, 2, 3), y = c(10, 15, 12))
  mappedDataWithX <- MappedDataBoxplot$new(
    data = testDataWithX,
    mapping = ggplot2::aes(x = x, y = y)
  )
  expect_true(mappedDataWithX$hasXmapping)
})

test_that("MappedDataBoxplot boxwhiskerMapping active field works correctly", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(
    x = c(1, 2, 3),
    y = c(10, 15, 12)
  )

  # Test with x mapping
  mappedDataWithX <- MappedDataBoxplot$new(
    data = testData,
    mapping = ggplot2::aes(x = x, y = y)
  )

  boxwhiskerMapping <- mappedDataWithX$boxwhiskerMapping
  expect_s3_class(boxwhiskerMapping, "uneval")
  # When x is mapped, should return empty aes()
  expect_equal(length(boxwhiskerMapping), 0)

  # Test without x mapping
  testDataNoX <- data.frame(y = c(10, 15, 12))
  mappedDataNoX <- MappedDataBoxplot$new(
    data = testDataNoX,
    mapping = ggplot2::aes(y = y)
  )

  boxwhiskerMappingNoX <- mappedDataNoX$boxwhiskerMapping
  expect_s3_class(boxwhiskerMappingNoX, "uneval")
  expect_true("x" %in% names(boxwhiskerMappingNoX))
})

test_that("MappedDataBoxplot doAdjustmentsWithMetaData works correctly", {
  skip_if_not_installed("ggplot2")

  # Test data with numeric x
  testDataNumeric <- data.frame(
    x = c(1.5, 2.7, 3.1),
    y = c(10, 15, 12)
  )

  mappedDataNumeric <- MappedDataBoxplot$new(
    data = testDataNumeric,
    mapping = ggplot2::aes(x = x, y = y)
  )

  # Test with linear scale for numeric data
  expect_warning(
    {
      mappedDataNumeric$doAdjustmentsWithMetaData(
        originalmapping = ggplot2::aes(x = x, y = y),
        xScale = "linear",
        xScaleArgs = list()
      )
    },
    regexp = messages$warningNoMetaDataForXAxis(),
    fixed = TRUE
  )

  mappedDataNumeric$addMetaData(
    metaData = list(x = list(
      dimension = "time",
      unit = "h"
    ))
  )

  # Test with linear scale for numeric data
  expect_no_error({
    mappedDataNumeric$doAdjustmentsWithMetaData(
      originalmapping = ggplot2::aes(x = x, y = y),
      xScale = "linear",
      xScaleArgs = list()
    )
  })


  expect_equal(mappedDataNumeric$xScale, "linear")

  # Test data with factor x
  testDataFactor <- data.frame(
    x = factor(c("A", "B", "C")),
    y = c(10, 15, 12)
  )

  mappedDataFactor <- MappedDataBoxplot$new(
    data = testDataFactor,
    mapping = ggplot2::aes(x = x, y = y)
  )

  mappedDataFactor$addMetaData(
    metaData = list(x = list(
      dimension = "time",
      unit = "h"
    ))
  )

  # Test with discrete scale for factor data
  expect_no_error({
    mappedDataFactor$doAdjustmentsWithMetaData(
      originalmapping = ggplot2::aes(x = x, y = y),
      xScale = "auto",
      xScaleArgs = list()
    )
  })

  expect_equal(mappedDataFactor$xScale, "discrete")
})

test_that("MappedDataBoxplot scale validation works correctly", {
  skip_if_not_installed("ggplot2")

  # Test error when trying to use continuous scale with factor data
  testDataFactor <- data.frame(
    x = factor(c("A", "B", "C")),
    y = c(10, 15, 12)
  )

  mappedDataFactor <- MappedDataBoxplot$new(
    data = testDataFactor,
    mapping = ggplot2::aes(x = x, y = y)
  )
  mappedDataFactor$addMetaData(
    metaData = list(x = list(
      dimension = "time",
      unit = "h"
    ))
  )

  expect_error(
    {
      mappedDataFactor$doAdjustmentsWithMetaData(
        originalmapping = ggplot2::aes(x = x, y = y),
        xScale = "linear", # This should fail for factor data
        xScaleArgs = list()
      )
    },
    regexp = messages$errorContinuousXScaleNotPossibleForFactors(AxisScales$discrete),
    fixed = TRUE
  )

  # Test error when trying to use discrete scale with numeric data
  testDataNumeric <- data.frame(
    x = c(1, 2, 3),
    y = c(10, 15, 12)
  )

  mappedDataNumeric <- MappedDataBoxplot$new(
    data = testDataNumeric,
    mapping = ggplot2::aes(x = x, y = y)
  )
  mappedDataNumeric$addMetaData(
    metaData = list(x = list(
      dimension = "time",
      unit = "h"
    ))
  )

  expect_error(
    {
      mappedDataNumeric$doAdjustmentsWithMetaData(
        originalmapping = ggplot2::aes(x = x, y = y),
        xScale = "discrete", # This should fail for numeric data
        xScaleArgs = list()
      )
    },
    regexp = messages$errorDiscreteXScaleNotPossibleForContinuous(AxisScales$linear, AxisScales$log),
    fixed = TRUE
  )
})

test_that("MappedDataBoxplot input validation works", {
  skip_if_not_installed("ggplot2")

  testData <- data.frame(x = c(1, 2, 3), y = c(10, 15, 12))
  mappedData <- MappedDataBoxplot$new(
    data = testData,
    mapping = ggplot2::aes(x = x, y = y)
  )
  mappedData$addMetaData(
    metaData = list(x = list(
      dimension = "time",
      unit = "h"
    ))
  )


  # Test doAdjustmentsWithMetaData input validation
  expect_error({
    mapped_data$doAdjustmentsWithMetaData(
      originalmapping = "not_a_mapping", # Should be a list/mapping
      xScale = "linear",
      xScaleArgs = list()
    )
  })

  expect_error({
    mapped_data$doAdjustmentsWithMetaData(
      originalmapping = ggplot2::aes(x = x, y = y),
      xScale = c("linear", "log"), # Should be single character
      xScaleArgs = list()
    )
  })
})
