# The next line simply remove the variables from the environment
# Similar to clear from Matlab
rm(list = ls())

library(ggplot2)
library(tlf)

# Set your folder to test folder:
# setwd("C:/Design2Code/TLF-Playground/TLF-Playground/R/")
useTheme(tlfTheme)

# -------------------------------------------------
# Get the data and metadata for PK Ratio plot

nPopulation <- 20

# -------------------------------------------------

testData <- data.frame(
  IndivdualID = rep(1, 24),
  Time = seq(1, 24),
  Mean = 10 * exp(-0.06 * seq(1, 24)),
  Min = 5 * exp(-0.06 * seq(1, 24)),
  Max = 15 * exp(-0.06 * seq(1, 24))
)


testMetaData <- list(
  IndivdualID = list("unit" = "", "dimension" = ""),
  Time = list("unit" = "min", "dimension" = "Time"),
  Mean = list("unit" = "mg/L", "dimension" = "Concentration"),
  Min = list("unit" = "mg/L", "dimension" = "Concentration"),
  Max = list("unit" = "mg/L", "dimension" = "Concentration")
)

# -------------------------------------------------
# Define Default plot Configuration & Mapping from R6 class for PK Ratio

meanDataMapping <- TimeProfileDataMapping$new(
  x = "Time",
  y = "Mean"
)

errorDataMapping <- TimeProfileDataMapping$new(
  x = "Time",
  y = "Mean",
  ymin = "Min",
  ymax = "Max"
)

rangeDataMapping <- TimeProfileDataMapping$new(
  x = "Time",
  ymin = "Min",
  ymax = "Max"
)
# -------------------------------------------------
# Plot PK Ratio using the previously defined variables
meanPlot <- plotTimeProfile(data = testData, metaData = testMetaData, dataMapping = meanDataMapping)

errorPlot <- plotTimeProfile(data = testData, metaData = testMetaData, dataMapping = errorDataMapping)

rangePlot <- plotTimeProfile(data = testData, metaData = testMetaData, dataMapping = rangeDataMapping)
