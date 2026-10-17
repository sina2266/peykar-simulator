#' @title Printable
#' @docType class
#'
#' @description
#' Base class that implements some basic properties for printing to console.
#'
#' @importFrom R6 R6Class
#' @importFrom lifecycle deprecate_warn
#'
#' @examples
#' myPrintable <- R6::R6Class(
#'   "myPrintable",
#'   inherit = Printable,
#'   public = list(
#'     x = NULL,
#'     y = NULL,
#'     print = function() {
#'       private$printClass()
#'       private$printLine("x", self$x)
#'       private$printLine("y", self$y)
#'       invisible(self)
#'     }
#'   )
#' )
#'
#' x <- myPrintable$new()
#' x
#' @export
Printable <- R6::R6Class(
  "Printable",
  cloneable = TRUE,
  public = list(
    #' @description
    #' Create a new Printable object.
    initialize = function() {
      private$deprecated()
    }
  ),
  private = list(
    deprecated = function() {
      lifecycle::deprecate_warn(
        when = "1.6.2",
        what = I("ospsuite.utils::Printable"),
        with = I("ospsuite.utils::ospPrint*()")
      )
    },
    printLine = function(entry, value = NULL, addTab = TRUE) {
      private$deprecated()
      entries <- paste0(entry, ":", sep = "")

      # helps to visually distinguish class name from its entries
      if (addTab) {
        entries <- c("  ", entries)
      }

      value <- format(value)
      entries <- c(entries, value, "\n")
      cat(entries, sep = " ")
      invisible(self)
    },
    printClass = function() {
      private$deprecated()
      cat(class(self)[1], ": \n", sep = "")
    }
  )
)
