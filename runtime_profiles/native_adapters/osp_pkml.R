# Execute admitted PKML using the pinned OSPSuite/.NET runtime and original schedule.
library(ospsuite)
library(jsonlite)
simulation <- loadSimulation("/work/model.xml")
results <- runSimulations(simulations = simulation)[[1]]
values <- getOutputValues(results)
stopifnot(nrow(values$data) > 1, all(is.finite(as.matrix(values$data))))
stopifnot(all(diff(values$data$Time) >= 0))
stopifnot(length(values$metaData) == length(results$allQuantityPaths) + 1)
stopifnot(all(vapply(values$metaData, function(x) nzchar(x$unit) && nzchar(x$dimension), logical(1))))
write.csv(values$data, "/work/native.csv", row.names = FALSE)
write_json(list(engine = "OSPSuite", version = as.character(packageVersion("ospsuite")),
                samples = nrow(values$data), quantities = values$metaData,
                scientific_validity = "not_established",
                limitations = c("Original PKML output schedule and native units retained.",
                                "Native execution does not establish independent biological validity.")),
           "/work/quantities.json", pretty = TRUE, auto_unbox = TRUE)
# Agent independently hashes these exact output bytes; no shell/hash command is invoked.
write_json(list(status = "succeeded", files = c("native.csv", "quantities.json")),
           "/work/native-completion.json", pretty = TRUE, auto_unbox = TRUE)
