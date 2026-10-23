# Completed injury observations and retained native failure

Inspected23:04–23:15UTC October23 (October24 local). No original input,
frozen acceptance threshold, native runtime or live API volume was changed.

## January injury: six full native exits

All six January `stash-hpc-5.0.0` injury jobs actually reached50400MCS/5040h
with captured exit0 in11311.59–11557.02 seconds. Every retained7200MCS pulse
hash matches its earlier interim observation and actual completion proof.
All six full50401-sample chemical traces are finite/nonnegative on the original
0.1h grid. Actual pulses have one positive site[100,75], at720h, with the
original declared750/1500/2500 doses. Original count/height output and every
observed lattice/cell volume/count were independently checked.

The unchanged comparison protocol and original mathematical analysis were run:

| Case | Seed | Stable means | Hourly trajectories | Thickness bins | All scalar checks |
|---|---:|---:|---:|---:|---|
| Slight | 7 | 3/4 | 4/4 | 7/10 | Fail |
| Slight | 11 | 4/4 | 4/4 | 9/10 | Fail |
| Mild | 7 | 3/4 | 4/4 | 10/10 | Fail |
| Mild | 11 | 4/4 | 4/4 | 10/10 | Pass |
| Moderate | 7 | 4/4 | 4/4 | 10/10 | Pass |
| Moderate | 11 | 4/4 | 4/4 | 10/10 | Pass |

Slight7's Wing mean is128.29267 versus124.44863, difference3.84404 against
allowed3.61171 cells. Three thickness regions exceed their bounds. No failing
seed/region was dropped, rerounded or averaged away. This is a **negative
injury-only diagnostic**, not a completed full publication comparison.
Original-analysis and spatial-analysis stages both exit0, comparison exits2.
Final180-day recovery states and4320h post-pulse EGF fields are included.
Both slight and both moderate structure figures, mild7 structure, slight7
count overlay and moderate11 EGF were visually inspected. Structural proxies
remain descriptive; matching scalar means do not establish clinical healing.

- [All six manifest/exit proofs and pulse continuity](observed-2026-10-24/january-six-injury-completion.json)
- [Full chemical verification](observed-2026-10-24/january-six-injury-full-chemical-audit.json)
- [Exact per-seed comparison](observed-2026-10-24/january-injury-diagnostic-report.json)
  and [readable result](observed-2026-10-24/january-injury-diagnostic-report.md)
- [Original-analysis receipt](observed-2026-10-24/january-injury-original-analysis.json),
  [spatial observations](observed-2026-10-24/january-injury-diagnostic-spatial.json)
  and [three actual stage exits](observed-2026-10-24/january-injury-analysis-stages.json)

## Derived injury: genuine failed job, not missing evidence

Five source4 jobs genuinely completed; moderate7 aborted after its last observed
25000MCS progress sample. The guard captured actual child returncode-6; the outer
wrapper returned134. macOS's contemporaneous diagnostic identifies allocator
`nanov2_guard_corruption_detected`, while allocating in BoundaryPixelTracker from
a Potts parallel worker. This is the detection stack, **not proof that the
corruption originated in that function**. The private original crash report,
partial arrays/progress and failed job remain; only relevant sanitized metadata
and stack symbols are exported in the [terminal receipt](observed-2026-10-24/derived4-terminal-execution-and-abort.json).
No native bug fix or biological explanation is claimed from that stack.

The combined derived full analysis correctly stopped. Its five full jobs cannot
replace six actual native completions. Existing negative slight/mild diagnostic
results remain intact. No failed job is silently retried or promoted.

## Explicit capacity release for never-started formation

The old January formation plan failed its success prerequisite without starting
either native job. Its original two prepared manifests remain frozen. At the
later inspection no native solver/guard was active; all January injury jobs had
completed. An explicitly reviewed [finite offline operator](observed-2026-10-24/prepared-formation-operator.py.txt)
verified original manifest hashes, absent record/proof/log, unchanged protocol/
runtime and terminal prior processes, then started those **same prepared inputs**
with at most two four-worker jobs. The original failed plan is retained; a new
`.peykar/cornea-stash-prepared-formation46-20261024` plan records this decision.
This is neither an automatic retry nor a change to the original prerequisite.

A separate bounded follow-on at `.peykar/cornea-stash-full-analysis-20261024`
waits for actual formation completion, then combines it with the verified injury6
for all four groups under the original protocol. No recurring automation is
created/reactivated. Full reconstruction and independent experimental biological
validation remain open. The original seeds/per-run parameter files are still not
identified by the published source/object audits.
