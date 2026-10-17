#' @title object to map data for rangeplots
#' @description R6 class for mapping variable to `data`
#' @export
#' @family MappedData classes
MappedDataRangeDistribution <- R6::R6Class( # nolint
  "MappedDataRangeDistribution",
  inherit = MappedData,
  public = list(
    #' @field xScale scale of x axis
    xScale = NULL,

    #' @param data data.frame used for mapping
    #' @param mapping list of aesthetic mappings
    #' @param groupAesthetics vector of aesthetics, which are used for columns mapped with aesthetic `groupby`
    #' @param direction direction of plot either "x" or "y"
    #' @param isObserved A `boolean` if TRUE mappings mdv, lloq, error and error_relative are evaluated
    #' @param xlimits limits for x-axis (may be NULL)
    #' @param ylimits limits for y-axis (may be NULL)
    #' @param xScale scale of x-axis either 'linear' or 'log'
    #' @param yScale scale of y-axis either 'linear' or 'log'
    #' @param modeOfBinning method of binning (e.g., 'breaks', 'number', 'interval')
    #' @param numberOfBins number of bins to use for binning
    #' @param breaks breaks for binning if `modeOfBinning` is 'breaks'
    #' @description Create a new `MappedDataRangeDistribution` object
    #' @return `MappedDataRangeDistribution` class object
    initialize = function(data,
                          mapping,
                          groupAesthetics = NULL,
                          direction = "y",
                          isObserved = TRUE,
                          xlimits = NULL,
                          ylimits = NULL,
                          xScale = "linear",
                          yScale = "linear",
                          modeOfBinning = NA,
                          numberOfBins = NA,
                          breaks = NA) {
      super$initialize(
        data = data,
        mapping = mapping,
        groupAesthetics = groupAesthetics,
        direction = direction,
        isObserved = isObserved,
        xlimits = xlimits,
        ylimits = ylimits,
        xScale = xScale,
        yScale = yScale
      )

      # add specifics for rangeplots
      match.arg(modeOfBinning, unlist(BINNINGMODE, use.names = FALSE), several.ok = FALSE)
      if (modeOfBinning == BINNINGMODE$breaks) {
        checkmate::assertNumeric(breaks, any.missing = FALSE)
        private$breaks <- sort(unique(breaks))
        numberOfBins <- length(private$breaks)
      } else {
        checkmate::assertIntegerish(numberOfBins, lower = 2, any.missing = FALSE, len = 1)
      }

      private$modeOfBinning <- modeOfBinning
      private$numberOfBins <- numberOfBins
      private$breaks <- breaks
      self$xScale <- xScale
    },

    #' @description Set binning columns. This method sets the bins for the data based on the specified mode of binning.
    #' @return The object itself (invisible)
    setBins = function() {
      if (private$modeOfBinning == BINNINGMODE$number) {
        functionName <- "cut_number"
        functionArgs <- list(n = private$numberOfBins)
      } else if (private$modeOfBinning == BINNINGMODE$interval) {
        if (self$xScale == "log") {
          cutIntervalLog <- function(x, ...) {
            cut_interval(log(x), ...)
          }
          functionName <- "cutIntervalLog"
        } else {
          functionName <- "cut_interval"
        }
        functionArgs <- list(n = private$numberOfBins)
      } else if (private$modeOfBinning == BINNINGMODE$breaks) {
        functionName <- "cut"
        functionArgs <- list(breaks = private$breaks, include.lowest = TRUE)
      }
      self$data <- self$data |>
        dplyr::mutate(.bin = do.call(
          what = functionName,
          args = c(
            list(
              x = !!self$mapping[["x"]],
              right = FALSE,
              ordered_result = TRUE
            ),
            functionArgs
          )
        ))

      return(invisible(self))
    },

    #' @description Create a data table with bin border information.
    #' This method sets up a data table containing border information for the bins.
    #' @param identifier Identifier for the data table (default is 'IndividualId')
    setBorderDataTable = function(identifier = "IndividualId") {
      checkmate::assertNames(c(".bin", identifier), subset.of = names(self$data))

      # get datatable with unique x values
      tmp <- copy(self$data) |>
        dplyr::select(dplyr::all_of(c(".bin", identifier)))
      tmp$.x <- private$getDataForAesthetic(aesthetic = "x")
      tmp <- tmp |>
        unique() |>
        setDT()

      borders <- tmp[!is.na(.bin), .(
        N = .N,
        minValue = min(.x),
        medianX = median(.x),
        maxValue = max(.x)
      ),
      by = ".bin"
      ]

      setorderv(borders, "minValue")

      # duplicate last row to determine right border of table/plot
      borders <- rbind(borders, borders[nrow(borders)])
      borders[nrow(borders), maxValue := maxValue * 1.001]
      borders[nrow(borders), minValue := maxValue]

      # Set breaks based on the mode of binning
      if (private$modeOfBinning == BINNINGMODE$breaks) {
        # If the mode is 'breaks', directly use the predefined breaks
        borders$breaks <- private$breaks
      } else {
        # If not, duplicate the first row to support the calculation of the minimal break
        borders <- rbind(borders[1], borders)
        borders[1, minValue := minValue / 1.001]
        borders[1, maxValue := minValue]

        # Calculate raw breaks as the average of the minValue and the previous maxValue
        borders[, breaksRaw := (minValue + shift(maxValue, type = "lag")) / 2]

        # Calculate the difference between consecutive min and max values
        borders[, diff := minValue - shift(maxValue, type = "lag")]

        # Calculate an adjustment factor for rounding based on the difference
        borders[, diffN := 10^ceiling(-log10(diff))]

        # Round the breaks to the nearest adjusted value
        borders[, breaks := round(breaksRaw * diffN) / diffN]

        # Remove the first row used for calculations
        borders <- borders[-1, ]
        # Clean up the temporary columns used for calculations
        borders[, breaksRaw := NULL]
        borders[, diff := NULL]
        borders[, diffN := NULL]
      }

      private$.borders <- borders
    },

    #' @description Set x mapping for the plot.
    #' This method sets the x mapping for the plot based on the specified parameters.
    #' @param asStepPlot Logical indicating if the plot should be a step plot.
    setXMapping = function(asStepPlot) {
      private$asStepPlot <- asStepPlot
      if (asStepPlot) {
        tmp <- setDT(copy(self$border))
        if (self$xScale == "linear") {
          tmp[, breaksRight := shift(breaks, type = "lead") - 0.001 * (maxValue - minValue)]
        } else {
          tmp[, breaksRight := exp(log(shift(breaks, type = "lead")) - 0.001 * (log(maxValue) - log(minValue)))]
        }
        tmp <- tmp[!is.na(breaksRight)]

        self$data <- rbind(
          self$data |>
            merge(tmp[, c(".bin", "breaks")],
              by = ".bin"
            ),
          self$data |>
            merge(tmp[, c(".bin", "breaksRight")],
              by = ".bin"
            ) |>
            setnames(old = "breaksRight", new = "breaks"),
          setDT(self$data)[is.na(.bin)] |>
            dplyr::mutate(breaks = min(self$border$breaks, na.rm = TRUE)),
          setDT(self$data)[is.na(.bin)] |>
            dplyr::mutate(breaks = max(self$border$breaks, na.rm = TRUE))
        )

        private$addOverwriteAes(aes(x = breaks))
      } else {
        self$data <- rbind(
          self$data |>
            merge(self$border[, c(".bin", "medianX")],
              by = ".bin"
            ),
          setDT(self$data)[is.na(.bin)] |>
            dplyr::mutate(medianX = min(self$border$medianX, na.rm = TRUE)),
          setDT(self$data)[is.na(.bin)] |>
            dplyr::mutate(medianX = max(self$border$medianX, na.rm = TRUE))
        )

        private$addOverwriteAes(aes(x = medianX))
      }
    }
  ),

  ## active -------
  active = list(
    #' @field border borders of the binning.
    border = function() {
      return(private$.borders)
    }
  ),

  ## private -------
  private = list(
    modeOfBinning = NA,
    numberOfBins = NA,
    breaks = NA,
    .borders = NA,
    asStepPlot = NA
  )
)
