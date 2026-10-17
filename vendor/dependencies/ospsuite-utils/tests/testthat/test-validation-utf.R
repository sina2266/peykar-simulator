# Example of non UTF character taken from Reporting Engine issue #1265
nonUTFText <- c("Hello, world!", "Here is a non-utf encoded unit: \xb5g/L")
utfText <- c("Hello, world!", "Here is a utf encoded unit: \u03bcg/L")
writeLines(
  # Convert Latin1 to local encoding
  iconv(nonUTFText, from = "LATIN1"),
  file("non-utf.txt", encoding = "LATIN1")
)
writeLines(
  # Convert UTF-8 to local encoding
  iconv(utfText, from = "UTF-8"),
  file("utf.txt", encoding = "UTF-8")
)

test_that("isUTF8 and isFileUTF8 return `TRUE` if text is UTF-8 encoded", {
  expect_true(isUTF8(paste(letters, collapse = "")))
  expect_true(isUTF8("@#$%^&*()_+"))
  expect_true(isUTF8(utfText))
  expect_true(isFileUTF8("utf.txt"))
})

test_that("isUTF8 returns `FALSE` if text is NOT UTF-8 encoded", {
  expect_false(isUTF8(nonUTFText))
  expect_false(isFileUTF8("non-utf.txt"))
})

test_that("validateIsFileUTF8 returns NULL if file is UTF-8 encoded", {
  expect_null(validateIsFileUTF8("utf.txt"))
})

test_that("validateIsFileUTF8 produces error if file is NOT UTF-8 encoded", {
  expect_error(
    validateIsFileUTF8("non-utf.txt"),
    messages$errorFileNotUTF8("non-utf.txt"),
    fixed = TRUE
  )
})

# Clean the created files
unlink("utf.txt")
unlink("non-utf.txt")
