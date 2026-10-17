# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("Methods with variable number of parameters with c# 'params' keyword", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  actual <- testObj$call(
    "TestParams",
    "Hello, ",
    "World!",
    1L,
    2L,
    3L,
    6L,
    5L,
    4L
  )
  expected <- "Hello, World!123654"
  expect_equal(actual, expected = expected)
  actual <- testObj$call("TestParams", "Hello, ", "World!", as.integer(1:6))
  expected <- "Hello, World!123456"
  expect_equal(actual, expected = expected)
})

test_that("Vignette examples work", {
  assemblyPath <- system.file(
    "extdata",
    "rSharp.Examples.dll",
    package = "rSharp"
  )
  # expect_true(loadAssembly(assemblyPath)) # Returns FALSE on macos but TRUE on other platforms
  expect_no_error(loadAssembly(assemblyPath))
  expect_true(isAssemblyLoaded("rSharp.Examples"))
  testObj <- newObjectFromName("rSharp.Examples.SampleInstanceClass")
  expect_equal(
    callStatic("rSharp.Examples.SampleStaticClass", "GetAString"),
    "A string from static class"
  )
  expect_equal(testObj$call("GetAString"), "A string from instance class")
})

test_that("Correct method binding based on parameter types", {
  mkArrayTypeName <- function(typeName) {
    paste(typeName, "[]", sep = "")
  }
  f <- function(...) {
    callStatic("ClrFacade.TestMethodBinding", "SomeStaticMethod", ...)
  }
  printIfDifferent <- function(got, expected) {
    if (any(got != expected)) {
      print(paste("got", got, ", expected", expected))
    }
  }
  g <- function(values, typeName) {
    if (is.list(values)) {
      # this is what one gets with a concatenation of S4 objects, when we use c(testObj,testObj) with CLR objects
      printIfDifferent(f(values[[1]]), typeName)
      printIfDifferent(f(values), mkArrayTypeName(typeName)) # This is not yet supported?
      printIfDifferent(f(values[[1]], values[[2]]), rep(typeName, 2))
      expect_equal(f(values[[1]]), typeName)
      expect_equal(f(values), mkArrayTypeName(typeName))
      expect_equal(f(values[[1]], values[[2]]), rep(typeName, 2))
    } else {
      printIfDifferent(f(values[1]), typeName)
      printIfDifferent(f(values), mkArrayTypeName(typeName))
      printIfDifferent(f(values[1], values[2]), rep(typeName, 2))
      expect_equal(f(values[1]), typeName)
      expect_equal(f(values), mkArrayTypeName(typeName))
      expect_equal(f(values[1], values[2]), rep(typeName, 2))
    }
  }
  intName <- "System.Int32"
  doubleName <- "System.Double"
  stringName <- "System.String"
  boolName <- "System.Boolean"
  dateTimeName <- "System.DateTime"
  objectName <- "System.Object"
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)

  testMethodBinding <- function() {
    g(1:3, intName)
    g(1.2 * 1:3, doubleName)
    g(letters[1:3], stringName)
    g(rep(TRUE, 3), boolName)
    g(as.Date("2001-01-01") + 1:3, dateTimeName)
    g(c(testObj, testObj, testObj), objectName)

    expect_equal(f(1.0, "a"), c(doubleName, stringName))
    expect_equal(f(1.0, "a", "b"), c(doubleName, stringName, stringName))
    expect_equal(
      f(1.0, letters[1:2]),
      c(doubleName, mkArrayTypeName(stringName))
    )
    expect_equal(
      f(1.0, letters[1:10]),
      c(doubleName, mkArrayTypeName(stringName))
    )

    expect_equal(
      f("a", letters[1:3]),
      c(stringName, mkArrayTypeName(stringName))
    )
    expect_equal(
      f(letters[1:3], "a"),
      c(mkArrayTypeName(stringName), stringName)
    )
    expect_equal(
      f(letters[1:3], letters[4:6]),
      c(mkArrayTypeName(stringName), mkArrayTypeName(stringName))
    )
  }
  testMethodBinding()
  obj <- newObjectFromName("ClrFacade.TestMethodBinding")
  f <- function(...) {
    obj$call("SomeInstanceMethod", ...)
  }
  testMethodBinding()
  # Test that methods implemented to comply with an interface are found, even if the method is explicitely implemented.
  # We do not want the users to have to figure out which interface type they deal with, at least not for R users.
  f <- function(...) {
    obj$call("SomeExplicitlyImplementedMethod", ...)
  }
  testMethodBinding()
})

test_that("Conversion of non-bijective types can be turned on/off", {
  # When the conversion is turned off, a `NetObject` is returned, which holds a reference to the .NET object.
  setConvertAdvancedTypes(FALSE)
  expect_true(is(callTestCase("CreateStringDictionary"), "NetObject"))
  expect_true(is(
    callTestCase("CreateStringDoubleArrayDictionary"),
    "NetObject"
  ))
  # When the conversion is turned on, the .NET object is converted to an R list.
  setConvertAdvancedTypes(TRUE)
  expect_equal(callTestCase("CreateStringDictionary"), list(a = "A", b = "B"))
  expect_equal(
    callTestCase("CreateStringDoubleArrayDictionary"),
    list(
      a = c(1.0, 2.0, 3.0, 3.5, 4.3, 11),
      b = c(1.0, 2.0, 3.0, 3.5, 4.3),
      c = c(2.2, 3.3, 6.5)
    )
  )
})

test_that("toStringNET works for primitive types", {
  expect_equal(toStringNET(1), "1")
  expect_equal(toStringNET(1.0), "1")
  expect_equal(toStringNET("a"), "a")
  expect_equal(toStringNET(TRUE), "True")
  expect_equal(toStringNET(FALSE), "False")
  # Check of correct behavior
  #  expect_equal(toStringNET(NA), "null")
  expect_equal(toStringNET(NULL), "null")
  expect_equal(toStringNET(NaN), "NaN")
})

test_that("Print traceback", {
  expected <- "Type:    System.Exception
Message: An exception designed with a particular stack trace length
Method:  Void ThrowException(Int32)
Stack trace:
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at ClrFacade.TestCases.ThrowException(Int32 stackDepth)
   at InvokeStub_TestCases.ThrowException(Object, Span`1)
   at System.Reflection.MethodBaseInvoker.InvokeWithOneArg(Object obj, BindingFlags invokeAttr, Binder binder, Object[] parameters, CultureInfo culture)"

  expect_error(callStatic(rSharpEnv$testCasesTypeName, "ThrowException", 10L)) # will be truncated by the Rf_error API
  # Dont know how to test only for the first xx lines of code. The message thrown when executing tests is different from the one thrown when running the code in the console.
  # expect_output(printTraceback(), expected, fixed = TRUE) # prints the full stack trace
})
