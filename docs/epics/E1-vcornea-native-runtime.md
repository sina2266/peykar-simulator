# E1: native vCornea operator workflow

Engineering EPIC completed on 2026-10-16. Its deliverable is a reviewed local
source → isolated native runtime → actual output → immutable import → API/CLI
retrieval path. It is the first working external biological-model integration.
The P1 published-reproduction scientific gate remains open.

## Acceptance and observed results

| Task | Delivered evidence |
|---|---|
| Establish source/data permissions and exact identity | Zenodo 16884414 CC-BY-4.0; complete code MD5/SHA verified; 27 reviewed files pinned; no .git extraction |
| Reconstruct actual native engine | Separate CompuCell3D 4.7.0 Linux/AMD64 image; 424 exact package builds/hashes; imports and real Parquet round trip pass |
| Execute admitted model with bounded resources | Private source copies, no network, dropped capabilities, no API credentials/data volume, wall/CPU/memory/PID/log/work bounds, cleanup and immutable image ID |
| Ingest actual published native output | Three severities, six complete CSV/Parquet files; 5,041 / 4,621 / 5,041 hourly samples; raw/decoded/normalized lineage |
| Preserve stochastic/measurement semantics | Unknown archived seeds stay null; native Time is hours; four count types; height is lattice y coordinate; no automatic physical-thickness claim |
| Freeze and compare repeat execution | Reference and candidate runs for seeds 7 and 11 at 60 MCS; frozen zero-count tolerance; both seven-sample Basal comparisons pass with RMSE 0 |
| Cross the native injury event | 600 MCS, seed 7: 61 count samples over 0–60 hours; raw surfactant integral first positive at hour 50 with value 750; 221.085 seconds elapsed |
| Deliver output access | CLI native count/height import, count extraction and lineage; authenticated count POST and output/lineage GET; immutable conflicts and reader permissions tested |
| Assess independent data and complementary tools | Attributed experimental aggregate ingestion; COPASI/BioModels assessment; explicit context and EGF process-ownership gaps |
| Verify existing platform | Final Linux checks: 242 tests pass, 88% observed statement coverage, lint/type/schema checks and all three package builds pass |

All three raw output files (counts, surfactant, thickness Parquet) are byte-identical
between the two repetitions of each tested seed. This is a bounded repeatability
check of the same model/runtime; it is not a distribution test or improvement claim.
No biological mechanism was changed merely to make a regression pass.

## Reproduce the operator workflow

Source preparation and image build are shown in the [root README](../../README.md).
Source paths and file digests are fixed by
[`vcornea-source-16884414.json`](../../engine_adapters/compucell3d/vcornea-source-16884414.json).
The [runtime README](../../engine_adapters/compucell3d/runtime/README.md) describes
the exact lock, notices, AMD64 emulation and verified image receipt.

Acquire explicit reference members with `scripts/acquire_vcornea.py members`;
[the candidate assessment](../integration-candidates.md) lists the exact six paths.
Pass its acquisition manifest to the repeatable acceptance command:

```bash
uv run python scripts/verify_vcornea_epic.py \
  --source .peykar/external/vcornea-archive-16884414 \
  --references .peykar/external/vcornea-reference-16884414 \
  --acquisition .peykar/external/research/selected-reference-acquisition.json \
  --pins engine_adapters/compucell3d/vcornea-source-16884414.json \
  --output .peykar/external/epic60 \
  --data-dir .peykar/vcornea-epic \
  --image peykar-cc3d:4.7.0-linux64 --steps 60 --wall-seconds 300
```

The observed run uses `epic60`; choose a fresh output directory for another run.
The script writes acceptance before execution, imports actual reference files,
runs both reference seeds, freezes the protocol before candidate runs, and stores
the report. Exact archived parameters/seeds are not inferred from folder labels.

```bash
uv run python scripts/run_vcornea.py \
  --source .peykar/external/vcornea-archive-16884414 \
  --pins engine_adapters/compucell3d/vcornea-source-16884414.json \
  --output .peykar/external/injury600 --scenario injury \
  --steps 600 --seed 7 --image peykar-cc3d:4.7.0-linux64 --wall-seconds 900
```

Use another fresh directory to repeat this observed command. The optional
`stem-only` scenario can be prepared, but its native execution is not claimed by
these results. Omitting the horizon uses the full upstream duration, which was
not run here. Execution completion requires native count output; failures retain
their records and cleanup instead of returning manufactured success.

## Evidence locations

- Portable identities/results: [verification receipt](../../engine_adapters/compucell3d/vcornea-verification-2026-10-16.json).
- Runtime identity: [native probe receipt](../../engine_adapters/compucell3d/runtime/runtime-receipt-2026-10-16.json).
- Local raw source, data and runs: ignored `.peykar/external/`; full code ZIP,
  extracted source, selected member acquisition manifest, `epic60/` and `injury600/`.
- Separate acceptance registry: `.peykar/vcornea-epic/`; immutable native outputs,
  scalar benchmarks, frozen protocol/report and the Rho2015 control import.
- [Native import/API contract](../vcornea-native-output.md),
  [observed commands and deployment](../verification.md).

Raw simulations and large dependencies are not bundled into the default packages.
CC-BY archive attribution is in [integration admission](../vcornea-integration.md).
CompuCell3D core is MIT, Player LGPL and runtime dependencies retain their own
licenses/notices. Existing Peykar license texts remain unchanged.

## Remaining scientific gate and next work

The published references lack verified seed/per-run parameter files and predate
the final paper. Original environment, exact parameter combination, analysis and
full-horizon stochastic agreement need explicit recovery/comparison. Bounded runs
do not establish full published-scenario reproduction. The stored comparison
class is `simulation_reference_comparison`, never independent biological validation.

The experimental [Rho2015 aggregate](../../experiments/data/rho2015-scratch/README.md)
has a different cell/assay/GM-CSF context. No validated wound-area observation
operator links it to the stratified vCornea lattice; it is not a vCornea holdout.
Adding a molecular EGF model first needs process ownership to avoid counting the
existing field's uptake/decay effects twice. See [E2 on the roadmap](../roadmap.md).
