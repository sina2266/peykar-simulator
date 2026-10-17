test_that("DDIRatioDataMapping does not crash for NULL delta guest value", {
  ddiMapping1 <- DDIRatioDataMapping$new()
  ddiMapping2 <- DDIRatioDataMapping$new(deltaGuest = NULL)
  ddiMapping3 <- DDIRatioDataMapping$new(deltaGuest = 1)
  ddiMapping4 <- DDIRatioDataMapping$new(deltaGuest = 1.5)

  expect_equal(ddiMapping1$deltaGuest, 1)
  expect_equal(ddiMapping2$deltaGuest, 1)
  expect_equal(ddiMapping3$deltaGuest, 1)
  expect_equal(ddiMapping4$deltaGuest, 1.5)

  ddiMapping3$deltaGuest <- NULL
  ddiMapping4$deltaGuest <- NULL
  expect_equal(ddiMapping3$deltaGuest, 1)
  expect_equal(ddiMapping4$deltaGuest, 1.5)
})

test_that("DDIRatioDataMapping warn for outlying delta guest values", {
  expect_silent(ddiMapping1 <- DDIRatioDataMapping$new(deltaGuest = 1))
  expect_silent(ddiMapping2 <- DDIRatioDataMapping$new(deltaGuest = 1.5))
  expect_warning({
    ddiMapping3 <- DDIRatioDataMapping$new(deltaGuest = 3)
  })
  expect_warning({
    ddiMapping1$deltaGuest <- 2.2
  })
})

test_that("Guest lines always frame data x range", {
  # default is [1e-2, 1e2]
  ddiMapping1 <- DDIRatioDataMapping$new(x = "x", y = "y")
  ddiMapping2 <- DDIRatioDataMapping$new(x = "x", y = "y", minRange = c(0.1, 10))
  ddiMapping3 <- DDIRatioDataMapping$new(x = "x", y = "y", minRange = c(0.5, 2))
  ddiData <- data.frame(x = c(5, 0.2, 2, 3, 4), y = c(1, 2, 1, 2, 3))

  guest1 <- getGuestValuesFromDataMapping(
    data = ddiData,
    dataMapping = ddiMapping1
  )
  guest2 <- getGuestValuesFromDataMapping(
    data = ddiData,
    dataMapping = ddiMapping2
  )
  guest3 <- getGuestValuesFromDataMapping(
    data = ddiData,
    dataMapping = ddiMapping3
  )
  expect_s3_class(guest1, "data.frame")
  expect_s3_class(guest2, "data.frame")
  expect_s3_class(guest3, "data.frame")
  expect_equal(names(guest1), c("x", "ymin", "ymax"))
  expect_equal(names(guest2), c("x", "ymin", "ymax"))
  expect_equal(names(guest3), c("x", "ymin", "ymax"))

  # Guest curves are wider than observed x
  expect_gte(max(guest1$x), max(ddiData$x))
  expect_lte(min(guest1$x), min(ddiData$x))
  expect_gte(max(guest2$x), max(ddiData$x))
  expect_lte(min(guest2$x), min(ddiData$x))
  # lte/gte can't be used for guest3
  # because they do not use tolerance and Difference is 2.78e-17
  # leading to test failure
  expect_equal(max(guest3$x), max(ddiData$x))
  expect_equal(min(guest3$x), min(ddiData$x))
})
