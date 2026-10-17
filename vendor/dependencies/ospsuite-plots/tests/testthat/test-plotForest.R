# Sample data for testing, similar to the vignette
histData <- exampleDataCovariates |>
  dplyr::filter(SetID == "DataSet1") |>
  dplyr::select(c("ID", "Country", "Age", "AgeBin", "Obs", "Pred")) |>
  melt(
    id.vars = c("ID", "Country", "Age", "AgeBin"),
    value.name = "value",
    variable.name = "DataType"
  )

plotData <- histData[,
  .(Mean = mean(value), SD = sd(value)),
  by = c("Country", "AgeBin", "DataType")
]

# Unit tests for plotForest function with vdiffr
test_that("plotForest function generates correct plots", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  # Basic plot test
  plotObject <- plotForest(
    plotData = plotData,
    mapping = aes(x = Mean, error = SD, y = AgeBin, groupby = DataType),
    xLabel = "Mean",
    yFacetColumns = "Country",
    tableColumns = c("Mean", "SD"),
    tableLabels = c("Mean", "SD")
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("basic_forest_plot", plotObject)

  # Plot without table
  plotObjectNoTable <- plotForest(
    plotData = plotData,
    mapping = aes(x = Mean, error = SD, y = AgeBin, groupby = DataType),
    xLabel = "Mean",
    yFacetColumns = "Country",
    tableColumns = c("Mean", "SD"),
    tableLabels = c("Mean", "SD"),
    withTable = FALSE
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("forest_plot_no_table", plotObjectNoTable)

  # Faceting by DataType
  plotObjectFacetDataType <- plotForest(
    plotData = plotData,
    mapping = aes(x = Mean, error = SD, y = AgeBin),
    xLabel = "Mean",
    yFacetColumns = "Country",
    xFacetColumn = "DataType",
    tableColumns = c("Mean", "SD"),
    tableLabels = c("Mean", "SD"),
    withTable = FALSE
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger("facet_forest_plot", plotObjectFacetDataType)

  # Test for valid output when xFacetColumn is provided
  plotObjectWithFacet <- plotForest(
    plotData = plotData,
    mapping = aes(x = Mean, error = SD, y = AgeBin),
    xLabel = "Mean",
    yFacetColumns = "Country",
    xFacetColumn = "DataType",
    tableColumns = c("Mean", "SD"),
    tableLabels = c("Mean", "SD")
  )

  # Visual test using vdiffr
  vdiffr::expect_doppelganger(
    "facet_forest_plot_with_data_type",
    plotObjectWithFacet
  )
})

# Additional test cases
test_that("plotForest handles edge cases and invalid inputs", {
  # Test for missing required columns in plotData
  invalidData <- data.table(
    Country = c("USA", "Canada"),
    AgeBin = c("18-25", "26-35"),
    Mean = c(5, 10)
  )

  expect_error(
    plotForest(
      plotData = invalidData,
      mapping = aes(x = Mean, error = SD, y = AgeBin, groupby = DataType),
      xLabel = "Mean",
      yFacetColumns = "Country",
      tableColumns = c("Mean", "SD"),
      tableLabels = c("Mean", "SD")
    ),
    "must include"
  )

  # Test for invalid xscale argument
  expect_error(
    plotForest(
      plotData = plotData,
      mapping = aes(x = Mean, error = SD, y = AgeBin, groupby = DataType),
      xLabel = "Mean",
      yFacetColumns = "Country",
      xScale = "invalid_scale", # Invalid scale
      tableColumns = c("Mean", "SD"),
      tableLabels = c("Mean", "SD")
    )
  )

  # Test for missing yFacetColumns
  expect_no_error(
    plotForest(
      plotData = plotData,
      mapping = aes(x = Mean, error = SD, y = AgeBin, groupby = DataType),
      xLabel = "Mean",
      yFacetColumns = NULL, # NULL should be valid, but check for warnings
      tableColumns = c("Mean", "SD"),
      tableLabels = c("Mean", "SD")
    )
  )

  # Test for empty plotData
  emptyData <- data.table()
  expect_error(
    plotForest(
      plotData = emptyData,
      mapping = aes(x = Mean, error = SD, y = AgeBin, groupby = DataType),
      xLabel = "Mean",
      yFacetColumns = "Country",
      tableColumns = c("Mean", "SD"),
      tableLabels = c("Mean", "SD")
    )
  )
})
