test_that("isFileExtension returns `TRUE` if extension is as expected", {
  expect_true(isFileExtension("enum.R", "R"))
  expect_true(isFileExtension("DESCRIPTION", ""))
  expect_true(isFileExtension("foo.", ""))
  expect_true(isFileExtension("C:/Users/.gitignore", "gitignore"))
})

test_that("isFileExtension returns `FALSE` if extension is unexpected", {
  expect_false(isFileExtension("enum.R", "pkml"))
  expect_false(isFileExtension("DESCRIPTION", "R"))
})

test_that("validateIsFileExtension returns NULL if extension is as expected", {
  expect_null(validateIsFileExtension("enum.R", "R"))
  expect_null(validateIsFileExtension("DESCRIPTION", ""))
  expect_null(validateIsFileExtension("foo.", ""))
  expect_null(validateIsFileExtension("C:/Users/.gitignore", "gitignore"))
})

test_that("validateIsFileExtension produces error if extension is unexpected", {
  expect_error(
    validateIsFileExtension("enum.R", "pkml"),
    messages$errorWrongFileExtension("R", "pkml"),
    fixed = TRUE
  )

  expect_error(
    validateIsFileExtension("DESCRIPTION", "R"),
    messages$errorWrongFileExtension("", "R"),
    fixed = TRUE
  )
})
