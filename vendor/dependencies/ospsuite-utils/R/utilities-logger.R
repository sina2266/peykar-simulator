#' @title tic
#' @description Trigger time tracker
#' @return System time
#' @export
#' @examples
#' tic()
#'
tic <- function() {
  return(Sys.time())
}

#' @title toc
#' @description
#' Get elapsed time between `tic` trigger and now
#' @param tic Start time
#' @param unit display unit of elapsed time
#' @return Character displaying elapsed time in `unit`
#' @export
#' @examples
#' t0 <- tic()
#' Sys.sleep(2)
#' # Get elapsed time in seconds
#' toc(t0, "s")
#' # Get elapsed time in minutes
#' toc(t0, "min")
toc <- function(tic, unit = "min") {
  elapsedTime <- difftime(
    Sys.time(),
    tic,
    # Use switch to map between difftime and ospsuite units
    units = switch(unit, "h" = "hours", "min" = "mins", "s" = "secs", unit)
  )
  return(sprintf("%.1f %s", elapsedTime, unit))
}

#' @title setLogFolder
#' @description
#' Initialize logs and their settings
#' @param logFolder Optional folder path to save log file
#' @export
#' @keywords logging
#' @import logger
setLogFolder <- function(logFolder = NULL) {
  # validateIsCharacter(logFolder, allowNull = TRUE)
  ospsuiteUtilsEnv$logging$folder <- logFolder
  logger::log_threshold(logger::INFO, index = 1)
  logger::log_formatter(logger::formatter_paste, index = 1)
  logger::log_layout(consoleLayout, index = 1)
  logger::log_appender(logger::appender_stdout, index = 1)

  if (is.null(logFolder)) {
    if (logger::log_indices() > 1) {
      logger::delete_logger_index(index = 2)
    }
    return(invisible())
  }
  logFile <- file.path(logFolder, "log.txt")
  logger::log_threshold(logger::DEBUG, index = 2)
  logger::log_formatter(logger::formatter_paste, index = 2)
  logger::log_layout(fileLayout, index = 2)
  logger::log_appender(logger::appender_file(logFile), index = 2)
  return(invisible())
}

#' @title setErrorMasking
#' @description Mask error trace messages
#' @param patterns
#' Character patterns to identify with `grepl()` when masking messages
#' @export
#' @keywords logging
#' @examples \dontrun{
#' setErrorMasking(c("tryCatch", "withCallingHandlers"))
#' }
setErrorMasking <- function(patterns) {
  ospsuiteUtilsEnv$logging$errorMasking <- patterns
  return(invisible())
}

#' @title setWarningMasking
#' @description Mask warning messages
#' @param patterns
#' Character patterns to identify with `grepl()` when masking messages
#' @export
#' @keywords logging
#' @examples \dontrun{
#' # Mask ggplot2 warning message when missing values are found
#' setWarningMasking("rows containing missing values")
#' }
setWarningMasking <- function(patterns) {
  ospsuiteUtilsEnv$logging$warningMasking <- patterns
  return(invisible())
}

#' @title setInfoMasking
#' @description Mask info messages
#' @param patterns
#' Character patterns to identify with `grepl()` when masking messages
#' @export
#' @keywords logging
#' @examples \dontrun{
#' # Mask ggplot2 message when line is used with 1 value per group
#' setInfoMasking("Each group consists of only one observation")
#' }
setInfoMasking <- function(patterns) {
  ospsuiteUtilsEnv$logging$infoMasking <- patterns
  return(invisible())
}

#' @title getLogFolder
#' @description Get current log folder where logs are saved
#' @export
#' @keywords logging
#' @examples \dontrun{
#' # Set/get log folder to a temporary directory
#' setLogFolder(tempdir())
#' getLogFolder()
#'
#' # Set/get logFolder to `NULL`, cancel saving of logs
#' setLogFolder()
#' getLogFolder()
#' }
getLogFolder <- function() {
  return(ospsuiteUtilsEnv$logging$folder)
}

#' @title logInfo
#' @description
#' Log information with time stamp accounting for message type.
#' Message type will point toward the most appropriate `cli` function display.
#' @param msg Character values of message to log that leverages `cli` formatting.
#' @param type Name of the message type to toward best `cli` display:
#' - `"info"`: uses `cli::cli_alert_info()`
#' - `"success"`: uses `cli::cli_alert_success()`
#' - `"h1"`: uses `cli::cli_h1()`
#' - `"h2"`: uses `cli::cli_h2()`
#' - `"h3"`: uses `cli::cli_h3()`
#' - `"text"`: uses `cli::cli_text()`
#' - `"alert"`: uses `cli::cli_alert()`
#' - `"li"`: uses `cli::cli_li()`
#' - `"ol"`: uses `cli::cli_ol()`
#' - `"progress_step"`: uses `cli::cli_progress_step()`
#' @export
#' @keywords logging
#' @examples
#' # Log information
#' logInfo(cliFormat("This is an {.strong info} message"))
#'
#' # Log a title
#' logInfo(cliFormat("Task: {.strong tic toc test}"), type = "h1")
#'
#' # Log success
#' t0 <- tic()
#' Sys.sleep(3)
#' logInfo(cliFormat("Task: {.strong tic toc test} completed [{toc(t0, \"s\")}]"), type = "success")
#'
logInfo <- function(msg, type = "info") {
  if (
    isIncluded(
      type,
      c("h1", "h2", "h3", "text", "alert", "li", "ol", "progress_step")
    )
  ) {
    switch(
      type,
      "h1" = cli::cli_h1(msg),
      "h2" = cli::cli_h2(msg),
      "h3" = cli::cli_h3(msg),
      "text" = cli::cli_text(msg),
      "alert" = cli::cli_alert(msg),
      "li" = cli::cli_li(msg),
      "ol" = cli::cli_ol(msg),
      "progress_step" = cli::cli_progress_step(msg)
    )
    logger::log_debug(msg)
    return(invisible())
  }
  if (type %in% "success") {
    logger::log_success(msg)
    return(invisible())
  }
  logger::log_info(msg)
  return(invisible())
}

#' @title logWarning
#' @description
#' Log warning with time stamp
#' @param msg Character values of message to log that leverages `cli` formatting.
#' @export
#' @keywords logging
#' @examples
#' # Log warning
#' logWarning(cliFormat("This is a {.strong warning} message"))
#'
logWarning <- function(msg) {
  logger::log_warn(msg)
  return(invisible())
}

#' @title logDebug
#' @description
#' Log debug with time stamp
#' @param msg Character values of message to log that leverages `cli` formatting.
#' @export
#' @keywords logging
#' @examples
#' # Log debug
#' logDebug("This is a debugging message")
#'
logDebug <- function(msg) {
  logger::log_debug(msg)
  return(invisible())
}

#' @title logError
#' @description
#' Log error with time stamp
#' @param msg Character values of message to log that leverages `cli` formatting.
#' @export
#' @keywords logging
#' @examples
#' \dontshow{
#' # Initialize logger settings
#' setLogFolder()
#' }
#' # Log error
#' logError(cliFormat("This is an {.strong error} message"))
#'
#' # Log error with indications
#' logError(cliFormat(
#'   "This is an {.strong error} message",
#'   "Check these {.val values} or this {.fn function}"
#' ))
#'
logError <- function(msg) {
  logger::log_error(msg)
  return(invisible())
}

# Initialize logger settings and default masking
setLogFolder()

# Initialize default error masking
setErrorMasking(
  patterns = c(
    "logCatch",
    "qualificationCatch",
    "stop",
    "tryCatch",
    "withCallingHandlers",
    "simpleError",
    "eval\\(ei, envir\\)"
  )
)
