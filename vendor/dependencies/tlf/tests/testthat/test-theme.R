test_that("PlotConfigurations from default Theme includes all available atom and molecule plots", {
  plotConfigurations <- names(getTLFSettings("currentTheme")$Value$plotConfigurations)
  expect_true(all(as.character(MoleculePlots) %in% plotConfigurations))
  expect_true(all(as.character(setdiff(AtomPlots, "initializePlot")) %in% plotConfigurations))
})

test_that("PlotConfigurations from any new Theme includes all atom and molecule plots", {
  newTheme <- Theme$new()
  plotConfigurations <- names(newTheme$plotConfigurations)
  expect_true(all(as.character(MoleculePlots) %in% plotConfigurations))
  expect_true(all(as.character(setdiff(AtomPlots, "initializePlot")) %in% plotConfigurations))
})
