# Create a simple ggplot object for testing
testPlot <- ggplot(mtcars, aes(x = wt, y = mpg)) +
  geom_point()

# Create a facet plot wit aspect ratio for testing
facetPlot <- ggplot(mtcars, aes(x = wt, y = mpg)) +
  geom_point() +
  facet_wrap(~cyl) +
  theme(aspect.ratio = 1)

# Create a plot with a  legend
legendPlot <- ggplot(mtcars, aes(x = wt, y = mpg, color = factor(cyl))) +
  geom_point() +
  labs(color = "Number of Cylinders")


testthat::test_that("validateFilename replaces forbidden characters with a warning and sets device", {
  expect_warning(
    validateFilename("concentration in µg/L", device = NULL),
    regexp = "Exported filename 'concentration in µg/L.png' included forbidden characters and was replaced by 'concentration in ug_L.png'",
    fixed = TRUE
  )
  expect_warning(
    validateFilename("project:details", device = "pdf"),
    regexp = "Exported filename 'project:details.pdf' included forbidden characters and was replaced by 'project_details.pdf'",
    fixed = TRUE
  )
  expect_warning(
    validateFilename("invalid*filename?", device = NULL),
    regexp = "Exported filename 'invalid*filename?.png' included forbidden characters and was replaced by 'invalid_filename_.png'",
    fixed = TRUE
  )
  expect_warning(
    validateFilename("<test>|doc.pdf", device = NULL),
    regexp = "Exported filename '<test>|doc.png' included forbidden characters and was replaced by '_test__doc.png'",
    fixed = TRUE
  )
})


# Test for valid export with default width and height
test_that("exportPlot saves a plot correctly", {
  tempDir <- tempdir()
  filename <- "testPlot.png"

  expect_silent(exportPlot(testPlot, filepath = tempDir, filename = filename))

  # Check if the file was created
  expect_true(file.exists(file.path(tempDir, filename)))

  img <- png::readPNG(file.path(tempDir, filename))
  width <- dim(img)[2]
  height <- dim(img)[1]

  expect_equal(object = height, expected = width, tolerance = 0.05)

  # Clean up
  file.remove(file.path(tempDir, filename))
})

# Test for custom width and height
test_that("exportPlot accepts custom width and height", {
  tempDir <- tempdir()
  filename <- "testPlotCustom.png"

  expect_silent(exportPlot(
    testPlot,
    filepath = tempDir,
    filename = filename,
    width = 10,
    height = 5
  ))

  # Check if the file was created
  expect_true(file.exists(file.path(tempDir, filename)))

  img <- png::readPNG(file.path(tempDir, filename))
  width <- dim(img)[2]
  height <- dim(img)[1]

  expect_equal(object = width / height, expected = 2, tolerance = 0.05)

  # Clean up
  file.remove(file.path(tempDir, filename))
})

# Test for facet plot
test_that("exportPlot saves a facet plot correctly", {
  tempDir <- tempdir()
  filename <- "facetPlot.png"

  expect_silent(exportPlot(facetPlot, filepath = tempDir, filename = filename))

  # Check if the file was created
  expect_true(file.exists(file.path(tempDir, filename)))

  img <- png::readPNG(file.path(tempDir, filename))
  width <- dim(img)[2]
  height <- dim(img)[1]

  # expected is a plot with 3 square panel in a row
  # width/height: (offset yaxis + 3 panels)/(offset xaxis + 1 panel) estimated as 2.5 with large tolerance
  expect_equal(object = width / height, expected = 2.5, tolerance = 0.2)

  # Clean up
  file.remove(file.path(tempDir, filename))
})

# Test for legend offset
test_that("exportPlot handles legend offsets", {
  tempDir <- tempdir()
  filename <- "legendPlot.png"

  expect_silent(exportPlot(
    legendPlot + theme(legend.position = "none"),
    filepath = tempDir,
    filename = filename
  ))

  # dimensions for plot without legend
  img <- png::readPNG(file.path(tempDir, filename))
  widthNoLegend <- dim(img)[2]
  heightNoLegend <- dim(img)[1]

  filename <- "legendPlotTop.png"

  expect_silent(exportPlot(
    legendPlot + theme(legend.position = "top", legend.direction = "vertical"),
    filepath = tempDir,
    filename = filename
  ))

  # dimensions for plot with legend on top
  img <- png::readPNG(file.path(tempDir, filename))
  widthTopLegend <- dim(img)[2]
  heightTopLegend <- dim(img)[1]

  # width stays the same
  expect_equal(widthNoLegend, expected = widthTopLegend)
  # High is adjusted
  expect_lt(heightNoLegend, expected = heightTopLegend)

  filename <- "legendPlotRight.png"

  expect_silent(exportPlot(
    legendPlot +
      theme(legend.position = "right", legend.direction = "horizontal"),
    filepath = tempDir,
    filename = filename
  ))

  # dimensions for plot with legend on side
  img <- png::readPNG(file.path(tempDir, filename))
  widthRightLegend <- dim(img)[2]
  heightRightLegend <- dim(img)[1]

  # width stays the same
  expect_equal(widthNoLegend, expected = widthRightLegend)
  # High is adjusted
  expect_gt(heightNoLegend, expected = heightRightLegend)

  # Clean up
  pngFiles <- list.files(tempDir, pattern = "legendPlot")
  file.remove(file.path(tempDir, pngFiles))
})

# Test for legend offset
test_that("exportPlot handles legend offsets", {
  tempDir <- tempdir()
  filename <- "legendPlot.png"

  expect_silent(exportPlot(
    legendPlot +
      theme(legend.position = "none"),
    filepath = tempDir,
    filename = filename
  ))

  # dimensions for plot without legend
  img <- png::readPNG(file.path(tempDir, filename))
  widthNoLegend <- dim(img)[2]
  heightNoLegend <- dim(img)[1]

  # build a plot with a label longer the the width
  filename <- "legendPlotLong.png"
  plotObject <- legendPlot +
    theme(legend.position = "top") +
    labs(color = paste(rep("Very Long Label", 6), collapse = ", "))
  expect_silent(exportPlot(
    plotObject = plotObject,
    filepath = tempDir,
    filename = filename
  ))

  # dimensions for plot without legend
  img <- png::readPNG(file.path(tempDir, filename))
  widthLongLegend <- dim(img)[2]
  heightLongLegend <- dim(img)[1]

  # width was enlarged to accomodate long label
  expect_lt(widthNoLegend, widthLongLegend)

  # Clean up
  pngFiles <- list.files(tempDir, pattern = "legendPlot")
  file.remove(file.path(tempDir, pngFiles))
})

# Test for filename containing path
test_that("exportPlot warns if filename contains path and saves with cleaned name", {
  tempDir <- tempdir()
  cleanedFilename <- "invalid_path_testPlot.png"

  expect_warning(
    exportPlot(
      testPlot,
      filepath = tempDir,
      filename = "invalid/path/testPlot.png"
    ),
    regexp = "included forbidden characters"
  )

  # Check the file was created with the cleaned filename
  expect_true(file.exists(file.path(tempDir, cleanedFilename)))

  # Clean up
  file.remove(file.path(tempDir, cleanedFilename))
})

# Test for invalid plot object
test_that("exportPlot fails with invalid plot object", {
  expect_error(exportPlot(
    NULL,
    filepath = tempdir(),
    filename = "testPlot.png"
  ))
})

# Test for missing file path
test_that("exportPlot fails with missing file path", {
  expect_error(
    exportPlot(testPlot, filepath = NULL, filename = "testPlot.png"),
  )
})

# Test for missing filename
test_that("exportPlot fails with missing filename", {
  expect_error(exportPlot(testPlot, filepath = tempdir(), filename = NULL))
})

test_that("exportPlot with invalid filename warns and saves with cleaned name", {
  tempDir <- tempdir()
  filename <- "my:<Invalid>Filename-withµ.png"

  expect_warning(
    exportPlot(testPlot, filepath = tempDir, filename = filename),
    regexp = "included forbidden characters"
  )

  # Check if the file was created
  expect_true(file.exists(file.path(tempDir, "my__Invalid_Filename-withu.png")))

  # Clean up
  file.remove(file.path(tempDir, "my__Invalid_Filename-withu.png"))
})

# Test for change of options
test_that("exportPlot uses export options correctly", {
  tempDir <- tempdir()
  filename <- "testPlot.png"

  # export
  expect_silent(exportPlot(
    testPlot,
    filepath = tempDir,
    filename = filename,
    width = 16
  ))

  img <- png::readPNG(file.path(tempDir, filename))
  width <- dim(img)[2]
  height <- dim(img)[1]

  setOspsuite.plots.option(optionKey = OptionKeys$exportUnits, value = "mm")

  expect_silent(exportPlot(
    testPlot,
    filepath = tempDir,
    filename = filename,
    width = 160
  ))

  img <- png::readPNG(file.path(tempDir, filename))
  widthmm <- dim(img)[2]
  heightmm <- dim(img)[1]

  expect_equal(widthmm, expected = width)
  expect_equal(heightmm, expected = height)

  setOspsuite.plots.option(
    optionKey = OptionKeys$exportUnits,
    value = getDefaultOptions()[["ospsuite.plots.exportUnits"]]
  )

  # test dpi
  setOspsuite.plots.option(optionKey = OptionKeys$exportDpi, value = 150)

  expect_silent(exportPlot(testPlot, filepath = tempDir, filename = filename))

  img <- png::readPNG(file.path(tempDir, filename))
  widthdpi <- dim(img)[2]

  expect_equal(width / widthdpi, expected = 2, tolerance = 0.01)
  setOspsuite.plots.option(
    optionKey = OptionKeys$exportDpi,
    value = getDefaultOptions()[["ospsuite.plots.exportDpi"]]
  )

  # Check if the file was created with correct device
  setOspsuite.plots.option(optionKey = OptionKeys$exportDevice, value = "pdf")
  expect_silent(exportPlot(testPlot, filepath = tempDir, filename = filename))

  expect_true(file.exists(file.path(
    tempDir,
    fs::path_ext_set(filename, "pdf")
  )))

  setOspsuite.plots.option(
    optionKey = OptionKeys$exportDevice,
    value = getDefaultOptions()[["ospsuite.plots.exportDevice"]]
  )

  # Clean up
  exportedFiles <- list.files(tempDir, pattern = fs::path_ext_remove(filename))
  invisible(file.remove(file.path(tempDir, exportedFiles)))
})
