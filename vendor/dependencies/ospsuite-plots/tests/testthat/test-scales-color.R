# Tests for the per-plot OSP color/fill scales (#130)

test_that(".ospColorPalette reproduces the stacked OSP palette", {
  # up to 6 groups -> colorMaps$default
  expect_equal(ospsuite.plots:::.ospColorPalette(3), colorMaps$default[1:3])
  expect_equal(ospsuite.plots:::.ospColorPalette(6), colorMaps$default)
  # more than 6 groups -> colorMaps$ospDefault
  expect_equal(ospsuite.plots:::.ospColorPalette(7), colorMaps$ospDefault[1:7])
})

test_that("scale_colour_osp and scale_fill_osp return discrete scales", {
  expect_s3_class(scale_colour_osp(), "ScaleDiscrete")
  expect_s3_class(scale_fill_osp(), "ScaleDiscrete")
  expect_equal(scale_colour_osp()$aesthetics, "colour")
  expect_equal(scale_fill_osp()$aesthetics, "fill")
})

test_that("scale_color_osp is an alias of scale_colour_osp", {
  expect_identical(scale_color_osp, scale_colour_osp)
})

test_that("building an OSP plot does not mutate global ggplot2 state (#130)", {
  withr::local_options(
    ggplot2.discrete.colour = "SENTINEL",
    ggplot2.discrete.fill = "SENTINEL"
  )
  beforeGeom <- ggplot2::GeomBoxplot$default_aes

  df <- data.frame(g = rep(letters[1:3], each = 8), y = 1:24)
  plotObject <- plotBoxWhisker(df, ggplot2::aes(x = g, y = y, fill = g))
  invisible(ggplot2::ggplot_build(plotObject))

  expect_equal(getOption("ggplot2.discrete.colour"), "SENTINEL")
  expect_equal(getOption("ggplot2.discrete.fill"), "SENTINEL")
  expect_identical(beforeGeom, ggplot2::GeomBoxplot$default_aes)
})

test_that("OSP plots use the OSP palette and alpha without setDefaults() (#130)", {
  # Neutralise any global styling so only the per-plot path can supply colors.
  withr::local_options(
    ggplot2.discrete.colour = NULL,
    ggplot2.discrete.fill = NULL,
    ospsuite.plots.watermarkEnabled = FALSE
  )

  df <- data.frame(g = rep(letters[1:3], each = 8), y = 1:24)
  plotObject <- plotBoxWhisker(df, ggplot2::aes(x = g, y = y, fill = g))
  built <- ggplot2::ggplot_build(plotObject)$data[[1]]

  expect_equal(unique(built$fill), colorMaps$default[1:3])
  expect_equal(unique(built$alpha), 0.5)
})

test_that("a user-supplied color scale overrides the OSP scale without a message", {
  df <- data.frame(g = rep(letters[1:3], each = 8), y = 1:24)
  plotObject <- plotBoxWhisker(df, ggplot2::aes(x = g, y = y, fill = g)) +
    ggplot2::scale_fill_manual(values = c(a = "red", b = "green", c = "blue"))

  expect_no_message(built <- ggplot2::ggplot_build(plotObject))
  expect_equal(unique(built$data[[1]]$fill), c("red", "green", "blue"))
})

test_that("a continuous color mapping does not get the discrete OSP scale", {
  withr::local_options(ospsuite.plots.watermarkEnabled = FALSE)
  df <- data.frame(time = 1:10, y = 1:10, c = as.numeric(1:10))
  plotObject <- plotTimeProfile(df, ggplot2::aes(x = time, y = y, colour = c))

  # Would error if scale_colour_osp() (discrete) were applied to numeric data.
  expect_no_error(ggplot2::ggplot_build(plotObject))
})
