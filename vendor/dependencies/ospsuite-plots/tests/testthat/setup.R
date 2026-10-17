# A few tests still exercise the soft-deprecated setDefaults()/setDefaultTheme()
# (they assert the deprecation explicitly with lifecycle::expect_deprecated()).
# Default the session to "warning" verbosity so those assertions see the
# condition, and rely on the tests themselves to scope it.
withr::local_options(
  lifecycle_verbosity = "warning",
  .local_envir = teardown_env()
)
