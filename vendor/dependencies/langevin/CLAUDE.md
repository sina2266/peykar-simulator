# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

LangevinNoVis01 is the particle-based, spatial, stochastic solver for the **SpringSaLaD** simulation program — a "no visualization" (headless) variant. It reads a SpringSaLaD model input file, runs a Langevin-dynamics simulation of molecules/sites/bonds in a box, writes per-run time-series data, and can consolidate multiple runs into averaged statistics. It is consumed both as a standalone CLI and as a solver invoked by VCell (hence the messaging layer).

Requires **Java 17**. Built with Maven; shipped as a **GraalVM native image** (`target/langevin`).

## Common commands

```bash
mvn test                       # compile + run JUnit 5 tests (the CI gate on mac/windows)
mvn package                    # build jar-with-dependencies (target/langevin-*-jar-with-dependencies.jar)
mvn -Pnative -DskipTests=true package          # native image on macOS/Windows  -> target/langevin
mvn -Pnative-static -DskipTests=true package   # static native image (Linux, musl libc)

mvn test -Dtest=MySystemTest                   # single test class
mvn test -Dtest=MySystemTest#someMethod        # single test method
```

Run the solver (main class `edu.uchc.cam.langevin.cli.CliMain`):

```bash
# via the native binary
./target/langevin simulate    <model.txt> <runCounter>   # run one trial; run 0 also emits movie/viewer data
./target/langevin postprocess <model.txt> <numRuns>      # consolidate runs into averages/std/min/max

# via maven without building a binary
mvn exec:java -Dexec.mainClass=edu.uchc.cam.langevin.cli.CliMain -Dexec.args="simulate /path/MyModel.txt 0"
```

Useful options on both subcommands: `--output-log <file>` (write progress to a file instead of stdout), `--vc-print-status` (emit VCell status to stdout/stderr), `--vc-send-status-config <props>` (send progress to a VCell message broker over REST).

## Architecture

### CLI layer (`cli/`)
picocli-based. `CliMain` registers two subcommands: `RunCommand` (`simulate`) and `PostCommand` (`postprocess`). Each wires up a `VCellMessaging` implementation, constructs a `Global` from the model file, then runs `MySystem` or `ConsolidationPostprocessor`. `Version` supplies the git version and is `--initialize-at-build-time` for the native image.

### The "G" convention (read `Info.txt` before touching the object/reaction model)
Classes prefixed **`G`** (`GMolecule`, `GSite`, `GState`, `GBindingReaction`, …) are the *global/immutable* description parsed from the input file — the template. The non-`G` classes (`Molecule`, `Site`, `Link`, `Bond`) are the *live simulation instances*. Key differences: the instance side has no `SiteType` class (properties are baked into each `Site` for speed), reaction data is attached to sites rather than living in standalone reaction objects, and `location` is an int rather than a string for fast boundary checks. When adding a field that's read from input, it usually belongs on the `G` class; when it changes during simulation, it belongs on the instance class.

### Input parsing — `Global`
`Global` (langevinnovis01/) is a near-immutable holder for everything read from the model file. The file is a flat text format split into labeled sections — the section header strings are the `public static final String` constants at the top of `Global` (`SYSTEM INFORMATION`, `TIME INFORMATION`, `MOLECULES`, `BIMOLECULAR BINDING REACTIONS`, `SIMULATION OPTIONS`, the various counter sections, etc.). Box geometry lives in `GBoxGeometry`, timing in `GSystemTimes`.

### Simulation engine — `MySystem` (the core, ~1200 lines)
Constructed from a `Global` + run counter. `runSystem()` is the main loop: integrate one `dt` step via `update()`, optionally `relaxSprings()` for sub-steps (`dtspring`), and periodically sample counters at `dtdata` and positions at `dtimage`. The spring constant ratio is fixed (`SpringConstant = 100`). Outputs per run: position/viewer file (`.ida`), cluster file (`.json`), `RunningTime.txt`, and counter data. After the loop it invokes `LangevinPostprocessor` to post-process the run's raw data.

### Counters (`counter/`)
Each sampled observable has a counter that accumulates during the run and writes at the end: `MoleculeCounter`, `StateCounter`, `BondCounter`, `ClusterCounter` (cluster-size distributions; gated by the "count clusters" option), `SitePropertyCounter`, `LocationTracker`. Their `G*Counter` configuration counterparts live in `g/counter/`.

### Reactions (`reaction/`)
`BindingReactions`, `TransitionReactions`, `AllostericReactions` apply the stochastic reaction rules each step. `OnRateSolver` (helpernovis/) calibrates microscopic binding rates to a desired macroscopic on-rate.

### Postprocessing
Two distinct things, don't conflate them:
- `LangevinPostprocessor` (`org/vcell/data/`) — invoked at the end of a *single* run to transform that run's raw output (writes ND-JSON via `NdJsonUtils`).
- `ConsolidationPostprocessor` (`langevinnovis01/`) — the `postprocess` subcommand; reads the `.ida` files from *all* runs and computes per-time-point averages / std / min / max into `SolverResultSet`s, plus advanced cluster statistics (`ClusterStatisticsCalculator`). Input/output staging is split across the `Consolidation*Input`/`*Output` classes.

### Messaging (`org/vcell/messaging/`)
`VCellMessaging` is the progress-reporting interface. Pick the implementation by CLI flags: `VCellMessagingNoop` (default), `VCellMessagingLocal` (`--vc-print-status`), `VCellMessagingRest` (`--vc-send-status-config`, sends `WorkerEvent`s to a VCell broker). Progress events are throttled inside the implementation, so calling `sendWorkerEvent` every step is fine.

### Logging (`org/…/logging/`, `log4j2.xml`)

Logging is Log4j2, configured by a single authoritative file — `src/main/resources/log4j2.xml` (console appender, root at INFO, `%msg%n`). The config is intentionally **immutable**: `edu.uchc.cam.langevin.logging.BundledLog4jConfigurationFactory` is installed as the first statement of `CliMain.main()` and forces Log4j2 to load the bundled `log4j2.xml`, ignoring the `log4j2.configurationFile` system property, the `LOG4J_CONFIGURATION_FILE` env var, and any external `log4j2.xml`. To change verbosity, edit `log4j2.xml` and rebuild.

Two constraints when touching logging:
- **Keep `log4j-core`/`log4j-api` at ≥ 2.25.** Those versions ship GraalVM reachability metadata that bundles the `log4j2.*` config and the plugin registry into the native image; downgrading silently breaks logging in the native binary (the config isn't found and Log4j2 falls back to its ERROR-only default).
- **Don't reintroduce programmatic reconfiguration.** An earlier `LoggingInit` that called `LoggerContext.stop()` and hand-built appenders was removed — it double-configured under the JVM and didn't work under the native image. Configure through `log4j2.xml` only; the factory above is the one sanctioned bit of code, and it just pins that file.

## CI/CD

`.github/workflows/CI.yml` runs on push/PR across macos-15-intel, macos-14, windows-latest (run `mvn test` + non-static native build) and ubuntu-latest (static native build, tests skipped), then smoke-tests `./target/langevin --help`. `CD.yml` runs the same native builds on GitHub release and uploads the binaries as artifacts. Native builds use GraalVM CE 17.0.7. Native-image reflection config lives in `src/main/resources/META-INF/native-image/`.

## Notes

- Documentation `.md` files are authored in Eclipse with the WikiText plugin (per README).
- Test fixtures live under `src/test/resources/simdata/` (e.g. a multi-run `SimID_..._FOLDER` with `Run0/Run1/Run2` data) and are used by the consolidation/cluster-analysis tests.
- Much dead/commented code remains in the engine (e.g. `LocationTracker`, `writePartialData` calls); leave it unless asked.
