# Workflow combining data mapping to a plot configuration

library(ggplot2)
library(tlf)

load("pkRatioDataExample.RData")
useTheme(tlfTheme)
map <- XYDataMapping$new(
  x = "Age",
  y = "Ratio",
  color = c("Gender"),
  linetype = c("Dose", "Compound")
)

# If data, metaData and data mapping are used as
# input of the configuration, they can be used
config <- PlotConfiguration$new(
  title = "PK Ratio",
  data = pkRatioData,
  metaData = pkRatioMetaData,
  dataMapping = map
)

# Create empty plot object (initiate the plot)
plotObject <- ggplot2::ggplot()

# Define the labels of titles, legend and axes
plotObject <- config$setPlotLabels(plotObject)

# Add the geom (may be replaced by a plot config function)
mappedData <- map$getMappedData(data = pkRatioData, metaData = pkRatioMetaData)

plotObject <- plotObject + geom_point(aes(
  x = mappedData$x, y = mappedData$y,
  color = mappedData$color,
  shape = mappedData$shape,
  size = mappedData$size
))

# Set the plot properties to the previous plot object
plotObject <- config$setPlotProperties(plotObject)

show(plotObject)
