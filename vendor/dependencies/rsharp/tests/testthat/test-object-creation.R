# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("newObjectFromName calls constructors when a valid name is provided", {
  tName <- "ClrFacade.TestObject"
  i1 <- as.integer(23)
  i2 <- as.integer(42)
  d1 <- 1.234
  d2 <- 2.345
  # Constructor without arguments
  obj <- newObjectFromName(tName)
  # Constructor with one argument
  obj <- newObjectFromName(tName, i1)
  expect_equal(obj$get("FieldIntegerOne"), i1)
  # Constructor with two arguments
  obj <- newObjectFromName(tName, i1, i2)
  expect_equal(obj$get("FieldIntegerOne"), i1)
  expect_equal(obj$get("FieldIntegerTwo"), i2)
  # Constructor with two double arguments
  obj <- newObjectFromName(tName, d1, d2)
  expect_equal(obj$get("FieldDoubleOne"), d1)
  expect_equal(obj$get("FieldDoubleTwo"), d2)
})

test_that("newObjectFromName returns an error when an invalid name is provided", {
  expect_error(newObjectFromName("InvalidTypeName"))
})

test_that("Basic objects are created correctly", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(inherits(testObj, "NetObject"))
  expect_equal(testObj$type, rSharpEnv$testObjectTypeName)
  rm(testObj)

  # Call to a static method creates an S4 object
  testObj <- .External(
    "r_call_static_method",
    rSharpEnv$testCasesTypeName,
    "CreateTestObject",
    PACKAGE = rSharpEnv$nativePkgName
  )
  expect_false(is.null(testObj))
  expect_equal(testObj@clrtype, rSharpEnv$testObjectTypeName)
  rm(testObj)
})
