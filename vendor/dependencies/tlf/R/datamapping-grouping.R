#' @title Grouping
#' @description  R6 class for mapping a `group` of variable(s) and their `label` to `data`
#' @export
#' @family DataMapping classes
Grouping <- R6::R6Class(
  "Grouping",
  public = list(
    #' @field group data.frame or character defining the groups or group variables to group by
    group = NULL,
    #' @field label character printed name of the `grouping`
    label = NULL,

    #' @description Create a new `Grouping` object
    #' @param group data.frame or character vector of groups
    #' @param label character name of the `group`
    #' @return A new `Grouping` object
    initialize = function(group, label = NULL) {
      validateIsOfType(group, c("data.frame", "character"), nullAllowed = TRUE)

      self$group <- group

      if (is.data.frame(group)) {
        # For data.frame, label is last column as default
        self$label <- utils::tail(names(group), 1)
      }
      if (is.character(group)) {
        self$label <- paste(group, collapse = "-")
      }

      self$label <- label %||% self$label
    },

    #' @description Get the caption associated to each `group`
    #' @param data data.frame to map
    #' @param metaData list of information on the `data`
    #' @return A vector of characters containing the captions associated to each `group` of `data`
    getCaptions = function(data, metaData = NULL) {
      validateIsOfType(self$group, c("data.frame", "character"), nullAllowed = TRUE)

      if (is.data.frame(self$group)) {
        captions <- .getCustomCaptions(data, self$group)
      }
      if (is.character(self$group)) {
        captions <- getDefaultCaptions(data, metaData, variableList = self$group)
      }
      return(captions)
    }
  )
)
