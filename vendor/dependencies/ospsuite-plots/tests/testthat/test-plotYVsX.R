test_that("plot Residuals vs Covariate works", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Age", "Obs", "gsd", "Pred", "Sex")) |>
    dplyr::mutate(logResiduals = log(Pred) - log(Obs))

  attr(
    x = data$logResiduals,
    which = "label"
  ) <- "residuals\nlog(predicted) - log(observed)"

  fig <- plotResVsCov(
    data = data,
    mapping = aes(
      x = Age,
      y = logResiduals,
      groupby = Sex
    ),
    addRegression = TRUE
  )

  vdiffr::expect_doppelganger(
    title = "plotResVsCov",
    fig = fig
  )
})

test_that("plot Observed vs Predicted works", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  lloqData <- signif(quantile(data$Obs, probs = 0.1), 1)

  data <- data |>
    dplyr::mutate(lloq = lloqData) |>
    dplyr::mutate(Obs = ifelse(Obs <= lloq, lloq / 2, Obs))

  metaData <- attr(exampleDataCovariates, "metaData")
  metaData <- metaData[intersect(names(data), names(metaData))]

  fig <- plotPredVsObs(
    data = data,
    mapping = aes(
      x = Obs,
      y = Pred,
      lloq = lloq,
      groupby = Sex,
      error_relative = gsd,
      ymin = Pred / 1.1,
      ymax = Pred * 1.1
    ),
    metaData = metaData,
    addRegression = TRUE,
    # now width is independent from scale
    geomErrorbarAttributes = list(width = 2)
  )

  vdiffr::expect_doppelganger(
    title = "res",
    fig = fig
  )
})

test_that("plotRatioVsCov works", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  dDIdata <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet3") |>
    dplyr::select(c("ID", "Obs", "Pred")) |>
    dplyr::mutate(Ratio = Obs / Pred)

  attr(
    x = dDIdata$Ratio,
    which = "label"
  ) <- "residuals\nobserved / predicted"

  dDImetaData <- list(
    Obs = list(
      dimension = "DDI AUC Ratio",
      unit = ""
    )
  )

  dDIdata$ID <- factor(dDIdata$ID, levels = unique(dDIdata$ID))

  fig <- plotRatioVsCov(
    data = dDIdata,
    mapping = aes(
      x = Obs,
      y = Ratio,
      groupby = ID
    ),
    metaData = dDImetaData,
    addGuestLimits = TRUE,
    deltaGuest = 1.2
  )

  vdiffr::expect_doppelganger(
    title = "plotRatioVsCov",
    fig = fig
  )
})

test_that("getCountsWithin works for Ratio", {
  pkRatioData <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet1") |>
    dplyr::select(-c("SetID"))

  metaData <- attr(exampleDataCovariates, "metaData")
  metaData <- metaData[intersect(names(pkRatioData), names(metaData))]

  plotObject <- plotRatioVsCov(
    data = pkRatioData,
    mapping = aes(
      x = Age,
      y = Ratio,
      groupby = Sex
    ),
    metaData = metaData,
  )

  vdiffr::expect_doppelganger(
    title = "pk Ratio plot",
    fig = plotObject
  )

  expect_equal(
    as.vector(unlist(plotObject$countsWithin[1, c(4, 6)])),
    expected = c(0.48, 0.64)
  )

  expect_equal(
    plotObject$countsWithin$`1.5 fold Fraction`,
    expected = c(0.48, 0.44, 0.52)
  )
})

test_that("getCountsWithin works for Guest Criteria", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  # Load example
  dDIdata <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet3") |>
    dplyr::select(c("ID", "Obs", "Pred")) |>
    dplyr::mutate(Type = ifelse(ID <= 5, "A", "B")) |>
    dplyr::mutate(Ratio = Obs / Pred)

  attr(
    x = dDIdata$Ratio,
    which = "label"
  ) <- "residuals\nobserved / predicted"

  dDImetaData <- list(
    Obs = list(
      dimension = "DDI AUC Ratio",
      unit = ""
    )
  )

  plotObject <- plotRatioVsCov(
    data = dDIdata,
    mapping = aes(
      x = Obs,
      y = Ratio,
      groupby = Type
    ),
    addGuestLimits = TRUE,
    comparisonLineVector = getFoldDistanceList(2),
    deltaGuest = 1
  )

  expect_equal(
    plotObject$countsWithin$`guest criteria Fraction`,
    expected = c(0.6, 0.6, 0.6)
  )

  plotObjectDiag <- plotPredVsObs(
    data = dDIdata,
    metaData = dDImetaData,
    mapping = aes(
      predicted = Pred,
      observed = Obs,
      groupby = Type
    ),
    addGuestLimits = TRUE,
    comparisonLineVector = getFoldDistanceList(2)
  )

  expect_equal(plotObject$countsWithin, plotObjectDiag$countsWithin)

  plotObjectUngrouped <- plotPredVsObs(
    data = dDIdata,
    metaData = dDImetaData,
    mapping = aes(
      predicted = Pred,
      observed = Obs
    ),
    addGuestLimits = TRUE,
    comparisonLineVector = getFoldDistanceList(2)
  )

  expect_equal(plotObjectUngrouped$countsWithin$Fraction, c(1, 0.6, 1))
})
test_that("adjust lines works withot error", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Age", "Obs", "gsd", "Pred", "Sex")) |>
    dplyr::mutate(logResiduals = log(Pred) - log(Obs))

  attr(
    x = data$logResiduals,
    which = "label"
  ) <- "residuals\nlog(predicted) - log(observed)"

  # case with lines which are no interval
  expect_no_error(plotResVsCov(
    data = data,
    mapping = aes(
      x = Age,
      y = logResiduals,
      groupby = Sex
    ),
    comparisonLineVector = list(
      zero = 0,
      "lower limit" = -0.25,
      "upper limit" = 0.25
    )
  ))

  # case with unnamed intervals
  expect_no_error(plotPredVsObs(
    data = data,
    mapping = aes(
      x = Obs,
      y = Pred,
      groupby = Sex
    ),
    comparisonLineVector = unname(getFoldDistanceList(c(1.2, 1.5))),
    geomComparisonLineAttributes = list(linetype = "dotted")
  ))
})

test_that("plotYVsX with LLOQ works for observedDataDirection = y", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  lloqData <- signif(quantile(data$Obs, probs = 0.1), 1)

  data <- data |>
    dplyr::mutate(lloq = lloqData) |>
    dplyr::mutate(Obs = ifelse(Obs <= lloq, lloq / 2, Obs))

  expect_no_error(
    fig <- plotYVsX(
      data = data,
      mapping = aes(
        x = Pred,
        y = Obs,
        lloq = lloq,
        groupby = Sex
      ),
      observedDataDirection = "y"
    )
  )

  vdiffr::expect_doppelganger(
    title = "plotLLOQforYdirection",
    fig = fig
  )
})

test_that("plotYVsX with LLOQ works for observedDataDirection = x", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  lloqData <- signif(quantile(data$Obs, probs = 0.1), 1)

  data <- data |>
    dplyr::mutate(lloq = lloqData) |>
    dplyr::mutate(Obs = ifelse(Obs <= lloq, lloq / 2, Obs))

  expect_no_error(
    fig <- plotYVsX(
      data = data,
      mapping = aes(
        x = Obs,
        y = Pred,
        lloq = lloq,
        groupby = Sex
      ),
      observedDataDirection = "x"
    )
  )

  vdiffr::expect_doppelganger(
    title = "plotLLOQforXdirection",
    fig = fig
  )
})

test_that("plotYVsX with LLOQ works for lloqOnBothAxes = TRUE", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  lloqData <- signif(quantile(data$Obs, probs = 0.1), 1)

  data <- data |>
    dplyr::mutate(lloq = lloqData) |>
    dplyr::mutate(Obs = ifelse(Obs <= lloq, lloq / 2, Obs))

  expect_no_error(
    fig <- plotYVsX(
      data = data,
      mapping = aes(
        x = Obs,
        y = Pred,
        lloq = lloq,
        groupby = Sex
      ),
      observedDataDirection = "x",
      lloqOnBothAxes = TRUE
    )
  )

  vdiffr::expect_doppelganger(
    title = "plotLLOQforBothDirections",
    fig = fig
  )
})


test_that("residualScale deprecation warning is issued", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Age", "Obs", "Pred", "Sex")) |>
    dplyr::mutate(logResiduals = log(Pred) - log(Obs))

  expect_warning(
    plotResVsCov(
      data = data,
      mapping = aes(x = Age, y = logResiduals, groupby = Sex),
      residualScale = "log"
    ),
    regexp = "residualScale.*deprecated"
  )
})

test_that("'lin' is accepted as shorthand for 'linear' in plotYVsX", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Age", "Obs", "Pred", "Sex")) |>
    dplyr::mutate(logResiduals = log(Pred) - log(Obs))

  expect_no_error(
    plotYVsX(
      data = data,
      mapping = aes(x = Age, y = logResiduals, groupby = Sex),
      xScale = "lin",
      yScale = "lin"
    )
  )
})

test_that("'lin' is accepted as shorthand for 'linear' in plotPredVsObs", {
  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "Pred", "Sex"))

  expect_no_error(
    plotPredVsObs(
      data = data,
      mapping = aes(x = Obs, y = Pred),
      xyScale = "lin"
    )
  )
})

# Regression for the swapped-orientation bug in the error bar layers.
#
# `geom_errorbar_osp()` reads `xmin/xmax` when `orientation == "x"` and
# `ymin/ymax` otherwise. `plotYVsX()` builds error from the observed-data
# direction, so an x-direction error populates only `xmin/xmax` and a
# y-direction error populates only `ymin/ymax`.
#
# Before the fix the branches passed the opposite orientation: the geom looked
# for the range columns of the *other* axis, found none, and silently drew
# nothing (no error bars, no warning). These snapshots show the bars present on
# the correct axis; under the buggy code they would be empty.
test_that("x-direction error bars render on the x-axis", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  fig <- plotPredVsObs(
    data = data,
    mapping = aes(x = Obs, y = Pred, groupby = Sex, error_relative = gsd),
    geomErrorbarAttributes = list(width = 2)
  )

  vdiffr::expect_doppelganger("xdir-errorbars", fig)
})

test_that("y-direction error bars render on the y-axis", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  fig <- plotYVsX(
    data = data,
    mapping = aes(x = Obs, y = Pred, groupby = Sex, error_relative = gsd),
    observedDataDirection = "y",
    geomErrorbarAttributes = list(width = 2)
  )

  vdiffr::expect_doppelganger("ydir-errorbars", fig)
})
