test_that("plot histogram works for all stacked  frequency distribution fit combinations", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  myData <- rbind(
    data.table(
      Ratio = qnorm(p = seq(0.01, 0.99, length.out = 1000)),
      Sex = "female"
    ),
    data.table(
      Ratio = qnorm(p = seq(0.01, 0.99, length.out = 1000), mean = 1),
      Sex = "male"
    )
  )

  vdiffr::expect_doppelganger(
    title = "noStack_noFrequency",
    fig = plotHistogram(
      data = myData,
      mapping = aes(
        x = Ratio,
        fill = Sex
      ),
      geomHistAttributes = utils::modifyList(
        getDefaultGeomAttributes("Hist"),
        list(
          position = "dodge",
          bins = 30
        )
      ),
      distribution = "normal",
      plotAsFrequency = FALSE
    )
  )

  vdiffr::expect_doppelganger(
    title = "stack_noFrequency",
    fig = plotHistogram(
      data = myData,
      mapping = aes(
        x = Ratio,
        fill = Sex
      ),
      geomHistAttributes = utils::modifyList(
        getDefaultGeomAttributes("Hist"),
        list(
          position = "stack",
          bins = 30
        )
      ),
      distribution = "normal",
      meanFunction = "median",
      plotAsFrequency = FALSE
    )
  )

  vdiffr::expect_doppelganger(
    title = "noStack_frequency",
    fig = plotHistogram(
      data = myData,
      mapping = aes(
        x = Ratio,
        fill = as.factor(Sex)
      ),
      geomHistAttributes = utils::modifyList(
        getDefaultGeomAttributes("Hist"),
        list(
          position = "dodge",
          bins = 30
        )
      ),
      distribution = "normal",
      plotAsFrequency = TRUE
    )
  )

  vdiffr::expect_doppelganger(
    title = "stack_frequency",
    fig = plotHistogram(
      data = myData,
      mapping = aes(
        x = Ratio,
        fill = Sex
      ),
      geomHistAttributes = utils::modifyList(
        getDefaultGeomAttributes("Hist"),
        list(
          position = "stack",
          bins = 30
        )
      ),
      distribution = "normal",
      meanFunction = "median",
      plotAsFrequency = TRUE
    )
  )
})

test_that("plot histogram works for absolute distribution fit on logscale", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  histDataDistr <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "AgeBin", "Sex", "Obs"))

  metaData <- attr(exampleDataCovariates, "metaData")
  metaDataDistr <- metaData[intersect(names(histDataDistr), names(metaData))]

  vdiffr::expect_doppelganger(
    title = "logScaleDistribution",
    fig = plotHistogram(
      data = histDataDistr,
      mapping = aes(
        x = Obs,
        fill = Sex,
      ),
      metaData = metaDataDistr,
      xScale = AxisScales$log,
      distribution = "norm",
      meanFunction = "median"
    )
  )
})


test_that("plot histogram works for categoricalData", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  histData <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet1") |>
    dplyr::select(c("ID", "Sex", "Age", "AgeBin", "Ratio"))

  metaData <- attr(exampleDataCovariates, "metaData")
  metaData <- metaData[intersect(names(histData), names(metaData))]

  vdiffr::expect_doppelganger(
    title = "barPlot",
    fig = plotHistogram(
      data = histData,
      mapping = mapping <- aes(
        x = AgeBin,
        fill = Sex
      ),
      metaData = metaData
    )
  )
})
