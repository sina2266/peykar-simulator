Symbol <- enum(c(Diamond = 1, Triangle = 2, Circle = 2))

Units <- enum(c(
  "g/l" = "g/l",
  "mg/l" = "mg/l",
  "ug/l" = "ug/l",
  "ng/l" = "ng/l",
  "pg/l" = "pg/l",
  "g/ml" = "g/ml",
  "mg/ml" = "mg/ml",
  "ug/ml" = "ug/ml",
  "ng/ml" = "ng/ml",
  "pg/ml" = "pg/ml",
  "mol/l" = "mol/l",
  "mmol/l" = "mmol/l",
  "umol/l" = "umol/l",
  "nmol/l" = "nmol/l",
  "pmol/l" = "pmol/l",
  "M" = "M",
  "mM" = "mM",
  "uM" = "uM",
  "nM" = "nM",
  "pM" = "pM",
  "mol/ml" = "mol/ml",
  "mmol/ml" = "mmol/ml",
  "umol/ml" = "umol/ml",
  "nmol/ml" = "nmol/ml",
  "pmol/ml" = "pmol/ml"
))

test_that("validateEnumValue returns `NULL` when validation is successful", {
  expect_null(validateEnumValue(NULL, nullAllowed = TRUE))
  expect_null(validateEnumValue(1, Symbol))
})

test_that("validateEnumValue produces error when validation is unsuccessful", {
  expect_error(validateEnumValue(4, Symbol))
  expect_error(validateEnumValue(NULL))
})


test_that("validateEnumValue prints suggestions for close matches and fallback hint otherwise", {
  # Close match (edit distance 1): suggestions are shown and include the likely candidate
  expect_error(
    validateEnumValue("mol/L", Units),
    regexp = "Did you mean one of these:"
  )
  expect_error(
    validateEnumValue("mol/L", Units),
    regexp = "mol/l" # Should suggest the correct lowercase version
  )
  expect_error(
    validateEnumValue("mol/L", Units),
    regexp = "All valid values can be found using" # Generic hint is always shown
  )

  # Another close match (edit distance 1): single character typo
  expect_error(
    validateEnumValue("ug/m", Units), # missing 'l' at the end
    regexp = "Did you mean one of these:"
  )

  # Close match (edit distance 2): two character difference
  expect_error(
    validateEnumValue("mmol/ML", Units), # ML vs ml = 2 chars different
    regexp = "Did you mean one of these:"
  )
  expect_error(
    validateEnumValue("mmol/ML", Units),
    regexp = "mmol/ml" # Should suggest the correct version
  )

  # Boundary case: edit distance exactly 2 (should still show suggestions)
  expect_error(
    validateEnumValue("mol/LL", Units), # LL vs l = 2 char edit distance
    regexp = "Did you mean one of these:"
  )

  # Far match (edit distance > 2): no suggestions, only generic hint is shown
  err_msg <- tryCatch(
    validateEnumValue("mol/MLK", Units),
    error = function(e) conditionMessage(e)
  )
  expect_match(err_msg, "All valid values can be")
  expect_false(grepl("Did you mean one of these:", err_msg)) # Should NOT contain suggestions

  # Completely unrelated value: no suggestions
  err_msg2 <- tryCatch(
    validateEnumValue("kilometers", Units),
    error = function(e) conditionMessage(e)
  )
  expect_match(err_msg2, "All valid values can be found")
  expect_false(grepl("Did you mean one of these:", err_msg2)) # Should NOT contain suggestions

  # Multiple possible suggestions: when several values are close
  expect_error(
    validateEnumValue("mg/M", Units), # could suggest mg/ml or mg/l
    regexp = "Did you mean one of these:"
  )
})
