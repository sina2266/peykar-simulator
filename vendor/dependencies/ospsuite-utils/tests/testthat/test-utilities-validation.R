test_that(".getCallingFunctionName produces error message in R6 active bindings", {
  # Not fully unit-testable: any named wrapper in sys.calls() masks anonymous
  # frames (e.g. R6 active bindings).
  TestClass <- R6::R6Class(
  "TestClass",
  active = list(
    value = function(v) {
      if (missing(v)) {
        return(private$.value)
      }
      ospsuite.utils::validateIsCharacter(v)
      private$.value <- v
    }
  ),
  private = list(.value = "a")
)
  obj <- TestClass$new()
  expect_error(
    obj$value <- 123,
    regexp = "(argument).*(but expected).*"
  )
})
