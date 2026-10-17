# Test utilities_enum.R enumerations

test_that("OptionKeys enum works correctly", {
  # OptionKeys should be a list with enumerated values
  expect_type(OptionKeys, "list")
  expect_true(length(OptionKeys) > 0)

  # All values should be character strings
  expect_true(all(sapply(OptionKeys, is.character)))

  # Names and values should be identical for enum structure
  expect_equal(names(OptionKeys), unlist(OptionKeys, use.names = FALSE))

  # Should include watermarkEnabled as mentioned in the code
  expect_true("watermarkEnabled" %in% names(OptionKeys))

  # All keys should be camelCase (no underscores or dots)
  expect_false(any(grepl("[_.]", names(OptionKeys))))
})

test_that("AxisScales enum works correctly", {
  # AxisScales should contain expected scale types
  expect_type(AxisScales, "list")
  expect_true("linear" %in% names(AxisScales))
  expect_true("log" %in% names(AxisScales))
  expect_true("discrete" %in% names(AxisScales))

  # Values should match names (enum structure)
  expect_equal(AxisScales$linear, "linear")
  expect_equal(AxisScales$log, "log")
  expect_equal(AxisScales$discrete, "discrete")

  # Should be character strings
  expect_true(all(sapply(AxisScales, is.character)))
})
