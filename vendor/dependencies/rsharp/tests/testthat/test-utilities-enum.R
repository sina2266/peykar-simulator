# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("enums get/set", {
  # very basic support for the time being. Behavior to be defined for cases such as enums with binary operators ([FlagsAttribute])
  eType <- "ClrFacade.TestEnum"
  expect_equal(getEnumNames(eType), c("A", "B", "C"))
  e <- callStatic(rSharpEnv$testCasesTypeName, "GetTestEnum", "B")
  expect_false(is.null(e))
  expect_equal(e$call("ToString"), "B")
})

test_that("instance enumerators are evaluated as int", {
  tName <- "ClrFacade.TestObjectWithEnum"

  obj <- newObjectFromName(tName)
  value <- getStatic(obj, "EnumValue")
  expect_equal(value, 2)

  setStatic(obj, "EnumValue", 0, asInteger = TRUE)

  value <- getStatic(obj, "EnumValue")
  expect_equal(value, 0)
})
