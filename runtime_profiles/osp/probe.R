# Execute original OSP PKML fixtures using the real .NET/SimModel solver.
library(ospsuite)
library(jsonlite)
stopifnot(as.character(packageVersion("ospsuite")) == "12.4.5")
stopifnot(as.character(packageVersion("rSharp")) == "1.2.3")
fixture <- system.file("extdata", "Aciclovir.pkml", package = "ospsuite")
stopifnot(file.exists(fixture))
file.copy(fixture, "/work/model.pkml")
simulation <- loadSimulation(fixture)
results <- runSimulations(simulations = simulation)[[1]]
values <- getOutputValues(results)
stopifnot(nrow(values$data) > 1, all(is.finite(as.matrix(values$data))))
stopifnot(all(diff(values$data$Time) >= 0))
write.csv(values$data, "/work/native.csv", row.names = FALSE)
stopifnot(length(values$metaData) == length(results$allQuantityPaths) + 1)
stopifnot(all(vapply(values$metaData, function(x) nzchar(x$unit) && nzchar(x$dimension), logical(1))))
write_json(values$metaData, "/work/quantities.json", pretty = TRUE, auto_unbox = TRUE)
receipt <- list(
  engine = "OSPSuite", version = as.character(packageVersion("ospsuite")),
  native_execution = "passed", scientific_validity = "not_established",
  model = "Original OSPSuite-R Aciclovir.pkml; PK-Sim/MoBi headless computation",
  samples = nrow(values$data), time_unit = "minute",
  quantities = results$allQuantityPaths,
  limitations = c("This Linux profile executes existing PKML simulations.",
                 "Windows PK-Sim/MoBi graphical project authoring is a separate runtime.",
                 "Fixture execution is not independent pharmacological validation.")
)
write_json(receipt, "/work/osp-receipt.json", pretty = TRUE, auto_unbox = TRUE)
print(receipt)
