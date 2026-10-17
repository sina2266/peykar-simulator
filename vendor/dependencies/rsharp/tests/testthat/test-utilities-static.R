# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("callStatic returns a NetObject when the static .NET method returns a non-primitive object", {
  testObj <- callTestCase("CreateTestObject")
  expect_true(inherits(testObj, "NetObject"))
  expect_equal(testObj$type, rSharpEnv$testObjectTypeName)

  testObj <- callTestCase("CreateTestObjectGenericInstance")
  expect_true(inherits(testObj, "NetObject"))
  # 2DO: Test for IS!
  # https://github.com/Open-Systems-Pharmacology/rSharp/issues/67

  # Not clear what should be tested here...
  testObj <- callTestCase("CreateTestArrayGenericObjects")
  testObj <- callTestCase("CreateTestArrayInterface")
  testObj <- callTestCase("CreateTestArrayGenericInterface")
})

test_that("getStatic and setStatic works as expected", {
  fieldName <- "StaticFieldIntegerOne"
  propName <- "StaticPropertyIntegerOne"

  setStatic(
    type = rSharpEnv$testObjectTypeName,
    name = fieldName,
    value = as.integer(0)
  )
  setStatic(
    type = rSharpEnv$testObjectTypeName,
    name = propName,
    value = as.integer(0)
  )

  expect_equal(
    getStatic(type = rSharpEnv$testObjectTypeName, name = fieldName),
    0
  )
  expect_equal(
    getStatic(type = rSharpEnv$testObjectTypeName, name = propName),
    0
  )

  setStatic(
    type = rSharpEnv$testObjectTypeName,
    name = fieldName,
    value = as.integer(2)
  )
  setStatic(
    type = rSharpEnv$testObjectTypeName,
    name = propName,
    value = as.integer(2)
  )

  expect_equal(
    getStatic(type = rSharpEnv$testObjectTypeName, name = fieldName),
    2
  )
  expect_equal(
    getStatic(type = rSharpEnv$testObjectTypeName, name = propName),
    2
  )
})
