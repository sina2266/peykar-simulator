# The next line simply remove the variables from the environment
# Similar to clear from Matlab
rm(list = ls())

library(ggplot2)
library(tlf)

# Load Data
load("../data/tlf-output.RData")

# Set default Config
useTheme(tlfTheme)

variableNames <- names(outputValues$data)
# Show Variable Names of Data
print("Variable Names")
print(variableNames)

# Define what to plot
timeProfileMapping <- TimeProfileDataMapping$new(x = "Time", y = variableNames[7])

# -------------------------------------------------
# Plot Time Profile with default configuration
timeProfilePlot <- plotTimeProfile(
  data = outputValues$data,
  metaData = outputValues$metaData,
  dataMapping = timeProfileMapping
)

print(timeProfilePlot)
