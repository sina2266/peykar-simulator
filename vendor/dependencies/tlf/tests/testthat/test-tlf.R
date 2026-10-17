# Get a few setting values for testing
maxCharacterWidth <- getTLFSettings(tlfSettingsNames$maxCharacterWidth)$Value
alphaRatio <- getTLFSettings(tlfSettingsNames$DefaultAlphaRatio)$Value
lloqLinetype <- getTLFSettings(tlfSettingsNames$defaultLLOQLinetype)$Value

test_that("Changes of default settings work correctly", {
  setDefaultMaxCharacterWidth(100)
  expect_equal(getTLFSettings(tlfSettingsNames$maxCharacterWidth)$Value, 100)

  setDefaultExportParameters(format = "pdf")
  expect_equal(getTLFSettings(tlfSettingsNames$defaultExportParameters)$Value$format, "pdf")

  setDefaultAggregationFunctions(y = "mean")
  yFun <- getTLFSettings(tlfSettingsNames$defaultAggregation)$Value$functions$y
  expect_type(yFun, "closure")
  expect_equal(yFun(1:10), mean(1:10))

  setDefaultAggregationLabels(y = "Mean")
  expect_equal(getTLFSettings(tlfSettingsNames$defaultAggregation)$Value$labels$y, "Mean")

  setDefaultAggregationBins(5)
  expect_equal(getTLFSettings(tlfSettingsNames$defaultAggregation)$Value$bins, 5)

  setDefaultLogTicks(1:10)
  expect_equal(getTLFSettings(tlfSettingsNames$logTicks)$Value, 1:10)

  setDefaultErrorbarCapSize(2)
  expect_equal(getTLFSettings(tlfSettingsNames$defaultErrorbarCapSize)$Value, 2)

  setDefaultLLOQLinetype("dashed")
  expect_equal(getTLFSettings(tlfSettingsNames$defaultLLOQLinetype)$Value, "dashed")

  setDefaultAlphaRatio(0.5)
  expect_equal(getTLFSettings(tlfSettingsNames$DefaultAlphaRatio)$Value, 0.5)

  setDefaultWatermark("My Watermark")
  expect_equal(getTLFSettings(tlfSettingsNames$currentTheme)$Value$background$watermark, "My Watermark")

  setDefaultLegendPosition(LegendPositions$outsideBottom)
  expect_equal(getTLFSettings(tlfSettingsNames$currentTheme)$Value$background$legendPosition, LegendPositions$outsideBottom)

  setDefaultLegendTitle("My Legend")
  expect_equal(getTLFSettings(tlfSettingsNames$currentTheme)$Value$background$legendTitle, "My Legend")
})

test_that("Default settings can be saved", {
  saveTLFSettings("test-settings.RData")
  expect_true(file.exists("test-settings.RData"))
})

test_that("Default settings are reset to TLF Default", {
  resetTLFSettingsToDefault()
  expect_equal(getTLFSettings(tlfSettingsNames$maxCharacterWidth)$Value, maxCharacterWidth)
  expect_equal(getTLFSettings(tlfSettingsNames$DefaultAlphaRatio)$Value, alphaRatio)
  expect_equal(getTLFSettings(tlfSettingsNames$defaultLLOQLinetype)$Value, lloqLinetype)
})

test_that("Previous default settings are reloaded", {
  loadTLFSettings("test-settings.RData")
  expect_equal(getTLFSettings(tlfSettingsNames$maxCharacterWidth)$Value, 100)
  expect_equal(getTLFSettings(tlfSettingsNames$DefaultAlphaRatio)$Value, 0.5)
  expect_equal(getTLFSettings(tlfSettingsNames$defaultLLOQLinetype)$Value, "dashed")
})

# Clean up
resetTLFSettingsToDefault()
unlink("test-settings.RData")
