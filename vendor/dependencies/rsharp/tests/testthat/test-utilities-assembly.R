# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("Assembly loading", {
  expect_no_error(loadAssembly(
    "System.Net.Http, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a"
  ))

  # The use of partial assembly names is discouraged; nevertheless it is supported
  expect_no_error(loadAssembly("System.Net.Http"))
  expect_error(loadAssembly("Something.That.Isnt.There"))
})

test_that("Loaded assemblies discovery", {
  expect_true("ClrFacade" %in% getLoadedAssemblies())
  d <- getLoadedAssemblies(fullname = TRUE, filenames = TRUE)
  expect_true(is.data.frame(d))
})

test_that("getTypesInAssembly works as expected()", {
  expect_true("ClrFacade.TestObject" %in% getTypesInAssembly("ClrFacade"))
})

test_that("isAssemblyLoaded works as expected()", {
  expect_true(isAssemblyLoaded("ClrFacade"))
  expect_false(isAssemblyLoaded("SomeAssemblyThatIsntThere"))
})
