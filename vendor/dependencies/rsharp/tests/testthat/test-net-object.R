# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("It creates a `NetObject` from a valid pointer is provided", {
  testClassName <- "ClrFacade.Tests.RefClasses.LevelOneClass"
  o <- .External(
    "r_create_clr_object",
    testClassName,
    PACKAGE = rSharpEnv$nativePkgName
  )
  netObj <- NetObject$new(o)

  # Check if the object is a NetObject
  expect_true(inherits(netObj, "NetObject"))
  # Check the type
  expect_equal(netObj$type, testClassName)
  # Check the print method
  expect_snapshot(netObj$print())
})

# Re-enable once https://github.com/Open-Systems-Pharmacology/rSharp/issues/67
# test_that("CLR type compatibility checking", {
#   testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
#   expect_true(clrIs(testObj, rSharpEnv$testObjectTypeName))
#   expect_true(clrIs(testObj, "System.Object"))
#   expect_false(clrIs(testObj, "System.Double"))
#   testObj <- newObjectFromName("ClrFacade.TestMethodBinding")
#   expect_true(clrIs(testObj, "ClrFacade.ITestMethodBindings"))
#   expect_true(clrIs(testObj, getType("ClrFacade.ITestMethodBindings")))
#   expect_true(clrIs(testObj, getType("ClrFacade.TestMethodBinding")))
#   expect_false(clrIs(testObj, getType("System.Reflection.Assembly")))
#   expect_error(clrIs(testObj, testObj))
# })

test_that("$getMethods lists all methods of an object", {
  expectedMethods <- c(
    "Equals",
    "get_PropertyIntegerOne",
    "get_PropertyIntegerTwo",
    "GetFieldIntegerOne",
    "GetFieldIntegerTwo",
    "GetHashCode",
    "GetMethodWithParameters",
    "GetPublicInt",
    "GetType",
    "set_PropertyIntegerOne",
    "set_PropertyIntegerTwo",
    "TestDefaultValues",
    "TestParams",
    "ToString"
  )

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(testObj$getMethods() %in% expectedMethods))
})

test_that("$getMethods lists all methods of an object that include a given string", {
  expectedMethods <- c(
    "get_PropertyIntegerOne",
    "GetFieldIntegerOne",
    "set_PropertyIntegerOne"
  )

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(
    testObj$getMethods(contains = "IntegerOne") %in% expectedMethods
  ))
})


test_that("$getFields lists all fields of an object", {
  expectedFields <- c(
    "FieldDoubleOne",
    "FieldDoubleTwo",
    "FieldIntegerOne",
    "FieldIntegerTwo",
    "PublicInt"
  )

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(testObj$getFields() %in% expectedFields))
})

test_that("$getFields lists all fields of an object that include a given string", {
  expectedFields <- c("FieldIntegerOne")

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(
    testObj$getFields(contains = "IntegerOne") %in% expectedFields
  ))
})

test_that("$getProperties lists all properties of an object", {
  expectedProperties <- c("PropertyIntegerOne", "PropertyIntegerTwo")

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(testObj$getProperties() %in% expectedProperties))
})

test_that("$getProperties lists all properties of an object that include a given string", {
  expectedProperties <- c("PropertyIntegerOne")

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(
    testObj$getProperties(contains = "IntegerOne") %in% expectedProperties
  ))
})

test_that("$getStaticMethods lists all methods of an object", {
  expectedMethods <- c(
    "get_StaticPropertyIntegerOne",
    "get_StaticPropertyIntegerTwo",
    "set_StaticPropertyIntegerOne",
    "set_StaticPropertyIntegerTwo",
    "StaticGetFieldIntegerOne",
    "StaticGetFieldIntegerTwo",
    "StaticGetMethodWithParameters",
    "StaticGetPublicInt"
  )

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(testObj$getStaticMethods() %in% expectedMethods))
})

test_that("$getStaticMethods lists all methods of an object that include a given string", {
  expectedMethods <- c(
    "get_StaticPropertyIntegerOne",
    "set_StaticPropertyIntegerOne",
    "StaticGetFieldIntegerOne"
  )

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(
    testObj$getStaticMethods(contains = "IntegerOne") %in% expectedMethods
  ))
})


test_that("$getStaticFields lists all fields of an object", {
  expectedFields <- c(
    "StaticFieldIntegerOne",
    "StaticFieldIntegerTwo",
    "StaticPublicInt"
  )

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(testObj$getStaticFields() %in% expectedFields))
})

test_that("$getStaticFields lists all fields of an object that include a given string", {
  expectedFields <- c("StaticFieldIntegerOne")

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(
    testObj$getStaticFields(contains = "IntegerOne") %in% expectedFields
  ))
})

test_that("$getProperties lists all properties of an object", {
  expectedProperties <- c("PropertyIntegerOne", "PropertyIntegerTwo")

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(testObj$getProperties() %in% expectedProperties))
})

test_that("$getProperties lists all properties of an object that include a given string", {
  expectedProperties <- c("PropertyIntegerOne")

  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  expect_true(all(
    testObj$getProperties(contains = "IntegerOne") %in% expectedProperties
  ))
})


test_that("Object members discovery behaves as expected", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)

  expect_equal(
    testObj$getMemberSignature(memberName = "GetFieldIntegerOne"),
    "Method: Int32 GetFieldIntegerOne"
  )

  expect_equal(
    testObj$getMemberSignature(memberName = "GetMethodWithParameters"),
    "Method: Int32 GetMethodWithParameters, Int32, String"
  )
})

# Expect an error? https://github.com/Open-Systems-Pharmacology/rSharp/issues/69
# test_that("$getMemberSignature() when a member with the given name is not found", {
#   testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
#   expect_error(testObj$getMemberSignature(memberName = "NonExistentMethod"))
# })

# https://github.com/Open-Systems-Pharmacology/rSharp/issues/70
# test_that("It throws an error when trying to call a method that does not exist", {
#   testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
#   expect_error(testObj$call(methodName = "NonExistentMethod"), regexp = messages$errorMethodNotFound("NonExistentMethod", rSharpEnv$testObjectTypeName))
# })

test_that("$get works as expected", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  fieldName <- "FieldIntegerOne"
  propName <- "PropertyIntegerOne"

  expect_equal(testObj$get(fieldName), 0)
  expect_equal(testObj$get(propName), 0)
})

test_that("$set works as expected", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)
  fieldName <- "FieldIntegerOne"
  propName <- "PropertyIntegerOne"

  expect_equal(testObj$get(fieldName), 0)
  expect_equal(testObj$get(propName), 0)

  testObj$set(fieldName, as.integer(2))
  expect_equal(testObj$get(fieldName), 2)

  testObj$set(propName, as.integer(2))
  expect_equal(testObj$get(propName), 2)
})

test_that("Deprecated printing methods show appropriate warnings", {
  testClassName <- "ClrFacade.Tests.RefClasses.LevelOneClass"
  o <- .External(
    "r_create_clr_object",
    testClassName,
    PACKAGE = rSharpEnv$nativePkgName
  )
  netObj <- NetObject$new(o)

  # Test .printLine deprecation warning with correct message format
  expect_warning(
    netObj$.printLine("test"),
    "`NetObject\\$.printLine\\(\\)` was deprecated in rSharp 1\\.1\\.2\\."
  )

  # Test .printClass deprecation warning with correct message format
  expect_warning(
    netObj$.printClass(),
    "`NetObject\\$.printClass\\(\\)` was deprecated in rSharp 1\\.1\\.2\\."
  )

  # Also verify that the warning has the correct class
  expect_warning(
    netObj$.printLine("test"),
    class = "lifecycle_warning_deprecated"
  )

  expect_warning(
    netObj$.printClass(),
    class = "lifecycle_warning_deprecated"
  )
})

test_that("A NetObject can be passed as argument to callStatic", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName)

  expect_true(callTestCase(
    "SingleObjectArgument",
    testObj
  ))
})

test_that("An array of NetObjects can be passed as argument to callStatic", {
  testObj1 <- newObjectFromName(rSharpEnv$testObjectTypeName)
  testObj2 <- newObjectFromName(rSharpEnv$testObjectTypeName)
  testArray <- array(c(testObj1, testObj2), dim = c(2))

  expect_true(callTestCase(
    "ArrayOfObjectsArgument",
    testArray
  ))
})
