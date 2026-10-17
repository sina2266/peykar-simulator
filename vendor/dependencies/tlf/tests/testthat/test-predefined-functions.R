test_that("tlfStatFunctions is enum", {
  expect_type(tlfStatFunctions, "list")

  listNames <- names(tlfStatFunctions)
  listValues <- as.character(sapply(tlfStatFunctions, identity))
  expect_equal(listNames, listValues)
})

test_that("All predefined functions exist", {
  for (tlfFunction in tlfStatFunctions) {
    if (tlfFunction %in% c("min", "max")) {
      expect_type(match.fun(tlfFunction), c("builtin"))
    } else {
      expect_type(match.fun(tlfFunction), c("closure"))
    }
  }
})

test_that("Predefined functions give correct output", {
  testInput <- 0:100
  testInputSD <- c(-2, -2, 0, 2, 2)

  mean <- match.fun(tlfStatFunctions$mean)
  expect_equal(mean(testInput), 50)

  sd <- match.fun(tlfStatFunctions$sd)
  expect_equal(sd(testInputSD), 2)
  meanMinusSD <- match.fun(tlfStatFunctions$`mean-sd`)
  expect_equal(meanMinusSD(testInputSD), -2)
  meanPlusSD <- match.fun(tlfStatFunctions$`mean+sd`)
  expect_equal(meanPlusSD(testInputSD), 2)
  meanMinus1.96SD <- match.fun(tlfStatFunctions$`mean-1.96sd`)
  expect_equal(meanMinus1.96SD(testInputSD), -2 * 1.96)
  meanPlus1.96SD <- match.fun(tlfStatFunctions$`mean+1.96sd`)
  expect_equal(meanPlus1.96SD(testInputSD), 2 * 1.96)

  min <- match.fun(tlfStatFunctions$min)
  expect_equal(min(testInput), 0)
  max <- match.fun(tlfStatFunctions$max)
  expect_equal(max(testInput), 100)

  P0 <- match.fun(tlfStatFunctions$`Percentile0%`)
  expect_equal(P0(testInput), 0)
  P1 <- match.fun(tlfStatFunctions$`Percentile1%`)
  expect_equal(P1(testInput), 1)
  P2.5 <- match.fun(tlfStatFunctions$`Percentile2.5%`)
  expect_equal(P2.5(testInput), 2.5)
  P5 <- match.fun(tlfStatFunctions$`Percentile5%`)
  expect_equal(P5(testInput), 5)
  P10 <- match.fun(tlfStatFunctions$`Percentile10%`)
  expect_equal(P10(testInput), 10)
  P15 <- match.fun(tlfStatFunctions$`Percentile15%`)
  expect_equal(P15(testInput), 15)
  P20 <- match.fun(tlfStatFunctions$`Percentile20%`)
  expect_equal(P20(testInput), 20)
  P25 <- match.fun(tlfStatFunctions$`Percentile25%`)
  expect_equal(P25(testInput), 25)
  P50 <- match.fun(tlfStatFunctions$`Percentile50%`)
  expect_equal(P50(testInput), 50)
  P75 <- match.fun(tlfStatFunctions$`Percentile75%`)
  expect_equal(P75(testInput), 75)
  P80 <- match.fun(tlfStatFunctions$`Percentile80%`)
  expect_equal(P80(testInput), 80)
  P85 <- match.fun(tlfStatFunctions$`Percentile85%`)
  expect_equal(P85(testInput), 85)
  P90 <- match.fun(tlfStatFunctions$`Percentile90%`)
  expect_equal(P90(testInput), 90)
  P95 <- match.fun(tlfStatFunctions$`Percentile95%`)
  expect_equal(P95(testInput), 95)
  P97.5 <- match.fun(tlfStatFunctions$`Percentile97.5%`)
  expect_equal(P97.5(testInput), 97.5)
  P99 <- match.fun(tlfStatFunctions$`Percentile99%`)
  expect_equal(P99(testInput), 99)
  P100 <- match.fun(tlfStatFunctions$`Percentile100%`)
  expect_equal(P100(testInput), 100)

  medianMinusIQR <- match.fun(tlfStatFunctions$`median-IQR`)
  expect_equal(medianMinusIQR(testInput), 0)
  medianMinus1.5IQR <- match.fun(tlfStatFunctions$`median-1.5IQR`)
  expect_equal(medianMinus1.5IQR(testInput), -25)
  medianPlusIQR <- match.fun(tlfStatFunctions$`median+IQR`)
  expect_equal(medianPlusIQR(testInput), 100)
  medianPlus1.5IQR <- match.fun(tlfStatFunctions$`median+1.5IQR`)
  expect_equal(medianPlus1.5IQR(testInput), 125)

  lowOutlierLimit <- match.fun(tlfStatFunctions$`Percentile25%-1.5IQR`)
  expect_equal(lowOutlierLimit(testInput), -50)
  highOutlierLimit <- match.fun(tlfStatFunctions$`Percentile75%+1.5IQR`)
  expect_equal(highOutlierLimit(testInput), 150)
})
