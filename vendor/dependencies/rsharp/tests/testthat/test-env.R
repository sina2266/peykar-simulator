test_that("It returns a value of a setting", {
  expect_equal(getRSharpSetting("nativePkgName"), rSharpEnv$nativePkgName)
})


test_that("It throws an error when the setting does not exist", {
  expect_error(getRSharpSetting("someSetting"))
})
