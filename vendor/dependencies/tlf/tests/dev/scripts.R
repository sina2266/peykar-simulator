library(tlf)
library(ggplot2)

getData <- function() {
  age <- c(10, 20, 30, 5, 12, 15)
  ratio <- c(0.5, 0.8, 0.4, 1.5, 2.4, 3.8)
  gender <- c("M", "M", "M", "F", "F", "F")
  # gender <- as.factor(gender)
  data <- data.frame(age, ratio, gender)
  data
}

data <- getData()
metaData <- NULL

pkRatioDataMapping <- PKRatioDataMapping$new(x = "age", y = "ratio", grouping = "gender")
pkRatio <- plotPKRatio(data, metaData, pkRatioDataMapping)

plot(pkRatio)
