# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

# calling constructor with wrong arguments

# r_create_clr_object
test_that("Trying to create an object with wrong arguments throws an error", {
  expect_error(newObjectFromName(rSharpEnv$testObjectTypeName, 1, 1, 1))
})

test_that("Trying to create an object with wrong type name throws an error", {
  expect_error(newObjectFromName("foo", 1, 1))
})

# r_call_method
test_that("Calling a method with wrong arguments throws an error", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName, 1, 1)
  expect_error(testObj$call(methodName = "TestDefaultValues", "a", 3))
})

test_that("Calling a method with wrong method name throws an error", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName, 1, 1)
  expect_error(testObj$call(methodName = "foo", "a", as.integer(3)))
})

# r_get_object_direct - don't know how to invoke an error

# r_get_typename_externalptr - don't know how to invoke an error

# r_get_sexp_type - don't know how to invoke an error

# r_show_args

# r_call_static_method
test_that("Calling a static method with wrong arguments throws an error", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName, 1, 1)
  expect_error(callStatic(
    rSharpEnv$clrFacadeTypeName,
    "GetInstanceFields",
    testObj$pointer,
    2
  ))
})

test_that("Calling a static method with wrong method name throws an error", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName, 1, 1)
  expect_error(callStatic(rSharpEnv$clrFacadeTypeName, "foo", testObj$pointer))
})

test_that("Calling a static method with wrong type name throws an error", {
  testObj <- newObjectFromName(rSharpEnv$testObjectTypeName, 1, 1)
  expect_error(callStatic("foo", "GetInstanceFields", testObj$pointer))
})

# Load assembly
