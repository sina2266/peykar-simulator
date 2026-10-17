# test histogram

rm(list = ls())

library(ggplot2)
library(tlf)

# Set your folder to test folder:
useTheme(tlfTheme)

# Dummy data frame for test
dat <- data.frame(
  cond = factor(rep(c("A", "B"), each = 200)),
  rating = c(rnorm(200), rnorm(200, mean = .8))
)

binWidth <- 2
bins <- 20
histGrouping <- GroupMapping$new(fill = Grouping$new(group = "cond"))
hdm <- HistogramDataMapping$new(x = "rating", groupMapping = histGrouping)

hpc <- HistogramPlotConfiguration$new(
  data = dat, dataMapping = hdm, bins = bins, binWidth = NULL,
  verticalLineFunctions = c(mean, median), verticalLineFunctionNames = c("Mean", "Median")
)

histo <- plotHistogram(dat, dataMapping = hdm, plotConfiguration = hpc)
