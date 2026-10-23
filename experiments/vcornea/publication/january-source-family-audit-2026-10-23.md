# Original January archive family mechanisms

The complete January23,2025 stash `c7f95d28dfa2329abb7353caab724c52f9341dee`
was recovered from Git objects inside the original licensed Code ZIP, without
loading downloaded Git configuration/hooks or executing model code. All50 files,
2,783,833 bytes and original executable modes are retained under
[`vendor/models/vcornea-history-c7f95d28`](../../../vendor/models/vcornea-history-c7f95d28).
The immutable tree digest is
`cf708f1341da82cf4cc7de42173a38c3b642b95fcfffa45bf80eafddc5305e06`;
[pins](../../../engine_adapters/compucell3d/vcornea-history-c7f95d28.json)
retain all original SHA256 and Git blob identities. This postdates the July2024
data and does not establish exact source correspondence for a selected cohort.

## What this source actually contains

| Control | HPC formation | HPC injury |
|---|---|---|
| Basal initial target / EGF half-max | 40 / 8 | 25 / 7 |
| Stem initial target / EGF half-max | 40 / 4.5 | 25 / 3.5 |
| Superficial EGF diffusion | 12 | 20 |
| Potts temperature | 8 | 8 |
| Original requested CC3D workers | 4 | 1 |
| Original duration | 43200 MCS | 46200 MCS |
| Original injury activation / time / dose | False / 4000000000 / 1700 | False / 3000 / 1000 |
| Original spatial output bins | 10 | 10 |

Formation's non-plot method ASTs are identical to the July snapshot. Its injury
methods are different: epithelial EGF/SLS sensing uses volume normalization,
growth half-max and SLS threshold scale by the stored initial volume, the stem
growth multiplier changes, and tear-volume handling differs. The original
stochastic superficial sloughing and lineage initializer remain. Nine methods
differ between the two January HPC families; exact method fingerprints are in
[the read-only audit](observed-2026-10-23/january-source-family-audit.json).
Do not replace both families with one common steppables file.

The injury PIFF has4 stem,32 basal,127 wing and46 superficial cells. Its original
SHA256 is `a4da5c3c786581165ff0cf642ea017d242d7bb4d50fe3b573cd7a97627953edf`.
It differs from the later archive PIFF only in LF versus CRLF line endings;
the original January bytes are copied unchanged. No external state is substituted.
The conditional40/40 target volumes observed in the separate original MildComb0
spatial cohort are not claimed for this candidate's25/25 injury defaults or
for the selected MildComb1 scalar cohort.

## Explicit fifth reconstruction

`stash-hpc-5.0.0` keeps both complete original HPC project families, their own
batch defaults, temperature, worker count and native ten-bin observation grid.
The only injury reconstruction overrides are `IsInjury=True`, pulse7200MCS,
duration50400MCS and dose750/1500/2500. In this source, `IsInjury` enables exposure
and `InjuryType=False` selects chemical injury. This Boolean differs from the
later archive's branch structure. The original supplied batch otherwise never
activates injury. Formation remains43200MCS. Both retain explicit new seeds7/11,
the separately pinned4.6.0 runtime and native lifetime overlay2.

This candidate was selected from original source differences, without fitting
controls to candidate results. Earlier source2/3/4 runs and negative findings
remain intact. The comparison protocol and every threshold remain unchanged;
full original math, structural checks and all four groups/two seeds are still
required before a full passing comparison can be reported. Exact historical
seeds, per-run configurations and full environment remain unknown. Independent
biological validation is a separate unclosed gate.

The explicit operator now checks requested CC3D workers from every prepared XML.
It admits at most four multiworker jobs (at most16 requested workers) or six jobs
only when all request one worker. This is a per-series admission bound, not a
hard OS CPU cap or an automatic cross-series scheduler. Concurrent series require
an explicit operator capacity check. No API execution or heartbeat is enabled.

## Actual short checks and frozen full execution

Actual formation20MCS runs with/without added observations both captured exit0
(22.12/15.09 seconds). All five original numeric CSV/Parquet outputs are byte
identical, including counts, thickness and original cell-level tracking. The
moderate600MCS probe captured exit0 in139.91 seconds with an explicit early
pulse20MCS at exactly[100,75]/2500. It retains61 count rows,610 ten-bin height
rows and601 finite/nonnegative chemical samples on the complete0.1h grid.
Every saved cell volume agrees with lattice occupancy and every observed cell
count agrees with the original tracker. Basement sites990→901 and epithelial
sites5015→4204 are actual short-run observations, not full healing or acceptance.
[Exact native manifests/records/proofs](observed-2026-10-23/january-native-probes.json)
and the [read-only audit recipe](observed-2026-10-23/january-probe-audit.py.txt)
are retained. An initial wrapper's missing-driver filename stopped before its
unobserved/native injury attempts; its preparation and log remain unchanged.

All eight full jobs are prepared/frozen before execution in two independent
series, using the same protocol1189fd62... and runtime d29865e8... . Portable
[formation](observed-2026-10-23/january-formation-prepared-plan.json) and
[injury](observed-2026-10-23/january-injury-prepared-plan.json) plans retain every
manifest identity. The six one-worker injury jobs wait for all four active
source4 slight/mild jobs to actually complete/verify. At that point only two
four-worker source4 moderate jobs can remain: maximum8+6=14 requested workers.
The two formation jobs wait for the entire source4 series, then request8 while
source5 injury can request6. No healthy job is interrupted or duplicated.
A separate explicitly invoked bounded follow-on waits for BOTH source5 series
before combining all eight records and running all three full analysis stages.
Heartbeat remains paused. No API/Worker deployment or live-volume mutation occurs.
