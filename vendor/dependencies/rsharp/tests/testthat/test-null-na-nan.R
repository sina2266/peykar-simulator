# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

########## R to .NET tests##########
test_that("Basic NULL is passed to .NET as null", {
  expect_true(callTestCase("IsNull", NULL))
})

# What arrives at .NET side is TRUE - questionable. See https://github.com/Open-Systems-Pharmacology/rSharp/issues/110
test_that("Basic NA is passed to .NET as NA", {
  expect_true(callTestCase("IsNA", NA))
})
test_that("Basic NaN is passed to .NET as NaN", {
  expect_true(callTestCase("IsNaN", NaN))
})

test_that("NULL in an array is passed to .NET as NULL", {
  expect_true(callTestCase("IsNullInArray", list(1, NULL, 3)))
})

# What arrives at .NET side is TRUE - questionable. See https://github.com/Open-Systems-Pharmacology/rSharp/issues/110
# Currently NA is interpreted as Na? Why?
# test_that("NA in an array is passed to .NET as NA", {
#   expect_true(callTestCase("IsNAInArray", c(1, NA, 3)))
# })

test_that("NaN in an array is passed to .NET as NaN", {
  expect_true(callTestCase("IsNaNInArray", c(1, NaN, 3)))
})

########## .NET to R tests##########
test_that("Basic NULL is passed from .NET as null", {
  expect_null(callTestCase("GetNull"))
})

# No such concept in .NET
# https://github.com/Open-Systems-Pharmacology/rSharp/issues/110
# test_that("Basic NA is passed from .NET as NA", {
#   expect_true(callTestCase("GetNA", NA))
# })

test_that("Basic NaN is passed from .NET as NaN", {
  expect_equal(callTestCase("GetNaN"), NaN)
})

# test_that("Array NULL is passed from .NET as null", {
#   expect_equal(callTestCase("GetNullArray"), list(1, NULL, 3))
# })

# # No such concept in .NET
# #https://github.com/Open-Systems-Pharmacology/rSharp/issues/110
# # test_that("Array NA is passed from .NET as NA", {
# #   expect_equal(callTestCase("GetNAArray", c(1, NA, 3)))
# # })

test_that("Array NaN is passed from .NET as NaN", {
  expect_equal(callTestCase("GetNaNArray"), c(1, NaN, 3))
})
