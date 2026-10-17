# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

# Name of the testcase class
cTypename <- "ClrFacade.TestArrayMemoryHandling"

# Test for expected length and type of an array returned from .NET
expectArrayTypeConv <- function(type, arrayLength, expectedRObj, ...) {
  arrayLength <- as.integer(arrayLength)
  obj <- callStatic(cTypename, paste0("CreateArray_", type), arrayLength, ...)
  expect_equal(obj, expectedRObj)
}

# Test that R passes correct type of array to .NET
expectArrayElementType <- function(rObj, expectedTypeName) {
  netType <- callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetType",
    expectedTypeName
  )
  expect_true(callStatic(cTypename, "CheckElementType", rObj, netType))
}


########## R to .NET tests##########
test_that("Basic types of length zero are passed correctly from R to .NET", {
  expectArrayElementType(numeric(0), "System.Double")
  expectArrayElementType(integer(0), "System.Int32")
  expectArrayElementType(raw(0), "System.Byte")
  expectArrayElementType(logical(0), "System.Boolean")
  expectArrayElementType(character(0), "System.String")

  aPosixCt <- numeric(0)
  attributes(aPosixCt) <- list(tzone = "UTC")
  class(aPosixCt) <- c("POSIXct", "POSIXt")

  expect_equal(callStatic(cTypename, "CreateArray_DateTime", 0L), aPosixCt)

  tdiff <- numeric(0)
  class(tdiff) <- "difftime"
  attr(tdiff, "units") <- "secs"
  expect_equal(callStatic(cTypename, "CreateArray_TimeSpan", 0L), tdiff)

  expectArrayElementType(aPosixCt, "System.DateTime")
  expectArrayElementType(tdiff, "System.TimeSpan")
})

# Same for length 1!
# https://github.com/Open-Systems-Pharmacology/rSharp/issues/108
# test_that("Basic types of length one are passed correctly from R to .NET", {
#   expectArrayElementType(numeric(1), "System.Double")
#   expectArrayElementType(integer(1), "System.Int32")
#   expectArrayElementType(raw(1), "System.Byte")
#   expectArrayElementType(logical(1), "System.Boolean")
#   expectArrayElementType(character(1), "System.String")
#   expectArrayElementType("a", "System.String")
# })

# Same for length >1!
test_that("Basic types of length >one are passed correctly from R to .NET", {
  expectArrayElementType(numeric(2), "System.Double")
  expectArrayElementType(integer(2), "System.Int32")
  expectArrayElementType(raw(2), "System.Byte")
  expectArrayElementType(logical(2), "System.Boolean")
  expectArrayElementType(character(2), "System.String")
})

# https://github.com/Open-Systems-Pharmacology/rSharp/issues/58
# test_that("Numerical bi-dimensional arrays are marshalled correctly from R to .NET", {
#   numericMat <- matrix(as.numeric(1:15), nrow = 3, ncol = 5, byrow = TRUE)
#   expect_that( callTestCase( "NumericMatrixEquals", numericMat), equals(numericMat))
# })

########## .NET to R tests##########
test_that("Basic types of length zero are returned correctly from .NET", {
  expectArrayTypeConv("float", 0, numeric(0))
  expectArrayTypeConv("double", 0, numeric(0))
  expectArrayTypeConv("int", 0, integer(0))
  expectArrayTypeConv("byte", 0, raw(0))
  expectArrayTypeConv("char", 0, character(0))
  expectArrayTypeConv("bool", 0, logical(0))
  expectArrayTypeConv("string", 0, character(0))

  # Empty arrays of a certain type are empty lists in R
  expectArrayTypeConv("object", 0, list())
  expectArrayTypeConv("Type", 0, list())
})

test_that("Basic types of length 1 are returned correctly from .NET", {
  expectArrayTypeConv("float", 1, numeric(1))
  expectArrayTypeConv("double", 1, numeric(1))
  expectArrayTypeConv("int", 1, integer(1))
  expectArrayTypeConv("byte", 1, raw(1))
  expectArrayTypeConv("char", 1, character(1))
  expectArrayTypeConv("bool", 1, logical(1))
  expectArrayTypeConv("string", 1, character(1), "")

  # Empty arrays of a certain type are empty lists in R
  expectArrayTypeConv("object", 1, vector("list", 1))
  expectArrayTypeConv("Type", 1, vector("list", 1))
})

test_that("Basic types of length >1 are returned correctly from .NET", {
  expectArrayTypeConv("float", 2, numeric(2))
  expectArrayTypeConv("double", 2, numeric(2))
  expectArrayTypeConv("int", 2, integer(2))
  expectArrayTypeConv("byte", 2, raw(2))
  expectArrayTypeConv("char", 2, character(2))
  expectArrayTypeConv("bool", 2, logical(2))
  expectArrayTypeConv("string", 2, character(2), "")

  # Empty arrays of a certain type are empty lists in R
  expectArrayTypeConv("object", 2, vector("list", 2))
  expectArrayTypeConv("Type", 2, vector("list", 2))
})

test_that("non-empty arrays of non-basic .NET objects are created and passed from .NET to R", {
  tn <- "ClrFacade.TestArrayMemoryHandling"
  tName <- "ClrFacade.TestObject"

  testListEqual <- function(expectObj, expectedLength, actual) {
    expect_equal(expectedLength, length(actual))
    expect_true(is.list(actual))
    expect_true(all(sapply(actual, FUN = function(x) {
      areClrRefEquals(expectObj, x)
    })))
  }

  obj <- newObjectFromName(tName)
  actual <- callStatic(tn, "CreateArray_object", 3L, obj)
  testListEqual(obj, 3L, actual)

  aType <- getType("System.Double")
  actual <- callStatic(tn, "CreateArray_Type", 3L, aType)
  testListEqual(aType, 3L, actual)
})

test_that("String arrays are marshalled correctly", {
  ltrs <- paste(letters[1:5], letters[2:6], sep = "")
  expect_true(callTestCase("StringArrayEquals", ltrs))
  expect_equal(callTestCase("CreateStringArray"), ltrs)

  # One entry is NA.
  ltrs[[3]] <- NA
  expect_true(callTestCase("StringArrayMissingValuesEquals", ltrs))
})

test_that("Numerical bi-dimensional arrays are marshalled correctly from .NET to R", {
  numericMat <- matrix(as.numeric(1:15), nrow = 3, ncol = 5, byrow = TRUE)
  # A natural marshalling of jagged arrays is debatable. For the time being assuming that they are matrices, due to the concrete use case.
  expect_equal(callTestCase("CreateJaggedFloatArray"), numericMat)
  expect_equal(callTestCase("CreateJaggedDoubleArray"), numericMat)
  expect_equal(callTestCase("CreateRectFloatArray"), numericMat)
  expect_equal(callTestCase("CreateRectDoubleArray"), numericMat)
})


########## Mixed tests##########
test_that("Numeric arrays are marshalled correctly", {
  expectedNumArray <- 1:5 * 1.1
  expect_equal(callTestCase("CreateNumArray"), expectedNumArray)
  expect_equal(
    callTestCase("CreateFloatArray"),
    expected = expectedNumArray,
    tolerance = 5e-8
  )
  expect_true(callTestCase("NumArrayEquals", expectedNumArray))

  numDays <- 5
  expect_equal(
    callTestCase("CreateIntArray", as.integer(numDays)),
    expected = 0:(numDays - 1)
  )

  expectedNumArray[3] <- NA
  expect_equal(callTestCase("CreateNumArrayMissingVal"), expectedNumArray)
  expect_true(callTestCase("NumArrayMissingValuesEquals", expectedNumArray))
})
