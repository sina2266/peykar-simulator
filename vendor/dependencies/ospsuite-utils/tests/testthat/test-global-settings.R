test_that("Names for settings are as expected", {
  expect_snapshot(ospsuiteUtilsSettingNames)
})

test_that("Check that values for package environment bindings are correct", {
  expect_error(
    getOSPSuiteUtilsSetting("xyz"),
    "(No global setting with the name).*(xyz).*(exists)"
  )

  expect_equal(getOSPSuiteUtilsSetting("packageName"), "ospsuite.utils")
  expect_equal(
    getOSPSuiteUtilsSetting("suiteName"),
    "Open Systems Pharmacology"
  )
  expect_equal(getOSPSuiteUtilsSetting("formatNumericsDigits"), 2L)
})
