# The next line simply remove the variables from the environment
# Similar to clear from Matlab
rm(list = ls())

library(ggplot2)
library(tlf)

# Set your folder to test folder:
useTheme(tlfTheme)

# -------------------------------------------------
# Data is saved in R/sysdata.rda
# pkRatioData and pkRatioMetaData already exist

# -------------------------------------------------
# Define Default plot Configuration & Mapping from R6 class for PK Ratio

groupings <- GroupMapping$new(
  color = c("Gender"),
  shape = c("Dose", "Compound")
)

pkRatioDataMapping <- PKRatioDataMapping$new(groupMapping = groupings)

# Renaming of Label from initialize
pkRatioConfiguration <- PKRatioPlotConfiguration$new(
  data = pkRatioData,
  metaData = pkRatioMetaData,
  dataMapping = pkRatioDataMapping
)

# -------------------------------------------------
# Plot PK Ratio using the previously defined variables
pkrp <- plotPKRatio(
  data = pkRatioData, metaData = pkRatioMetaData,
  dataMapping = pkRatioDataMapping, plotConfiguration = pkRatioConfiguration
)

# -------------------------------------------------
# Test of getPKRatioMeasure
PKRatioMeasure <- getPKRatioMeasure(data = pkRatioData, dataMapping = pkRatioDataMapping)

print(PKRatioMeasure)

# Saving of plot
# testConfiguration$savePlot(pkrp)
