test_that("tic toc time recording", {
  t0 <- tic()
  expect_match(toc(t0, "s"), "\\d\\.\\d s")
  expect_match(toc(t0, "min"), "\\d\\.\\d min")

  expect_match(
    timeStamp(),
    # Date - Time
    "\\d\\d/\\d\\d/\\d\\d\\d\\d - \\d\\d\\:\\d\\d\\:\\d\\d"
  )
})

test_that("setLogFolder initialize/terminate logger appropriately", {
  setLogFolder("./")
  expect_equal(getLogFolder(), "./")
  expect_false(file.exists("./log.txt"))
  setLogFolder("./")
  logInfo("This is an info message")
  expect_true(file.exists("./log.txt"))
  unlink("./log.txt")
  setLogFolder(NULL)
  expect_null(getLogFolder())
  logInfo("This is an info message")
  expect_false(file.exists("./log.txt"))
})

setLogFolder("./")

test_that("logInfo displays, logs and appends info messages", {
  # Display
  expect_message(
    logInfo("This is a first info message"),
    "(Info)*(a first info message)"
  )
  expect_message(
    logInfo("This is a second info message"),
    "(Info)*(This is a second info message)"
  )
  logContent <- readLines("./log.txt")
  # Logs and appends
  expect_match(logContent[1], "(INFO)*(This is a first info message)")
  expect_match(logContent[2], "(INFO)*(This is a second info message)")
})

test_that("logWarning displays, logs and appends warning messages", {
  # Display
  expect_message(
    logWarning("This is a first warning message"),
    "(Warning)*(a first warning message)"
  )
  expect_message(
    logWarning("This is a second warning message"),
    "(Warning)*(This is a second warning message)"
  )
  logContent <- readLines("./log.txt")
  # Logs and appends
  expect_match(logContent[3], "(WARN)*(This is a first warning message)")
  expect_match(logContent[4], "(WARN)*(This is a second warning message)")
})

test_that("logError displays, logs and appends error messages", {
  # Display
  expect_message(
    logError("This is a first error message"),
    "(Error)*(a first error message)"
  )
  expect_message(
    logError("This is a second error message"),
    "(Error)*(This is a second error message)"
  )
  logContent <- readLines("./log.txt")
  # Logs and appends
  expect_match(logContent[5], "(ERROR)*(This is a first error message)")
  expect_match(logContent[6], "(ERROR)*(This is a second error message)")
})

test_that("logDebug logs and appends debug messages but does not display them", {
  # Display
  expect_silent(logDebug("This is a first debug message"))
  expect_silent(logDebug("This is a second debug message"))
  logContent <- readLines("./log.txt")
  # Logs and appends
  expect_match(logContent[7], "(DEBUG)*(This is a first debug message)")
  expect_match(logContent[8], "(DEBUG)*(This is a second debug message)")
})

unlink("./log.txt")

test_that("cli formatting displays message content", {
  # Display
  expect_message(
    logInfo("This is a title message", type = "h1"),
    "This is a title message"
  )
  expect_message(
    logInfo("This is a second title message", type = "h2"),
    "This is a second title message"
  )
  expect_message(
    logInfo("This is a third title message", type = "h3"),
    "This is a third title message"
  )
  expect_message(
    logInfo("This is a text message", type = "text"),
    "This is a text message"
  )
  expect_message(
    logInfo("This is an alert message", type = "alert"),
    "This is an alert message"
  )
  expect_message(
    logInfo("This is a list message", type = "li"),
    "This is a list message"
  )
  expect_message(
    logInfo("This is a numbered message", type = "ol"),
    "This is a numbered message"
  )
  expect_message(
    logInfo("This is a progress step message", type = "progress_step"),
    "This is a progress step message"
  )
  expect_message(
    logInfo("This is a success message", type = "success"),
    "(Info)*(This is a success message)"
  )
  x <- "ospsuite"
  cliMsg <- cliFormat("I am a cli message showing {x}")
  expect_match(cliMsg, "I am a cli message showing ospsuite")
  expect_message(
    logInfo(cliMsg),
    "(Info)*(I am a cli message showing ospsuite)"
  )
  expect_message(
    logInfo(c("Main message", "Helper indications")),
    "(Info)*(Main message)*(Helper indications)"
  )
  logContent <- readLines("./log.txt")
  logContent <- logContent[logContent != ""]
  # Logs and appends
  expect_match(logContent[1], "(DEBUG)*(This is a title message)")
  expect_match(logContent[2], "(DEBUG)*(This is a second title message)")
  expect_match(logContent[3], "(DEBUG)*(This is a third title message)")
  expect_match(logContent[4], "(DEBUG)*(This is a text message)")
  expect_match(logContent[5], "(DEBUG)*(This is an alert message)")
  expect_match(logContent[6], "(DEBUG)*(This is a list message)")
  expect_match(logContent[7], "(DEBUG)*(This is a numbered message)")
  expect_match(logContent[8], "(DEBUG)*(This is a progress step message)")
  expect_match(logContent[9], "(SUCCESS)*(This is a success message)")
  expect_match(logContent[10], "(INFO)*(I am a cli message showing ospsuite)")
  expect_match(logContent[11], "(INFO)*(Main message)")
  expect_match(logContent[12], "Helper indications")
})

unlink("./log.txt")

test_that("logCatch displays according to masking and appends everything in the logs", {
  setInfoMasking("Info to hide")
  setWarningMasking("Warning to hide")
  setErrorMasking("Error to hide")

  expect_message(
    logCatch(message("Info to show")),
    "(Info)*(Info to show)"
  )
  expect_silent(logCatch(message("Info to hide")))
  expect_message(
    logCatch(warning("Warning to show")),
    "(Warning)*(Warning to show)"
  )
  expect_silent(logCatch(warning("Warning to hide")))

  expect_error(
    logCatch(stop("Error to show")),
    "(Error)*(Error to show)"
  )
  # Logs
  logContent <- readLines("./log.txt")
  logContent <- logContent[logContent != ""]
  expect_match(logContent[1], "(INFO)*(Info to show)")
  expect_match(logContent[2], "(DEBUG)*(Info to hide)")
  expect_match(logContent[3], "(WARNING)*(Warning to show)")
  expect_match(logContent[4], "(DEBUG)*(Warning to hide)")
  expect_match(logContent[5], "(ERROR)*(Error to show)")
  # Error trace is recorded
  expect_match(logContent[6], "Error trace")
})

setLogFolder(NULL)
unlink("./log.txt")
