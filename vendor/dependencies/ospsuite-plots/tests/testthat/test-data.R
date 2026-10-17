# Test data.R - example datasets

test_that("exampleDataTimeProfile exists and has correct structure", {
  # Test that the dataset exists
  expect_true(exists("exampleDataTimeProfile"))

  # Test basic structure
  expect_s3_class(exampleDataTimeProfile, "data.frame")
  expect_true(nrow(exampleDataTimeProfile) > 0)
  expect_true(ncol(exampleDataTimeProfile) > 0)

  # Test for common time profile columns (if they exist)
  columnNames <- names(exampleDataTimeProfile)
  expect_type(columnNames, "character")
  expect_true(length(columnNames) > 0)
})

test_that("exampleDataCovariates exists and has correct structure", {
  # Test that the dataset exists
  expect_true(exists("exampleDataCovariates"))

  # Test basic structure
  expect_s3_class(exampleDataCovariates, "data.frame")
  expect_true(nrow(exampleDataCovariates) > 0)
  expect_true(ncol(exampleDataCovariates) > 0)

  # Test for metadata attribute (commonly used in the package)
  if (!is.null(attr(exampleDataCovariates, "metaData"))) {
    metadata <- attr(exampleDataCovariates, "metaData")
    expect_type(metadata, "list")
  }

  # Test column names exist
  columnNames <- names(exampleDataCovariates)
  expect_type(columnNames, "character")
  expect_true(length(columnNames) > 0)
})

test_that("Example datasets can be used for basic operations", {
  # Test that datasets can be used in basic data operations without errors

  # Test exampleDataTimeProfile
  expect_no_error({
    summary(exampleDataTimeProfile)
  })

  expect_no_error({
    head(exampleDataTimeProfile)
  })

  # Test exampleDataCovariates
  expect_no_error({
    summary(exampleDataCovariates)
  })

  expect_no_error({
    head(exampleDataCovariates)
  })
})

test_that("Example datasets have appropriate data types", {
  # Test that datasets don't have completely invalid data types

  # For time profile data, we expect at least some numeric columns
  timeProfileTypes <- sapply(exampleDataTimeProfile, class)
  expect_true(any(sapply(timeProfileTypes, function(x) any(x %in% c("numeric", "integer", "double")))))

  # For covariate data, we expect mixed data types
  covariateTypes <- sapply(exampleDataCovariates, class)
  expect_true(length(covariateTypes) > 0)

  # Should not have any completely invalid column types
  expect_false(any(sapply(covariateTypes, function(x) any(x %in% c("NULL", "logical") & length(x) == 1))))
})
