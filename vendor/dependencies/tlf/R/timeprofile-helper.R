TimeProfileHelper <- R6::R6Class(
  "TimeProfileHelper",
  public = list(
    data = NULL,
    timeColumnName = NULL,
    groupingColumnNames = NULL,
    valuesColumnNames = NULL,
    aggregationFunctionsVector = NULL,
    aggregationFunctionNames = NULL,
    dfHelper = NULL,
    initialize = function(data,
                          timeColumnName,
                          groupingColumnNames = NULL,
                          valuesColumnNames = NULL,
                          aggregationFunctionsVector = NULL,
                          aggregationFunctionNames = NULL) {
      self$data <- data
      self$timeColumnName <- timeColumnName
      self$groupingColumnNames <- groupingColumnNames
      self$valuesColumnNames <- valuesColumnNames
      self$aggregationFunctionsVector <- c(aggregationFunctionsVector)
      self$aggregationFunctionNames <- aggregationFunctionNames
      self$generateAggregatedValues()
    },
    applyAggregationFunctions = function(x) {
      # input the vector of aggregated x into each aggregation function y and return the results in a vector res
      res <- sapply(self$aggregationFunctionsVector, function(y) {
        y(x)
      })
      return(res)
    },
    generateAggregatedValues = function() {
      xGroupingColNames <- c(self$timeColumnName, self$groupingColumnNames) # Get names of grouping columns and groups then into a vector xGroupingColNames
      xGroupingCols <- lapply(xGroupingColNames, function(x) {
        self$data[[x]]
      }) # Extract grouping columns from dataframe and group them into a list called xGroupingCols
      yValuesCol <- self$data[self$valuesColumnNames] # Extract column of values from dataframe
      dfHelper <- aggregate(yValuesCol, xGroupingCols, function(x) {
        res <- self$applyAggregationFunctions(x)
        return(res)
      })

      # For each unique combination of entries in the rows of the grouping columns xGroupingCols, get corresponding values from yValuesCol and group these values into a vector.
      # Then for each such vector apply the functions in the vector aggregationFunctionsVector.  Output over all rows in xGroupingCols and functions in aggregationFunctionsVector forms a matrix
      # xGroupingCols and the output matrix are combined to form one dataframe called dfHelper.

      dfHelper <- cbind(dfHelper[, seq(1, ncol(dfHelper) - 1)], as.data.frame(dfHelper$Simulated)) # Split last (matrix) column of dfHelper into a series of columns, each corresponding to a different function in aggregationFunctionsVector
      colnames(dfHelper)[seq(1, length(xGroupingColNames))] <- xGroupingColNames # Rename columns corresponding to xGroupingCols in dfHelper
      resultCols <- seq(length(xGroupingColNames) + 1, length(xGroupingColNames) + length(self$aggregationFunctionsVector)) # Rename the split columns according to the names of the functions supplied in aggregationFunctionNames
      colnames(dfHelper)[resultCols] <- self$aggregationFunctionNames
      self$dfHelper <- dfHelper
    }
  )
)
