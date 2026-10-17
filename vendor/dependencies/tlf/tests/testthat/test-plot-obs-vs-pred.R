set.seed(42)

x <- sort(abs(rnorm(20, 2.5, 1)))
obsVsPredData <- data.frame(
  x = x,
  y = sort(abs(rnorm(20, 2.5, 1))),
  group = c(rep("A", 10), rep("B", 10)),
  lloq = 1,
  xmin = x - abs(rnorm(20, 0.1, 0.03)),
  xmax = x + abs(rnorm(20, 0.1, 0.05))
)


test_that("plotObservedVsSimulated works ", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")
  vdiffr::expect_doppelganger(
    title = "basic",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
      plotConfiguration = ObsVsPredPlotConfiguration$new(
        xScale = Scaling$log, xAxisLimits = c(0.05, 50),
        yScale = Scaling$log, yAxisLimits = c(0.05, 50),
      )
    )
  )
})


test_that("plotObservedVsSimulated works with error bars", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")
  vdiffr::expect_doppelganger(
    title = "basic with error bars",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y", xmin = "xmin", xmax = "xmax")
    )
  )
})


test_that("plotObservedVsSimulated works with data point with and without error bars", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  set.seed(42)
  x <- sort(abs(rnorm(20, 2.5, 1)))

  obsVsPredData <- data.frame(
    x = x,
    y = sort(abs(rnorm(20, 2.5, 1))),
    group = c(rep("A", 10), rep("B", 10)),
    lloq = 1,
    xmin = c(x[1], x[2:20] - abs(rnorm(19, 0.1, 0.03))),
    xmax = c(x[1], x[2:20] + abs(rnorm(19, 0.1, 0.05)))
  )
  vdiffr::expect_doppelganger(
    title = "basic with and without error bars",
    fig =
      plotObsVsPred(
        data = obsVsPredData,
        dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y", xmin = "xmin", xmax = "xmax"),
      )
  )
})



test_that("foldDistance are plotted correctly", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")
  plotConfiguration <- ObsVsPredPlotConfiguration$new(
    foldLinesLegend = TRUE,
    xScale = Scaling$log, xAxisLimits = c(0.05, 50),
    yScale = Scaling$log, yAxisLimits = c(0.05, 50)
  )

  vdiffr::expect_doppelganger(
    title = "fold distance simple abline",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
      plotConfiguration = plotConfiguration,
      foldDistance = 1
    )
  )

  vdiffr::expect_doppelganger(
    title = "fold distance one fold",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
      plotConfiguration = plotConfiguration,
      foldDistance = 2
    )
  )

  vdiffr::expect_doppelganger(
    title = "fold distance several folds",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
      plotConfiguration = plotConfiguration,
      foldDistance = c(1, 2, 3)
    )
  )

  vdiffr::expect_doppelganger(
    title = "fold distance many folds",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
      plotConfiguration = plotConfiguration,
      foldDistance = c(1, 5, 10, 15, 20, 25)
    )
  )


  vdiffr::expect_doppelganger(
    title = "fold distance many folds diagonal",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(x = "x", y = "y"),
      plotConfiguration = ObsVsPredPlotConfiguration$new(
        foldLinesLegend = TRUE,
        foldLinesLegendDiagonal = TRUE,
        legendTitle = "diagonal lines",
        xScale = Scaling$log, xAxisLimits = c(0.05, 50),
        yScale = Scaling$log, yAxisLimits = c(0.05, 50),
      ),
      foldDistance = c(1, 5, 10, 15, 20, 25)
    )
  )
})


test_that("plotObservedVsSimulated with LLOQ works ", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  vdiffr::expect_doppelganger(
    title = "lloq default",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(
        x = "x", y = "y",
        lloq = "lloq"
      )
    )
  )

  vdiffr::expect_doppelganger(
    title = "lloq horizontal",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(
        x = "x", y = "y",
        lloq = "lloq"
      ),
      plotConfiguration = ObsVsPredPlotConfiguration$new(lloqDirection = "horizontal")
    )
  )


  vdiffr::expect_doppelganger(
    title = "lloq vertical",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(
        x = "x", y = "y",
        lloq = "lloq"
      ),
      plotConfiguration = ObsVsPredPlotConfiguration$new(lloqDirection = "vertical")
    )
  )

  vdiffr::expect_doppelganger(
    title = "lloq both",
    fig = plotObsVsPred(
      data = obsVsPredData,
      dataMapping = ObsVsPredDataMapping$new(
        x = "x", y = "y",
        lloq = "lloq"
      ),
      plotConfiguration = ObsVsPredPlotConfiguration$new(lloqDirection = "both")
    )
  )

  set.seed(42)

  obsVsPredDataGroup <- data.frame(
    x = sort(abs(rnorm(20, 2.5, 1))),
    y = sort(abs(rnorm(20, 2.5, 1))),
    group = c(rep("A", 10), rep("B", 10)),
    lloq = c(rep(1, 10), rep(1.2, 10))
  )

  vdiffr::expect_doppelganger(
    title = "lloq with groups",
    fig = plotObsVsPred(
      data = obsVsPredDataGroup,
      dataMapping = ObsVsPredDataMapping$new(
        x = "x", y = "y",
        lloq = "lloq",
        group = "group"
      ),
      plotConfiguration = ObsVsPredPlotConfiguration$new(lloqDirection = "both")
    )
  )
})

test_that("Long group names are correctly displayed", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")
  set.seed(42)

  obsVsPredData <- data.frame(
    x = sort(abs(rnorm(20, 2.5, 1))),
    y = sort(abs(rnorm(20, 2.5, 1))),
    group = c(
      rep("A: ThisIsAVeryLongPath|thisIsAVeryLongPath|thisIsAVeryLongPath|thisIsAVeryLongPath|thisIsAVeryLongPath|thisIsAVeryLongPath", 10),
      rep("B: ThisIsAnotherVeryLongPath|thisIsAnotherVeryLongPath|thisIsAnotherVeryLongPath|thisIsAnotherVeryLongPath|thisIsAnotherVeryLongPath|thisIsAnotherVeryLongPath", 10)
    )
  )

  vdiffr::expect_doppelganger("Very long group names",
    fig =
      plotObsVsPred(
        data = obsVsPredData,
        dataMapping = ObsVsPredDataMapping$new(
          x = "x", y = "y",
          group = "group"
        )
      )
  )
})
