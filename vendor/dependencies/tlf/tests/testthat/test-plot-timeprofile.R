obsData <- data.frame(
  name = c("dataset1", "dataset1", "dataset2", "dataset2", "dataset2"),
  x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 1, 3),
  group = c("group A", "group A", "group A", "group B", "group B"),
  lloq = c(rep(0.5, 3), rep(1.5, 2))
)

simTime <- seq(1, 10, 0.1)

simData <- data.frame(
  x = simTime,
  y = 10 * exp(-simTime),
  ymin = 8 * exp(-simTime),
  ymax = 12 * exp(-simTime)
)




test_that("plotTimeProfile works ", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  color <- fill <- "group"
  linetype <- shape <- "name"

  vdiffr::expect_doppelganger(
    title = "basic",
    fig = plotTimeProfile(
      data = simData,
      observedData = obsData,
      dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax"),
      observedDataMapping = ObservedDataMapping$new(x = "x", y = "y", group = "group", shape = shape, color = shape)
    )
  )


  vdiffr::expect_doppelganger(
    title = "with lloq",
    fig = plotTimeProfile(
      data = simData,
      observedData = obsData,
      dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax"),
      observedDataMapping = ObservedDataMapping$new(x = "x", y = "y", group = "group", shape = shape, color = shape, lloq = "lloq")
    )
  )


  vdiffr::expect_doppelganger(
    title = "with lloq and plotConfiguration",
    fig = plotTimeProfile(
      data = simData,
      observedData = obsData,
      dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax"),
      observedDataMapping = ObservedDataMapping$new(x = "x", y = "y", group = "group", shape = shape, color = shape, lloq = "lloq"),
      plotConfiguration = TimeProfilePlotConfiguration$new(lloqDirection = "horizontal", xAxisLimits = c(0, 10), xValuesLimits = c(1, 5))
    )
  )
})


# Test data

set.seed(42)

obsData <- data.frame(
  x = rep(1:7, 2),
  y = c(10 * exp(-1:-7) + rnorm(7, 0, .25), 10 * exp(-1:-7) + rnorm(7, 0, .25)),
  error = abs(rnorm(14, 0, 0.1)),
  group = c(rep("A", 7), rep("B", 7)),
  lloq = c(rep(0.15, 7), rep(0.25, 7))
)

simTime <- seq(1, 10, 0.1)
simData <- data.frame(
  x = simTime,
  y = 10 * exp(-simTime),
  ymin = 8 * exp(-simTime),
  ymax = 12 * exp(-simTime)
)


test_that("Time profile plot without errors bars works", {
  # Produce a Time profile plot with observed and simulated data
  obsData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4), ymin = NA_real_, ymax = NA_real_)
  simTime <- seq(1, 10, 0.1)
  simData <- data.frame(
    x = simTime,
    y = 10 * exp(-simTime),
    ymin = 8 * exp(-simTime),
    ymax = 12 * exp(-simTime)
  )

  vdiffr::expect_doppelganger(
    title = "timeprofile without error bars",
    fig = plotTimeProfile(
      data = simData,
      observedData = obsData,
      dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax"),
      observedDataMapping = ObservedDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax")
    )
  )
})

test_that("Time profile plot with errors bars works", {
  # Produce a Time profile plot with observed and simulated data
  y <- c(5, 0.2, 2, 3, 4)
  obsData <- data.frame(x = c(1, 2, 3, 4, 5), y = y, ymin = y - 0.5, ymax = y + 0.3)
  simTime <- seq(1, 10, 0.1)
  simData <- data.frame(
    x = simTime,
    y = 10 * exp(-simTime),
    ymin = 8 * exp(-simTime),
    ymax = 12 * exp(-simTime)
  )

  vdiffr::expect_doppelganger(
    title = "timeprofile with error bars",
    fig = plotTimeProfile(
      data = simData,
      observedData = obsData,
      dataMapping = TimeProfileDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax"),
      observedDataMapping = ObservedDataMapping$new(x = "x", y = "y", ymin = "ymin", ymax = "ymax")
    )
  )
})

# LLOQ plots


test_that("plotTimeProfile works with LLOQ ", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")
  vdiffr::expect_doppelganger(
    title = "with observed data only",
    fig = plotTimeProfile(
      observedData = obsData,
      observedDataMapping = ObservedDataMapping$new(
        x = "x",
        y = "y",
        group = "group",
        lloq = "lloq"
      )
    )
  )

  vdiffr::expect_doppelganger(
    title = "with observed and sim data",
    fig = plotTimeProfile(
      observedData = obsData,
      observedDataMapping = ObservedDataMapping$new(
        x = "x",
        y = "y",
        group = "group",
        lloq = "lloq"
      ),
      data = simData,
      dataMapping = TimeProfileDataMapping$new(
        x = "x",
        y = "y"
      )
    )
  )
})







# Legacy tests

#
#
# test_that("plotTimeProfile works with dual axis", {
#   skip_if_not_installed("vdiffr")
#   skip_if(getRversion() < "4.1")
#
#   obsData <- data.frame(
#     name = c("dataset1", "dataset1", "dataset2", "dataset2", "dataset2"),
#     x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 1, 3),
#     y2 = c(rep(FALSE, 3), rep(TRUE, 2)),
#     group = c("group A", "group A", "group A", "group B", "group B"),
#     lloq = c(rep(0.5, 3), rep(1.5, 2))
#   )
#
#   simTime <- seq(0, 10, 0.1)
#   simData <- data.frame(
#     x = c(simTime, simTime),
#     y = c(10 * exp(-simTime), 1 - exp(-simTime)),
#     y2 = rep(c(FALSE, TRUE), each = length(simTime)),
#     group = rep(c("sim 1", "sim 2"), each = length(simTime))
#   )
#
#   color <- fill <- "group"
#   linetype <- shape <- "name"
#
#   vdiffr::expect_doppelganger("double axis timeprofile",
#                               fig = plotTimeProfile(
#                                 data = simData,
#                                 observedData = obsData,
#                                 dataMapping = TimeProfileDataMapping$new(
#                                   x = "x", y = "y", y2Axis = "y2",
#                                   group = "group"
#                                 ),
#                                 observedDataMapping = ObservedDataMapping$new(
#                                   x = "x", y = "y", y2Axis = "y2",
#                                   group = "group", shape = shape, color = shape
#                                 ),
#                                 plotConfiguration = TimeProfilePlotConfiguration$new(
#                                   xlabel = "Time",
#                                   ylabel = "Left Axis",
#                                   y2label = "Right Axis"
#                                 )
#                               )
#   )
# })

# -------------------------------------------------
# Get the data and metadata for PK Ratio plot

# nPopulation <- 20
#
# # -------------------------------------------------
#
# testData <- data.frame(
#   IndivdualID = c(rep(1, 24), rep(2, 24)),
#   Time = c(seq(1, 24), seq(1, 24)),
#   Mean = c(10 * exp(-0.06 * seq(1, 24)), 10 * exp(-0.1 * seq(1, 24))),
#   Min = c(5 * exp(-0.06 * seq(1, 24)), 8 * exp(-0.1 * seq(1, 24))),
#   Max = c(15 * exp(-0.06 * seq(1, 24)), 12 * exp(-0.1 * seq(1, 24)))
# )
#
#
#
# testMetaData <- list(
#   IndivdualID = list("unit" = "", "dimension" = ""),
#   Time = list("unit" = "min", "dimension" = "Time"),
#   Mean = list("unit" = "mg/L", "dimension" = "Concentration", "LLOQ" = 2),
#   Min = list("unit" = "mg/L", "dimension" = "Concentration", "LLOQ" = 2.2),
#   Max = list("unit" = "mg/L", "dimension" = "Concentration")
# )
#
# # -------------------------------------------------
# # Define Default plot Configuration & Mapping from R6 class for PK Ratio
# group <- GroupMapping$new(color = "IndivdualID")
#
# lloqDataMapping <- TimeProfileDataMapping$new(
#   x = "Time",
#   y = "Mean",
#   groupMapping = group
# )
#
# group <- GroupMapping$new(fill = "IndivdualID")
# lloqRangeDataMapping <- TimeProfileDataMapping$new(
#   x = "Time",
#   ymin = "Min",
#   ymax = "Max",
#   groupMapping = group
# )
# # -------------------------------------------------
# # Plot PK Ratio using the previously defined variables
# lloqPlot <- plotTimeProfile(data = testData, metaData = testMetaData, dataMapping = lloqDataMapping)
#
# lloqRangePlot <- plotTimeProfile(data = testData, metaData = testMetaData, dataMapping = lloqRangeDataMapping)
