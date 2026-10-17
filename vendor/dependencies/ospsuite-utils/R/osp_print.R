#' Print an object's class name
#'
#' @description
#' Prints the class name of an object with nice formatting using cli.
#'
#' @param x An R object
#' @return Invisibly returns the input object
#' @importFrom cli cli_text cli_format_method
#' @export
#'
#' @family print functions
#'
#' @examples
#' # Print class name of a data frame
#' ospPrintClass(iris)
ospPrintClass <- function(x) {
  # Get class name of the object
  class_name <- class(x)[1]

  # Use cli_format_method to capture output and print to stdout
  output <- cli::cli_format_method({
    # Use inline markup for class name formatting
    # .cls style is nice for highlighting named entities like class names
    cli::cli_text("{.cls {class_name}}")
  })
  cat(output, sep = "\n")

  # Return the input invisibly
  invisible(x)
}

#' Print a header with specified level
#'
#' @description
#' Prints a header with the specified level (H1, H2, or H3) using cli.
#'
#' @param text The text to print as a header
#' @param level The header level (1, 2, or 3)
#' @return Invisibly returns NULL
#' @importFrom cli cli_h1 cli_h2 cli_h3
#' @export
#'
#' @family print functions
#'
#' @examples
#' # Print different header levels
#' ospPrintHeader("Main Title", 1)
#' ospPrintHeader("Section Title", 2)
#' ospPrintHeader("Subsection Title", 3)
ospPrintHeader <- function(text, level = 1) {
  # Input validation
  if (!is.numeric(level) || level < 1 || level > 3 || length(level) != 1) {
    stop("'level' must be 1, 2, or 3")
  }

  # Use cli_format_method to capture output and print to stdout
  output <- cli::cli_format_method({
    # Select header function based on level and call it
    switch(
      level,
      cli::cli_h1("{.strong {text}}"),
      cli::cli_h2("{.strong {text}}"),
      cli::cli_h3("{.strong {text}}")
    )
  })
  cat(output, sep = "\n")

  # Return invisibly
  invisible(NULL)
}

#' Helper function to check if a value is considered "empty"
#'
#' Determines if a value is empty (NULL, NA, empty string, empty vector, or empty list)
#'
#' @param val The value to check
#' @return TRUE if the value is considered empty, FALSE otherwise
#' @keywords internal
.isEmpty <- function(val) {
  if (is.null(val)) {
    return(TRUE)
  }
  if (is.atomic(val) && length(val) == 1) {
    return(is.na(val) || identical(val, ""))
  }
  # Consider empty vectors (length 0) as empty values
  if (is.atomic(val) && length(val) == 0) {
    return(TRUE)
  }
  # Consider empty lists (length 0) as empty values
  if (is.list(val) && length(val) == 0) {
    return(TRUE)
  }
  return(FALSE)
}

#' Format a value for display
#'
#' Converts a value into a string representation that properly shows its type
#'
#' @param value The value to format
#' @return A string representation of the value
#' @keywords internal
.formatValue <- function(value) {
  if (is.null(value)) {
    return("NULL")
  }
  if (is.atomic(value) && length(value) == 1 && is.na(value)) {
    return("NA")
  }
  if (is.atomic(value) && length(value) == 1 && identical(value, "")) {
    return("<empty string>")
  }
  if (is.atomic(value) && length(value) == 0) {
    return("<empty vector>")
  }
  if (is.list(value) && length(value) == 0) {
    return("<empty list>")
  }
  if (is.vector(value) && length(value) > 1) {
    return(paste(value, collapse = ", "))
  }
  return(as.character(value))
}

#' Print a list of items with an optional title
#'
#' @description
#' Prints a list of items from a vector or list, with an optional title.
#' Items are indented and prefixed with a dash.
#'
#' @param x A vector or list
#' @param title Optional title to display before the list (default: NULL)
#' @param print_empty Whether to print empty values (NULL, NA, empty string) (default: FALSE)
#' @return Invisibly returns the input object
#' @importFrom cli cli_text cli_ol cli_li cli_end
#' @export
#'
#' @family print functions
#'
#' @examples
#' # Print a simple vector with title
#' vector <- c("A", "B", "C")
#' ospPrintItems(vector, title = "Letters")
#'
#' # Print a named vector with title
#' named_vector <- c(A = 1, B = 2, C = 3)
#' ospPrintItems(named_vector, title = "Letters")
#'
#' # Print a list including empty values
#' list_with_nulls <- list("Min" = NULL, "Max" = 100, "Unit" = NA)
#' ospPrintItems(list_with_nulls, title = "Parameters", print_empty = TRUE)
ospPrintItems <- function(x, title = NULL, print_empty = FALSE) {
  # if x is empty and no title provided, return invisibly
  if (.isEmpty(x) && is.null(title)) {
    return(invisible(x))
  }

  # Count items within x that will be printed and check if all are "empty"
  items_to_print <- 0
  all_items_empty <- TRUE

  # Check all items first (only if x is not empty)
  if (!.isEmpty(x)) {
    for (i in seq_along(x)) {
      value <- x[[i]]
      if (!.isEmpty(value)) {
        all_items_empty <- FALSE
        items_to_print <- items_to_print + 1
      } else if (print_empty) {
        items_to_print <- items_to_print + 1
      }
    }

    # If no items to print and title is not provided, just return invisibly
    if (all_items_empty && is.null(title) && !print_empty) {
      return(invisible(x))
    }
  }

  # Use cli_format_method to capture output and print to stdout
  output <- cli::cli_format_method({
    # Print title if provided
    if (!is.null(title)) {
      cli::cli_text("{.strong {title}}:")
    }

    # Handle empty containers
    if (.isEmpty(x)) {
      # For empty containers, just print the title (already handled above)
      # No additional content needed
    } else if (all_items_empty && !is.null(title) && !print_empty) {
      # Special case: all items are empty and title is provided
      # When print_empty is FALSE, show the message
      # Start the list
      cli::cli_div(
        theme = list(
          ul = list(
            "margin-left" = 2 # Add indentation
          )
        )
      )
      cli::cli_li("All items are NULL, NA, or empty")
      cli::cli_end()
    } else if (!.isEmpty(x)) {
      # Process non-empty containers
      # Start the list
      cli::cli_div(
        theme = list(
          ul = list(
            "margin-left" = 2 # Add indentation
          )
        )
      )

      # If input is a named vector/list
      if (!is.null(names(x)) && any(names(x) != "")) {
        # For each item in the list
        for (i in seq_along(x)) {
          name <- names(x)[i]
          value <- x[[i]]

          # Skip empty values if print_empty is FALSE
          if (.isEmpty(value) && !print_empty) {
            next
          }

          # Format value using format_value function
          formatted_value <- .formatValue(value)

          # Apply markup to values
          if (!is.na(name) && name != "") {
            cli::cli_li("{name}: {formatted_value}")
          } else {
            # No name, just value
            cli::cli_li("{formatted_value}")
          }
        }
      } else {
        # For unnamed vectors or lists
        for (i in seq_along(x)) {
          value <- x[[i]]

          # Skip empty values if print_empty is FALSE
          if (.isEmpty(value) && !print_empty) {
            next
          }

          # Format value using format_value function
          formatted_value <- .formatValue(value)

          # Apply markup to values if specified
          cli::cli_li("{formatted_value}")
        }
      }

      # End the list
      cli::cli_end()
    }
  })
  cat(output, sep = "\n")

  # Return input invisibly
  invisible(x)
}
