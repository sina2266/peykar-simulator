test_that(".dotnetMajorVersions parses major versions from dotnet output", {
  lines <- c(
    "Microsoft.AspNetCore.App 8.0.11 [/usr/share/dotnet/shared/Microsoft.AspNetCore.App]",
    "Microsoft.NETCore.App 8.0.11 [/usr/share/dotnet/shared/Microsoft.NETCore.App]",
    "Microsoft.NETCore.App 10.0.0 [/usr/share/dotnet/shared/Microsoft.NETCore.App]"
  )
  expect_equal(.dotnetMajorVersions(lines), c(8L, 10L))
})

test_that(".dotnetMajorVersions returns an empty integer vector when nothing matches", {
  expect_equal(.dotnetMajorVersions(character()), integer())
  expect_equal(.dotnetMajorVersions(c("", "no runtimes here")), integer())
})

test_that(".dotnetMajorVersions ignores non NETCore.App runtimes", {
  lines <- "Microsoft.AspNetCore.App 9.0.0 [/path]"
  expect_equal(.dotnetMajorVersions(lines), integer())
})

test_that(".checkDotnetPrerequisites reports a missing runtime when the required major is absent", {
  # Force the non-Windows branch so the check depends only on the runtime list.
  local_mocked_bindings(
    .dotnetMajorVersions = function(...) 10L
  )
  withr::local_options(list(rSharp.test_os_type = NULL))
  skip_on_os("windows")
  msg <- .checkDotnetPrerequisites()
  expect_type(msg, "character")
  expect_match(msg, "No .NET 8 runtime was found", fixed = TRUE)
})

test_that(".checkDotnetPrerequisites passes when the required major is present", {
  skip_on_os("windows")
  local_mocked_bindings(
    .dotnetMajorVersions = function(...) c(8L, 10L)
  )
  expect_null(.checkDotnetPrerequisites())
})

test_that(".ensureRuntime surfaces the recorded reason when native init fails", {
  # Prerequisites pass, but the native host fails to load: `.loadAndInit()`
  # records the reason and leaves `runtimeLoaded` unset instead of throwing.
  # `.ensureRuntime()` must then abort with that recorded reason.
  local_mocked_bindings(
    .checkDotnetPrerequisites = function() NULL,
    .loadAndInit = function() {
      rSharpEnv$runtimeLoaded <- FALSE
      rSharpEnv$loadError <- "boom"
    }
  )
  withr::defer({
    rSharpEnv$runtimeLoaded <- NULL
    rSharpEnv$loadError <- NULL
  })
  rSharpEnv$runtimeLoaded <- NULL
  expect_error(.ensureRuntime(), "boom", fixed = TRUE)
})

test_that("errorRuntimeInitFailed includes the underlying details", {
  msg <- messages$errorRuntimeInitFailed("Failure: load_hostfxr()")
  expect_match(msg, "could not be initialised", fixed = TRUE)
  expect_match(msg, "Failure: load_hostfxr()", fixed = TRUE)
})

test_that("dotnetAvailable reports FALSE without erroring when init fails", {
  local_mocked_bindings(
    .ensureRuntime = function() stop("no runtime")
  )
  expect_false(dotnetAvailable())
})

test_that("dotnetAvailable reports TRUE when the runtime is initialised", {
  local_mocked_bindings(
    .ensureRuntime = function() invisible()
  )
  withr::defer(rSharpEnv$runtimeLoaded <- NULL)
  rSharpEnv$runtimeLoaded <- TRUE
  expect_true(dotnetAvailable())
})
