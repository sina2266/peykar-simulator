#' @title GroupMapping
#' @description  R6 class for mapping `Grouping` variables to `data`
#' @export
#' @family DataMapping classes
GroupMapping <- R6::R6Class(
  "GroupMapping",
  public = list(
    #' @field color R6 class `Grouping` object
    color = NULL,
    #' @field fill R6 class `Grouping` object
    fill = NULL,
    #' @field linetype R6 class `Grouping` object
    linetype = NULL,
    #' @field shape R6 class `Grouping` object
    shape = NULL,
    #' @field size R6 class `Grouping` object
    size = NULL,

    #' @description Create a new `GroupMapping` object
    #' @param color R6 class `Grouping` object or its input
    #' @param fill R6 class `Grouping` object or its input
    #' @param linetype R6 class `Grouping` object or its input
    #' @param shape R6 class `Grouping` object or its input
    #' @param size R6 class `Grouping` object or its input
    #' @return A new `GroupMapping` object
    initialize = function(color = NULL,
                          fill = NULL,
                          linetype = NULL,
                          shape = NULL,
                          size = NULL) {
      # If mappings are not of type Grouping, they are assumed as Grouping inputs
      groupingInputs <- c("color", "fill", "linetype", "shape", "size")
      eval(parse(text = paste0(
        "if(!isOfType(", groupingInputs, ", 'Grouping')){",
        groupingInputs, " <- Grouping$new(group = ", groupingInputs, ")}"
      )))
      eval(.parseVariableToObject("self", groupingInputs, keepIfNull = FALSE))
    }
  )
)
