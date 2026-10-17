# All tests in this file call into .NET; skip the whole file when the
# runtime is unavailable (see helper-for-tests.R).
skip_if_no_dotnet()

testGarbageCollection <- function(
  getObjCountMethodName = "GetMemTestObjCounter",
  createTestObjectMethodName = "CreateMemTestObj"
) {
  callGcMethname <- "CallGC"
  forceDotNetGc <- function() {
    callTestCase(callGcMethname)
  }
  checkPlusOne <- function() {
    expect_equal(callTestCase(getObjCountMethodName), counter + 1)
  }

  counter <- callTestCase(getObjCountMethodName)
  expect_equal(counter, 0) # make sure none of these test objects instances are hanging in the CLR
  testObj <- callTestCase(createTestObjectMethodName)
  checkPlusOne()
  forceDotNetGc()
  # the object should still be in memory.
  checkPlusOne()
  gc()
  # the object should still be in memory, since testObj is in use and thus the underlying clr handle should be pinned too.
  checkPlusOne()
  rm(testObj)
  gc()
  forceDotNetGc()
  expect_equal(callTestCase(getObjCountMethodName), counter)

  counter <- callTestCase(getObjCountMethodName)
  expect_equal(counter, 0)
  testObj <- callTestCase(createTestObjectMethodName)
  testObj$set(
    name = "Text",
    value = "et nous alimentons nos aimables remords comme les mendiants nourissent leur vermine"
  )
  forceDotNetGc()
  checkPlusOne()
  testObj$set(name = "Text", value = "Sur l'oreiller du mal...")
  checkPlusOne()
  rm(testObj)
  gc()
  forceDotNetGc()
}

test_that("Garbage collection in R and the .NET behaves as expected", {
  testGarbageCollection(
    getObjCountMethodName = "GetMemTestObjCounter",
    createTestObjectMethodName = "CreateMemTestObj"
  )
})

# https://github.com/Open-Systems-Pharmacology/rSharp/issues/59
# test_that("Garbage collection of R.NET objects", {
#   testGarbageCollection( getObjCountMethodName = 'GetMemTestObjCounterRDotnet', createTestObjectMethodName = 'CreateMemTestObjRDotnet')
# })
