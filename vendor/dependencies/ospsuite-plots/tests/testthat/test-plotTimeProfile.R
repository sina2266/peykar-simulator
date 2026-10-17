test_that("plotTimeProfile works basic", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "maxValues", "minValues", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "maxValues", "minValues", "caption"))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      ymin = minValues,
      ymax = maxValues,
      groupby = caption
    ),
  ) +
    theme(
      legend.position = "top"
    )

  vdiffr::expect_doppelganger(
    title = "basic",
    fig = fig,
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      ymin = minValues,
      ymax = maxValues,
      groupby = caption
    ),
    yScale = "log",
    yScaleArgs = list(limits = c(3, 16)),
    xScaleArgs = list(limits = c(2, 18), expand = expansion())
  ) +
    theme(
      legend.position = "top"
    )

  vdiffr::expect_doppelganger(
    title = "with Limits",
    fig = fig,
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      ymin = minValues,
      ymax = maxValues,
      groupby = caption
    ),
    yScale = "log",
    yScaleArgs = list(limits = c(3, 16), oob = scales::oob_censor),
    xScaleArgs = list(limits = c(2, 18), expand = expansion())
  ) +
    theme(
      legend.position = "top"
    )

  vdiffr::expect_doppelganger(
    title = "with Limits censor",
    fig = fig,
  )
})


test_that("plotTimeProfile works logscale", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "maxValues", "minValues", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "maxValues", "minValues", "caption"))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  fig <- plotTimeProfile(
    data = simData[simData$time > 0, ],
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      ymin = minValues,
      ymax = maxValues,
      groupby = caption
    ),
    yScale = AxisScales$log
  ) +
    theme(
      legend.position = "top",
      legend.title = element_blank()
    )

  vdiffr::expect_doppelganger(
    title = "basic_log",
    fig
  )
})

test_that("plotTimeProfile works mapping observed plot", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "minValues", "maxValues", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c(
      "time",
      "values",
      "sd",
      "maxValues",
      "minValues",
      "caption"
    ))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  # map with reverse order to make sure
  mapSimulatedAndObserved <- data.frame(
    simulated = rev(unique(simData$caption)),
    observed = unique(obsData$caption)
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping <- aes(
      x = time,
      y = values,
      groupby = caption
    ),
    mapSimulatedAndObserved = mapSimulatedAndObserved
  ) +
    theme(
      legend.position = "top"
    )

  vdiffr::expect_doppelganger(
    title = "mapped-observed-and-simulated",
    fig
  )
})

test_that("plotTimeProfile works lloq", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID == c("DataSet3")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::filter(dimension == "concentration") |>
    dplyr::select(c("time", "values", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID == "DataSet3") |>
    dplyr::filter(Type == "observed") |>
    dplyr::filter(dimension == "concentration") |>
    dplyr::select(c("time", "values", "caption", "lloq", "error_relative"))

  obsData$lloq[3] <- NA
  obsData$lloq[4] <- obsData$lloq[4] / 2

  metaData <- attr(exampleDataTimeProfile, "metaData")

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      error_relative = error_relative,
      groupby = caption,
      lloq = lloq
    ),
    yScale = AxisScales$log,
    yScaleArgs = list(limits = c(0.01, NA)),
    geomLineAttributes = list(color = "black")
  ) +
    theme(
      legend.position = "top",
      legend.title = element_blank()
    )

  vdiffr::expect_doppelganger(
    title = "with lloq",
    fig
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      linetype = "Simulated"
    ),
    observedMapping = aes(
      x = time,
      y = values,
      error_relative = error_relative,
      groupby = caption,
      lloq = lloq
    ),
    groupAesthetics = c("color", "shape", "fill"),
    yScale = AxisScales$log,
    yScaleArgs = list(limits = c(0.01, NA)),
    geomLineAttributes = list(color = "black")
  ) +
    theme(
      legend.position = "top",
      legend.title = element_blank()
    )

  vdiffr::expect_doppelganger(
    title = "with lloq and dashed lines",
    fig
  )
})

test_that("plotTimeProfile works secondary axis", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  metaData <- list(
    time = list(
      dimension = "Time",
      unit = "h"
    ),
    values = list(
      dimension = "Concentration",
      unit = "mg/l"
    ),
    y2 = list(
      dimension = "Fraction",
      unit = ""
    )
  )

  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID == "DataSet3") |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "dimension", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID == "DataSet3") |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c(
      "time",
      "values",
      "dimension",
      "caption",
      "lloq",
      "error_relative"
    ))

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    mapping = aes(
      x = time,
      y = values,
      error_relative = error_relative,
      lloq = lloq,
      shape = caption,
      y2axis = (dimension == "fraction"),
      groupby = dimension
    ),
    metaData = metaData,
    yScale = AxisScales$log,
    yScaleArgs = list(limits = c(0.01, NA)),
    y2Scale = AxisScales$linear,
    y2ScaleArgs = list(limits = c(0, 1.1)),
    groupAesthetics = c("color", "fill")
  ) +
    theme(
      axis.title.y.right = element_text(angle = 90),
      legend.position = "top"
    ) +
    guides(shape = guide_legend(order = 2))

  vdiffr::expect_doppelganger(
    title = "secAxis linLog",
    fig = fig
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData |>
      dplyr::filter(dimension != "fraction"),
    mapping = aes(
      x = time,
      y = values,
      error_relative = error_relative,
      lloq = lloq,
      shape = caption,
      y2axis = (dimension == "fraction"),
      groupby = dimension
    ),
    metaData = metaData,
    yScale = AxisScales$log,
    yScaleArgs = list(limits = c(0.01, NA)),
    y2Scale = AxisScales$linear,
    y2ScaleArgs = list(limits = c(0, 1.1)),
    groupAesthetics = c("color", "fill")
  ) +
    theme(
      axis.title.y.right = element_text(angle = 90),
      legend.position = "top"
    ) +
    guides(shape = guide_legend(order = 2))

  vdiffr::expect_doppelganger(
    title = "secAxis linLog only simulation",
    fig = fig
  )

  metaData <- list(
    time = list(
      dimension = "Time",
      unit = "h"
    ),
    y2 = list(
      dimension = "Concentration",
      unit = "mg/l"
    ),
    values = list(
      dimension = "Fraction",
      unit = ""
    )
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    mapping = mapping <- aes(
      x = time,
      y = values,
      shape = caption,
      error_relative = error_relative,
      lloq = lloq,
      y2axis = (dimension != "fraction"),
      color = dimension,
      linetype = dimension
    ),
    metaData = metaData,
    y2Scale = AxisScales$log,
    y2ScaleArgs = list(limits = c(0.01, NA)),
    yScale = AxisScales$linear,
    yScaleArgs = list(limits = c(0, 1)),
  ) +
    theme(
      axis.title.y.right = element_text(angle = 90),
      legend.position = "top",
      legend.title = element_blank()
    )

  vdiffr::expect_doppelganger(
    title = "secAxis logLin",
    fig = fig
  )
})


test_that("plotTimeProfile works with formula as aesthic", {
  obsData <- data.table(
    x = rep(seq(1, 10), 2),
    y = c(rnorm(10, mean = 1), rnorm(10, mean = 2)),
    dose = rep(c(1, 2), each = 10)
  )

  mapping <- aes(
    x = x,
    y = y / dose,
    color = as.factor(dose)
  )

  expect_no_error(plotTimeProfile(
    data = obsData,
    observedData = obsData,
    mapping = aes(
      x = x,
      y = y / dose,
      y2axis = dose == 1,
      color = as.factor(dose)
    ),
    yScale = AxisScales$log
  ))
})

test_that("plotTimeProfile works with different observedMapping", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "minValues", "maxValues", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "minValues", "maxValues", "caption"))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  # Test with different observedMapping (different aesthetics for observed data)
  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      ymin = minValues,
      ymax = maxValues,
      groupby = caption
    ),
    observedMapping = aes(
      x = time,
      y = values,
      ymin = minValues,
      ymax = maxValues,
      groupby = caption,
      shape = caption
    )
  ) +
    theme(
      legend.position = "top"
    )

  vdiffr::expect_doppelganger(
    title = "different-observedMapping",
    fig = fig
  )
})

test_that("plotTimeProfile suppresses duplicate shape and fill legends with mixed obs+sim data", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "caption"))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      groupby = caption
    )
  )

  # shape and fill guides should be suppressed (auto-expanded from groupby)
  expect_null(ggplot2::get_guide_data(fig, "shape"))
  expect_null(ggplot2::get_guide_data(fig, "fill"))
  # colour guide must remain (suppression must not touch it)
  expect_false(is.null(ggplot2::get_guide_data(fig, "colour")))
})

test_that("plotTimeProfile preserves user-explicit fill mapping in legend", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::mutate(fillVar = caption) |>
    dplyr::select(c("time", "values", "caption", "fillVar"))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      groupby = caption
    ),
    observedMapping = aes(
      x = time,
      y = values,
      groupby = caption,
      fill = fillVar
    )
  )

  # shape should be suppressed (same as colour from groupby)
  expect_null(ggplot2::get_guide_data(fig, "shape"))
  # fill should NOT be suppressed (user-explicit, maps to different variable)
  expect_false(is.null(ggplot2::get_guide_data(fig, "fill")))
})

test_that("plotTimeProfile preserves user-explicit shape mapping in legend", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::mutate(shapeVar = caption) |>
    dplyr::select(c("time", "values", "caption", "shapeVar"))

  metaData <- attr(exampleDataTimeProfile, "metaData")

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    metaData = metaData,
    mapping = aes(
      x = time,
      y = values,
      groupby = caption
    ),
    observedMapping = aes(
      x = time,
      y = values,
      groupby = caption,
      shape = shapeVar
    )
  )

  # fill should be suppressed (same as colour from groupby)
  expect_null(ggplot2::get_guide_data(fig, "fill"))
  # shape should NOT be suppressed (user-explicit, maps to different variable)
  expect_false(is.null(ggplot2::get_guide_data(fig, "shape")))
})

test_that("plotTimeProfile does not suppress guides for sim-only data", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  fig <- plotTimeProfile(
    data = simData,
    mapping = aes(x = time, y = values, groupby = caption)
  )

  # obsMappedData NULL: no suppression, colour guide active
  expect_false(is.null(ggplot2::get_guide_data(fig, "colour")))
})

test_that("plotTimeProfile does not suppress guides for obs-only data", {
  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "caption"))

  fig <- plotTimeProfile(
    observedData = obsData,
    mapping = aes(x = time, y = values, groupby = caption)
  )

  # simMappedData NULL: no suppression, all point guides active
  expect_false(is.null(ggplot2::get_guide_data(fig, "colour")))
  expect_false(is.null(ggplot2::get_guide_data(fig, "fill")))
  expect_false(is.null(ggplot2::get_guide_data(fig, "shape")))
})

test_that("plotTimeProfile does not suppress guides when mapSimulatedAndObserved is set", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "caption"))

  mapSimulatedAndObserved <- data.frame(
    simulated = unique(simData$caption),
    observed = unique(obsData$caption)
  )

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    mapping = aes(x = time, y = values, groupby = caption),
    mapSimulatedAndObserved = mapSimulatedAndObserved
  )

  # mapSimulatedAndObserved set: no suppression, point guides active
  expect_false(is.null(ggplot2::get_guide_data(fig, "shape")))
  expect_false(is.null(ggplot2::get_guide_data(fig, "fill")))
})

test_that("plotTimeProfile does not suppress guides when colour is not in groupAesthetics", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  obsData <- exampleDataTimeProfile |>
    dplyr::filter(SetID %in% c("DataSet1", "DataSet2")) |>
    dplyr::filter(Type == "observed") |>
    dplyr::select(c("time", "values", "caption"))

  fig <- plotTimeProfile(
    data = simData,
    observedData = obsData,
    mapping = aes(x = time, y = values, groupby = caption),
    groupAesthetics = c("fill", "shape")
  )

  # colour not in groupAesthetics: no suppression, point guides active
  expect_false(is.null(ggplot2::get_guide_data(fig, "fill")))
  expect_false(is.null(ggplot2::get_guide_data(fig, "shape")))
})

test_that("'lin' is accepted as shorthand for 'linear' in plotTimeProfile", {
  simData <- exampleDataTimeProfile |>
    dplyr::filter(SetID == "DataSet1") |>
    dplyr::filter(Type == "simulated") |>
    dplyr::select(c("time", "values", "caption"))

  expect_no_error(
    plotTimeProfile(
      data = simData,
      mapping = aes(x = time, y = values, groupby = caption),
      xScale = "lin",
      yScale = "lin"
    )
  )
})
