#' Check whether the .NET runtime is available
#'
#' Reports whether rSharp can currently call into .NET. The runtime is
#' initialised when the package loads, but loading never fails when the runtime
#' is missing or cannot be initialised (see the package startup message).
#' `dotnetAvailable()` lets you branch on that state, for example to conditionally
#' run code, examples, or vignette chunks that require .NET.
#'
#' If the runtime was not initialised at load time (for instance because .NET was
#' installed afterwards), a single initialisation attempt is made. The attempt
#' never raises an error; it only reports success or failure.
#'
#' @return A single logical: `TRUE` if the .NET runtime is available and
#'   initialised, `FALSE` otherwise.
#' @export
#'
#' @examples
#' if (dotnetAvailable()) {
#'   callStatic("System.Environment", "get_Version")
#' }
dotnetAvailable <- function() {
  tryCatch(
    {
      .ensureRuntime()
      isTRUE(rSharpEnv$runtimeLoaded)
    },
    error = function(e) FALSE
  )
}

.onLoad <- function(...) {
  # nocov start
  # Detect the runtime prerequisites and initialise the .NET domain. Loading the
  # package must never fail when the runtime is missing: rSharp still has to
  # install and load on machines and build environments (CRAN check machines,
  # R-universe builders) that do not have the .NET runtime. When a prerequisite
  # is missing we record the reason in `rSharpEnv$loadError` and return quietly.
  # The reason is surfaced to the user in `.onAttach()` and on the first call
  # into .NET (see `.ensureRuntime()`).
  rSharpEnv$loadError <- .checkDotnetPrerequisites()
  if (is.null(rSharpEnv$loadError)) {
    .loadAndInit()
  }
  invisible()
}

.onAttach <- function(...) {
  # Surface a missing runtime only when the user explicitly attaches the package
  # (`library(rSharp)`), using packageStartupMessage() so it can be suppressed.
  # `.onLoad()` stays silent, as required for a bare namespace load.
  if (!is.null(rSharpEnv$loadError)) {
    packageStartupMessage(rSharpEnv$loadError)
  }
  invisible()
} # nocov end

# Checks the runtime prerequisites without side effects. Returns `NULL` when the
# runtime can be initialised, or a message string describing the missing
# prerequisite otherwise. Safe to call from both `.onLoad()` and
# `.ensureRuntime()`.
.checkDotnetPrerequisites <- function() {
  # On Windows, the .NET runtime needs the Visual C++ redistributable.
  if (.Platform$OS.type == "windows") {
    if (Sys.which(rSharpEnv$msvcrFileName) == "") {
      return(messages$errorMsvcrNotFound())
    }
  }

  # Require a runtime whose major version matches the bundled assemblies. A
  # higher major (for example .NET 10) is not accepted: the assemblies target
  # `net8.0` and do not roll forward across major versions, so a newer runtime
  # would pass a naive check and then fail when .NET is actually called.
  # `dotnet` may be absent entirely, in which case `system()` signals an error
  # rather than returning output, so guard the call and treat any failure as
  # "no runtime".
  runtimes <- tryCatch(
    suppressWarnings(system(
      "dotnet --list-runtimes",
      intern = TRUE,
      ignore.stderr = TRUE
    )),
    error = function(e) character()
  )
  majors <- .dotnetMajorVersions(runtimes)
  if (!(rSharpEnv$requiredDotnetVersion %in% majors)) {
    return(messages$errorDotnetRuntimeNotFound())
  }

  NULL
}

# Extracts the major versions of the installed `Microsoft.NETCore.App` runtimes
# from the lines returned by `dotnet --list-runtimes`. Kept pure so it can be
# unit tested without a .NET runtime present. Each relevant line looks like:
#   "Microsoft.NETCore.App 8.0.11 [/usr/share/dotnet/shared/...]"
.dotnetMajorVersions <- function(lines) {
  if (length(lines) == 0) {
    return(integer())
  }
  appLines <- grep("Microsoft.NETCore.App", lines, value = TRUE)
  majors <- sub(
    "^Microsoft\\.NETCore\\.App\\s+([0-9]+)\\..*$",
    "\\1",
    appLines
  )
  majors <- suppressWarnings(as.integer(majors))
  majors[!is.na(majors)]
}

# Ensures the .NET runtime is initialised before a call into .NET. Re-attempts
# initialisation once (the runtime may have been installed after the package was
# loaded) and otherwise aborts with the recorded reason, so callers get an
# actionable error instead of a low-level "PACKAGE not loaded" failure from
# `.External()`.
.ensureRuntime <- function() {
  if (isTRUE(rSharpEnv$runtimeLoaded)) {
    return(invisible())
  }
  rSharpEnv$loadError <- .checkDotnetPrerequisites()
  if (!is.null(rSharpEnv$loadError)) {
    stop(rSharpEnv$loadError, call. = FALSE)
  }
  # Prerequisites passed, but native initialisation can still fail (see
  # `.loadAndInit()`), in which case it records the reason without throwing.
  # Surface that recorded reason instead of letting the caller proceed against
  # an uninitialised runtime.
  .loadAndInit()
  if (!isTRUE(rSharpEnv$runtimeLoaded)) {
    stop(rSharpEnv$loadError, call. = FALSE)
  }
  invisible()
}

.loadAndInit <- function() {
  # nocov start
  # Path to the folder where the libraries are located
  srcPkgLibPath <- system.file("lib", package = rSharpEnv$packageName)
  nativeLibraryPath <- file.path(
    srcPkgLibPath,
    paste0(rSharpEnv$nativePkgName, .Platform$dynlib.ext)
  )

  # Initialising the native host can fail even when `.checkDotnetPrerequisites()`
  # passed: that check only inspects the `dotnet` CLI output, whereas the native
  # loader locates the runtime host (`hostfxr`) itself and can disagree, for
  # example on a machine that ships the `dotnet` SDK but no host the loader can
  # resolve. A native failure here must not propagate out of `.onLoad()` and
  # abort `loadNamespace()`, so record the reason in `rSharpEnv$loadError` and
  # return quietly, exactly as for a missing prerequisite. `rSharpEnv$runtimeLoaded`
  # stays unset, so `.ensureRuntime()` surfaces the recorded reason on the first
  # call into .NET.
  tryCatch(
    {
      # Load C++ library
      dyn.load(nativeLibraryPath, DLLpath = srcPkgLibPath)

      # Load .NET library through C++
      # The method returns 0 if successful. Otherwise, an error is thrown.
      result <- .External(
        "rSharp_create_domain",
        enc2native(srcPkgLibPath),
        PACKAGE = rSharpEnv$nativePkgName
      )

      # The native library is loaded and the domain created, so calls into .NET
      # are possible. Mark the runtime as loaded before the callStatic() below,
      # which itself routes through `.ensureRuntime()`; setting the flag first
      # avoids re-entering initialisation.
      rSharpEnv$runtimeLoaded <- TRUE

      # Turn on the the conversion of advanced data types with R.NET.
      callStatic("ClrFacade.ClrFacade", "SetRDotNet", TRUE)
    },
    error = function(e) {
      rSharpEnv$runtimeLoaded <- FALSE
      rSharpEnv$loadError <- messages$errorRuntimeInitFailed(conditionMessage(
        e
      ))
    }
  )
  invisible()
} # nocov end
