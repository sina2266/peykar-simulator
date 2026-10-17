# this file is automatically loaded by testthat when running tests

# Skips a test when the .NET runtime is not actually available. Keyed off
# whether the native runtime finished initialising (`dotnetAvailable()`), not
# merely whether a `dotnet` CLI is on PATH: build environments such as the
# R-universe macOS runners ship the .NET SDK (so the CLI check passes) but the
# native host cannot be loaded, and tests must skip in that case too.
skip_if_no_dotnet <- function() {
  if (!dotnetAvailable()) {
    testthat::skip("The .NET runtime is not available.")
  }
}

# Functions
expectArrayTypeConv <- function(clrType, arrayLength, expectedRObj) {
  tn <- "ClrFacade.TestArrayMemoryHandling"
  arrayLength <- as.integer(arrayLength)
  expect_equal(
    callStatic(tn, paste0("CreateArray_", clrType), arrayLength),
    expectedRObj
  )
}

createArray <- function(clrType, arrayLength, elementObject) {
  tn <- "ClrFacade.TestArrayMemoryHandling"
  arrayLength <- as.integer(arrayLength)
  if (missing(elementObject)) {
    return(callStatic(tn, paste0("CreateArray_", clrType), arrayLength))
  }
  callStatic(tn, paste0("CreateArray_", clrType), arrayLength, elementObject)
}

expectClrArrayElementType <- function(rObj, expectedClrTypeName) {
  tn <- "ClrFacade.TestArrayMemoryHandling"
  expect_true(callStatic(
    tn,
    "CheckElementType",
    rObj,
    getType(expectedClrTypeName)
  ))
}

callTestCase <- function(...) {
  callStatic(rSharpEnv$testCasesTypeName, ...)
}

# Check if references are equal
areClrRefEquals <- function(x, y) {
  callStatic("System.Object", "ReferenceEquals", x, y)
}
