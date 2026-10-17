simData1 <- exampleDataTimeProfile |>
  dplyr::filter(SetID == "DataSet1") |>
  dplyr::filter(Type == "simulated") |>
  dplyr::select(c("time", "values", "maxValues", "minValues", "caption"))

mappedData <- MappedData$new(
  data = simData1,
  xScale = AxisScales$linear,
  yScale = AxisScales$linear,
  mapping = aes(
    x = time,
    y = values,
    groupby = caption
  ),
)

obsData <- exampleDataTimeProfile |>
  dplyr::filter(SetID == "DataSet3") |>
  dplyr::filter(Type == "observed") |>
  dplyr::filter(dimension == "concentration") |>
  dplyr::select(c("time", "values", "caption", "lloq", "error_relative"))

mappedDataLLOQ <- MappedData$new(
  data = obsData,
  xScale = AxisScales$linear,
  yScale = AxisScales$linear,
  mapping = aes(
    x = time,
    y = values,
    lloq = lloq,
    groupby = caption
  ),
)
# Test for initializePlot
test_that("initializePlot returns a ggplot object", {
  plotObject <- initializePlot(mappedData)
  expect_s3_class(plotObject, "gg")
})

test_that("initializePlot handles NULL mappedData", {
  plotObject <- initializePlot(mappedData = NULL)
  expect_s3_class(plotObject, "gg")
})

# Test for addLayer
test_that("addLayer adds a layer to the ggplot object", {
  plotObject <- ggplot() +
    geom_blank() # Start with an empty ggplot object
  updatedPlot <- addLayer(
    mappedData,
    geomAttributes = list(),
    geom = "point",
    plotObject,
    layerToCall = geom_point
  )
  expect_contains(names(updatedPlot@layers), "geom_point_osp")
})

test_that("addLayer with LLOQ data adds alpha scale using lloqAlphaVector option", {
  plotObject <- ggplot() +
    geom_blank()
  # addLayer with point geom and LLOQ data exercises OptionKeys$lloqAlphaVector
  updatedPlot <- addLayer(
    mappedDataLLOQ,
    geomAttributes = list(),
    geom = "point",
    plotObject,
    layerToCall = geom_point
  )
  expect_contains(names(updatedPlot@layers), "geom_point_osp")
  expect_no_error(print(updatedPlot))
})

# Test for addLLOQLayer
test_that("addLLOQLayer adds LLOQ lines to the ggplot object", {
  plotObject <- ggplot() +
    geom_blank()
  # mappedData without LLOQ does nothing
  updatedPlot <- addLLOQLayer(
    plotObject,
    mappedData,
    layerToCall = geom_hline,
    useLinetypeAsAttribute = FALSE,
    geomLLOQAttributes = list()
  )
  expect_length(names(updatedPlot@layers), 1)
  expect_no_error(print(updatedPlot))

  # mappedData with LLOQ adds lloq line (exercises OptionKeys$lloqLineType via scale_linetype_manual)
  updatedPlot <- addLLOQLayer(
    plotObject,
    mappedDataLLOQ,
    layerToCall = geom_hline,
    useLinetypeAsAttribute = FALSE,
    geomLLOQAttributes = list()
  )
  expect_contains(names(updatedPlot@layers), "geom_hline")
  expect_no_error(print(updatedPlot))
})

test_that("addLLOQLayer with useLinetypeAsAttribute=TRUE exercises lloqLineType option", {
  plotObject <- ggplot() +
    geom_blank()
  # useLinetypeAsAttribute = TRUE exercises OptionKeys$lloqLineType in the attribute path
  updatedPlot <- addLLOQLayer(
    plotObject,
    mappedDataLLOQ,
    layerToCall = geom_hline,
    useLinetypeAsAttribute = TRUE,
    geomLLOQAttributes = list()
  )
  expect_contains(names(updatedPlot@layers), "geom_hline")
  expect_no_error(print(updatedPlot))
})

# Test for addXYScale
test_that("addXYScale adds scales to the ggplot object", {
  plotObject <- ggplot(mtcars, aes(mpg, wt)) +
    geom_point()
  updatedPlot <- addXYScale(plotObject, xScale = "linear", yScale = "log")
  expect_no_error(print(updatedPlot))
  expect_false(is.null(updatedPlot@scales$get_scales("x")))
  expect_false(is.null(updatedPlot@scales$get_scales("y")))
})

# Test for addXScale
test_that("addXScale adds x-scale to the ggplot object", {
  plotObject <- ggplot(mtcars, aes(mpg, wt)) +
    geom_point()
  updatedPlot <- addXScale(
    plotObject,
    xScale = "log",
    xScaleArgs = list(breaks = c(10, 15, 20, 30))
  )
  expect_no_error(print(updatedPlot))
  expect_false(is.null(updatedPlot@scales$get_scales("x")))
  expect_true(is.null(updatedPlot@scales$get_scales("y")))
})

# Test for addYScale
test_that("addYScale adds y-scale to the ggplot object", {
  plotObject <- ggplot(mtcars, aes(mpg, wt)) +
    geom_point()
  updatedPlot <- addYScale(
    plotObject,
    yScale = "log",
    yScaleArgs = list(limits = c(1, 10)),
    secAxis = sec_axis(~ . * 2, name = "Secondary Axis")
  )
  expect_no_error(print(updatedPlot))
  expect_true(is.null(updatedPlot@scales$get_scales("x")))
  expect_false(is.null(updatedPlot@scales$get_scales("y")))
})


test_that("addXScale does not set oob for discrete scale", {
  # discrete scale does not use oob - should render without error
  data <- data.frame(x = c("a", "b", "c"), y = c(1, 2, 3))
  plotObject <- ggplot(data, aes(x, y)) +
    geom_point()
  expect_no_error(addXScale(plotObject, xScale = "discrete"))
})

test_that("'lin' is accepted as shorthand for 'linear' in addXScale", {
  plotObject <- ggplot(mtcars, aes(mpg, wt)) +
    geom_point()
  expect_no_error(addXScale(plotObject, xScale = "lin"))
})

test_that("'lin' is accepted as shorthand for 'linear' in addYScale", {
  plotObject <- ggplot(mtcars, aes(mpg, wt)) +
    geom_point()
  expect_no_error(addYScale(plotObject, yScale = "lin"))
})

test_that("'lin' is accepted as shorthand for 'linear' in addXYScale", {
  plotObject <- ggplot(mtcars, aes(mpg, wt)) +
    geom_point()
  expect_no_error(addXYScale(plotObject, xScale = "lin", yScale = "lin"))
})
