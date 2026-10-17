# test histogram

rm(list = ls())

library(ggplot2)
library(tlf)

# Set your folder to test folder:
useTheme(tlfTheme)

df <- data.frame(
  x = c(rep("A", 100), rep("B", 100), rep("C", 100)),
  y = c(rnorm(100) + 5, 10 + rnorm(100), 10 - 2 * rnorm(100))
)

map <- BoxWhiskerDataMapping$new(x = "x", y = "y")

# load("../data/tlf-output.RData")
# variableNames <- names(outputValues$data)

# group <- GroupMapping$new(fill = "x")
# map <- BoxWhiskerDataMapping$new(x = "Gender", y = variableNames[6])

boxplot <- plotBoxWhisker(df, dataMapping = map)
show(boxplot)

# boxtable <- getBoxWhiskerMeasure(outputValues$data, dataMapping = map)
# show(boxtable)
