test_that("isPathAbsolute returns TRUE for absolute paths", {
  expect_true(isPathAbsolute(""))
  expect_true(isPathAbsolute("Organism|path"))
  expect_true(isPathAbsolute("Organism|path|subpath"))
})

test_that("isPathAbsolute returns FALSE for relative paths with wildcard", {
  expect_false(isPathAbsolute("*"))
  expect_false(isPathAbsolute("Organism|*path"))
  expect_false(isPathAbsolute("*path|subpath"))
})

test_that("validateIsPathAbsolute accepts an empty string", {
  expect_null(validateIsPathAbsolute(""))
})

test_that("validateIsPathAbsolute accepts a path without wildcard", {
  path <- "Organism|path"
  expect_no_error(validateIsPathAbsolute(path))
})

test_that("validateIsPathAbsolute throws an error for a path with a wildcard", {
  givenPath <- "*"
  expect_error(
    validateIsPathAbsolute(givenPath)
  )

  givenPath <- "Organism|*path"
  expect_error(
    validateIsPathAbsolute(givenPath)
  )
})

test_that("validatePathIsAbsolute (alias) accepts an empty string", {
  expect_null(validatePathIsAbsolute(""))
  expect_error(validatePathIsAbsolute("*"))
})
