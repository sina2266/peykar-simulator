# Workflow combining data mapping to a plot configuration

library(ggplot2)
library(tlf)

useTheme(defaultTheme)

load("timeProfileDataExample.RData")

map <- XYEDataMapping$new(
  x = "Time",
  y = "Conc",
  error = "SD"
)

# If data, metaData and data mapping are used as
# input of the configuration, they can be used
config <- PlotConfiguration$new(
  title = "PK Ratio",
  data = observedData,
  metaData = observedMetaData,
  dataMapping = map
)

# Create empty plot object (initiate the plot)
plotObject <- ggplot2::ggplot()

# Define the labels of titles, legend and axes
plotObject <- config$setPlotLabels(plotObject)

# Add the geom (may be replaced by a plot config function)
mappedData <- map$getMappedData(data = observedData, metaData = observedMetaData)

plotObject <- plotObject +
  geom_errorbar(aes(
    x = mappedData$x, ymin = mappedData$ymin, ymax = mappedData$ymax,
    color = mappedData$color,
    size = mappedData$size
  ), show.legend = FALSE)
plotObject <- plotObject + geom_point(aes(
  x = mappedData$x, y = mappedData$y,
  color = mappedData$color,
  shape = mappedData$shape,
  size = mappedData$size
), show.legend = TRUE)

# Set the plot properties to the previous plot object
plotObject <- config$setPlotProperties(plotObject)

show(plotObject)
