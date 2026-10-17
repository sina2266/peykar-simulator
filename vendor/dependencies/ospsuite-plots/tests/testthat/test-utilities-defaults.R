# Test utilities-defaults.R functions

test_that("getDefaultGeomAttributes works correctly", {
  # Test with valid geom types
  lineAttrs <- getDefaultGeomAttributes("Line")
  expect_type(lineAttrs, "list")

  pointAttrs <- getDefaultGeomAttributes("Point")
  expect_type(pointAttrs, "list")

  ribbonAttrs <- getDefaultGeomAttributes("Ribbon")
  expect_type(ribbonAttrs, "list")
  expect_equal(ribbonAttrs$color, NA)

  # Test error for invalid geom type
  expect_error(getDefaultGeomAttributes("InvalidGeom"))
})

test_that("getOspsuite.plots.option works correctly", {
  # watermarkEnabled defaults to TRUE — no manual set needed
  watermarkEnabled <- getOspsuite.plots.option("watermarkEnabled")
  expect_type(watermarkEnabled, "logical")

  # Test getting watermark label
  watermarkLabel <- getOspsuite.plots.option("watermarkLabel")
  expect_type(watermarkLabel, "character")
  expect_equal(watermarkLabel, "preliminary analysis")

  # Test error for invalid option key
  expect_error(getOspsuite.plots.option("invalid_option"))
})

test_that("setOspsuite.plots.option works correctly", {
  # Test setting a valid option
  setOspsuite.plots.option("watermarkEnabled", TRUE)
  originalValue <- getOspsuite.plots.option("watermarkEnabled")
  setOspsuite.plots.option("watermarkEnabled", FALSE)
  newValue <- getOspsuite.plots.option("watermarkEnabled")
  expect_false(newValue)

  # Reset to original value
  setOspsuite.plots.option("watermarkEnabled", originalValue)

  # Test setting NULL value clears the option and go back to default as TRUE
  setOspsuite.plots.option("watermarkEnabled", NULL)
  clearedValue <- getOspsuite.plots.option("watermarkEnabled")
  expect_true(clearedValue)

  # Reset for other tests
  setOspsuite.plots.option("watermarkEnabled", TRUE)

  # Test error for invalid option key
  expect_error(setOspsuite.plots.option("invalid_option", TRUE))
})

test_that("getDefaultOptions returns complete options list", {
  optionsList <- getDefaultOptions()
  expect_type(optionsList, "list")
  expect_true(length(optionsList) > 0)

  # Test presence of all key options (verifies the renaming)
  expectedOptions <- c(
    "ospsuite.plots.watermarkEnabled",
    "ospsuite.plots.watermarkLabel",
    "ospsuite.plots.watermarkFormat",
    "ospsuite.plots.geomLineAttributes",
    "ospsuite.plots.geomPointAttributes",
    "ospsuite.plots.alpha",
    "ospsuite.plots.lloqAlphaVector",
    "ospsuite.plots.lloqLineType",
    "ospsuite.plots.percentiles",
    "ospsuite.plots.defaultPercentiles",
    "ospsuite.plots.exportWidth",
    "ospsuite.plots.exportUnits",
    "ospsuite.plots.exportDevice",
    "ospsuite.plots.exportDpi"
  )
  expect_true(all(expectedOptions %in% names(optionsList)))

  # Test specific default values for the renamed keys
  expect_true(optionsList$ospsuite.plots.watermarkEnabled)
  expect_equal(
    optionsList$ospsuite.plots.watermarkLabel,
    "preliminary analysis"
  )
  expect_equal(optionsList$ospsuite.plots.alpha, 0.5)
  expect_equal(optionsList$ospsuite.plots.lloqLineType, "dashed")
  expect_equal(
    optionsList$ospsuite.plots.percentiles,
    c(0.05, 0.25, 0.5, 0.75, 0.95)
  )
  expect_equal(
    optionsList$ospsuite.plots.defaultPercentiles,
    c(0.05, 0.5, 0.95)
  )
  expect_equal(optionsList$ospsuite.plots.exportWidth, 16)
  expect_equal(optionsList$ospsuite.plots.exportUnits, "cm")
  expect_equal(optionsList$ospsuite.plots.exportDevice, "png")
  expect_equal(optionsList$ospsuite.plots.exportDpi, 300)
  expect_equal(
    optionsList$ospsuite.plots.lloqAlphaVector,
    c("TRUE" = 0.3, "FALSE" = 1)
  )
})

test_that("setDefaultColorMapDistinct works correctly", {
  skip_if_not_installed("grDevices")

  # Test with default colors (NULL)
  oldColors <- setDefaultColorMapDistinct()
  expect_type(oldColors, "list")
  expect_true("ggplot2.discrete.colour" %in% names(oldColors))
  expect_true("ggplot2.discrete.fill" %in% names(oldColors))

  # Test with custom colors
  customColors <- list(c("#FF0000", "#00FF00", "#0000FF"))
  oldColors2 <- setDefaultColorMapDistinct(customColors)
  expect_type(oldColors2, "list")

  # Test with character vector (should be converted to list)
  colorVector <- c("red", "green", "blue")
  oldColors3 <- setDefaultColorMapDistinct(colorVector)
  expect_type(oldColors3, "list")

  # Test error for invalid colors
  expect_error(setDefaultColorMapDistinct(list(c("invalidcolor123"))))

  # Reset to original
  resetDefaultColorMapDistinct(oldColors)
})

test_that("resetDefaultColorMapDistinct works correctly", {
  # Save current colors
  originalColors <- setDefaultColorMapDistinct()

  # Set new colors
  newColors <- list(c("red", "blue"))
  setDefaultColorMapDistinct(newColors)

  # Reset to original
  resetDefaultColorMapDistinct(originalColors)

  # Verify reset worked (this would need access to actual ggplot2 options)
  expect_no_error(resetDefaultColorMapDistinct(originalColors))

  # Test error for invalid input
  expect_error(resetDefaultColorMapDistinct("not_a_list"))
})

test_that("setDefaults does not break raw ggplot2::geom_point() with mapped shape (#118)", {
  # setDefaults() is soft-deprecated; it still works for raw ggplot2 plots.
  lifecycle::expect_deprecated(oldDefaults <- setDefaults())
  withr::defer(suppressWarnings(resetDefaults(oldDefaults)))

  df <- data.frame(x = 1:5, y = 1:5, g = letters[1:5])
  p <- ggplot2::ggplot(
    df,
    ggplot2::aes(x = x, y = y, color = g, shape = g)
  ) +
    ggplot2::geom_point()

  expect_no_error(ggplot2::ggplotGrob(p))
})

test_that("theme_osp returns a ggplot2 theme object", {
  theme <- theme_osp()
  expect_s3_class(theme, "theme")
  expect_s3_class(theme, "gg")
})

test_that("theme_osp has the expected OSP element values", {
  theme <- theme_osp()
  expect_equal(theme$legend.position, "right")
  expect_equal(theme$legend.direction, "vertical")
  expect_equal(theme$legend.justification, 0.5)
  expect_s3_class(theme$panel.grid.minor, "element_blank")
})

test_that("theme_osp is a complete theme (ggplot2 recommendation)", {
  expect_true(attr(theme_osp(), "complete"))
})

test_that("theme_osp centres titles while keeping theme_bw spacing", {
  # %+replace% drops unspecified element properties, so the constructor must
  # restate size/margin to avoid regressing title/subtitle spacing.
  theme <- theme_osp()
  expect_equal(theme$plot.title$hjust, 0.5)
  expect_equal(theme$plot.title$size, ggplot2::rel(1.2))
  expect_equal(theme$plot.title$margin, theme_bw()$plot.title$margin)
  expect_equal(theme$plot.subtitle$hjust, 0.5)
  expect_equal(theme$plot.subtitle$margin, theme_bw()$plot.subtitle$margin)
})

test_that("theme_osp forwards arguments to theme_bw", {
  expect_equal(theme_osp(base_size = 20)$text$size, 20)
})

test_that("setDefaultTheme applies theme_osp globally and returns the previous theme", {
  withr::defer(ggplot2::theme_set(oldTheme))
  oldTheme <- ggplot2::theme_set(ggplot2::theme_grey())

  # setDefaultTheme() is soft-deprecated; it still mutates the global theme.
  lifecycle::expect_deprecated(previousTheme <- setDefaultTheme())

  # the returned theme is the one that was active before the call
  expect_equal(
    previousTheme$panel.background,
    ggplot2::theme_grey()$panel.background
  )
  # the global theme now matches theme_osp
  expect_equal(ggplot2::theme_get()$legend.position, "right")
})

test_that("ospsuite.plots plots carry theme_osp without setDefaults() (#130)", {
  # Force a non-OSP global theme so any dependence on theme_set() would surface.
  withr::defer(ggplot2::theme_set(oldTheme))
  oldTheme <- ggplot2::theme_set(ggplot2::theme_grey())
  withr::local_options(ospsuite.plots.watermarkEnabled = FALSE)

  mappedData <- MappedData$new(
    data = data.frame(x = 1:5, y = 1:5),
    mapping = ggplot2::aes(x = x, y = y),
    xScale = "linear",
    yScale = "linear"
  )
  plotObject <- initializePlot(mappedData)

  # The plot's own theme, not the (grey) global theme, drives the layout.
  expect_equal(plotObject$theme$legend.position, "right")
  expect_s3_class(plotObject$theme$panel.grid.minor, "element_blank")
})

test_that("constructLabelWithUnit handles edge cases", {
  # This test is for the function defined in utilities.R but may be called from defaults
  expect_equal(constructLabelWithUnit("Test", "unit"), "Test [unit]")
  expect_equal(constructLabelWithUnit("Test", ""), "Test")
  expect_null(constructLabelWithUnit(NULL, "unit"))
})
