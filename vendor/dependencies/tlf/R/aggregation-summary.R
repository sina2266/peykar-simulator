#' @title AggregationSummary
#' @description R6 class to split a data.frame data into subsets defined by unique combinations of elements
#' in the columns `xColumnNames` and `groupingColumnNames`.
#' Applies functions defined in `aggregationFunctionsVector` to column `yColumnNames.`
#' Returns a list of data.frame, one data.frame for each function listed in `aggregationFunctionsVector`.
#' Each data.frame in list element is named after the function's corresponding string in `aggregationFunctionNames`.
#' The summary statistic column name in each data.frame is the same as the name of the data.frame in the returned list.
#' @export
AggregationSummary <- R6::R6Class(
  "AggregationSummary",
  public = list(
    #' @field data data.frame
    data = NULL,
    #' @field metaData list of information on `data`
    metaData = NULL,
    #' @field xColumnNames character names of grouping variables
    xColumnNames = NULL,
    #' @field groupingColumnNames character names of grouping variables
    groupingColumnNames = NULL,
    #' @field yColumnNames character names of dependent variables (that are grouped)
    yColumnNames = NULL,
    #' @field aggregationInputsVector list of R6 class `AggregationInput` objects
    aggregationInputsVector = NULL,
    #' @field aggregationFunctionsVector list of functions to use for aggregation
    aggregationFunctionsVector = NULL,
    #' @field aggregationFunctionNames vector of function names that will be used as variable name of the aggregation
    aggregationFunctionNames = NULL,
    #' @field aggregationUnitsVector character vector of units of aggregation output
    aggregationUnitsVector = NULL,
    #' @field aggregationDimensionsVector character vector of dimensions of aggregation output
    aggregationDimensionsVector = NULL,
    #' @field dfHelper data.frame of aggregated values
    dfHelper = NULL,
    #' @field metaDataHelper list of information on `dfHelper`
    metaDataHelper = NULL,

    #' @description Create a new `AggregationSummary` object
    #' @param data data.frame
    #' @param metaData list of information on `data`
    #' @param xColumnNames character names of grouping variables
    #' @param groupingColumnNames character names of grouping variables
    #' @param yColumnNames character names of dependent variables (that are grouped)
    #' @param aggregationInputsVector list of R6 class `AggregationInput` objects
    #' @param aggregationFunctionsVector list of functions to use for aggregation
    #' @param aggregationFunctionNames vector of function names that will be used as variable name of the aggregation
    #' @param aggregationUnitsVector character vector of units of aggregation output
    #' @param aggregationDimensionsVector character vector of dimensions of aggregation output
    #' @return A new `AggregationSummary` object
    initialize = function(data,
                          metaData = NULL,
                          xColumnNames = NULL,
                          groupingColumnNames = NULL,
                          yColumnNames = NULL,
                          aggregationInputsVector = NULL,
                          aggregationFunctionsVector = NULL,
                          aggregationFunctionNames = NULL,
                          aggregationUnitsVector = NULL,
                          aggregationDimensionsVector = NULL) {
      self$data <- data
      self$metaData <- metaData
      self$xColumnNames <- xColumnNames
      self$groupingColumnNames <- groupingColumnNames
      self$yColumnNames <- yColumnNames

      if (!is.null(aggregationInputsVector)) {
        for (aggregationInputValue in aggregationInputsVector) {
          stopifnot(is(aggregationInputValue, "AggregationInput"))

          self$aggregationFunctionsVector <- append(self$aggregationFunctionsVector, aggregationInputValue$aggregationFunction)
          self$aggregationFunctionNames <- append(self$aggregationFunctionNames, aggregationInputValue$aggregationFunctionName)
          self$aggregationUnitsVector <- append(self$aggregationUnitsVector, aggregationInputValue$aggregationUnit)
          self$aggregationDimensionsVector <- append(self$aggregationDimensionsVector, aggregationInputValue$aggregationDimension)
        }
      } else {
        self$aggregationFunctionsVector <- c(aggregationFunctionsVector)
        self$aggregationFunctionNames <- aggregationFunctionNames
        self$aggregationUnitsVector <- aggregationUnitsVector
        self$aggregationDimensionsVector <- aggregationDimensionsVector
      }
      self$generateAggregatedValues()
    },

    #' @description Apply aggregation functions on `x`
    #' @param x numeric vector
    #' @return A list or vector of aggregated values
    applyAggregationFunctions = function(x) {
      # input the vector of aggregated x into each aggregation function y and return the results in a vector res
      res <- sapply(self$aggregationFunctionsVector, function(y) {
        y(x)
      })
      return(res)
    },

    #' @description Generate aggregated values
    #' @return A list or vector of aggregated values
    generateAggregatedValues = function() {
      xGroupingColNames <- c(self$xColumnNames, self$groupingColumnNames) # Get names of grouping columns and groups then into a vector xGroupingColNames

      xGroupingCols <- self$data[xGroupingColNames] # Extract grouping columns from dataframe and group them into a list called xGroupingCols

      yValuesCol <- self$data[self$yColumnNames] # Extract column of values from dataframe

      aggSummaries <- aggregate(yValuesCol, xGroupingCols, function(x) {
        res <- self$applyAggregationFunctions(x)
        return(res)
      })

      summaryMatrix <- matrix(aggSummaries[[self$yColumnNames]], ncol = length(self$aggregationFunctionsVector))
      self$dfHelper <- aggSummaries[xGroupingColNames]
      self$metaDataHelper <- self$metaData[xGroupingColNames]

      for (n in seq(1, length(self$aggregationFunctionNames))) {
        dF <- data.frame(summaryMatrix[, n])
        colnames(dF)[1] <- self$aggregationFunctionNames[n]
        self$dfHelper <- cbind(self$dfHelper, dF)

        self$metaDataHelper[[self$aggregationFunctionNames[n]]] <- list(unit = self$aggregationUnitsVector[n], dimension = self$aggregationDimensionsVector[n])
      }
    }
  )
)
