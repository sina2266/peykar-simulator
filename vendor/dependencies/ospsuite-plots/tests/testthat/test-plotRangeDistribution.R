# Sample data for testing
n <- 1000
exampleData <- data.table(
  IndividualId = 1:n,
  Age = seq(2, 18, length.out = n),
  Group = rep(c("A", "B"), n / 2)
)
exampleData[, value := rep(qnorm(seq(0.1, 0.9, length.out = 20)), round(n / 20)) + ifelse(Group == "A", Age, 10)]

metaData <- list(Age = list(
  dimension = "Age",
  unit = "year(s)"
))

# Unit tests for BINNINGMODE
test_that("BINNINGMODE enumeration works correctly", {
  expect_equal(BINNINGMODE$number, "Equal Frequency Binning")
  expect_equal(BINNINGMODE$interval, "Equal Width Binning")
  expect_equal(BINNINGMODE$breaks, "Custom Binning")
})

# Unit tests for plotRangeDistribution function
test_that("plotRangeDistribution generates correct plots", {
  # Basic plot test
  plotObject <- plotRangeDistribution(
    data = exampleData,
    mapping = aes(x = Age, y = value, groupby = Group),
    modeOfBinning = BINNINGMODE$number,
    numberOfBins = 20,
    statFun = NULL,
    percentiles = c(0.05, 0.5, 0.95)
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("basic_range_plot", plotObject)

  # Test with custom breaks
  customBreaks <- c(2, 6, 12, 18)
  plotObject <- plotRangeDistribution(
    data = exampleData,
    mapping = aes(x = Age, y = value, groupby = Group),
    modeOfBinning = BINNINGMODE$breaks,
    breaks = customBreaks,
    statFun = NULL,
    percentiles = c(0.05, 0.5, 0.95)
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("custom_binning_range_plot", plotObject)

  # Test with equal width binning
  plotObject <- plotRangeDistribution(
    data = exampleData,
    mapping = aes(x = Age, y = value, groupby = Group),
    modeOfBinning = BINNINGMODE$interval,
    numberOfBins = 9,
    asStepPlot = TRUE,
    statFun = NULL,
    percentiles = c(0.05, 0.5, 0.95)
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("equal_width_range_plot", plotObject)


  # Test with a custom aggregation function
  customStatFun <- function(y) {
    return(c(ymin = mean(y) - sd(y), y = mean(y), ymax = mean(y) + sd(y)))
  }

  plotObject <- plotRangeDistribution(
    data = exampleData,
    mapping = aes(x = Age, y = value, groupby = Group),
    modeOfBinning = BINNINGMODE$number,
    numberOfBins = 20,
    statFun = customStatFun,
    percentiles = c(0.05, 0.5, 0.95)
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("custom_aggregation_range_plot", plotObject)

  # Test for step plot
  plotObject <- plotRangeDistribution(
    data = exampleData,
    mapping = aes(x = Age, y = value, groupby = Group),
    modeOfBinning = BINNINGMODE$number,
    numberOfBins = 4,
    asStepPlot = TRUE,
    statFun = NULL,
    percentiles = c(0.05, 0.5, 0.95)
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("step_plot_range_plot", plotObject)
})

# Edge case tests
test_that("plotRangeDistribution handles edge cases", {
  # Test for missing data
  emptyData <- data.table(IndividualId = integer(0), Value = numeric(0), Group = character(0))
  expect_error(
    plotRangeDistribution(
      data = emptyData,
      mapping = aes(x = Group, y = Value),
      modeOfBinning = BINNINGMODE$number,
      numberOfBins = 20
    ),
    "Assertion on 'data' failed: Must have at least 1 rows, but has 0 rows."
  )

  # Test for invalid modeOfBinning
  expect_error(
    plotRangeDistribution(
      data = exampleData,
      mapping = aes(x = Age, y = value, groupby = Group),
      modeOfBinning = "Invalid Binning Mode",
      numberOfBins = 20
    )
  )

  # Test for invalid percentiles
  expect_error(
    plotRangeDistribution(
      data = exampleData,
      mapping = aes(x = Age, y = value, groupby = Group),
      modeOfBinning = BINNINGMODE$number,
      numberOfBins = 20,
      percentiles = c(0.1, 0.5, 1.1) # Invalid percentile
    ),
    "Assertion on 'percentiles' failed: Element 3 is not <= 1."
  )

  # Test for invalid statFun
  expect_error(
    plotRangeDistribution(
      data = exampleData,
      mapping = aes(x = Group, y = Value),
      modeOfBinning = BINNINGMODE$number,
      numberOfBins = 20,
      statFun = "not_a_function" # Invalid statFun
    )
  )
})
