#' @title getPKRatioMeasure
#' @description
#' Get a summary table of PK Ratio within specific limits
#'
#' @inheritParams plotPKRatio
#' @param ratioLimits Numeric positive values limits
#' @return A data.frame of summary of PK Ratio within specific limits
#' @export
#' @examples
#' # Get summary of usual PK Ratio limits
#' pkData <- data.frame(x = c(1, 2, 1, 2, 3), y = c(5, 0.2, 2, 3, 4))
#'
#' getPKRatioMeasure(data = pkData, dataMapping = PKRatioDataMapping$new(x = "x", y = "y"))
#'
#' # Get summary of other PK Ratio limits
#' getPKRatioMeasure(
#'   data = pkData,
#'   dataMapping = PKRatioDataMapping$new(x = "x", y = "y"),
#'   ratioLimits = seq(1.5, 5, 0.5)
#' )
#'
getPKRatioMeasure <- function(data, dataMapping = NULL, ratioLimits = c(1.5, 2)) {
  if (is.numeric(data)) {
    PKratios <- data[!is.na(data)]
  } else {
    # If no data mapping is input, use default
    dataMapping <- dataMapping %||% PKRatioDataMapping$new(data = data)

    stopifnot("PKRatioDataMapping" %in% class(dataMapping))

    y <- dataMapping$y

    # Remove NA values
    PKratios <- data[!is.na.data.frame(data[, y]), y]
  }

  PointsTotal <- length(PKratios)

  PKRatioMeasure <- data.frame("Number" = PointsTotal, "Ratio" = NA, row.names = "Points Total")

  for (limit in ratioLimits) {
    PointsWithin <- sum(PKratios <= limit & PKratios >= 1 / limit)
    PKRatioMeasureLim <- data.frame("Number" = PointsWithin, "Ratio" = PointsWithin / PointsTotal, row.names = paste("Points within ", limit, "-fold", sep = ""))

    PKRatioMeasure <- rbind.data.frame(PKRatioMeasure, PKRatioMeasureLim)
  }

  return(PKRatioMeasure)
}
