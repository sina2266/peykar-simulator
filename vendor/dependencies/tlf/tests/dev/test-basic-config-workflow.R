# Workflow defining a basic plot configuration

library(ggplot2)
library(tlf)

config <- PlotConfiguration$new(
  title = "Title",
  xlabel = "X-label",
  ylabel = "Y-label"
)

# Create empty plot object
plotObject <- ggplot2::ggplot()

# Every plot configuration can set a watermark layer
plotObject <- config$setWatermark(plotObject)

# Define the labels of titles, legend and axes
plotObject <- config$setPlotLabels(plotObject)
