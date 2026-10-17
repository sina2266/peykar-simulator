tornadoPopData <- data.frame(
  sensitivity = c(c(1, 2, 3, -1, -2, -3), c(1, 2, 3, -1, -2, -3) - 0.2),
  path = rep(rep(c("liver", "kidney"), each = 3), 2),
  quantile = rep(rep(c("5th", "50th", "95th"), 2), 2),
  population = rep(c("Adults", "Peds"), each = 6)
)

tornadoData <- tornadoPopData[tornadoPopData$population %in% "Adults" &
  tornadoPopData$quantile %in% "50th", ]

# Direct Tornado Plot
plotTornado(
  x = tornadoData$sensitivity,
  y = tornadoData$path
)

# Options/Features
plotTornado(
  x = tornadoData$sensitivity,
  y = tornadoData$path,
  colorPalette = "Dark2"
)

plotTornado(
  x = tornadoData$sensitivity,
  y = tornadoData$path,
  bar = FALSE
)

# Higher level with dataMapping
plotTornado(
  data = tornadoPopData[tornadoPopData$population %in% "Adults", ],
  dataMapping = TornadoDataMapping$new(
    x = "sensitivity",
    y = "path",
    color = "quantile"
  )
)
plotTornado(
  data = tornadoPopData,
  dataMapping = TornadoDataMapping$new(
    x = "sensitivity",
    y = "path",
    color = "quantile"
  ),
  bar = FALSE
)

plotTornado(
  data = tornadoPopData,
  dataMapping = TornadoDataMapping$new(
    x = "sensitivity",
    y = "path",
    color = "quantile",
    shape = "population"
  ),
  bar = FALSE
)
