# Test data ----------------------------------------------------

dfVertical <- data.frame(
  x = 1:3,
  y = c(1, 2, 3),
  ymin = c(0.5, 1.5, 2.5),
  ymax = c(1.5, 2.5, 3.5)
)

dfHorizontal <- data.frame(
  x = c(1, 2, 3),
  y = c(1, 2, 3),
  xmin = c(0.5, 1.5, 2.5),
  xmax = c(1.5, 2.5, 3.5)
)

dfZeroRange <- data.frame(
  x = c(1, 2),
  y = c(1, 2),
  ymin = c(0.5, 2),
  ymax = c(1.5, 2)
)

dfMultiGroup <- data.frame(
  x = rep(1:3, 2),
  y = c(1, 2, 3, 1.5, 2.5, 3.5),
  ymin = c(0.5, 1.5, 2.5, 1.0, 2.0, 3.0),
  ymax = c(1.5, 2.5, 3.5, 2.0, 3.0, 4.0),
  group = rep(c("A", "B"), each = 3)
)

test_that("vertical error bars render correctly", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfVertical,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 1.5)

  vdiffr::expect_doppelganger("vertical-errorbars-default-width", p)
})

test_that("horizontal error bars render correctly", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfHorizontal,
    ggplot2::aes(y = y, xmin = xmin, xmax = xmax)
  ) +
    geom_errorbar_osp(width = 1.5, orientation = "x")

  vdiffr::expect_doppelganger("horizontal-errorbars-default-width", p)
})

test_that("wide caps render correctly with width = 4", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfVertical,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 4)

  vdiffr::expect_doppelganger("vertical-errorbars-wide-caps", p)
})

test_that("custom linewidth and colour render correctly", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfVertical,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 2, linewidth = 1.2, colour = "steelblue")

  vdiffr::expect_doppelganger("vertical-errorbars-custom-style", p)
})

test_that("multiple groups with colour mapping render correctly", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfMultiGroup,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax, colour = group)
  ) +
    geom_errorbar_osp(
      width = 2,
      position = ggplot2::position_dodge(width = 0.3)
    )

  vdiffr::expect_doppelganger("vertical-errorbars-multi-group", p)
})

test_that("zero-range row is suppressed and only valid bar renders", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfZeroRange,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 2, na.rm = TRUE)

  vdiffr::expect_doppelganger("vertical-errorbars-zero-range-suppressed", p)
})

test_that("log-scale vertical error bars render correctly", {
  skip_if_not_installed("vdiffr")

  dfLog <- data.frame(
    x = 1:3,
    y = c(10, 100, 1000),
    ymin = c(5, 50, 500),
    ymax = c(20, 200, 2000)
  )

  p <- ggplot2::ggplot(
    dfLog,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 2) +
    ggplot2::scale_y_log10()

  vdiffr::expect_doppelganger("vertical-errorbars-log-scale", p)
})


# orientation argument -------------------------------------------------

test_that("default orientation (NA) produces vertical bars", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfVertical,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 2)

  vdiffr::expect_doppelganger("orientation-default-vertical", p)
})

test_that("orientation = 'y' produces vertical bars", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfVertical,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 2, orientation = "y")

  vdiffr::expect_doppelganger("orientation-y-vertical", p)
})

test_that("orientation = 'x' produces horizontal bars", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfHorizontal,
    ggplot2::aes(y = y, xmin = xmin, xmax = xmax)
  ) +
    geom_errorbar_osp(width = 2, orientation = "x")

  vdiffr::expect_doppelganger("orientation-x-horizontal", p)
})

test_that("orientation = NA defaults to vertical (same as 'y')", {
  skip_if_not_installed("vdiffr")

  p <- ggplot2::ggplot(
    dfVertical,
    ggplot2::aes(x = x, y = y, ymin = ymin, ymax = ymax)
  ) +
    geom_errorbar_osp(width = 2, orientation = NA)

  vdiffr::expect_doppelganger("orientation-na-defaults-to-vertical", p)
})

test_that("width must be a single non-negative finite number", {
  expect_snapshot(geom_errorbar_osp(width = -1), error = TRUE)
  expect_snapshot(geom_errorbar_osp(width = c(1, 2)), error = TRUE)
  expect_snapshot(geom_errorbar_osp(width = "wide"), error = TRUE)
})

test_that("orientation is passed through the layer params", {
  layerY <- geom_errorbar_osp(orientation = "y")
  layerX <- geom_errorbar_osp(orientation = "x")
  layerNA <- geom_errorbar_osp(orientation = NA)

  expect_equal(layerY$geom_params$orientation, "y")
  expect_equal(layerX$geom_params$orientation, "x")
  expect_true(is.na(layerNA$geom_params$orientation))
})

test_that("setup_data only suppresses ymin/ymax zero-ranges for horizontal orientation", {
  df <- data.frame(
    x = c(1, 2, 3),
    y = c(1, 2, 3),
    ymin = c(0.5, 2, 2.5),
    ymax = c(1.5, 2, 3.5),
    xmin = c(0.5, 2, 2.5),
    xmax = c(1.5, 2, 3.5)
  )

  # With orientation = "x", only xmin/xmax are checked
  resultH <- GeomErrorbarOsp$setup_data(df, list(orientation = "x"))
  expect_true(is.na(resultH$xmin[2]))
  expect_true(is.na(resultH$xmax[2]))
  expect_equal(resultH$ymin[2], 2) # ymin/ymax untouched for horizontal

  # With orientation = "y" (default), only ymin/ymax are checked
  resultV <- GeomErrorbarOsp$setup_data(df, list(orientation = "y"))
  expect_true(is.na(resultV$ymin[2]))
  expect_true(is.na(resultV$ymax[2]))
  expect_equal(resultV$xmin[2], 2) # xmin/xmax untouched for vertical
})
