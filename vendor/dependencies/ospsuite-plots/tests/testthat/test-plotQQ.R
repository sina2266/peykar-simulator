test_that("plot Observed vs Predicted works", {
  skip_if_not_installed("vdiffr")
  skip_if(getRversion() < "4.1")

  data <- exampleDataCovariates |>
    dplyr::filter(SetID == "DataSet2") |>
    dplyr::select(c("ID", "Obs", "gsd", "Pred", "Sex"))

  lloqData <- signif(quantile(data$Obs, probs = 0.1), 1)

  data <- data |>
    dplyr::mutate(lloq = lloqData) |>
    dplyr::mutate(Obs = ifelse(Obs <= lloq, lloq / 2, Obs)) |>
    dplyr::mutate(residuals = Pred - Obs)

  metaData <- attr(exampleDataCovariates, "metaData")
  metaData <- metaData[intersect(names(data), names(metaData))]

  attr(
    x = data$residuals,
    which = "label"
  ) <- "residuals\npredicted - observed"

  vdiffr::expect_doppelganger(
    title = "plotQQ",
    fig = plotQQ(
      data = data,
      mapping = aes(
        sample = residuals,
        groupby = Sex
      )
    )
  )
})
