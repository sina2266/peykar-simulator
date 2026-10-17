# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

test_that("Booleans are marshalled correctly", {
  expect_false(callTestCase("GetFalse"))
  expect_true(callTestCase("GetTrue"))
  expect_true(callTestCase("IsTrue", TRUE))
  expect_false(callTestCase("IsTrue", FALSE))
})

test_that("Basic types of length one are marshalled correctly", {
  # R to NET
  expect_true(callTestCase("DoubleEquals", 123.0))
  expect_true(callTestCase("IntEquals", as.integer(123)))
  expect_true(callTestCase("StringEquals", "ab"))
  # NET to R
  expect_equal(callTestCase("CreateDouble"), 123.0)
  expect_equal(callTestCase("CreateInt"), as.integer(123))
  expect_equal(callTestCase("CreateString"), "ab")
})

# Requires https://github.com/Open-Systems-Pharmacology/rSharp/issues/40
# test_that("Unicode strings are marshalled correctly", {
#   # R to NET
#   expect_true(callTestCase("UnicodeStringEquals", "äöü"))
#   # NET to R
#   expect_true(callTestCase("CreateUnicodeString", "äöü"))
# })

test_that("Complex numbers are converted", {
  z <- 1 + 2i
  expect_equal(callTestCase("CreateComplex", 1, 2), z)
  expect_true(callTestCase("ComplexEquals", z, 1, 2))
  z <- c(1 + 2i, 3 + 4i, 3.3 + 4.4i)
  expect_equal(callTestCase("CreateComplex", c(1, 3, 3.3), c(2, 4, 4.4)), z)
  expect_true(callTestCase("ComplexEquals", z, c(1, 3, 3.3), c(2, 4, 4.4)))
})
