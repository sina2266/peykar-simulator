# Explicit serial native execution derivative

Terminal update08:00UTC: both moderate jobs completed50400MCS/exit0, in10715.82
and11179.29s. Original math/spatial checks pass; unchanged frozen Wing trajectory
coverage fails in both. The completed serial January formation comparison is also
negative. [Actual final outputs and quantitative results](serial-final-results-2026-10-24.md)
supersede the launch-time pending state below. No native job remains active.

An actual source4 moderate7 corneal job aborted with a retained SIGABRT. Its
allocation detection stack is not sufficient to locate the original corruption.
A short separate engineering investigation therefore tested native parallelism
without changing any active corneal job or installed binary.

## Real bounded native probes

Two giant synthetic cells occupy a200×90 lattice, with all contact energies zero,
seed7 and10000MCS. This intentionally stresses cell updates across worker regions;
it is not a corneal biological model or proof that all four-worker models fail.
Each private subprocess had denied network, private writes, a90s wall timeout and
its actual exit recorded. The installed pinned4.6 runtime was verified read-only.

| Probe | Requested workers | Actual outcome | Last sampled observation |
|---|---:|---|---|
| PixelTracker + BoundaryPixelTracker | 1 | exit0,25.06s | 9900MCS,2cells,total native volume18000 |
| Same source/seed with both trackers | 4 | SIGSEGV/-11,1.85s | 0MCS,2cells,volume18000 |
| PixelTracker only | 4 | SIGABRT/-6,1.91s | 100MCS,volume17873 |
| BoundaryPixelTracker only | 4 | SIGSEGV/-11,1.89s | 100MCS,volume17964 |
| Neither tracker | 4 | SIGABRT/-6,2.09s | 700MCS,volume11437 |

The latter three results prevent attributing this stress failure uniquely to
BoundaryPixelTracker. No speculative plugin mutex patch or installed binary change
was made. The two earlier setup attempts failed before firstMCS (absolute PIF
path handling, then an observer import scope); both reports/operators remain.
The [actual observation receipt](observed-2026-10-24/native-concurrency-observations.json)
retains real exits, configurations, hashes and the sampled volume discrepancy.
These synthetic findings do not establish the exact cause of the original corneal
abort, or negate the separately checked saved corneal volume/lattice observations.

The upstream [known-problems page](https://compucell3d.org/Problems_and_bugs)
also documents random crashes with PixelTracker/multiple processors and reducing
processors as a possible workaround. It describes a known problem generally;
it does not identify our observed failure or validate a biological model.

## Separate source/control identity

`observed-injury-serial-6.0.0` is a new explicitly selectable offline profile.
It keeps source4's recovered July mechanisms, separately evidenced injury PIFF,
biological controls, doses,7200MCS pulse,50400MCS horizon, ten-bin observation and
lifetime repair2. Only the XML worker declaration changes4→1; new driver copies
also retain the previously reviewed fatal-signal diagnostics. Its manifest records
source/requested worker counts and the derivative reason. Source3/4/5 files,
existing copied drivers, model/schema versions and comparison1.0.0 are preserved.
Parallel stochastic equivalence and full-horizon stability are not assumed.

Both seed7 and11 full moderate jobs were explicitly prepared/frozen and started
under `.peykar/cornea-serial-derived-moderate46-20261024`, at most two requested
workers,24h each. These are declared execution-control derivatives, not silent
retries in failed workspaces or seed selection. The existing January formation2
continues unaltered (at most8requested workers), total native request at most10.
A finite follow-on at `.peykar/cornea-serial-derived-moderate-analysis-20261024`
waits for actual completion, then runs the original frozen **moderate-only**
diagnostic, original mathematics and spatial analysis. This subset cannot close
all four publication groups. No recurring automation is enabled.
