# Verification record
## Retinal-acuity tranches 3–4 and model 1.2 development: 2026-10-29 (night)
Commands run from the repository root; outcomes observed. Start state: main at
`a3489c25` (= origin; tranches 3–4 reserved before triage).
- `python3 .peykar/write_retina_acuity_tranche34_briefs.py`: wrote the triage brief
  (`01754f78…`) and the extraction brief (`32a8895b…`) from the tranche-2 briefs, with
  checks that every criterion line is copied. `triage_chunks.py --tranche 3 --chunks 4`
  and `--tranche 4 --chunks 1`: 4 × 30 and 1 × 18 articles; a pattern check found no
  decimal, ± or % outside DOI and licence lines (commit `5ad75a7b`).
- Five reviewer sub-agents: 15 + 14 + 13 + 17 admits in tranche 3 and 11 in tranche 4.
  `uv run --frozen python .peykar/write_retina_acuity_tranche34_triage.py`: 70 admitted, 68
  excluded; reservation 53 development, 70 sealed. `scripts/link_study_authors.py
  --dataset .../tranche-3 --prefix rac-`: 12 pairs, 3 development clusters, 6 sealed
  studies with training exclusions (commit `e4033419`).
- Custodians: C2, C3 and C4 finished (2, 5 and 5 records); C1 and C5 were throttled by
  the agent service mid-run ("Too many requests, client has been throttled") and finished
  on later attempts after rebuilding their earlier records with identical SHA-256 (6 and
  3 records); C6–C9 gave 1, 0, 3 and 2 records. `uv run --frozen python
  .peykar/verify_retina_acuity_tranche34_custody.py`: 70 reserved, 27 records and 43
  exclusions, 27/27 rebuilt the same, 70 raw copies equal to the screened sources, 143
  series, no problem. The developer opened one sealed spec and several custody reports
  by mistake while launching custodians (structure only; recorded in the protocol).
- `uv run --frozen python .peykar/write_retina_acuity_protocol_1_2.py`: protocol
  `experiments/eye/retina-acuity/model-1.2/protocol-1.2.0.json`, SHA-256 `a0618bbf…`, 94
  frozen files, 53 training and 27 sealed record hashes; the operator's own check passed
  first. `.peykar/check_frozen_protocols.py`: no problem in any frozen protocol,
  retinal acuity 1.2.0 included. `make check-docker` after writing the protocol: 934
  tests passed in 187.22 s, 89% coverage, mypy 165 source files, all packages built,
  EXIT=0 (`.peykar/retina-acuity12-freeze-check-docker-20261029.log`). Freeze commit
  `e4ac6cb9`, pushed (remote SHA equal).
- Then, once: `OMP_NUM_THREADS=1 uv run --frozen python
  scripts/evaluate_retina_acuity_population_1_2.py --sealed --protocol
  experiments/eye/retina-acuity/model-1.2/protocol-1.2.0.json --output
  .peykar/retina-acuity12-sealed-test-2026-10-29` (detached; log ends with `EXIT=0`):
  protocol check passed, 234.0 s, 53 training and 27 sealed record hashes, `report.json`
  `495c5f9f…`, `reference-x0-report.json` `bd8ec333…`.
- `OMP_NUM_THREADS=1 uv run --frozen python scripts/publish_retina_acuity_population_1_2.py
  --sealed --report .../report.json --reference .../reference-x0-report.json --output
  experiments/eye/retina-acuity/model-1.2/sealed-test-2026-10-29`: exit 0; H1–H5 all fail
  (coverage 42/71 and 59/71; next observation 4.51 against persistence 4.06 and the
  pooled-curve increment 3.82; landmark 5.85 against 4.52; initial state 8.82 against
  8.70; H5 next observation 4.51 against 4.42). 1.1.0's own H1–H4 fail too.
- `uv run --frozen python .peykar/check_retina_acuity_redaction_1_2.py sealed
  experiments/eye/retina-acuity/model-1.2/sealed-test-2026-10-29`: 14 public and 13
  private studies, 26 redacted study reports and 178 analysis rows checked, 0 leaks.
  `.peykar/retina_acuity12_sealed_breakdown.py`: per-study, per-target-month and
  per-context tables; no target before month 1; removing any single study changes no
  H1–H4 verdict.
- `.peykar/cornea-native47/bin/python
  experiments/eye/retina-acuity/model-1.2/sealed-test-2026-10-29/plot-results.py`: exit
  0; inspected through a local `http.server` and Playwright, re-rendered once (legend
  over a bar label).
- `make check-docker` before the results commit: 934 tests passed in 191.44 s, 89%
  coverage, mypy 165 source files, all packages built, EXIT=0
  (`.peykar/retina-acuity12-results-check-docker-20261029.log`).
- `OMP_NUM_THREADS=1 uv run --frozen pytest -q tests/unit/test_eye_support.py
  tests/unit/test_eye_retina_acuity_1_2.py tests/unit/test_eye_retina_acuity_1_1.py`: 9
  passed (the hold changes no prediction without early visits, holds early initial
  states at the support month, and leaves comparators unchanged).
- Round 0–1 declared in commit `56a2db0c` before any run; `OMP_NUM_THREADS=1 uv run
  --frozen python scripts/evaluate_retina_acuity_population_1_2.py --development
  --candidate x0|x1|x2|x3 --output .peykar/retina-acuity-1.2-dev/round1-<name>`: EXIT=0
  each. `.peykar/retina_acuity11_compare.py`: x0 3.628/4.921/5.556, x1 3.477/4.366/5.429,
  x2 3.482/4.380/5.428, x3 3.228/3.782/5.216 letters; all three adopted. Round 2 declared
  in `dfa874b7`; x4 3.233/3.806/5.218, not adopted against x3.
- `uv run --frozen python scripts/publish_retina_acuity_population_1_2.py --development
  --report .peykar/retina-acuity-1.2-dev/round1-x3/report.json --reference
  .peykar/retina-acuity-1.2-dev/round1-x0/report.json --output
  .peykar/retina-acuity-1.2-dev/round1-x3-published`: exit 0; H1–H5 pass on development
  (H2 3.23 against persistence 3.23; H5 against 1.1.0's frozen primaries 3.58/4.85/5.56).
- `make check-docker` with the new modules: 934 tests passed in 171.43 s, 89% coverage,
  mypy 165 source files, all packages built, EXIT=0
  (`.peykar/retina-acuity12-dev-check-docker-20261029.log`).
## Retinal visual-acuity model 1.1: development, protocol freeze and sealed test: 2026-10-29 (night)
Commands run from the repository root; outcomes observed. Start state: main at
`93283648` (= origin; IOP 1.3 results committed).
- Rounds 0–1 declared in the 1.1 development log, with the module
  `retina_acuity_population_1_1`, the operator and three unit tests, in commit
  `ac6f562c` before any run. Then `OMP_NUM_THREADS=1 uv run --frozen python
  scripts/evaluate_retina_acuity_population_1_1.py --development --candidate w0|w1|w2|w3
  --output .peykar/retina-acuity-1.1-dev/round1-<name>`: EXIT=0 each, 24 training studies,
  57–77 s.
- `uv run --frozen python .peykar/retina_acuity11_compare.py
  .peykar/retina-acuity-1.1-dev/round1-w{0,1,2,3}`: primaries w0 t4 4.327/5.827/7.615
  letters (next observation / landmark / initial state; coverage 0.770/0.830), w1
  anchored t4 3.460/3.965/4.873 (0.840/0.910), w2 t4 4.327/5.814/7.613, w3 t4
  4.515/6.516/8.790; persistence 3.468/4.463, pooled-curve increment 3.805/5.781,
  pooled training curve 5.524. w1 adopted (−0.867/−1.861/−2.742), w2 and w3 rejected by
  the declared rule; no round 2.
- `uv run --frozen python scripts/publish_retina_acuity_population_1_1.py --development
  --report .peykar/retina-acuity-1.1-dev/round1-w1/report.json --reference
  .peykar/retina-acuity-1.1-dev/round1-w0/report.json --output
  .peykar/retina-acuity-1.1-dev/round1-w1-published`: exit 0; H1–H5 pass (H2 against
  persistence 3.46 against 3.47 on 100 targets; H5 3.46/3.97/4.87 against
  4.33/5.83/7.62, the same numbers as the comparison script); model 1.0.0's own H1–H4 on
  the same LOSO: pass, fail, fail, fail.
- `OMP_NUM_THREADS=1 uv run --frozen pytest -q tests/unit/test_eye_retina_acuity_1_1.py
  tests/unit/test_eye_retina_acuity.py`: 10 passed (2 new publisher tests). `uv run
  --frozen ruff format --check .`: 644 files formatted; `ruff check .`: no issue; `mypy
  scripts/publish_retina_acuity_population_1_1.py`: no issue.
- Structure of the sealed set from the custody verifier and an inline read of visit
  months only: 19 series of 6 studies have a visit before month 1 (1 at day 1), last
  visits 1–36 months; custodian exclusion reasons by keyword 22/9/3/1.
- `uv run --frozen python .peykar/write_retina_acuity_protocol_1_1.py`: protocol
  `experiments/eye/retina-acuity/model-1.1/protocol-1.1.0.json`, SHA-256 `c0c37c7e…`, 87
  frozen files, 24 training and 29 sealed record hashes; the writer ran the operator's
  own protocol check first (no problem). `uv run --frozen python
  .peykar/check_frozen_protocols.py`: no problem in any frozen protocol, retinal acuity
  1.1.0 included (retina 1.2.0 has no protocol).
- `make check-docker` after writing the protocol: 930 tests passed in 170.51 s, 89%
  coverage, mypy 161 source files, 18 frontend tests, all packages built, EXIT=0
  (`.peykar/retina-acuity11-freeze-check-docker-20261029.log`). Freeze commit
  `7fce2b4f`, pushed; `git ls-remote origin refs/heads/main` returned the same SHA.
- Then, once: `OMP_NUM_THREADS=1 uv run --frozen python
  scripts/evaluate_retina_acuity_population_1_1.py --sealed --protocol
  experiments/eye/retina-acuity/model-1.1/protocol-1.1.0.json --output
  .peykar/retina-acuity11-sealed-test-2026-10-29` (detached; log ends with `EXIT=0`):
  protocol check passed, 169.7 s, 24 training and 29 sealed record hashes,
  `report.json` `eb8110f1…`, `reference-w0-report.json` `bd3cb6b6…`.
- `OMP_NUM_THREADS=1 uv run --frozen python scripts/publish_retina_acuity_population_1_1.py
  --sealed --report .peykar/retina-acuity11-sealed-test-2026-10-29/report.json
  --reference .peykar/retina-acuity11-sealed-test-2026-10-29/reference-w0-report.json
  --output experiments/eye/retina-acuity/model-1.1/sealed-test-2026-10-29`: exit 0; H1
  pass (77/106, 93/106), H2 fail (3.95 against persistence 3.00 and the pooled-curve
  increment 2.80), H3 fail (6.24 against 3.56 and 3.06), H4 fail (6.65 against 5.87),
  H5 fail (3.95/6.24/6.65 against 3.61/5.01/6.52); model 1.0.0's own H1–H4 all fail.
- `uv run --frozen python .peykar/check_retina_acuity_redaction_1_1.py sealed
  experiments/eye/retina-acuity/model-1.1/sealed-test-2026-10-29`: 15 public and 14
  private studies, 28 redacted study reports (both public reports) and 152 analysis rows
  checked, 0 leaks, exit 0. The publisher's licence rule classes 15 records as public;
  the custody verifier had counted 13 because custodian C7 wrote `license_class` for two
  of its records (both `public`).
- `uv run --frozen python .peykar/retina_acuity11_sealed_breakdown.py
  .peykar/retina-acuity11-sealed-test-2026-10-29/report.json
  .peykar/retina-acuity11-sealed-test-2026-10-29/reference-w0-report.json`: exit 0;
  per-study, per-target-month and per-context tables; removing any single study changes
  no H1–H4 verdict. Inline reads of the private reports: without the linked
  dexamethasone pair H5 would pass and H2–H4 would still fail; forecasts from origins
  before the earliest training control visit of their context (from the training
  records' visit months) 6.04 letters (29 targets) and 8.90 (45) against 2.78 (77) and
  3.02 (61) from later origins. The fitted context parameters in `report-public.json`
  give time constants of 7.58–12.64 days.
- `.peykar/cornea-native47/bin/python
  experiments/eye/retina-acuity/model-1.1/sealed-test-2026-10-29/plot-results.py`: exit
  0. The image viewer showed only the content files had at the start of this session,
  so the figure was opened through a local `python3 -m http.server` (127.0.0.1) in the
  Playwright browser and inspected there; re-rendered twice (legend over a bar label,
  overlapping annotation).
- `make check-docker` before the results commit: 930 tests passed in 163.89 s, 89%
  coverage, mypy 161 source files, all packages built, EXIT=0
  (`.peykar/retina-acuity11-results-check-docker-20261029.log`); the dashboard check
  step was a cached Docker layer (its inputs unchanged since the freeze check, where its
  18 tests ran and passed). `.peykar/check_frozen_protocols.py`: no problem in any
  frozen protocol.
## Acuity 1.1 (closed), generic eye model 4 and retinal visual-acuity model 1.0: 2026-10-29 (evening)
Commands run from the repository root; outcomes observed. Start state: main at
`f6c74736` (= origin; acuity 1.0 results committed). The IOP 1.3 sealed run started at
13:17 kept running throughout.
- Acuity tranche 2 (55 articles) was reserved as the sealed set of acuity 1.1 in commit
  `e2e4d9a3`, before triage. Two reviewer sub-agents read masked previews; the triage,
  split and author-overlap files (27 admitted, every admit sealed; training = the 42
  tranche-1 records) were committed in `7e403d4f`. Four custodian sub-agents built 11
  records with 23 series; `.peykar/verify_acuity_tranche2_custody.py` rebuilt them byte
  for byte with no problem.
- `uv run --frozen pytest -q tests/unit/test_eye_recovery_version4.py`: 8 passed
  (generic eye model 4, whose defaults reproduce version 2 bit for bit).
  `scripts/evaluate_acuity_population_1_1.py --development` over the 42 tranche-1
  studies (exit 0 each, `.peykar/acuity-1.1-dev/`): a0 (1.0.0) robust primary
  5.412/7.136/13.006 letters (next observation / landmark / initial state), pooled-curve
  increment 6.017/7.167, persistence 5.863/9.846, pooled training curve 14.135; b1
  (correlated level and transient, fitted ρ median −0.76) gains 0.084/0.067/0.002; b2
  (cross-linking slope group) loses 0.118 in the initial state. Both rejected by the
  declared 0.10-letter rule (commits `133eac51`, `fcaf7fbf`); no 1.1 protocol.
- Retinal acuity screen: `scripts/screen_corneal_wound_literature.py --queries
  experiments/data/retina-acuity-2026-10-29/screening/queries.json --reuse-fulltext`:
  3,660 articles, EXIT=0; `scripts/screen_corneal_tables.py`: 3,655 full texts, 3,623
  masked tables in 2,140 articles; `screening/triage_rule.py`: 378 articles, 119 further
  ones reserved elsewhere (694 reserved identifiers); tranche order 120/120/120/18.
  Commit `fa525d83` before triage. Four reviewer sub-agents: 62 admitted, 58 excluded;
  `scripts/reserve_retina_acuity_split.py`: 41 development, 21 sealed; author overlap 3
  pairs (commit `a02665d6`).
- Five development extractors and three custodians (sub-agents), addendum 1 between the
  waves: `.peykar/rebuild_retina_acuity_records.py development` rebuilt all 15
  development records byte for byte; `.peykar/verify_retina_acuity_custody.py`: 9
  sealed records rebuilt the same, 21 raw copies equal to the screened sources (commit
  `c162fa43`).
- Development LOSO (`scripts/evaluate_retina_acuity_population.py --development
  --candidate v0|v1|v2|v3`, exit 0 each): primaries by the declared rule 2.778/3.066/4.737
  (v0), 2.772/3.031/4.763 (v1), 2.737/3.066/4.737 (v2), 2.794/3.047/4.742 (v3,
  next-observation coverage 0.841/0.884); v3 adopted under the declared calibration
  clause (rounds declared in `409d5011` and `515c8f9e` before their runs).
- `make check-docker` before the freeze: 922 tests passed in 189.71 s, 89% coverage,
  mypy 160 source files, 18 frontend tests, all packages built, EXIT=0
  (`.peykar/retina-acuity-freeze-check-docker-20261029.log`). Protocol SHA-256
  `34494401…` (71 frozen files, 15 training and 9 sealed record hashes); freeze commit
  `f4352217`, pushed (HEAD = origin).
- Then, once: `uv run --frozen python scripts/evaluate_retina_acuity_population.py
  --sealed --protocol experiments/eye/retina-acuity/model-1.0/protocol-1.0.0.json
  --output .peykar/retina-acuity10-sealed-test-2026-10-29`: protocol check passed, exit
  0, 18.11 s, `report.json` SHA-256 `88e1b9ef…`.
- `uv run --frozen python scripts/publish_retina_acuity_population.py --sealed --report
  .peykar/retina-acuity10-sealed-test-2026-10-29/report.json --output
  experiments/eye/retina-acuity/model-1.0/sealed-test-2026-10-29`: H1–H4 all fail (11
  series, 31 targets; 34 initial-state targets): coverage 21/31 and 25/31; next
  observation 6.14 against persistence 4.81 letters; landmark 9.17 against 6.49; initial
  state 16.40 against the pooled training curve 5.18.
- `uv run --frozen python .peykar/check_retina_acuity_redaction.py sealed
  experiments/eye/retina-acuity/model-1.0/sealed-test-2026-10-29`: 4 public and 5
  private studies, 5 study reports and 20 analysis rows checked, 0 leaks, exit 0.
- `uv run --frozen python .peykar/retina_acuity10_sealed_breakdown.py
  .peykar/retina-acuity10-sealed-test-2026-10-29/report.json`: per-study tables; leaving
  out one study at a time, only the removal of `rac-pmc5377272` makes all four checks
  pass (next observation 3.90 against persistence 4.54; landmark 4.69 against 5.59;
  initial state 4.11 against 4.76; coverage 18/25 and 20/25). Inline reads of the
  records with `peykar.validation.eye_retina_acuity_data`: the earliest development
  visit is at 0.986 months; the only sealed series with a visit before month 1 are the
  two of `rac-pmc5377272` (0.033 months). The fitted transient at day 1 is 1.992 logMAR
  in every sealed fit and 0.58–4.00 across the development folds.
- `.peykar/cornea-native47/bin/python
  experiments/eye/retina-acuity/model-1.0/sealed-test-2026-10-29/plot-results.py`: exit
  0; the figure was inspected and re-rendered twice (legend and title overlap fixed).
- `uv run --frozen python .peykar/check_frozen_protocols.py`: no problem in any frozen
  protocol (retinal acuity 1.0.0 included; retina 1.2.0 has no protocol).
- `make check-docker` before the retinal acuity results commit: 922 tests passed in
  186.75 s, 89% coverage, mypy 160 source files, 18 frontend tests, all packages built,
  EXIT=0 (`.peykar/retina-acuity-results-check-docker-20261029.log`).
- Training-time support check in `peykar eye` (`eye_forecast.time_support`,
  `check_support`; stored fits keep `training_support`): `OMP_NUM_THREADS=1 uv run
  --frozen pytest -q tests/unit/test_eye_forecast.py`: 10 passed (3 new). On the public
  development records the earliest training visit is 0.0329 months for `iop`, the two
  corneal components, `refraction` and `acuity`, and 0.23 months for
  `retina-thickness` (latest 24–120 months). `uv run --frozen peykar eye forecast
  --component retina-thickness --records
  experiments/data/retina-cst-2026-10-28/development/records --context
  human/dme/anti_vegf --baseline 450 --n 30 --months 0.1,3,36`: exit 0, warnings for
  month 0.1 (before 0.23) and month 36 (after 24). `uv run --frozen mypy`: no issues in
  160 source files. `make check-docker`: 925 tests passed in 179.46 s, 89% coverage,
  EXIT=0 (`.peykar/eye-support-check-docker-20261029.log`).
- Retinal-acuity tranche 2 reserved before triage (commit `233dcd27`; briefs written by
  `.peykar/write_retina_acuity_tranche2_briefs.py`). `triage_chunks.py --tranche 1
  --chunks 4` rebuilt the four tranche-1 chunks and id lists byte for byte (`cmp`); the
  four tranche-2 chunks have no decimal outside DOI and licence lines. Four reviewer
  sub-agents: 16 + 15 + 17 + 16 admits; `.peykar/write_retina_acuity_tranche2_triage.py`:
  64 admitted, 56 excluded, reservation 24 development and 64 sealed;
  `scripts/link_study_authors.py --dataset .../tranche-2 --prefix rac-`: 4 sealed-sealed
  pairs, no training exclusion (commit `759c42b5`).
- Custodians: the first start of C1–C4 failed with "Usage limit reached" before any file
  was written (the sealed folder did not exist); C1, then C2–C5 and C6–C8, ran
  afterwards. `uv run --frozen python .peykar/verify_retina_acuity_tranche2_custody.py`:
  64 reserved, 29 records and 35 exclusions, 29/29 rebuilt the same, 64 raw copies
  equal to the screened sources, 81 series, no problem (commit `afa5a99b`).
- IOP 1.3 sealed run: log `.peykar/iop13-sealed-test-2026-10-29.log` ends with `EXIT=0`;
  execution record: 18,215.8 s, protocol SHA `0cac4daf…`, 229 training and 79 sealed
  record hashes, `report.json` `df84eff4…`, reference `3950f0b0…`.
  `OMP_NUM_THREADS=1 uv run --frozen python scripts/publish_iop_population_1_3.py
  --sealed --report .peykar/iop13-sealed-test-2026-10-29/report.json --reference
  .peykar/iop13-sealed-test-2026-10-29/reference-d0-report.json --output
  experiments/eye/iop/model-1.3/sealed-test-2026-10-29`: exit 0; H1–H4 pass, H5 fails
  (initial state 10.94 against 10.69 pp). `.peykar/check_iop_redaction_1_3.py sealed
  ...`: 86 redacted study reports, 0 leaks; an inline check of the 233 analysis rows of
  redacted studies: 0 leaks. `.peykar/iop13_sealed_breakdown.py`: per-time and
  per-context aggregates. The publisher's analysis applied to the reference report
  (inline): H1–H4 hold for 1.2.0 (coverage 0.774/0.872; 6.67, 8.00, 10.69 pp). The
  earliest control visit of the 229 training studies is 0.0329 months (inline, months
  only). `.peykar/cornea-native47/bin/python
  experiments/eye/iop/model-1.3/sealed-test-2026-10-29/plot-results.py`: exit 0; the
  pooled values it computes from the public files equal the published ones (6.642,
  7.923, 10.937; 1.2.0: 6.670, 8.002, 10.691); the figure was inspected and re-rendered
  twice (legend overlaps).
- `make check-docker` before the IOP 1.3 results commit: 925 tests passed in 166.32 s,
  89% coverage, mypy 160 source files, all packages built, EXIT=0
  (`.peykar/iop13-results-check-docker-20261029.log`); `.peykar/check_frozen_protocols.py`:
  no problem in any frozen protocol.
## Machine restart, retinal-thickness 1.2 round 2 and visual-acuity model 1.0: 2026-10-29 (afternoon)
Commands run from the repository root; outcomes observed. Start state: main at
`d7ded65b` (= origin; refraction results committed).
- The machine hung soon after 13:03 CET and its hardware watchdog restarted it at
  13:09 (`uptime`; `/Library/Logs/DiagnosticReports/ResetCounter-2026-10-29-130944.diag`:
  boot faults `wdog,reset_in_1`; no panic report; cause not determined). This stopped
  the IOP 1.3 sealed run after 41 of its 79 d1 studies (no report or execution record
  had been written; the log lists study identifiers only), the eight retina e6 shards
  after 10–12 of 16 studies each (no report), and four triage sub-agents (no decision
  file). Their logs and empty output folders were kept as
  `.peykar/iop13-sealed-test-2026-10-29-attempt1-killed{,.log}` and
  `.peykar/retina-1.2-dev/round2-e6-attempt1-killed/`. Docker Desktop was started
  again and the stopped API and Worker containers were restarted unchanged on the same
  volume (`docker start peykar3-api-1 peykar3-worker-1`; `/healthz` returned
  `{"status":"ok","version":"0.3.0"}`).
- Long runs now start through `.peykar/detach.py` (own session, `caffeinate -i`, exit
  code appended to the log). The IOP 1.3 sealed run was started again from the
  beginning at 13:17 with the same command and protocol (protocol check passed; run in
  progress). The eight e6 shards were rerun unchanged (13:18–14:24, all exit 0) and
  joined by `--merge`; `.peykar/compare_iop_12.py` gave next observation / landmark /
  initial state 7.181/9.412/12.922 pp against e0 7.193/9.376/12.892: e6 rejected;
  `.peykar/retina12_diag_regimen.py` on e6: the regimen biases are almost unchanged.
- Visual acuity (dataset `cornea-acuity-2026-10-29`):
  `scripts/screen_corneal_wound_literature.py --queries .../screening/queries.json
  --output .peykar/acuity-literature-screen-2026-10-29 --workers 6`: 1,994 articles,
  1,992 full texts (PMC13186544 and PMC5179223 HTTP 500); a first start failed before
  any request because the queries file did not exist yet. `scripts/screen_corneal_tables.py`
  with acuity topic, time and corneal-surgery context patterns: 1,185 masked tables in
  780 articles. `screening/triage_rule.py`: 175 articles, 0 reserved elsewhere
  (`screening/reserved.py`: 585 identifiers of retina tranches 3–4 and IOP tranches
  4–5); `screening/tranche_order.py`: 120 in tranche 1, 55 in tranche 2;
  `screening/triage_chunks.py --chunks 4`: four chunks of 30 with no decimal outside
  DOI and licence lines; regenerated after the restart and identical (`diff -r`).
  Commit `bc8fbb9b` before triage.
- Four reviewer sub-agents: 12 + 14 + 21 + 20 admits; `.peykar/write_acuity_triage.py`:
  67 admitted, 658 excluded (605 by the rule, 53 by the reviewers), 55 reserved; `scripts/reserve_acuity_split.py`:
  47 development, 20 sealed; `scripts/link_study_authors.py --prefix acu- --schema
  cornea.acuity-author-overlap-1.0.0`: 7 pairs, 2 development clusters, 2 sealed
  training exclusions. `uv run --frozen pytest -q tests/unit/test_cornea_acuity.py`: 3
  passed. `make check-docker`: 906 tests passed in 258.66 s, 89% coverage, mypy 153
  source files, EXIT=0. Commit `3877950b` before extraction.
- Five development extractors and three custodians (sub-agents), in two waves; addendum
  1 (`.peykar/acuity-extraction-brief-addendum-1.md`) was written between the waves.
  `uv run --frozen python .peykar/rebuild_acuity_records.py development`: 27 records
  rebuilt the same, 3 exclusion-only specs. `uv run --frozen python
  .peykar/verify_acuity_custody.py`: 20 reserved, 15 records and 5 exclusions, 15/15
  rebuilt the same, 20 raw copies equal to the screened sources, 25 series, no problem.
- Development LOSO (`scripts/evaluate_acuity_population.py --development --candidate
  <a0|a1|a2|a4> --shard i/4`, about 30–40 s per shard) joined by `--merge`;
  `.peykar/acuity_compare.py` (letters, next observation / landmark / initial state):
  a0 5.434/7.588/15.918, a1 5.332/7.283/16.107, a2 5.320/7.346/16.213, a4
  5.387/7.569/15.918; pooled-curve increment 5.130/7.179; pooled training curve
  16.804 (initial state). Round 1 declared in commit `a67bd141`, round 2 in `41ef088d`,
  each before its run.
- `uv run --frozen python scripts/publish_acuity_population.py --development --report
  .peykar/acuity-1.0-dev/round1-a0/report.json --output ...`: H1 and H4 pass, H2 and H3
  fail (pooled-curve increment).
- `make check-docker` before the freeze: a first run stopped at `ruff format --check`
  (the editor had re-wrapped `scripts/publish_acuity_population.py`); after `ruff
  format`, the not yet used protocol was deleted and written again; second run: 908
  tests passed in 164.07 s, 89% coverage, mypy 154 source files, EXIT=0.
  `uv run --frozen python .peykar/write_acuity_protocol.py`: protocol SHA-256
  `0c60925c…` (62 frozen files, 27 training and 15 sealed record hashes);
  `.peykar/check_frozen_protocols.py`: no problem in any of the twelve frozen protocols.
- Acuity freeze commit `baafe4cd`, pushed (HEAD = origin). Then, once,
  `uv run --frozen python scripts/evaluate_acuity_population.py --sealed --protocol
  experiments/eye/acuity/model-1.0/protocol-1.0.0.json --output
  .peykar/acuity10-sealed-test-2026-10-29`: protocol check passed, exit 0, 52.1 s,
  `report.json` SHA-256 `194c4ee7…`.
- `uv run --frozen python scripts/publish_acuity_population.py --sealed --report
  .peykar/acuity10-sealed-test-2026-10-29/report.json --output
  experiments/eye/acuity/model-1.0/sealed-test-2026-10-29`: H1–H4 pass (21 series, 41
  targets; 59 initial-state targets): coverage 34/41 and 38/41; next observation 5.00
  against persistence 5.06 letters; landmark 5.24 against 7.27; initial state 9.04
  against the pooled training curve 9.79. `.peykar/check_acuity_redaction.py` on the
  sealed and development folders: 0 leaks (6 and 10 redacted studies).
  `uv run --frozen python .peykar/acuity10_sealed_breakdown.py`: per-context and
  per-period tables. `.peykar/cornea-native47/bin/python
  experiments/eye/acuity/model-1.0/sealed-test-2026-10-29/plot-results.py`: exit 0; the
  figure was inspected.
- `uv run --frozen pytest -q tests/unit/test_eye_forecast.py` after adding `acuity` to
  `peykar eye`: 7 passed (a first run failed because the synthetic test records had
  too few follow-ups for a noise estimate). `uv run --frozen peykar eye forecast
  --component acuity --records experiments/data/cornea-acuity-2026-10-29/development/records
  --context human/dmek --baseline 0.9 --n 30 --months 3,12 --observed 1:0.4`: exit 0,
  robust assimilation, logMAR 0.22 and 0.17.
- `make check-docker` before the acuity results commit: a first run was stopped by hand
  after `ruff format --check .` reported `tests/unit/test_eye_forecast.py` (re-wrapped by
  the editor); after `ruff format`, 909 tests passed in 163.96 s, 89% coverage, mypy 154
  source files, EXIT=0 (`.peykar/acuity-results-check-docker-20261029.log`);
  `.peykar/check_frozen_protocols.py`: no problem in any frozen protocol.
## Generic eye model 3, IOP model 1.3, retinal-thickness model 1.2 and refraction model 1.0: 2026-10-29
Commands run from the repository root; outcomes observed. Start state: main at
`e6c077ee` (= origin; IOP 1.2 published, `peykar eye` CLI committed).
- `uv run --frozen pytest -q tests/unit/test_eye_recovery_version3.py` (7 tests: the
  point rule, harness 3 with the median reproduces harness 2, the lognormal mean moves
  only population points, the IOP sub-procedure rules, the 1.3 candidates, and the 1.3
  publisher's human-only split, initial-state allow-list and H5 rule): passed.
- IOP 1.3 development, each LOSO run over the 229 training studies in four shards
  (`scripts/evaluate_iop_population_1_3.py --development --candidate <d0|d1|d2>
  --shard i/4`, 9,058–11,801 s per shard) joined by `--merge`; human group means,
  next observation / landmark / initial state (pp): d0 (= 1.2.0) 6.464/8.348/10.209,
  d1 (sub-procedure level) 6.382/8.061/10.066, d2 (lognormal-mean point)
  6.528/8.421/10.215 (`.peykar/compare_iop_12.py`, `.peykar/iop13_round1_breakdown.py`).
  d1 adopted and d2 rejected by the declared rule; d3 not run.
- `uv run --frozen python scripts/publish_iop_population_1_3.py --development --report
  .peykar/iop-1.3-dev/round1-d1/report.json --reference
  .peykar/iop-1.3-dev/round1-d0/report.json --output
  experiments/eye/iop/model-1.3/development-loso-2026-10-29`: exit 0;
  `.peykar/check_iop_redaction_1_3.py development …`: 0 leaks in 254 reports of
  redacted studies (102 public and 127 private training studies).
- `ruff check .`, `ruff format --check .` (566 files) and `mypy` (149 source files):
  passed. `make check-docker` before the IOP 1.3 freeze: 897 tests passed in 378.67 s
  (twelve retina LOSO processes running alongside), 89% coverage, mypy 149 source
  files, 18 frontend tests, package builds; make finished without error
  (`.peykar/iop13-freeze-check-docker-20261029.log`).
- `uv run --frozen python .peykar/write_iop_protocol_1_3.py`: protocol SHA-256
  `0cac4daf…` (68 frozen files, 229 training and 79 sealed record hashes, 25 sealed
  studies with author-overlap training exclusions); the 1.3 operator's protocol check
  returned no problem, and `.peykar/check_frozen_protocols.py` found no problem in any
  of the ten frozen protocols (corneal 1.0.0 and 2.0.0, endothelium, corneal
  thickness, IOP 1.0.0–1.3.0, retina 1.0.0 and 1.1.0).
- IOP 1.3 freeze commit `38f4f874`, pushed (HEAD = origin); then
  `uv run --frozen python scripts/evaluate_iop_population_1_3.py --sealed --protocol
  experiments/eye/iop/model-1.3/protocol-1.3.0.json --output
  .peykar/iop13-sealed-test-2026-10-29` started (protocol check passed, run in progress).
- Refraction after corneal laser refractive surgery (dataset
  `cornea-refraction-2026-10-29`): `scripts/screen_corneal_wound_literature.py --queries
  experiments/data/cornea-refraction-2026-10-29/screening/queries.json --output
  .peykar/refraction-literature-screen-2026-10-29 --workers 6`: 2,169 articles, 2,168
  full texts (PMC8651116 HTTP 500). `scripts/screen_corneal_tables.py` with refraction
  topic, month/year time and refractive-surgery context patterns: 1,215 masked tables in
  793 articles (a first run without `--time` used the hour/day default and was deleted
  unread). `screening/triage_rule.py`: 89 articles; `screening/tranche_order.py`: all 89
  in tranche 1; `screening/triage_chunks.py --tranche 1`: three chunks of 30/30/29 whose
  only decimals are licence version numbers. Commit `2a525e17` before triage.
- Three reviewer sub-agents (masked previews only): 15 + 12 + 12 admits;
  `.peykar/write_refraction_triage.py`: 39 admitted, 50 excluded, no override;
  `scripts/reserve_refraction_split.py`: 31 development, 8 sealed;
  `scripts/link_study_authors.py --prefix ref- --schema
  cornea.refraction-author-overlap-1.0.0`: 4 pairs, 2 development clusters, no sealed
  training exclusion. Commit `e14aaf5c`.
- Three development extractors and two sealed custodians (sub-agents). Four admissible
  studies were blocked by the shared cell reader (a minus sign set apart from its digits
  by a no-break, thin or ordinary space; one figure dash). After `refraction_number`
  was extended, `.peykar/rebuild_refraction_records.py development` rebuilt all 21
  earlier development records byte for byte; the three blocked development records
  were built (24 records, 43 series), and the resumed custodian C2 built the blocked
  sealed record and found its three earlier records unchanged by rebuild hash.
  `uv run --frozen python .peykar/verify_refraction_custody.py`: 8 reserved, 8
  records, 8/8 rebuilt byte for byte, 8 raw copies equal to the screened sources, 14
  series, no problem.
- `uv run --frozen pytest -q tests/unit/test_cornea_refraction.py` (5 tests: record
  validation and exact pseudo-fractions, signed and spaced cells, additive scores equal
  to native errors in report units with the target comparator, and the refraction
  publisher's H4/H5 rules and redaction): passed.
- Refraction 1.0 development: `scripts/evaluate_refraction_population.py --development
  --candidate <f0|f1>` (24 studies, about 90 s each), rerun as `round1t-*` after the
  plano-target comparator was added (model forecasts identical); primary (robust
  variant) next observation / landmark / initial state in D: f0 0.153/0.326/0.552, f1
  0.157/0.322/0.538; persistence 0.163/0.348; plano target 0.616/0.616/0.575. f1
  adopted. `scripts/publish_refraction_population.py --development` on both reports:
  H1–H5 pass; `.peykar/check_refraction_redaction.py development`: 0 leaks in 22
  reports of redacted studies.
- `make check-docker` before the refraction freeze: 902 tests passed in 205.80 s, 89%
  coverage, mypy 152 source files, package builds, EXIT=0
  (`.peykar/refraction-freeze-check-docker-20261029.log`).
- `uv run --frozen python .peykar/write_refraction_protocol.py`: protocol SHA-256
  `8634f5a2…` (58 frozen files, 24 training and 8 sealed record hashes);
  `.peykar/check_frozen_protocols.py`: no problem in any of the eleven frozen protocols.
- Refraction freeze commit `a2eccf2c`, pushed (HEAD = origin). Then, once,
  `uv run --frozen python scripts/evaluate_refraction_population.py --sealed --protocol
  experiments/eye/refraction/model-1.0/protocol-1.0.0.json --output
  .peykar/refraction10-sealed-test-2026-10-29`: protocol check passed, f1 and the f0
  reference scored in the same run, exit 0, 39.9 s; `report.json` SHA-256 `4c3ca270…`,
  `reference-f0-report.json` `a05e719a…`.
- `uv run --frozen python scripts/publish_refraction_population.py --sealed --report
  .peykar/refraction10-sealed-test-2026-10-29/report.json --reference
  .peykar/refraction10-sealed-test-2026-10-29/reference-f0-report.json --output
  experiments/eye/refraction/model-1.0/sealed-test-2026-10-29`: H1–H5 pass (13 forecast
  series, 28 targets; 34 initial-state targets): coverage 23/28 and 24/28; next
  observation 2.93 against persistence 3.70 pp; landmark 4.68 against 7.80; initial
  state 3.97 against the plano target 5.01 and f0 4.13 (1 pp = 0.1 D).
  `uv run --frozen python .peykar/refraction10_sealed_breakdown.py`: per-period and
  per-procedure tables (after month 6: plano 0.750 D, model 0.204/0.631 D next
  observation/landmark). The development LOSO reports were published with
  `--development` into `development-loso-2026-10-29/`.
  `uv run --frozen python .peykar/check_refraction_redaction.py <development|sealed>
  <public folder>` on both folders: 0 leaks in 22 reports of the 11 redacted
  development studies and in 8 reports of the 4 redacted sealed studies.
- `.peykar/cornea-native47/bin/python
  experiments/eye/refraction/model-1.0/sealed-test-2026-10-29/plot-results.py`: exit 0;
  the figure was inspected, and the script was then changed (legend placement, x range
  from surgery, per-series legend, panel titles stating the severe-myopia miss) and
  rerun, exit 0.
- Retina 1.2 round 1, each candidate LOSO over the 128 training studies in four shards
  (`scripts/evaluate_retina_population_1_2.py --development --candidate <e0|e3|e4>
  --shard i/4`, 4,722–5,014 s per shard for e0) joined by `--merge`;
  `.peykar/compare_iop_12.py`: next observation / landmark / initial state (pp) e0
  7.193/9.376/12.892, e3 7.162/9.407/12.945, e4 7.196/9.381/12.818; e3 and e4 rejected.
  `uv run --frozen python .peykar/retina12_diag_regimen.py` (training LOSO of e0 only):
  initial-state bias by regimen −11.9 pp (fixed monthly, 10 targets) to +2.2 pp (fixed
  interval, 17). Round 2 (e6, regimen level) declared, commit `04dd314c`, pushed; eight
  shards started (`--candidate e6 --shard i/8`).
- `uv run --frozen pytest -q tests/unit/test_eye_forecast.py` after adding the additive
  refraction component to `peykar eye`: 11 tests passed; `ruff check`, `ruff format
  --check` and `mypy` (152 source files) passed.
- `make check-docker` before the refraction results commit (eight retina LOSO shards and
  the IOP 1.3 sealed run alongside): 903 tests passed in 346.84 s, 89% coverage, mypy
  152 source files, 18 frontend tests, package builds, EXIT=0
  (`.peykar/refraction-results-check-docker-20261029.log`).
  `.peykar/check_frozen_protocols.py`: no problem in any of the eleven frozen protocols.
## Fresh sealed tranches, generic eye model 2, retinal-thickness model 1.1 and IOP model 1.2: 2026-10-28/29
Commands run from the repository root; outcomes observed. Start state: main at
`76c575ce` (= origin; retina tranche 2 and IOP tranche 3 triaged and reserved).
- Seven retina tranche-2 custodian sub-agents, then
  `uv run --frozen python .peykar/verify_retina_tranche2_custody.py`: 85 reserved
  studies, 63 records and 22 exclusions, 63/63 records rebuilt byte for byte, 0 raw
  copies differing from the screened source, 135 series, no problem. Commit
  `d98aa812`, pushed.
- Eight IOP tranche-3 custodian sub-agents, then
  `uv run --frozen python .peykar/verify_iop_tranche3_custody.py`: 97 reserved
  studies, 83 records and 14 exclusions, 83/83 rebuilt byte for byte, 97 raw copies
  equal to the screened sources, 196 series, no problem. Commit `78ceec47`, pushed.
- `tests/unit/test_eye_recovery_population2.py` (11 tests: bit-for-bit equivalence
  with generic model 1, exact joint-Gaussian checks of early noise, wear-off,
  fallback, pooled noise, per-group study effects against `iop_population_1_1`) and
  `tests/unit/test_eye_recovery_version2_components.py` (4 tests): pass.
- `uv run --frozen python scripts/evaluate_retina_population_1_1.py --development
  --candidate <s0..s5>` and `--model 1.0.0` (65 studies, 1,100–1,800 s each): s0
  equals model 1.0.0 with its own harness in every score; group-mean next
  observation / landmark / initial state (pp): s0 7.204/9.047/13.285 (robust), s1
  7.225/9.056/13.302, s2 6.375/7.945/12.163, s4 6.473/8.003/11.755, s5
  6.368/7.952/12.153 with coverage 0.848/0.930. s2 then s5 adopted by the declared
  rules; a rerun of s2 with the final code reproduced its report.
- `make check-docker` before the retina freeze: 885 tests passed in 231.77 s, 89%
  coverage, mypy 140 source files, 18 frontend tests, package builds; EXIT=0
  (`.peykar/retina11-prefreeze-check-docker-2026-10-28.log`).
- `.peykar/write_retina_protocol_1_1.py .peykar/retina-1.1-dev/round3-s5 s5
  population_assimilation`: protocol SHA-256 `fd14a1c5…` (53 frozen files, 65 training
  and 63 sealed record hashes); the operator's protocol check returned no problem.
  Freeze commit `84ee1e9c`, pushed; HEAD = origin.
- `uv run --frozen python scripts/evaluate_retina_population_1_1.py --sealed
  --protocol experiments/eye/retina/model-1.1/protocol-1.1.0.json --output
  .peykar/retina11-sealed-test-2026-10-28`: hashes verified, 2,381.9 s, exit 0; report
  SHA-256 `00554481…`, reference 1.0.0 `85cb7cbb…`.
- `uv run --frozen python scripts/publish_retina_population_1_1.py --sealed --report …
  --reference … --output experiments/eye/retina/model-1.1/sealed-test-2026-10-28/public`
  (files then moved one level up): H1–H4 pass, H5 fails (next observation 7.91 vs
  1.0.0 7.79, landmark 9.90 vs 9.73, initial state 13.00 vs 13.25 pp); coverage
  0.844/0.896. `.peykar/check_retina_redaction_1_1.py sealed …`: no value in the 58
  report entries of the 29 non-CC-BY studies; 34 CC BY studies keep values.
- `.peykar/cornea-native47/bin/python
  experiments/eye/retina/model-1.1/sealed-test-2026-10-28/plot-results.py`: figure
  written.
- IOP 1.2 development, each LOSO run in four shards (`--shard i/4`, 2,650–3,270 s per
  shard) joined by `--merge`: c0 equals the frozen 1.1.0 with its own harness in all
  195,431 numbers (only a text label differs); next observation / landmark / initial
  state (pp) c0 6.840/8.803/10.585, c2 6.912/8.836/10.585, c1 6.851/8.808/10.583;
  first-month next-observation coverage c0 0.639/0.822, c1 0.826/0.904.
  `.peykar/iop12_early_noise_calibration.py` chose k = 8, T = 1 month before c1 ran.
  c1 adopted by the declared rule.
- `make check-docker` before the IOP freeze: 885 tests passed in 159.39 s, 89%
  coverage, mypy 140 source files, package builds; EXIT=0
  (`.peykar/iop12-prefreeze-check-docker-2026-10-29.log`).
- `.peykar/write_iop_protocol_1_2.py .peykar/iop-1.2-dev/round2-c1 c1
  population_assimilation`: protocol SHA-256 `4169750d…` (52 frozen files, 146 training
  and 83 sealed record hashes); protocol check returned no problem. Freeze commit
  `df8f389a`, pushed; HEAD = origin. This happened before any retina 1.1 sealed
  result was read.
- After the retina documentation: both protocols' checks still return no problem
  (every pinned file, training and sealed record unchanged); `make check-docker`: 885
  tests passed in 162.96 s, 89% coverage, mypy 140 source files, 18 frontend tests,
  package builds, EXIT=0 (`.peykar/retina11-results-check-docker-2026-10-29.log`);
  host `make build`: EXIT=0.
- Fresh tranches reserved before triage (retina tranche 3 for model 1.2, IOP tranche
  4 for model 1.3): commit `02b64d6e`, pushed. `screening/triage_chunks.py` of the
  retina dataset rebuilt the tranche-1 and tranche-2 chunks byte for byte, then wrote
  three tranche-3 chunks of 40; the IOP script with `--tranche 4 --check-tranche-1`
  rebuilt the tranche-1 chunks byte for byte and wrote three tranche-4 chunks. A scan
  of both chunk sets found no decimal and no ± followed by a digit.
- Six reviewer sub-agents (masked previews only): retina 27+28+33 admits, IOP
  30+31+30 admits. `.peykar/write_tranche_triage_reservation.py retina-t3`: 88
  admitted, 32 excluded, 128 development and 88 sealed studies, no override.
  `... iop-t4`: the first write applied the tranche-1 percent rule to two exclusions;
  a third (PMC4817329) was found from the reasons, the four outputs were deleted
  before any commit or extraction (hashes in
  `.peykar/iop-t4-first-write-superseded.sha256`) and rewritten: 94 admitted, 26
  excluded, 229 development and 94 sealed studies.
- `scripts/link_study_authors.py` on both tranche reservations: retina 60 pairs, 19
  sealed studies with training exclusions, 4 sealed–sealed pairs, 10 development
  clusters; IOP 154 pairs, 34, 17 and 35.
- Seven retina tranche-3 custodian sub-agents, then
  `uv run --frozen python .peykar/verify_retina_tranche3_custody.py`: 88 reserved
  studies, 67 records and 21 exclusions, 67/67 records rebuilt byte for byte, 67 raw
  copies equal to the screened sources, 134 series, no problem. (A first run of the
  derived check stopped on a covariate the retina validator does not return; the line
  was removed and the check rerun.)
- Eight IOP tranche-4 custodian sub-agents, then
  `uv run --frozen python .peykar/verify_iop_tranche4_custody.py`: 94 reserved
  studies, 79 records and 15 exclusions, 79/79 rebuilt byte for byte, 79 raw copies
  equal to the screened sources, 222 series, no problem.
- `uv run --frozen python scripts/evaluate_iop_population_1_2.py --sealed --protocol
  experiments/eye/iop/model-1.2/protocol-1.2.0.json --output
  .peykar/iop12-sealed-test-2026-10-29`: hashes verified, 7,260.0 s, exit 0; report
  SHA-256 `3275c150…`, reference 1.1.0 `e40762bd…`; 24 sealed studies trained without
  their author-linked studies.
- `uv run --frozen python scripts/publish_iop_population_1_2.py --sealed --report …
  --reference …/reference-1.1.0-report.json --output
  experiments/eye/iop/model-1.2/sealed-test-2026-10-29`: all five hypotheses pass on
  130 human group means (coverage 435/521 = 0.835 and 485/521 = 0.931; next
  observation 5.89 vs persistence 7.21 pp; landmark 7.76 vs 11.89; initial state 10.17
  vs 18.06; first-month coverage 0.824/0.939 vs 1.1.0 0.679/0.817 at 5.885 vs 5.877
  pp). The same publisher with `--development` on the c1 and 1.1.0 LOSO reports wrote
  `development-loso-2026-10-29/`.
- `.peykar/check_iop_redaction_1_2.py sealed …` (the retina 1.1 check with the IOP 1.2
  publisher's licence classes) first reported 92 leaks: the frozen publisher kept the
  `reference_value` field in initial-state entries of the 53 redacted studies (159 in
  the development folder). `experiments/eye/iop/model-1.2/redact-reference-values.py`
  removed them (a second run removed none); the check then found 0 leaks in 106 sealed
  and 148 development reports of redacted studies. No earlier public report carries
  that field for a redacted study (scan of every `report-public.json` and
  `reference*-public.json` under `experiments/`).
- `.peykar/cornea-native47/bin/python
  experiments/eye/iop/model-1.2/sealed-test-2026-10-29/plot-results.py`: figure written;
  `.peykar/iop12_sealed_breakdown.py` printed the aggregates quoted in the 1.2 README.
- After the documentation edits, the protocol checks of all nine frozen protocols
  (corneal 1.0.0 and 2.0.0, endothelium, corneal thickness, IOP 1.0.0, 1.1.0 and 1.2.0,
  retina 1.0.0 and 1.1.0) returned no problem.
- `peykar eye` CLI: `uv run --frozen pytest -q tests/unit/test_eye_forecast.py` (5 tests:
  a stored JSON fit round-trips, refuses another configuration and forecasts like the
  harness; the initial state applies the dose rule and warns for untrained contexts;
  the CLI fits and forecasts from record folders; both corneal components use their
  frozen primaries) passed; mypy on the three new modules passed;
  `uv run --frozen peykar eye components` printed the four components.
- `make check-docker` with the IOP 1.2 documentation and the CLI in place: 890 tests
  passed in 128.98 s, 89% coverage, mypy 143 source files, 18 frontend tests, package
  builds, EXIT=0 (`.peykar/iop12-results-check-docker-2026-10-29.log`); host `make
  build`: EXIT=0.

## Retinal thickness after treatment: dataset, model 1.0.0, protocol freeze and sealed test: 2026-10-28

Commands run from the repository root; outcomes observed.

- `uv run --frozen python scripts/screen_corneal_wound_literature.py --queries
  experiments/data/retina-cst-2026-10-28/screening/queries.json --output
  .peykar/retina-literature-screen-2026-10-28 --workers 6`: 3,128 articles screened,
  3,124 full texts (four HTTP 500: PMC11238598, PMC11764373, PMC8005793, PMC8898177).
- `scripts/screen_corneal_tables.py` with a central-thickness topic, time and
  retinal-treatment context pattern: 1,638 articles and 2,548 tables flagged, masked.
- `screening/triage_rule.py`: 395 articles qualify; `screening/tranche_order.py`:
  tranches of 120/120/120/35 (6 without DOI). Commit `176c82d1` before triage.
- Three triage sub-agents on masked chunks (`screening/triage_chunks.py`): 78 admit,
  42 exclude; `scripts/reserve_retina_split.py`: 54 development, 24 sealed test;
  `scripts/link_study_authors.py`: 8 pairs, 3 development clusters, 3 sealed training
  exclusions. Commit `421e88cf`, pushed.
- Seven extraction sub-agents: 45 development records and 19 sealed records first;
  after the change-sign fix in `thickness_number`, `.peykar/rebuild_retina_records.py`
  rebuilt all 64 records byte-identical (sealed by SHA-256 only), and the 46th
  development record was built. Commit `e7475e44`.
- `tests/unit/test_eye_recovery_population.py` (7 tests, including the IOP 1.1
  equivalence to 1e-9 and the exact joint-Gaussian check of the deviation forecast)
  and `tests/unit/test_eye_recovery_evaluation.py` (2 tests): pass.
- `uv run --frozen python scripts/evaluate_retina_population.py --development
  --candidate <r0|r1|r2a|r2b|r2c|r2d|r3> --output .peykar/retina-dev-<c>-2026-10-28`:
  seven LOSO runs over 46 studies (r0 160.8 s). Group-mean next observation /
  landmark / initial state (pp): r0 8.76/10.35/13.89, r1 8.65/10.05/13.96, r2d
  8.20/10.11/14.25, r3 8.23/10.31/13.89; persistence 8.67/13.20. r3 adopted by the
  declared rule.
- `make check-docker` before the freeze: 870 tests passed in 141.88 s, 89% coverage,
  mypy 134 source files, 18 frontend tests, package builds; EXIT=0
  (`.peykar/retina-freeze-check-docker-20261028.log`).
- `.peykar/write_retina_protocol.py .peykar/retina-dev-r3-2026-10-28 r3
  population_assimilation r0`: protocol SHA-256 `64b0ad17…` (40 frozen files, 46
  training and 19 sealed record hashes). Freeze commit `d40b6ed2`, pushed; HEAD =
  origin.
- `uv run --frozen python scripts/evaluate_retina_population.py --sealed --protocol
  experiments/eye/retina/model-1.0/protocol-1.0.0.json --output
  .peykar/retina-sealed-test-2026-10-28`: hashes verified, 146.6 s, EXIT=0; report
  SHA-256 `03917257…`, reference r0 `0c3dabbc…`.
- `uv run --frozen python scripts/publish_retina_population.py --sealed --report …
  --reference … --output experiments/eye/retina/model-1.0/sealed-test-2026-10-28`:
  H1–H5 pass (next observation 5.75 vs persistence 6.34 and r0 6.14 pp; coverage
  0.816/0.918; landmark 7.50 vs 9.50; initial state 11.35 vs 17.09). The redaction
  check found no value in the 7 non-CC-BY sealed studies; 12 CC BY studies keep
  values.
- `.peykar/cornea-native47/bin/python
  experiments/eye/retina/model-1.0/sealed-test-2026-10-28/plot-results.py`: figure
  written.
- After the results and documentation: every protocol-pinned file still matches its
  frozen hash (`.peykar/check_retina_frozen.py`); `make check-docker`: 870 tests
  passed in 124.15 s, 89% coverage, mypy 134 source files, 18 frontend tests, package
  builds, EXIT=0 (`.peykar/retina-results-check-docker-20261028.log`); host
  `make build`: EXIT=0.

## IOP tranche 2 and model 1.1: blind triage, custody, freeze and sealed test: 2026-10-28

- Start state: main at `c93d5018` (= origin, IOP 1.0.0 results committed).
- `experiments/data/iop-glaucoma-surgery-2026-10-28/screening/triage_chunks.py --tranche 1
  --check-tranche-1` rebuilt all three tranche-1 triage chunks and their id lists byte
  for byte. With `--tranche 2` it wrote three masked chunks of 40 articles. The
  reservation rule (tranche-2 README) was committed before triage (`ac2fad66`).
- Three reviewer sub-agents wrote the masked-preview decisions (84 admit, 36
  exclude). `.peykar/write_iop_tranche2_triage.py` applied the tranche-1
  percent-change rule to three exclusions and wrote `tranche-2/triage-1.0.0.json`
  (87 admitted, 33 excluded) and the reservation (72 development, 87 sealed). Commit
  `bd2882a8`.
- `scripts/link_study_authors.py --dataset .../tranche-2 --raw
  .peykar/iop-literature-screen-2026-10-28/raw --prefix iop- --schema
  eye.iop-author-overlap-1.0.0` found 37 pairs, 7 development clusters, 18 sealed
  studies with training exclusions and 10 sealed–sealed pairs.
- The first launch of seven custodian sub-agents returned "usage limit reached". Only
  C5 had written files (7 studies, no report). Resumed C5 rebuilt those 7 records with
  identical SHA-256 values and finished; C1–C4, C6 and C7 then ran from scratch. A
  structure-only check rebuilt all 74 records from their specs and hash-checked source
  copies (74/74 equal) and printed counts only: 192 series, 13 admitted studies
  without a record. Commit `d6a687ad`.
- `scripts/evaluate_iop_population_1_1.py --development --model 1.0.0` (frozen 1.0.0
  harness over the 72 tranche-1 records): exit 0, 380.2 s, report
  `8ddcd757cc8a5c2cb5a8353efaa0c278539a3c0afe5792b19eff433d99b4a819`. On group means
  the Gaussian variant scored 6.849 pp next observation and 8.757 landmark.
- `experiments/eye/iop/model-1.1/diagnostics_1_1.py` (training data only): without a
  transient the residual SD was 0.0755; with one it was 0.0636 (3 d), 0.0621 (7 d)
  and 0.0629 (14 d). Group-mean SD scales as `ln sd = −1.986 − 0.306 ln n`; lag-1
  residual correlation −0.294.
- `pytest tests/unit/test_iop_population_1_1.py`: the first run had **1 failure**. The
  test's own expected shape was wrong: with its parameters the course peaks at month
  1, not week 1. After correcting the expectation, **12 passed** (3.91 s). Strict
  `mypy` passed on the two new modules; ruff passed on all new files.
- `scripts/evaluate_iop_population_1_1.py --development` (model 1.1.0): exit 0, 642.2
  s, report `02ec89d5c73427e4906480c72c7cef311ee49ae858546228177e1a13134f1c76`.
  Gaussian group means: 6.773 pp next observation, 8.555 landmark, coverage
  0.754/0.866. A repeat run (661.0 s) produced the same report SHA.
- `scripts/publish_iop_population_1_1.py --development --reference <round-0 report>`:
  38 studies redacted in both public reports, and a scan found no values in them.
- The first `make check-docker` **exited 2**: the editor had re-wrapped
  `scripts/publish_iop_population_1_1.py` and `ruff format --check` failed. After
  `ruff format`, **exit 0: 858 Python tests passed in 134.79 s**, 89% coverage, strict
  mypy 130 files, 18 frontend tests, contracts and all builds
  (`.peykar/iop11-prefreeze-check-docker-2026-10-28.log`).
- `experiments/eye/iop/model-1.1/protocol-1.1.0.json` written exclusively, SHA
  `33dff2202a7853143faff4c13b3932028191beb890501093de2bbe3c0080352f`. It pins 42
  files, 72 training and 74 sealed record hashes; `check_protocol` reported no
  problem. Commit `ac8abf83` pushed; local `HEAD` equals `origin/main`.
- `scripts/evaluate_iop_population_1_1.py --sealed --protocol .../protocol-1.1.0.json`:
  run once, exit 0, 996.3 s, all pins matched. Report
  `3c126572aa10273ec2fd4509a9d46a72defd171ddd7e6b382f4c29940e3c209b`; 1.0.0 reference
  report `7e04c9b9e11c7113955f33645b2396e36ae0f1d07a28499579f35e534b28b681`.
- `scripts/publish_iop_population_1_1.py --sealed --reference ...`: verdicts H1–H5 all
  true.
  - H1: 341/425 = 0.802 and 381/425 = 0.896.
  - H2: 6.69 pp against 8.06, 44.56, 8.60 and 9.90.
  - H3: 8.95 against 15.10, 15.44 and 17.60.
  - H4: 9.71 against 17.88 for the pooled curve on its own groups, or 18.06 on the
    same 99 groups.
  - H5: 6.69 against 6.92 and 8.95 against 9.40.

  36 studies were redacted, and the same scan found nothing. The plot ran with
  `.peykar/cornea-native47/bin/python`; the PNG was inspected and the legend was
  moved before the final render.
- After the result documents: every pin of the IOP 1.1 and 1.0.0, thickness,
  endothelium and both corneal population protocols still matched. `make check-docker`
  **exit 0, 858 Python tests passed in 117.70 s**, 89% coverage, strict mypy 130
  files, 18 frontend tests, 479 files formatted, contracts and all builds
  (`.peykar/iop11-results-check-docker-2026-10-28.log`).

## IOP after glaucoma procedures: model 1.0.0, protocol freeze and sealed test: 2026-10-28

- Start state: main at `c125cf64` (= origin). The dataset split, development and
  sealed extraction were committed in `96d7b828`, `f5c5b535` and `c125cf64`; their
  commands are listed in the dataset README. The model, harness, operator and
  author-overlap file were uncommitted.
- `scripts/link_study_authors.py --dataset experiments/data/iop-glaucoma-surgery-2026-10-28
  --raw .peykar/iop-literature-screen-2026-10-28/raw --prefix iop- --schema
  eye.iop-author-overlap-1.0.0`: 13 linked pairs, 4 development clusters, 7 sealed
  training exclusions (3 for sealed studies with records), no sealed–sealed pair. A
  rerun into `/tmp` produced the same file apart from its UTC field.
- `experiments/eye/iop/round0_law_comparison.py` (development only): group-mean
  log-SD 0.0682 for the exponential return against 0.0716 (hyperbolic), 0.0760
  (power 0.5) and 0.0698 (power 2); late-level slope −0.765 over 76 control group
  means. The output equals that of the private round-0 script.
- `scripts/evaluate_iop_population.py --development`: round 1 exit 0, 162.6 s,
  report `0725e53d…` (robust next observation 9.45 pp). The first round-2 command
  chain stopped at a ruff SIM401 finding in `iop_population.py` before the
  evaluation started. After the one-line fix, round 2 exit 0, 167.2 s, report
  `d5a39284447596f222241a1b26bba68c72e80cb7a5ef38a17f8b2571c6d984a0` (robust 9.455 pp
  next observation and 13.389 landmark; group means 6.34 and 8.66). A repeat run
  (162.6 s) produced the same report SHA.
- `pytest tests/unit/test_iop_population.py tests/unit/test_eye_iop_data.py`:
  **21 passed** (3.34 s). `ruff check .`, `ruff format --check .` (478 files) and
  strict `mypy` (128 files) passed.
- `scripts/publish_iop_population.py --development`: the licence classifier marked
  23 development studies public and 24 private (sealed: 11 public, 14 private). A
  scan found no observed values, predictions, intervals, preoperative pressures or
  signed errors in redacted studies.
- A structure-only load of the 25 sealed records printed counts only: 68 series (44
  group means, 24 single eyes; 53 control, 15 treated), the custodians' contexts, and
  14 series with fewer than two follow-ups.
- `make check-docker` **exit 0: 846 Python tests passed in 99.67 s**, 88% coverage,
  strict mypy 128 files, 18 frontend tests, contracts and all builds
  (`.peykar/iop-prefreeze-check-docker-2026-10-28.log`).
- `experiments/eye/iop/protocol-1.0.0.json` written exclusively, SHA
  `f13238a2678e2057a76f51deabdafc7f436ff4cc500dcc47b9777d2ecf8f0417` (32 frozen
  files, 47 development and 25 sealed record hashes); `check_protocol` reported no
  problem. Commit `ff470345` pushed; local `HEAD` equals `origin/main`.
- `scripts/evaluate_iop_population.py --sealed --protocol
  experiments/eye/iop/protocol-1.0.0.json`: run once, exit 0, 88.8 s, all pins
  matched, report SHA `9c62c0f4faf92dc029abc97fccea400f04245c4f0190b1a86f969482c0efaabb`.
  25 studies, 54 forecast series (43 group means) and 198 group-mean
  next-observation targets.
  - H1: 151/198 = 0.763 (80%) and 177/198 = 0.894 (90%), both inside the frozen
    windows.
  - H2: robust 7.74 pp versus persistence 8.64, last-two trend 39.98, pooled-curve
    ratio 8.98 and analog kNN 9.77.
  - H3: landmark 9.92 versus 12.89, 14.24 and 14.82 pp; landmark coverage 0.652/0.823
    (reported).
  - H4: 10.96 versus 17.99 pp for the pooled curve; 10/40 control group means within
    5/10 pp.
  - Group means: 21/43 within 5/10 pp, 11/43 also ≥ 0.5 pp better than persistence.
- `scripts/publish_iop_population.py --sealed`: 14 studies reduced to metrics, the
  same scan found nothing, and verdicts H1–H4 were all true. The plot script ran
  with `.peykar/cornea-native47/bin/python`; the PNG was inspected and the legend
  position and per-series colours were changed before the final render.
- After the result documents: every pin of the IOP, thickness, endothelium and
  corneal population protocols still matched. `make check-docker` **exit 0, 846
  Python tests passed in 108.84 s**, 88% coverage, strict mypy 128 files, 18 frontend
  tests, 465 files formatted, contracts and all builds
  (`.peykar/iop-results-check-docker-2026-10-28.log`).

## Corneal thickness dataset, model 1.0.0, protocol freeze and sealed test: 2026-10-28

- Start state: main at `b34838cf` (= origin, thickness split committed), with
  uncommitted development extraction and the builder's thousands-separator fix.
- An in-memory rebuild of all 35 development specs from their hash-checked sources
  matched the stored records byte for byte (35/35). `ruff format`/`ruff check`/strict
  `mypy` on the changed files passed; `pytest tests/unit/test_cornea_thickness_data.py`
  **4 passed** (0.09 s). Commit `dc59a42c` pushed; local `HEAD` equals `origin/main`.
- `scripts/link_study_authors.py --dataset experiments/data/cornea-thickness-2026-10-28
  --raw .peykar/thickness-literature-screen-2026-10-28/raw --prefix cct- --schema
  cornea.thickness-author-overlap-1.0.0 --output .../author-overlap-1.0.0.json`:
  4 linked pairs, 1 development cluster, 2 sealed training exclusions, 1 sealed–sealed
  pair (file SHA `0882e5a8…`).
- `experiments/vcornea/thickness/round0_law_comparison.py` (development only):
  hyperbolic return host group-mean log-SD 0.0094 versus exponential 0.0120, power
  0.5 0.0123, power 2 0.0105 (rerun from the published script gave the same values).
- `scripts/evaluate_thickness_population.py --development`, each exit 0: round 1
  59.0 s (report `5145a91e…`; robust next observation 4.70 pp); round 2 61.8 s
  (report `b7a5c08f3b7527dab71bd0ca16176774ceb1766b977ae42901f5463943758ea9`; robust
  4.67 pp next observation, 4.75 landmark, initial state 4.28); a repeat of round 2
  (63.5 s) produced the identical report SHA.
- `pytest tests/unit/test_thickness_population.py` **14 passed** (2.25 s);
  `ruff check .`, `ruff format --check .` (458 files) and strict `mypy` (125 files) passed.
- `make check-docker` **exit 0: 825 Python tests passed in 94.87 s**, 88% coverage,
  strict mypy 125 files, 18 frontend tests, contracts and all builds
  (`.peykar/thickness-prefreeze-check-docker-2026-10-28.log`).
- `scripts/publish_thickness_population.py --development`: 16 studies with NC/ND/SA
  licences reduced to metrics; a scan found no observed values, predictions,
  reference thicknesses or signed errors in redacted studies.
- `experiments/vcornea/thickness/protocol-1.0.0.json` written exclusively, SHA
  `a97db853b9d6fdc75f25ddd8f4f2357a0f13e69373967c4206164a49b82c9601` (27 frozen
  files, 33 development and 15 sealed record hashes); `check_protocol` reported no
  problem. Commit `deef7664` pushed; local `HEAD` equals `origin/main`.
- `scripts/evaluate_thickness_population.py --sealed --protocol
  experiments/vcornea/thickness/protocol-1.0.0.json`: run once, exit 0, 28.2 s, all
  pins matched, report SHA `62bf7c9654843e0ed6700edae8ccfd0ac3f561d2b96e3d5cc49542fa951350df`.
  15 studies, 29 forecast series, 65 next-observation targets. H1: 58/65 = 0.892
  (80%) and 63/65 = 0.969 (90%), both inside the frozen windows. H2: robust 1.60 pp
  versus persistence 2.06 and last-two trend 22.70. H3: landmark 2.31 versus 4.01 pp,
  coverage 0.815/0.892. H4: 18/21 control group means within 5/10 pp, 5.17 pp versus
  pooled curve 6.82. 28/29 series within 5/10 pp, 14/29 also ≥0.5 pp better than
  persistence.
- `scripts/publish_thickness_population.py --sealed`: six studies without CC BY/CC0
  terms reduced to metrics. The plot script ran with `.peykar/cornea-native47/bin/python`;
  the PNG was inspected and the legends were made per group before the final render.
- After the result documents and plot script: `ruff format --check .` (462 files)
  and `ruff check .` passed; `make check-docker` **exit 0, 825 Python tests passed
  in 127.39 s**, 88% coverage, strict mypy 125 files, 18 frontend tests, contracts
  and all builds (`.peykar/thickness-results-check-docker-2026-10-28.log`).

## Corneal endothelial density model 1.0.0 and protocol freeze: 2026-10-28

- Start state: main at `b0c2b043` (= origin, outcome-blind split committed). The
  background `make check-docker` started after that commit ended **exit 0: 797
  Python tests passed in 79.00 s**, 87% coverage, strict mypy 120 files, 18 frontend
  tests, contracts and all builds (`.peykar/endothelium-split-check-docker-20261028.log`).
- Four development extractor sub-agents built 59 records (148 series; 7 reserved
  studies not extractable: medians, outcome-selected groups, missing references,
  time ranges); three custodian sub-agents built 17 sealed records and reported
  structure and hashes only (4 sealed studies excluded with reasons).
- Development-only diagnostics (private scripts under `.peykar/`): in-sample best
  grid fits of series with ≥4 follow-ups compared kinetic laws; smoke runs on 3–5
  development studies led to the declared changes listed in the protocol.
- `scripts/evaluate_endothelial_population.py --development`, each exit 0:
  round 1 (exponential law) 590.6 s; round 2 (power law) 508.5 s; round 3 (power law,
  author clusters) 500.2 s. Round 3: next observation `population_anchored_t4`
  pooled 4.02 pp versus persistence 5.14 pp, 99/115 series within 5/10 pp, 51 meeting
  all criteria; landmark 5.46 versus 10.07 pp; control group means 9.34 pp (pooled
  curve 9.45). Coverage of the primary forecaster 0.820/0.897 (next observation).
- `scripts/link_endothelium_authors.py` (first run failed strict mypy, fixed and
  regenerated): 8 linked pairs, 4 development clusters, 3 sealed training exclusions.
- `scripts/publish_endothelial_population.py --development`: public metrics in
  `experiments/vcornea/endothelium/development-loso-2026-10-28/`; 24 studies with
  NC/ND/SA licences reduced to metrics. The licence classifier agrees with the
  public/private storage split for all 59 development records.
- `pytest tests/unit/test_endothelial_population.py` **10 passed** (7.76 s).
- `make check-docker` **exit 0: 807 Python tests passed in 97.70 s**, 87% coverage,
  strict mypy 122 files, 18 frontend tests, contracts and all builds
  (`.peykar/endothelium-freeze-check-docker-20261028.log`).
- `experiments/vcornea/endothelium/protocol-1.0.0.json` written exclusively, SHA
  `fdec8d89a0d62f4edbae7f0106c941cd98f47a71a47a5a2675d0e229880715dc` (26 frozen
  files, 59 development and 17 sealed record hashes); the operator's
  `check_protocol` reports no problem before and after the full check.
- Commit `00d2cd09ff0762439b9277a1d84e705653d20acb` pushed; local `HEAD` and
  `git ls-remote origin refs/heads/main` equal (the remote reported a moved
  repository URL and accepted the push).
- `scripts/evaluate_endothelial_population.py --sealed --protocol
  experiments/vcornea/endothelium/protocol-1.0.0.json`: run once, exit 0, 141.1 s,
  all pins matched, report SHA `a9de39ad0690abf0b422f2eeec9a3bd72a7e3622ed204ddcdbc5083844c460da`.
  17 studies, 30 group-mean series, 76 next-observation targets; three sealed
  studies trained without their author-linked development study. H1: coverage
  54/76 = 0.711 (80%) and 63/76 = 0.829 (90%), both inside the frozen windows. H2:
  `population_anchored_t4` 5.60 pp versus persistence 7.36 and last-two trend 15.50.
  H3: landmark 6.85 versus 12.41 pp, coverage 0.724/0.908. H4: control group means
  7.05 pp (pooled curve 8.16), 13/29 within 5/10 pp. 23/30 series within 5/10 pp,
  14/30 also ≥0.5 pp better than persistence.
- `scripts/publish_endothelial_population.py --sealed`: declared analysis, public
  report and tables in `experiments/vcornea/endothelium/sealed-test-2026-10-28/`;
  five studies without CC BY/CC0 terms reduced to metrics. The plot script ran with
  `.peykar/cornea-native47/bin/python`; the PNG was inspected and its legend moved
  off the bars before the final render.
- After the result documents and plot script: `make check-docker` **exit 0, 807
  Python tests passed in 92.04 s**, 87% coverage, strict mypy 122 files, Ruff
  format/lint, 18 frontend tests, contracts and all builds
  (`.peykar/endothelium-results-check-docker-20261028.log`).

## Corneal dataset 1.1.0, closure-tail model 2.0.0 and protocol freeze: 2026-10-28

- Start state: main at `39f4c134` (= origin) with uncommitted v2 model/harness and
  screen-script options; no scientific process running.
- `scripts/screen_corneal_wound_literature.py --queries round2/queries.json
  --exclude-screen <round 1>`: 2,015 new full texts; supplement acquisition of 167
  candidates (78 with data members, 11 archives unavailable). Round 3
  (`round3/queries.json`, two excluded screens): 772 new full texts, 5 supplement
  candidates. No supplement study was admissible.
- `scripts/screen_corneal_tables.py`: 3,996 full texts → 143 articles/162 masked
  tables; round 3: 768 → 31/35. Masked display `scripts/show_masked_previews.py`
  additionally hides decimals, ±, % and equations in labels. One key-points box
  (non-wound outcome count) displayed an n/N fraction before n/N masking was added;
  recorded in triage.
- Triage 1.1.0 (310 entries) admitted 10 table studies; `reserve_corneal_population_split.py`
  wrote `split-reservation-1.1.0.json` (9 development, 1 sealed). Triage 1.2.0 (36
  entries) admitted none.
- Development records: 9 specs built and validated by `scripts/build_corneal_table_record.py`;
  the first build loop stopped on `Hour-0 value must normalize to one` (PJE tables),
  which led to extraction protocol 1.1.1 (written after those development values
  were read) and a clean rebuild of all nine. Eight public source XMLs copied and
  hash-checked against their specs.
- Custodian sub-agents: i-PRF sealed record (2 group means) and re-examination of
  the two excluded 1.0.0 sealed studies (IL-10 admitted with 55 per-mouse series
  under the definitional baseline; norepinephrine still excluded). Reports contain
  structure and hashes only.
- `pytest tests/unit/test_corneal_population2.py` **8 passed** (32.4 s);
  `pytest tests/unit/test_cornea_table_records.py` first failed on a malformed
  synthetic XML in the test itself, then **3 passed**.
- `scripts/evaluate_corneal_population2.py --development`: exit 0, 542.9 s, 16
  studies/147 series/462 targets, both versions. Model 2.0.0 cohort pooled RMSE
  9.23 pp (1.0.0: 10.27), cohort coverage 82.9%/90.8% (1.0.0: 79.9%/86.7%).
  Public metrics in `development-loso-2026-10-28/` (no-derivative sources reduced to
  metrics and verified free of observed values).
- First `make check-docker` stopped at `ruff format --check` (two files reformatted
  by the editor); log `.peykar/cornea-population2-check-docker-20261028.log`. After
  `ruff format`, `make check-docker` **exit 0: 794 Python tests passed in 72.79 s**,
  87% coverage, strict mypy 119 files, 18 frontend tests, contracts and all builds.
  Log `.peykar/cornea-population2-check-docker-20261028-attempt2.log`.
- `protocol-2.0.0.json` written exclusively, SHA
  `68ef4771bf31eef0afd3983f61c04c257e47eac1c878138dfa707607b6cd83ae`; the 2.0.0
  operator's `check_protocol` reports no problem, and the 1.0.0 operator's check
  still reports no changed file.
- Commit `2cee1b019b8b512cb72d110484bcff522c19ee5c` pushed; local `HEAD` and
  `git ls-remote origin refs/heads/main` equal.
- `scripts/evaluate_corneal_population2.py --sealed --protocol protocol-2.0.0.json`:
  exit 0, 126.9 s, all hashes matched, report SHA `6525c342…`. Results (both
  versions trained on the same 16 studies): 2.0.0 cohort coverage 116/165 (70.3%)
  and 136/165 (82.4%), both inside the frozen windows; 1.0.0 115/165 (69.7%,
  outside) and 139/165 (84.2%). Cohort RMSE 13.10 vs 16.27 pp last-two trend
  (1.0.0 cohort 13.01). Cohort series passing 5/10 pp: 3/55. Initial-state control
  groups: IL-10 18.0/24.6 pp, i-PRF 25.0/41.7 pp (both fail).
- `scripts/publish_corneal_population2.py --sealed` wrote the primary analysis and
  the public report; the figure script ran with the existing native Python and the
  PNG was inspected. Consumed CC BY sealed records and sources copied to
  `consumed-sealed/` and every record source hash re-checked (all OK).
- After the result documents and plot script: host `ruff format --check .` and
  `ruff check .` passed; `make check-docker` **exit 0, 794 tests in 73.93 s**, 87%,
  mypy 119 files. Log `.peykar/cornea-population2-results-check-docker-20261028.log`.

## Cross-study corneal population model and sealed test: 2026-10-27

- Start state: main clean at `1a285045c32f80eb2c5e6400afd1cec31129cc7a` (= origin);
  API/Worker up 9 days; no corneal/native scientific process running.
- `scripts/screen_corneal_wound_literature.py`: first sequential run stopped by the
  operator after 625 full texts for speed (recorded in its `STOPPED.json`); the
  concurrent rerun with `--workers 6 --reuse-fulltext` screened **1,993/1,993**
  PMCIDs (9 unavailable), 171 with data-like supplements.
- `scripts/acquire_corneal_wound_supplements.py`: 87 rule-selected candidates
  (51 with data members) plus 10 listed extras (5 with data members); repository
  metadata checked for Figshare/Zenodo/OSF/Mendeley links. Masked previews only.
  The masking initially showed numbers stored as text in one non-outcome b-actin
  sheet; fixed to hide them (`#t`) with a unit test.
- Legacy `.xls`: isolated `uv venv` plus `uv pip install --require-hashes` of
  xlrd 2.0.1; `convert_xls.py` converted 12 workbooks, 2 files were tab-separated
  text and were recorded as errors.
- Triage admitted 6 studies; `reserve_corneal_population_split.py` wrote
  `split-reservation-1.0.0.json` (SHA `1223827b…`) before any new value was read:
  4 sealed, 2 development. A custodian sub-agent admitted 2 sealed studies (16 and
  48 series), excluded 2, re-read all 280 cited cells and reported only structure
  and hashes. Development records: DIO 16 series, rat 15 series, validated.
- Targeted tests: `pytest tests/unit/test_corneal_population.py
  tests/unit/test_spreadsheet_reader.py tests/unit/test_cornea_population_data.py`
  **17 passed**; whole-tree Ruff, format and strict mypy (116 files) passed on host.
- `scripts/evaluate_corneal_population.py --development`: exit 0, 15.1 s,
  5 studies/47 series/168 targets; mostly failing results retained in
  `development-loso-2026-10-27/`.
- Protocol `protocol-1.0.0.json` SHA
  `de3c010ecce9cba4fe9684e9936f2a371e5c26b65d654379b9eccbd696766cce` committed and
  pushed as `de9762a0`; local and `git ls-remote` SHA equal. GitHub reported the
  repository moved to `sina2266/peykar-simulator`; the push succeeded.
- `scripts/evaluate_corneal_population.py --sealed --protocol … --draws 2000`:
  exit 0, protocol/code/data/sealed hashes all matched. Results: initial-state
  control means 2/2 pass (2.26/2.83 and 3.60/6.34 pp); next observation 52/64
  series for cohort assimilation vs 42/64 last-two trend; pooled RMSE 6.12 vs
  6.73 pp (nearest trajectories 5.86); 80% coverage 77.6% (pass), 90% coverage
  79.6% (fails the frozen 80% floor). Public artifacts written by
  `publish_corneal_population_sealed.py`; the CC BY-NC-ND study keeps metrics only.
- Figure rendered with the existing native Python (Matplotlib, JSON input only)
  and visually inspected; one clipped panel title was fixed and re-rendered.
- First `make check-docker` stopped at `ruff format --check` (unformatted plot
  script); log kept as `.peykar/cornea-population-check-docker-attempt1-format-fail-20261027.log`.
  After formatting, `make check-docker` **exit 0**: **783 Python tests passed in
  63.58 s**, 87% coverage, strict mypy 116 files, 18 frontend tests, contracts
  current and all package builds. Log `.peykar/cornea-population-final-check-docker-20261027.log`.
- Host `make build` exit 0; six new/changed platform modules and three domain
  READMEs match the built wheel byte for byte. `check_protocol` on the frozen
  protocol reports no changed file. `/healthz` returned ok/0.3.0 and the API still
  mounts `peykar3_peykar-data`; no service or volume was changed.

## Causal observation updates and continuation handoff: 2026-10-27

- Re-read AGENTS/.kiro/README/status/roadmap and domain documentation; main was
  clean at0001dc292dd7390f361b7bb5546fe2bfe9b4f699. Docker API/Worker were up;
  no old corneal scientific process was active. Continued corneal work and wrote
  the user's requested reusable Persian prompt/current handoff, without resuming
  the paused heartbeat or changing live service/data configuration.
- `uv run --frozen pytest tests/unit/test_corneal_online.py
  tests/unit/test_cornea_online_evidence.py tests/unit/test_corneal_online_operator.py
  tests/integration/test_corneal_online_forecasts.py -q`: **47 passed in1.58s**.
  Tests check causal prefixes, irregular time, expansion/reopening, source/cell/
  statistic preservation, derived-baseline exclusion and checked replay of real
  executions, rejecting failed children and changed/rehashed evidence. No new
  scientific job is launched by the replay tests.
- Targeted Ruff and strict mypy on both new source modules passed. Whole-tree
  host `uv run --frozen ruff check .` and `ruff format --check .` passed before
  final regression (382 formatted files).
- Frozen protocol1.0.0 SHA
  f801761c9ff9e05479b3562d95ffa2cb736f7f5870cb81741deb0da521ff8398:
  first explicit operator child exited1 at import, **before any forecast**.
  Original executed sources/logs/protocol remain under setup-failed-2026-10-27.
  A sandbox import-only probe confirmed the directory-listing correction. No
  method, source data, eligibility, comparator or threshold was changed.
- Frozen revision1.0.1 SHA
  92d7bb2f717b1122cdd23bd33a4bdc8db3e37fab5d037c793113ee1c57c2a9d5;
  actual command `uv run --frozen python scripts/replay_corneal_online_forecasts.py
  --output .peykar/cornea-online-forecast-retry1-20261027`: **98 actual child
  exits0**,16.2690675422s parent elapsed,10s per-child/300s overall limits.
  Every child actually checked five source/future read denials, network denial
  and denied writes outside its job. Per-job source outcome appeared only after
  exit0. Reviewed trusted-code isolation, not a universal hostile-code sandbox.
- Actual scientific comparison **exit2/rejected**: all16 separate series,
  96numeric targets,2 categorical50h forecasts without numerical substitution,
  32 explicit warmup rows. All16 improve on persistence; only5 meet the frozen
  per-series5/10pp accuracy and0.5pp improvement gates. RMSE1.8573–18.5612pp;
  largest error25.1600pp. No pooled acceptance; the candidate is not uniformly
  better than other simple baselines. These previously consumed studies supply
  retrospective prequential evaluation, not fresh independent biological validity
  or an initial-state simulator pass.
- Retained actual children ZIP:784 members/704283bytes. Public report, exact
  executed sources, source provenance/units, failure, plots and31-file frozen
  scientific inventory are under experiments/vcornea/repair-front/online-forecast.
  Ran retained plotting source with existing native46 Python (JSON/Matplotlib
  only; no package installation), generated PNG/PDF and visually inspected the
  corrected figure. No simulation, fitting or segmentation occurs in plotting.
- Verified all130 entries across the three previous retained-file inventories:
  zero hash mismatches. Original raw sources, scientific versions and licenses
  were not edited. `curl --fail --silent http://127.0.0.1:8000/healthz` returned
  statusok/version0.3.0; Docker inspect confirmed original peykar3_peykar-data
  mounted at/var/lib/peykar. No live redeployment or volume reset was performed.
- Final command
  `DOCKER_CONFIG="$PWD/.peykar/docker-public-check-20261023" DOCKER_HOST="unix:///Users/sina2266/.docker/run/docker.sock" make check-docker`
  **exit0**: **766 Python tests passed in47.69s**,87% coverage, strict mypy110
  source files,18 frontend tests, Ruff/format/contracts and all three wheel/sdist
  builds passed. Log `.peykar/cornea-online-final-check-docker-20261027.log`.
- Host `make build` **exit0**, all three wheels and source distributions built;
  log `.peykar/cornea-online-final-build-20261027.log`. Both new platform modules
  and both domain READMEs match the built platform wheel byte for byte. Checked
  all31 new inventory entries, zero mismatches. `git diff --check` passed.

## Post-injury corneal forecast and structural precision refusal: 2026-10-27

- Re-read AGENTS/.kiro/README/status/roadmap/domain docs and actual retained
  corneal failures. Main4904d9bd was clean. Latest user permits another eye
  component only when current scope is useful/accepted; independent failures
  still justify corneal work. Re-audited all60training/30heldout rows against
  original2017 source,2021 ratio/time/categorical meaning and Fischak raster
  constraints. No material mapping bug found; no old candidate was rerun.
- Targeted HF/Dryad/DataCite/Figshare primary-source search retained actual
  responses/failures. Acquired Suárez-Barrio2019 DOI10.3390/ijms20071655 original
 152084byteXML SHA4c2d3b588f040797ecb5967504349e563eeb76fae9ec8fa890edbc05b76504db
  and3058228bytePDF SHAb32b1c4ddb4559bf94ab83d6da3afb5438252af5afb854d8a7b7dab78a3c77d8;
  both MD5match primaryNCBIversion metadata, CC BY4 attribution retained. New
  inert reader preserves all28Table1 means/SD/sourcecells/mm² and separates
  only Control0/24h from future36/48/60/72/84h. No unknown subject count/ID,
  ratio uncertainty or missing SD invented.33eyes/17rabbits, paired-eye/care/time
  conflicts retained. Acquisition agent inspected full source before masking;
  not a whole-team blinded validation or initial-state-only prediction.
- New optional engine `cornea.conditioned_front`1.0.0/model1.0.0 estimates
  effective radius speed from actual0h72.32/24h47.02mm² only. No lag, window,
  architecture search or future assimilation. Frozen protocol
 2c8d7f806b01a440c5af4831d7b67d7386301436b5e89e4c73cc37c2124c72cd predates
  candidate and parent-side future evaluation. `uv run --frozen python
  scripts/check_corneal_conditioned_forecast.py --output
  .peykar/cornea-conditioned-forecast-20261027`: actual child0/0.00539325s,
  parent0.50408058s under60s. Seatbelt network/write denial and actual future
  JSON/fullXML/PDF read-denial probes retained.241samples/.25h cover forecast
  elapsed0..60h, explicitly mapped to injury24..84h by checked extra time output.
- Every future Control point retained,0/24h excluded from score. Actual
  future-onlyRMSE0.14988200098795731/max0.1965603072519207 fails fixed.05/.10;
  persistenceRMSE0.5316027811424765 remains. Initial1986 analytic-law comparator
  at the same future times:RMSE0.04888050726290779/max0.10834031185092868, also
  misses max criterion; different information, no claim of84h execution of the
  older bounded engine. Comparisonexit2/actualchild0; all results/model/plan/
  source/receipt bytes retained, unvalidated/default-catalog absent.
- Separate prefix-only inverse diagnostic, expressly retrospective after the
  failure: fixed engineeringfamilylag0..24h/speed0..200µm/h admits coupled
 38.7176..200µm/h/0..19.3539h. `uv run --frozen python
  scripts/audit_corneal_forecast_identifiability.py --output
  .peykar/cornea-forecast-identifiability-20261027` exit0; optimistic minimax
  pointRMSE0.14510388870513824/max0.20483746614935372 implies abstention from
  claiming5/10pp precision WITHIN the assumed family. No future observations,
  statistical coverage or biological guarantee; file-access regression test
  permits only prefix/protocol/source reads. Bounds and midpoint are not a
  newly validated trajectory or a change to the rejected candidate.
- Acquired actual Aung2025 original SI1307165bytes/SHAde5153060b6e19e48ba4c7f0423604e753cccbfcc4fc6617f8da7f8e7ee35278,
  primaryFigshare MD5matched, separateCC BY-NC4 terms. Four unlabeled PBS rows
  versus six reported eyes/~242x182pixels/unknowncalibration; no segmentation,
  complete-cohort score or invented subjects.2021 saline aggregate lead retains
  SE/paired-eye/depth limits, unused in current fit/comparison. Primary2019OCT
  fulltext confirms reanalysis of the Fischak2017 experiment; sourcehash52ac034f…
  and explicit no-additional-independent-animal link retained. No author contacts.
- Targeted87tests passed0.19s; Ruff full check/format and strict new-source mypy
  passed. Final `DOCKER_CONFIG="$PWD/.peykar/docker-public-check-20261023"
  DOCKER_HOST="unix:///Users/sina2266/.docker/run/docker.sock" make check-docker`
  passed **719tests/41.03s/87%coverage**, strict mypy108sources,18frontend tests,
  current exported contracts and all3wheel/sdist builds. Host `make build` passed;
  three new module bytes match built wheels. PNG/PDF comparison regenerated from
  retained results only, visually inspected. No extra experimental fitting/run.
- Logs `.peykar/cornea-conditioned-final-check-docker-20261027.log` and
  `.peykar/cornea-conditioned-final-build-20261027.log`; after package README
  updates, final host rebuild also passed in
  `.peykar/cornea-conditioned-final-build-docs-20261027.log`. Existing APIhealth200/
  version0.3.0, API/Worker up9days. No live deployment, volume/schema/source/license
  reset or automation change; heartbeat remains paused. Original publication
  and general biological validity remain open.

## Isolated corneal learning and new independent constraints: 2026-10-24

- Read-only `scripts/audit_corneal_shared_front.py --output
  .peykar/cornea-shared-front-audit-20261024` completed with exit0: exact shared
  18/24h observations reject a common deterministic displacement within10pp;
  optimistic maximum-error lower bound0.21285781453343008. No new forward
  simulation, kinetic fit or causal inference; not a bound on hierarchical models.
  Frozen protocol5a66ae57… and actual solver/operator/report retained.
- Acquired/inspected Fischak2017 original CC BY article/figure (PMC5485343.1,
  DOI10.1155/2017/5192924), PDF771631bytes/SHA17cf0784213d04d5cc3901333b32f1a5a79eb73f4c6be791678a438ef71991bd.
  Its MD5 matches primary NCBI OA metadata. Publisher-deposited JATS and original
  figure retained; failed HTML-as-PDF attempt remains private, unused. Read and
  visually inspected the original PDF page3 and native raster. OCT curvature
  correction and projected fluorescein endpoints remain distinct.
- Before extraction, froze raster/axis calibration and one-sided constraint
  protocol f4b2b582…; `uv run --frozen python
  scripts/extract_corneal_curve_constraints.py --output
  .peykar/cornea-fischak2017-constraints-20261024.json` passed. All five actual
  nonbaseline times12/24/36/48/72h retained. Unknown per-arm counts/IDs and
  overlapping curves yield conservative mean lower constraints only: sum of
  separated-cluster lower areas divided by the entire study20animals, fixed
  ±0.5mm² digitization allowance. No fabricated raw records, means, CIs or60hpoint.
- Implemented bounded empirical learner and optional `cornea.learned_front`1.0.0,
  preserving existing models/sources. Frozen learning protocol df8a95a4… predates
  the first actual experimental fit; exact sources, algorithm,60training/30heldout
  rows, criteria and independent challenges pinned. Six Control/2%SDP animals,
  54nonbaseline fitted values; all three0.5%SDP animals excluded. Previously
  inspected data: retrospective dose interpolation, not blinded independent study.
- `uv run --frozen python scripts/learn_corneal_front.py --output
  .peykar/cornea-learned-front-20261024`: actual child exit0/0.2255907s, parent
  0.5710581s;41640objective evaluations within45000cap/60s. Seatbelt denies
  network/writes outside output and heldout/raw/independent evidence reads;
  actual read-denial probes retained. Full54/50/72h/.25h predictions written
  before parent-side evaluation; exact model/plan/source/output checks precede metrics.
- Fixed primary5ppRMSE/10ppmax targets plus≥0.5ppRMSE improvement unchanged.
  Heldout meanRMSE0.05958317282881904/max0.12259994406922181: FAIL despite
  baselineRMSE0.10260416884976953 improving by0.04302099602095049. All3individual
  diagnostics retained.2021Blade/Ocu/Amo all fail:RMSE0.2945544/0.2828078/0.2837858;
 10numeric/2categorical observations retained, no categorical zeros. Independent
  Fischak2017 one-sided optimisticRMSE lower bound0.05567380580214574 rejects
  candidate; this is not its measured-studyRMSE. Overall comparison exit2, never
  biological promotion. Immutable descriptors/calibration/actual receipts/figures
  and executed source snapshots retained in the learning experiment.
- Targeted52tests passed0.55s; Ruff full check/format and strict mypy105source
  files passed. Synthetic fits test engineering only; evidence tests replay
  actual retained results without new experimental fitting/simulation. Failed
  child and changed-output evidence are rejected. PNG/PDF plot generated with
  the existing native profile and visually inspected; no profile mutation.
- Final `DOCKER_CONFIG="$PWD/.peykar/docker-public-check-20261023"
  DOCKER_HOST="unix:///Users/sina2266/.docker/run/docker.sock" make check-docker`
  passed: **632tests/35.41s/87%coverage**, strict mypy105sources,18frontend tests,
  Ruff/current exported contracts and all3wheel/sdist builds. Host `make build`
  passed separately; four new source modules verified byte-identical in built
  wheels. Logs: `.peykar/cornea-learning-final-check-docker-20261024.log` and
  `.peykar/cornea-learning-final-build-20261024.log`.
- Live API/Worker remain up; no live-volume/reset/deployment or default-engine
  registration. Existing source/license/model/data bytes and failures remain.
  Heartbeat remains paused. Full original publication and biological gates open.

## Real projected corneal-repair predictions and transfer failure: 2026-10-24

- Inspected primary Crosson1986 abstract/metadata (PMID3957565): reported5.5h lag
  and64µm/h radius motion after mechanical debridement with basal lamina intact.
  Exact4575byte metadata SHA retained; full publisher article403, no invented
  supportive-care detail or SD/SEM interpretation. Implemented separately pinned
  `cornea.projected_front`1.0.0 and two distinct immutable1.0.0 model contexts.
  The engine is excluded from the default API/Worker catalog.
- Froze2017 protocol1.0.0 SHA865f9672… before the new candidate. Explicit
  `uv run --frozen python scripts/check_corneal_repair_front.py --output
  .peykar/cornea-repair-front-cross-study-retry1-20261024` completed its actual
  child exit0 in0.335289s, under60s/network-denied Seatbelt. Full54h/.25h plan
  gives217samples, using7mm nominal initial diameter and unchanged1986 kinetics.
  Compared all3actual Control rabbits and all10times; primary excludes baseline:
  cohort-mean RMSE0.04021732891267079/max0.0586268618400031, passing fixed.05/.10
  engineering targets. Rabbit2 RMSE0.06646528662119855 remains explicit; no
  individual guarantee, fitted test data, parameter search or global approval.
- Acquired exact Chung2021 publisher-deposited XML from EuropePMC PMC8828349:
 65161bytes/SHA4e4cf5f510d88ebc0c5692431c3f668e11e40b8434a62c13b4d3955c60822998.
  Parsed Table1's10numeric aggregates and2categorical healing statements,
  preserving above-baseline1.02, original reported±, unknown per-time animal count
  and original attribution/license. No synthetic animals, CIs or measured zeros.
- Froze the independent6mm removal-method applicability challenge before its
  candidate. Unexecuted protocol1.0.0 remains;1.0.1 pins pre-execution parser
  formatting/type corrections only, unchanged data/parameters/criteria.
  SHA9c70007c0c3e6faba91c5f6914f0adcfb527762b4935159191919a5cf8e2b6b6.
  `uv run --frozen python scripts/check_corneal_repair_challenge.py --output
  .peykar/cornea-repair-front-independent-challenge-20261024`: actual child
  exit0/0.324017s, full50h/201samples, network denied; scientific comparison exit2,
  correctly negative. Blade/Ocu/Amo RMSE0.2644613/0.2480860/0.2493709, all fail.
  No failed group removed, criteria relaxed or parameter fitted. Normal saline,
  irregular/oval geometry and substrate/care differences remain context limits,
  without claiming an identified cause of failure.
- `uv run --frozen pytest -q tests/unit/test_corneal_front.py
  tests/unit/test_cornea_brush_evidence.py tests/unit/test_cornea_evidence.py
  tests/integration/test_corneal_repair_prediction.py`:39passed/0.17s. Targeted
  Ruff/format pass; strict mypy on both changed domain modules passes. Tests
  preserve real negative results and reject changed output/protocol/failed exits.
- One final `make check-docker` passed **580tests/34.05s/86% coverage**,
  strict mypy101files, Ruff/format321files,18frontend tests, generated contracts,
  and all wheel/sdist builds. Separate host `make build` also passed. Actual logs
  `.peykar/cornea-repair-front-final-check-docker-20261024.log` and
  `.peykar/cornea-repair-front-build-20261024.log` retained. Checks used the
  existing task-local anonymous Docker config/socket; live services/volume unchanged.
- Retained exact actual candidates/protocols/plans/process receipts/executed
  operators, the failed pre-child byte-writer setup, parser source and plotting
  script. Standard matplotlib plot/PDF generated with existing native Python and
  visually inspected. No native runtime packages changed. Read-only tests verify
  the retained results; no duplicate long/native jobs were started. API health200,
  original API healthy/Worker up6days. Original source/model/schema/license/lock
  files, failed publication evidence and paused scheduling preserved.
- [Detailed scope, original data, actual results and next model gap](../experiments/vcornea/repair-front/README.md).
  A limited projected-area agreement and failed transfer are distinct from the
  unresolved original cellular publication gate and independent clinical validation.

## Real corneal experimental data and observation operator: 2026-10-24

- Searched Hugging Face API for cornea/corneal/wound-healing, inspected all three
  returned corneal dataset identities/cards/file inventories and pinned their
  exact commits. These name searches are not an exhaustive full-text inventory.
  Downloaded complete303826byte human cell metadata Parquet; native PyArrow read
 22972rows/12sample prefixes/three columns, contradicting card~19472/viewer141k.
  File SHA matches its pinned LFS SHA. Downloaded/read the202eye biomechanics CSV;
  its Git object hash matches the pinned HF tree. Neither contains a matched
  corneal epithelial healing/thickness endpoint. No HF Python/model code executed.
- Downloaded actual original PLOS0188154 S1ZIP97548bytes and exact Fig2/3 XLSX
  members, plus PLOS0239124 publisher JATS. Original terms/credits/raw bytes
  retained in experiments/data/cornea-evidence-20261024. Normalized90measured
  rabbit wound rows (three groups ×three rabbits ×ten times0–54h), preserving
  source-cell IDs, unreported area unit and within-rabbit ratios. No fourth
  rabbit fabricated despite original caption n4. Clinical Normal group has22
  subjects/seven aggregate endpoints; central52.5±3.6µm, no individual rows invented.
- Dryad version467979 all47file metadata entries retained. Six small-file
  downloads failed API401/public UI403; no experimental measurements/image ZIP
  acquired from that source. Preserve this negative acquisition observation.
- New stdlib observation/parser module keeps source formulas inert, bounds input
  XML/XLSX, measures epithelial span/occupancy/gaps directly from lattice types,
  leaves empty spans null, rejects unknown context/units and never emits a
  biological approval. Original height yCOM is not relabeled physical thickness.
- `uv run pytest tests/unit/test_cornea_evidence.py -q`:18passed/0.05s; targeted
  Ruff/format checks and strict mypy1file pass. Tests include actual original
  source interpretation, n3/no synthetic eyes, entity/formula rejection, y-offset
  invariance, gaps/empty columns, invalid scales/grids and unknown endpoint context.
- Explicit `scripts/compare_corneal_evidence.py` command in
  [operator instructions](../experiments/vcornea/experimental/README.md) ran with
  native Python, final output cornea-experimental-diagnostic-retry1-20261024.
  Verified all four actual completed43200MCS formation manifests/proofs/outputs;
  final mean spans54.01010/55.62626/61.68687/61.40404µm, clinical52.5±3.6.
  Report blocks measurement/region equivalence, preserves descriptive differences,
  null pass/accuracy and biological-validation false. Plot visually inspected.
  Initial invocation exposed a Pydantic import absent from frozen native runtime;
  stdlib SHA-256 removes that management dependency, no runtime package changed.
- First full check stopped before pytest because Ruff attempted to reformat an
  original HF README code block. Original bytes preserved; only this new raw-data
  directory was added to existing upstream formatting exclusions. Final
  `make check-docker` passed559tests/34.35s/86% coverage, strict mypy100files,
 18frontend tests/current contracts/all wheel and sdist builds. Host `make build`
  also passed. Actual logs .peykar/cornea-experimental-evidence-final-check-docker-
  20261024.log and cornea-experimental-evidence-build-20261024.log retained;
  initial formatting-failure log remains separately.
- Rechecked23admitted evidence-file hashes, preparation-operator digest and both
  native report code hashes. API health200/ok/version0.3.0; original API/Worker
  remain up. No new native model execution, parameter fit, original protocol
  modification, data-volume mutation or recurring schedule activation occurred.
- Final local Ruff/format checks passed;159 local Markdown links and exact
  experimental acquisition/operator/report identities verified. `git diff --check`
  passed. Raw/normalized evidence bundle is745194bytes; original model/license/
  schema versions, comparison1.0.0 and uv.lock remain unchanged.

## Terminal serial corneal runs and retained outcomes: 2026-10-24 08:00UTC

- Inspected current plans, actual child records/proofs and both finite follow-on
  statuses before any new work. Source6 moderate7/11 completed50400MCS/exit0 in
  10715.82/11179.29s; source7 formation7/11 completed43200MCS/exit0 in
  8933.22/8964.58s. No native solver/guard is active; no new simulation launched.
- Both follow-ons finished comparison2/original math0/spatial0. Full January
  formation2 + reused injury6: not_reproduced, five scalar failures/three passes.
  All six reused comparison rows exactly equal their earlier diagnostic rows.
  Separate July moderate2: diagnostic_not_reproduced, Wing hourly coverage
  0.8992263439793692/0.8893076770482047 below unchanged0.90. No rounding/pass override.
- Ran `.peykar/cornea-native46/bin/python .peykar/retain_serial_cornea_results_20261024.py`:
  exit0, verified all ten native records/source/runtime/protocol/guard exits and
  all artifact hashes, plus both follow-on file inventories. Retained61 exact-byte
  analysis files, two full native receipts, terminal matrix and executed operator
  under experiments/vcornea/publication/observed-2026-10-24/serial-*.
- Ran `.peykar/cornea-native46/bin/python .peykar/audit_completed_cornea_chemical_20261023.py
  --runs .peykar/cornea-serial-derived-moderate46-20261024
  --output .peykar/cornea-serial-completed-chemical-audit-20261024.json`: exit0,
  each50401 finite/nonnegative samples on0.1h grid through5040h; real720h pulse
  2500 at[100,75]. Both previous interim pulse hashes match actual completion.
- Both new analyses verify all saved finite200x90 fields, cell counts and exact
  type-summed cell-volume/lattice occupancy. Old four-worker inconsistent frame
  remains rejected. Visually inspected final formation/recovery structures, EGF,
  reference-overlay counts and original authors' count-analysis figure.
- Ran `.peykar/cornea-native46/bin/python .peykar/audit_serial_basal_geometry_20261024.py`:
  exit0, read-only fixed original upper limb/membrane-surface audit of actual
  initial/pulse/final arrays. Wing sites below that reference start atzero in
  both, finish45/1333 and3150/5194 in198 eligible columns. This descriptive,
  non-preregistered observation has no new acceptance threshold and diagnoses no
  pathological invasion. Zero exposed columns is insufficient to claim normal repair.
- Current unchanged managed code already passed541 tests/37.66s,86% coverage,
  mypy99,18 frontend/current contracts/all wheel/sdist builds plus host build in
  the preceding milestone; checked actual retained logs again. No managed code
  changed in this results/documentation increment; the entire suite was not rerun.
- Docker API/Worker still up; GET /healthz200, status ok/version0.3.0. No deployment,
  live volume mutation, original data/source/license change or schedule activation.
  Full reproduction remains open on exact original run correspondence, independently
  from biological validation. [Plain outcome](../experiments/vcornea/publication/results-2026-10-24.fa.md).
- Read-only artifact/document check verified61 retained file hashes, recorded
  operator hashes, the terminal matrix (eight full-comparison jobs/three passes,
  two diagnostic failures), then all200 current local Markdown links. Exact-byte
  staging verified all86 intended files; `git diff --cached --check` passed.

## Complete January family rejection, serial formation and verified resume: 2026-10-24

- January formation2 genuinely completed43200MCS/exit0,4438.35/4461.59s.
  Combined with the six existing injury completions, all eight actual exit/source/
  runtime/output inventories verified. Full frozen comparison exit2/not_reproduced:
  five failures/three scalar passes. Original mathematics exit0. Spatial stage
  correctly rejects formation7 at43200MCS: tear native volume1974/lattice sites1973.
  Read-only scan of every saved formation frame/type found only that one mismatch.
  Both arrays/cell hashes, original negative report and failed stage are retained.
- Diagnostics1.2.1 adds exact case/seed/MCS/type/values to the same strict rejection;
  fresh explicit replay exits1 with those actual values. No output is repaired,
  gap filled or tolerance changed. Failure is distinct from the scalar mismatches.
- New stash-formation-serial-7.0.0 preserves original January formation biology,
  PIFF/T8/diffusion12/43200MCS/ten bins/lifetime2; explicitly changes workerXML4→1.
  Original profile5/executed files unchanged. Full formation7/11 jobs now run in
  cornea-stash-serial-formation46-20261024,24h/job/two workers total. Full follow-on
  reuses the existing verified one-worker injury6 without duplicate simulations.
  Separate serial source6 moderate2 continues; current native request at most4.
- Fixed resumer copied stale parent statuses even after true child-exit verification.
  Three stale-label regression cases pass. The actual original failed/prepared
  January parent was resumed to a new view with both verified completed children,
  preserved old bytes, elapsed4438.35/4461.59s and ZERO solver children/workers.
  This verifies native exit/artifact reuse, not the rejected scientific outputs.
- Targeted resume/completion22 tests/0.58s, then serial formation/operator/analysis
  33 tests/0.83s passed; targeted Ruff/mypy2 passed. Intermediate full540 tests/
  34.30s passed before the separate real spatial-quality failure/profile7 change.
- Final `make check-docker` passed541 tests/37.66s/86% coverage, strict mypy99
  source files,18 frontend tests/current contracts/all wheel/sdist builds. Host
  `make build` passed. Logs cornea-serial-formation-check-docker-20261024.log and
  cornea-serial-formation-host-build-20261024.log under.peykar. Native stability/
  publication results for new serial derivatives remain pending; no schedule,
  live deployment, API scientific-code execution or data-volume mutation occurred.


## Explicit serial corneal execution control: 2026-10-24 local

- Targeted history/January/operator checks passed37 tests/1.01s. The new recipe
  retains all source4 biological controls/PIFF; XML differs only in the declared
  worker count4→1. Old profile3/4 behavior, admission and executed copies remain.
  Targeted Ruff/mypy passed after correcting two long diagnostic string lines.
- Explicit bounded native stress probes: same two giant synthetic cells,seed7,
  200×90,10000MCS. One-worker control exit0/25.06s, last sampled9900MCS/18000volume.
  Four workers with both trackers SIGSEGV/-11/1.85s. Pixel-only SIGABRT/-6/1.91s;
  boundary-only SIGSEGV/-11/1.89s; neither tracker SIGABRT/-6/2.09s. Actual exits/
  XML/PIFF/operator hashes retained. Removing trackers did not eliminate failures;
  a BoundaryPixelTracker-only corruption-origin claim/patch is not justified.
- Two earlier probe setups failed before firstMCS (PIF path, observer import scope),
  retained with actual returncodes. Corrected probes are software stress controls,
  not corneal biology or full-horizon stability. No installed binary was modified.
- Exhaustive range-based Data.zip member-kind listing counted78574 entries:
  242 recognized scalar CSVs,147Parquets,3DML,9946VTK,68217PNG,6MP4,6PDF,6JPG,
  one original plot_manager.py. No extra parameter/config file was identified by
  this member inventory; arbitrary embedded content/whole ZIP MD5 remain unverified.
- New offline `observed-injury-serial-6.0.0` explicitly started moderate7/11 full
  50400MCS jobs with one worker each,24h/job. Original biology, pulse/doses, units,
  initial PIFF and acceptance remain fixed; stochastic parallel equivalence unknown.
  Plan `.peykar/cornea-serial-derived-moderate46-20261024`, finite three-stage
  moderate-only follow-on in `cornea-serial-derived-moderate-analysis-20261024`.
  Full native completion/scientific outcome remain pending. January formation2
  still runs original copied four-worker inputs; no healthy run duplicated/stopped.
- Final `make check-docker` passed537 tests/39.28s/86% coverage, strict mypy99 files,
  18frontend tests/current contracts/all package builds. Host `make build` passed.
  Logs `.peykar/cornea-serial-execution-check-docker-20261024.log` and
  `.peykar/cornea-serial-execution-host-build-20261024.log`. The previous534-test
  final gate preceded this separate critical native-concurrency discovery/change.
  No recurring schedule, deployment or live data mutation; heartbeat remains paused.


## Native failed-child diagnosis and retry bounds: 2026-10-24 local

- Targeted completion-guard/operator/publication tests passed29 tests/0.63s.
  Real subprocess fixtures retained exit9 and actual SIGTERM/-15 (outer143),
  partial hashes and an unchanged unadmitted outer record; wrong exit/artifact
  identities are rejected. Six four-worker retries are rejected before any child.
  Targeted Ruff check/format and mypy on both modified package files passed.
- The new failed-child extractor was run read-only on the actual source4 native
  abort. It verified SIGABRT/-6, outer134 and25 partial artifact hashes. Original
  failed record bytes are unchanged. No native corruption fix is claimed.
- `analyze_spatial.py` re-read all six complete January injury jobs into fresh
  output. Diagnostics1.2 adds complete original XML type legends; ALL earlier
  numeric snapshot rows and all six EGF PNG bytes are identical. Updated final
  moderate11 structure was visually inspected; scalar acceptance is unchanged.
- The existing completed four-job source4 view received a three-stage explicit
  follow-on: comparison2/diagnostic_not_reproduced, original0/spatial0. Every
  per-job comparison row equals the earlier negative subset report. The failed
  moderate7 job is not admitted or hidden by this diagnostic.
- Final `make check-docker` passed534 tests/38.00s/86% coverage, strict mypy99
  source files,18 frontend tests, current contracts and all wheel/sdist builds.
  Log `.peykar/cornea-terminal-failure-check-docker-20261024.log`.
  Host `make build` passed, log `.peykar/cornea-terminal-failure-host-build-20261024.log`.
  Docker uses the existing task-private anonymous config/Desktop socket; tests
  never mount/reset the live volume. `/healthz` returned200/ok/version0.3.0.
- Existing formation jobs retain their original copied driver hashes. New source
  enables fatal-signal Python stacks only in future private preparations. Neither
  ongoing inputs, original models/schemas, uv.lock nor frozen acceptance changed.


## Earlier original source and completed injury evidence: 2026-10-24 local

- Whole earlier Code ZIP16764319 downloaded and verified228667541 bytes,
  MD5 441c5af29b1e175e8bf12ded8b1f411b, SHA2567ceeab7d... . All26 ordinary files
  retained pristine with an independent inventory.23 match the later archive;
  two launchers and one newline-only parameter file differ, with no changed
  numeric/Boolean controls or steppables. All238 older Git object files match.
- Objects-only reachability checked all832 original objects:808 reachable from56
  commits,24 additional objects containing22trees/two unresolved XML merge blobs.
  Extra named parameters were already audited; raw conflict artifacts preserved,
  never resolved/installed/executed. Current public source/alternate PIFF checks
  recovered no selected-cohort seed/parameter files; no author messages sent.
- `uv run pytest tests/unit/test_vcornea_sources.py -q` passed20 tests/0.11s.
  `uv run python scripts/verify_vendor.py` verified29 distributions,53387 files,
  1824257046 bytes and37 materialized Git links. Existing pristine sources remain.
- Final `make check-docker` passed531 tests/39.04s/86% coverage, strict mypy99
  source files,18 frontend tests, current contracts and all wheel/sdist builds.
  Log `.peykar/cornea-prior-source-check-docker-20261023.log`; anonymous task-private
  Docker config uses the existing Desktop socket. Host `make build` passed,
  log `.peykar/cornea-prior-source-host-build-20261024.log`.
- Real source5 injury6 completed50400MCS/exit0,11311.59–11557.02s. Explicit native46
  chemical audit checked all50401-sample grids and actual720h [100,75] pulses.
  Read-only retention script verified all six durable proofs and interim pulse
  SHA continuity. `analyze_vcornea_series.py --diagnostic-case slight-injury
  --diagnostic-case mild-injury --diagnostic-case moderate-injury` completed all
  stages: frozen comparison2/diagnostic_not_reproduced, original0/spatial0.
  Three jobs pass all scalar checks, three fail; full family gate remains open.
- Source4 terminal inspection verified five full exits and retained moderate7's
  actual SIGABRT/-6 (outer134), last observed25000MCS. Original macOS allocator
  detection stack was inspected and sanitized evidence retained. Corruption
  origin remains undetermined; the full derived analysis correctly stopped.
- A finite explicitly invoked offline operator checked two January formation
  workspaces had never started, preserved the old failed prerequisite plan and
  started the same frozen prepared inputs after inspecting no active native jobs.
  New plan `.peykar/cornea-stash-prepared-formation46-20261024`; actual completion
  and full follow-on remain pending. No schedule/API/Worker deployment/data reset.


## Full corneal final-state panels and original inverse constraints: 2026-10-23

- Targeted original-field/publication-analysis checks passed **16 tests /0.10s**;
  targeted Ruff check/format passed. The added case rejects injury figures that
  have early20-day snapshots but lack the actual final50400MCS state.
- `analyze_spatial.py --runs .peykar/cornea-hpc46-resumed-20261023 --output
  .peykar/cornea-baseline-final-spatial-20261023` actually re-read all eight verified
  jobs and completed the lattice/cell/count checks. Every original numeric snapshot
  row is identical to the previous report. Diagnostics1.1 adds initial/final panels
  and explicit figure times; no simulation or frozen comparison changed.
- Both moderate seeds' full-horizon structure/EGF figures were visually inspected.
  Final descriptive basal-access widths0/4µm and membrane842/784 versus990 remain
  observations, not clinical closure or a new acceptance threshold. Full source2
  comparison remains not_reproduced; source4/5 full work is still pending.
- Explicit read-only original parameter search inspected32 unique Parameters.py
  blobs across all56 commit trees; output versions have100/14600MCS horizons.
  No selected-cohort controls or top-level seed assignment were identified by this
  bounded path/literal AST search. All78574 DataZIP pathnames were searched for
  parameter/config/seed/manifest/batch names; exact input hashes and scope retained.
- Separate exact original pressure inverse re-read418 initial cells with the checked
  ASCII reader. Under declared common-target/common-lambda and pressure-timing
  assumptions, all eight type/group systems uniquely yield targets40/40/25/25 and
  lambdas2/2/2/5. Every cell satisfies the exact rational equations; no outlier,
  candidate output or known numeric lambda/target was supplied. Original spatial
  groups differ from the frozen scalar cohorts. Earlier conditional receipt is unchanged.
- The installed4.6 getter matched its reviewed397f8b1 source formula at volume0
  for three target/lambda settings. An initial attempt to set read-only volume
  raised AttributeError; the first inverse attempt then lacked its probe receipt.
  Corrected read-only probes/inverse completed with preserved actual source/results.
  Official release chronology does not identify the original HPC binary; no runtime changed.
- Final **`make check-docker` passed531 tests /39.40s /86% coverage**, strict mypy99
  files,18 frontend tests, current contracts and all builds. Host **`make build`
  passed**. Logs `.peykar/cornea-final-state-check-docker-20261023.log` and
  `.peykar/cornea-final-state-build-20261023.log`. Frozen protocol/compare/source
  versions/licenses/live volume are unchanged; heartbeat remains paused.

## Separate January corneal source families: 2026-10-23

- `uv run python scripts/recover_vcornea_history.py --archive
  .peykar/external/archives/vCornea-16884414.zip --pins
  engine_adapters/compucell3d/vcornea-history-c7f95d28.json --output
  .peykar/cornea-stash-recovery-20261023` verified all50 original files from the
  pinned ZIP objects. No downloaded Git config/hooks/model code executed.
- `uv run python scripts/verify_vendor.py` verified28 distributions,53361 files,
  1820409663 bytes and37 materialized Git links. Earlier notices/source bytes remain.
- Targeted corneal stash/history/operator/publication/completion checks passed
  **46 tests /1.09s**. Targeted Ruff check/format and mypy on the new integration passed.
- Explicit native probes captured real exit0: formation20 with/without observer
  in22.12/15.09s, all five original numeric files byte-identical; moderate600 in
  139.91s, actual early pulse20MCS at[100,75]/2500. Native read-only audit checked
  61 count rows,610 height rows,601 full chemical samples and every saved cell/lattice
  volume/count. Basement990→901, epithelial5015→4204. These are engineering probes,
  never full publication or biological acceptance. The first wrapper's missing
  driver filename stopped after successful formation; its unexecuted preparation
  and log remain. Corrected fresh attempt retains actual native proof.
- Final **`make check-docker` passed530 tests /40.18s /86% coverage**, strict
  mypy99 source files,18 frontend tests, current contracts and all wheel/sdist builds.
  Log `.peykar/cornea-january-source-check-docker-20261023.log`; task-private anonymous
  Docker config uses the original Desktop socket. Host **`make build` passed**,
  log `.peykar/cornea-january-source-build-20261023.log`.
- Six source5 injury and two formation full jobs are frozen in separate prepared
  plans and wait for explicit source4 capacity/completion gates. Native worker counts
  are read from actual XML (injury1,formation4). A bounded follow-on waits for both
  families before the unchanged full comparison/original math/spatial analysis.
  Current scientific outcome is pending, not a full reproduction claim.

## Complete corneal baseline and original-analysis export: 2026-10-23

- Six resumed injury jobs actually completed50400MCS, exit0,11262.71–11443.62s.
  The full eight-job follow-on finished17:29UTC: comparison exit2/not_reproduced,
  original/spatial analysis exit0. Both moderate seeds pass the frozen scalar
  criteria; the other six jobs fail. Complete results and actual source/exit proofs
  are linked from `experiments/vcornea/publication/completed-baseline-audit-2026-10-23.md`.
- Explicit `.peykar/audit_completed_cornea_chemical_20261023.py --runs
  .peykar/cornea-hpc46-resumed-20261023 --output
  .peykar/cornea-hpc46-full-chemical-audit-20261023.json` ran in native46 Python.
  All six traces have50401 finite/nonnegative samples, full0.1h grids and actual
  one-site pulses[100,65], matching their declared source controls. The exact
  read-only recipe and report are retained; it does not execute model code.
- Read-only original audits checked44 scalar SLS traces (all exact720h/dose/grid),
  65 Parquet producer metadata (fastparquet2024.5.0 /pandas2.2.2, keypandas only),
  descriptive original population-count returns and the same2D basal-access proxy.
  MildComb0 is52µm exposed at24h and0µm at120h; its time/group remains separate from
  selected MildComb1. All original member hashes remain pinned. No new biological
  acceptance or source/runtime/seed inference was made from these observations.
- Original-analysis PNGs actually clipped some titles/axis labels. A bounds-only
  export attempt did not solve every figure and remains retained with its exact
  source versions. The final wrapper also fits the layout before export. Upstream
  plot/membrane source hashes and all scientific mathematics remain unchanged.
- `analyze_candidates.py --source experiments/vcornea/publication/upstream --runs
  .peykar/cornea-hpc46-resumed-20261023 --output
  .peykar/cornea-hpc46-original-layout-retry2-20261023` completed all8 native inputs.
  `analyze_original.py --source experiments/vcornea/publication/upstream --data
  .peykar/external/vcornea-publication-cohorts-16884414/Data --output
  .peykar/cornea-original-cohorts-layout-retry1-20261023` completed all65 originals.
  Numerical summaries and all eight full-horizon mean CSV byte sequences match the
  earlier analyses. The older65-run receipt has no per-input hash list; its summaries/
  CSVs were compared, and the new receipt retains the130 actual input hashes.
- Final nativeMild count and original formation-height/moderate-count exports were
  visually inspected with complete titles/axis labels. The earlier baseline layer
  figures for three severities, moderate EGF and all three original injury-count
  figures were also inspected. Comparison script302b06bc... and frozen protocol
  1189fd62... are unchanged. Layout wrapper SHA is
  d0c09adc23322eace61fe13a0212e13e7161efe729b1c050b9c2d20d4ad26a97.
- Derived operator53938 started its four slight/mild jobs after both native
  prerequisites actually completed; two moderate jobs wait for capacity. Combined
  follow-on54075 remains waiting for six actual records. The complete reconstructed
  comparison and independent biological validation remain open.
- Final **`make check-docker` passed518 tests /47.09s /86% coverage**, strict
  mypy98 source files,18 frontend tests, current contracts and all wheel/sdist
  builds. Log: `.peykar/cornea-completed-baseline-check-docker-20261023.log`.
  All130 fresh original-analysis input hashes exactly match the frozen protocol.
  Host `make build` also passed; log `.peykar/cornea-completed-baseline-build-20261023.log`.

## Completed corneal formation and original field integrity: 2026-10-23

- Existing `reproduce_vcornea.py --parameter-profile historical-hpc-3.0.0` jobs
  actually completed both seeds7/11 at43200MCS, exit0,5963.56/5962.51s. Existing
  `analyze_vcornea_series.py --diagnostic-case self-organization` completed all three
  stages exit0 at16:29UTC. Each seed passed4/4 stable count means,4/4 full hourly
  trajectories and10/10 thickness bins under the unchanged1189fd62... protocol.
  Both actual layer figures, both original-analysis figures and a count-overlay
  figure were visually inspected. Portable proofs/figures have formation-historical3-*
  names in `experiments/vcornea/publication/observed-2026-10-23`. The full injury
  publication gate remains pending; this is explicitly a formation subset.
- A read-only audit of all56 original Git commit metadata and two formation source
  fingerprints retained the January2025 stash and historical notebook4.5.0 stdout/
  failed simulation cell. Setup scripts request4.6.0; no exact HPC runtime or
  original per-run seed is inferred. Receipts are source-history-observations-2026-10-23.json.
- An initial original-EGF inspection failed on the missing finalCONTROL EGF_Seen
  array; `.peykar/cornea-original-sensing-audit-initial-failure-20261023.json` retains
  the observed error. Actual ASCII inspection then found Pressure9597/18000 values
  and three later fields absent in the correct-checksum original file. The checked
  reader rejects incomplete/nonfinite required fields and records optional gaps.
- `uv run pytest tests/integration/test_vcornea_original_fields.py
  tests/integration/test_vcornea_publication_analysis.py -q` passed **15 tests /0.09s**.
  Targeted Ruff check/format passed. These software fixtures are not tissue validation.
- `inspect_original_spatial.py --source .peykar/external/vcornea-original-spatial-16884414
  --catalog experiments/vcornea/publication/spatial-source-catalog-1.0.0.json
  --output .peykar/cornea-original-spatial-inspection-layout-20261023` actually
  inspected10 snapshots, recording one incomplete optional-field snapshot. Every
  earlier cell-type/EGF/SLS PNG hash matched exactly; no raw data changed.
- `audit_original_sensing.py` with the same source/catalog and output
  `.peykar/cornea-original-sensing-audit-retry2-20261023` retained1218 positive cells
  and one unavailable snapshot. Its per-cell CSV retains the Mild46200MCS stem8002
  sum discrepancy1.807958/relative0.893025; all other relative discrepancies are at
  most2.611e-7. No outlier was discarded. The earlier successful pre-format attempt
  retains its exact source copy and receipt; the final run pins the formatted source.
  These original-field observations change no candidate parameters or criteria.
- Final **`make check-docker` passed518 tests /37.63s /86% coverage**, strict mypy98
  files,18 frontend tests, current contract exports and all three wheel/sdist pairs.
  Host **`make build` passed**. Logs:
  `.peykar/cornea-formation-quality-check-docker-20261023.log` and
  `.peykar/cornea-formation-quality-build-20261023.log`. The existing task-private
  anonymous Docker config used the same Desktop socket. Original locks/schemas/
  model packages/licenses/vendor files have no Git changes; protocol SHA stays
  1189fd62ece9d90113e6096c51fa4490e6385901b0390e0a8520cb106c40bb43.
  API health returned200/ok; API/Worker still run on their original shared volume.

## Corneal original-state constraints and declared reconstruction: 2026-10-23

- `audit_original_state.py` actually read original MildComb0/moderateMCS0 pressure,
  volume and lineage fields plus MildComb0's3000MCS pulse. The immutable receipt
  `experiments/vcornea/publication/source-state-correspondence-1.0.0.json` hashes
  to `08b485a996d7ee5a636fa950f6b64d8c1ba05f18f7d7bf3319e70573c95a2d05`.
  Under the known source lambdas, all observed cells imply common initial targets
  40/40/25/25 for stem/basal/wing/superficial; no unknown lambda/seed is inferred.
  All original cell identities/types match the archived injury PIFF, while actual
  savedMCS0 geometry differs160/168 type pixels and748/730 ID pixels. The separate
  original MildComb0 pulse is exactly[100,75]/1500 at3000MCS, not selected Comb1 timing.
- An isolated installed CC3D4.6.0 `CellG` getter control gave pressure−160/−100 for
  volume0/lambda2/target40/25. Assigning its read-only volume property was rejected;
  no native tissue state or original dataset was changed to perform that check.
- The explicitly declared `observed-injury-4.0.0` uses complete historical HPC
  mechanisms/defaults and a separately evidenced original injury PIFF. Its source,
  original controls, new frozen dose/time/horizon and state-evidence hashes remain
  separate. It does not claim untouched historical Git execution, original seeds,
  an exact restart checkpoint or independent biological validation.
- Actual600MCS moderate probe completed **exit0 /130.004s** with early20MCS pulse:
  initial4/32/127/46 counts, actual[100,75]/2500 injection,61 hourly count rows,
  610 ten-bin height rows,601 per-MCS SLS rows. Exact grids/pulse and every observed
  lattice/cell-volume equality passed. Membrane990→869 sites; epithelium5013→2758.
  Original writer flushes before the SimTime+1 tracking sample; a manual inspection
  initially expected602 SLS rows, then checked the actual correct601-row0..600 grid.
  The [actual engineering receipt](../experiments/vcornea/publication/observed-2026-10-23/derived-injury-engineering.json)
  retains the native record/output hashes. This short probe is not publication acceptance.
- Six full source/state candidates were prepared and frozen in
  `.peykar/cornea-observed-injury46-20261023`, with actual execution gated behind
  both existing native series. Prerequisite definitions/runtime/protocol/real exits
  are checked, wait24h, at most4 jobs with original4 CC3D workers each,24h/job.
  The full combined comparison/original/spatial follow-on retains all source/input
  plans and checks actual disjoint jobs; it never mutates earlier series or criteria.
- First combined follow-on failed before initial plan publication; its log/state
  remain. The fixed bounded preparation wait was exercised by a targeted regression.
  Original-state/history/analysis/operator tests passed **28 tests /0.49s**; Ruff and
  strict mypy passed. Final **`make check-docker` passed510 tests /44.46s /86%
  coverage**, strict mypy98 files,18 frontend tests, contracts and all wheel/sdist
  builds. Host `make build` also passed. Logs:
  `.peykar/cornea-state-check-docker-20261023.log` and
  `.peykar/cornea-state-build-20261023.log`. The same task-private anonymous Docker
  configuration avoided the unrelated account-helper stall. Neither software
  success nor queued full jobs imply a scientific match.

## Corneal publication reconstruction and recovery: 2026-10-18 / 2026-10-23

This is an offline operator/scientific continuation. API/Worker were observed
healthy on October23 and were not redeployed; the original `peykar3_peykar-data`,
licenses, historical model/schema versions and `uv.lock` were not changed.

- Original acquisition retained174 scalar members for65 published replicates,
  two unchanged analysis files and17 final-paper DOCX supplements. Selected ZIP
  members passed size/CRC/SHA checks; whole Data.zip MD5 remains unverified.
  `analyze_original.py` actually executed the authors' unchanged count/thickness
  math on all65 replicates, retaining14 output identities/five figures; source
  audit read71 table controls. Receipts are under `experiments/vcornea/publication`.
- Official macOS ARM64 CC3D4.6.0 revision2 and400 exact packages were installed
  in a separate prefix. Capture pins the installed interpreter/engine; original
  notices are retained for484 distinct package identities/607 supplied files.
  This reconstructs the engine version, not the authors' complete original system.
- Original deleted FPP-link access actually crashed natively. Private lifetime
  overlay1 reacquires the link; overlay2 also stops iteration after deleting a
  membrane cell. Extracted-method regression tests exercise both invalid lifetimes
  and their repaired behavior; upstream/vendor bytes remain unchanged.
- Actual4.6.0 observer/unobserved20MCS outputs had byte-identical counts, SLS and
  Parquet data. An actual600MCS moderate-injury probe (pulse deliberately20MCS)
  completed exit0 in127.76s; membrane sites990→790 and epithelial sites5015→3994,
  with actual lattice occupancy matching independent cell volumes. These are
  instrumentation/memory-safety checks, not published full-horizon acceptance.
- October18 `make check-docker` completed **484 tests /38.36s /87% coverage**,
  strict mypy96 files,18 frontend tests, contracts and package builds.
  `make build` also produced all three wheel/sdist pairs. Logs:
  `.peykar/cornea-check-docker.log`, `.peykar/cornea-build.log`.
- On October23, the former supervisor was gone. Only two full43,200MCS formation
  runs had actual recorded exit0. Four injury datasets looked complete but lacked
  captured exits; two moderate jobs never started. Their original bytes/logs/records
  remain. The explicit resume operator reused only the two verified successes and
  started six fresh bounded jobs; it did not infer success or overwrite old runs.
- The copied completion guard was actually exercised in a20MCS native probe:
  exit0/64.56s and count/SLS/Parquet bytes identical to the earlier observer probe.
  Its durable proof captures real child exit/source/runtime/output hashes; three
  real-subprocess tests cover success, failure and proof/reconciliation identity.
- A completed formation-only diagnostic plan was derived from the two captured
  successes, retaining the parent plan bytes/digest. `compare.py --diagnostic-case
  self-organization` returned **exit2 / diagnostic_not_reproduced**: each seed
  passes2/4 stable count means and1/10 thickness bins; hourly coverage3/4 and4/4.
  Earlier analysis on the still-running parent correctly refused admission.
  `analyze_spatial.py` passed actual lattice/cell/count checks and rendered both
  full formation series; the seed7 figure was visually inspected. The authors'
  original math was separately replayed on those two completed outputs.
- `recover_vcornea_history.py --archive .peykar/external/archives/vCornea-16884414.zip
  --output .peykar/external/vcornea-history-reacquisition-check-20261023` recovered
  and hash-verified all50 files of July14 commit b6123adf directly from original
  licensed archive objects. Downloaded Git configuration/hooks were never loaded;
  no model/batch launcher executed during acquisition. The separate historical
  formation candidate retains complete HPC mechanics,95 original literal controls,
  Temperature10 and43200MCS. Two actual20MCS native five-/ten-bin probes completed
  exit0 in8.09s each with identical count bytes, cell observations and EGF/SLS arrays.
  Full historical seeds7/11 are still running; no scientific pass is asserted.
- Selected spatial acquisition retained10 original VTK snapshots/three descriptors.
  `inspect_original_spatial.py` actually checked their hashes/dimensions/finite
  fields/cell identities and rendered10 observations. Original MildComb0 at324h
  was visually inspected. These spatial groups differ from the four scalar cohorts;
  incomplete moderate/CONTROL horizons are explicit and no identity is invented.
- Targeted publication/lifetime/guard/history/analysis tests passed **27 tests**,
  followed by **15 tests** after the diagnostic-admission change. Native
  `test_analysis.py` passed4 synthetic software tests in3.11s. Its printed
  eight-run "passed" message refers solely to generated fixtures, not actual science.
  Ruff passed,300 files formatted; strict mypy passed98 files.
- `scripts/verify_vendor.py` rehashed **27 distributions /53,311 files /
  1,817,625,830bytes /37 materialized Git links**, with no missing source, unresolved
  LFS payload or altered original notice. `make build` on October23 produced all
  three wheel/sdist pairs. Final **`make check-docker` passed497 tests /41.90s /
  86% statement coverage**, strict
  mypy98 files,18 frontend tests, source/schema/OpenAPI checks and all package builds.
  The first build attempt stalled in `docker-credential-desktop get` before tests
  and was stopped as an owned process. A task-private empty `DOCKER_CONFIG` and
  the same inspected Docker Desktop socket allowed anonymous public-image access;
  existing credentials/settings and live services were unchanged. Final log:
  `.peykar/cornea-check-docker-public-20261023.log`; host build:
  `.peykar/cornea-build-20261023.log`.

The unchanged frozen comparison SHA256 is
`1189fd62ece9d90113e6096c51fa4490e6385901b0390e0a8520cb106c40bb43`.
Negative results are retained under `experiments/vcornea/publication/observed-2026-10-23`.
The two principal publication scenarios are **not yet reproduced**. Original seeds,
per-run parameter snapshots and independent applicable biological validation remain
unavailable; original simulation output is not experimental evidence.

## E7 multiparty processes, ownership and instance frames: 2026-10-18 local

[Observed receipt](process-verification-2026-10-18.json) and
[E7 scope](epics/E7-multiparty-processes.md) record bounded S2.5 delivery.

- Explicit `curl --fail --location` acquisition retained **19 original Reactome
  query JSON responses / 138,226 bytes**, now committed with original versions,
  SHA-256, URLs and CC0 attribution. An initial Python urllib request returned
  HTTP 403; curl retrieved the original responses successfully. No model code,
  linked website figure or media was fetched/executed.
- `uv run python scripts/verify_processes.py --output .peykar/e7-final` passed
  offline replay: **15 molecular states / 3 reactions**, exact raw hashes, net
  dimerization coefficient **−2**, ATP/ADP coefficients **−12/+12**, separate catalyst,
  seven ligand alternatives, original sequence ranges and six phosphorylation-site
  declarations. Explicit ownership exclusions, original-unit synthetic sample,
  rejected duplicate ownership/unresolved-set quantities/immutable overwrite and
  reconstruction passed. Synthetic samples remain isolated from production.
- `uv run pytest tests/integration/test_processes.py -q`: **38 passed**. Checks
  include malformed/checksum/revision/path/missing-root source bundles, rational
  coefficient bounds, composition cycles, unknown/duplicate states, context/model
  ownership partition, concurrent immutable admission, count/concentration/time
  units, corruption and API roles/body/pagination. No solver job is enqueued.
- Final **`make check-docker`: 469 passed in 35.56 seconds, 89% statement coverage,
  strict Mypy 94 files**, Ruff, current scientific schemas/OpenAPI, **18 frontend
  tests** and all wheels/sdists. Host **`make build`** passed. All previous scientific
  schemas/OpenAPI components/operations, root `uv.lock` and **24** original license
  hashes remain unchanged; prior third-party notices remain as an exact prefix.
- Actual Chromium (`agent-browser@0.39.1`) submitted structured ownership/exclusions
  and an original-unit synthetic state frame. Changing an existing ownership version
  returned an immutable conflict. **15** final axe/geometry checks include reaction,
  phosphorylated state, matrix, ownership, source views, sample form at desktop/
  tablet/mobile/landscape and Persian RTL: **zero violations and no page overflow**.
  Screenshots were inspected; automated manual/SVG contrast checks remain incomplete.
  Reader form mutations are disabled, reload clears session and browser storage/
  cookies remain empty. Temporary test API/browser sessions were closed.
- SQLite backup API/artifact copy retained `backups/pre-processes-20261018T110527Z/`
  in the original volume and locally: **101 objects, 44 reference runs/132 events,
  24 native runs/75 events, 7 cohorts/38 members/57 events, 4 hybrid runs/11 nodes/20
  events, 2 experiments/4 nodes/8 events, 67,937 vocabulary terms/4,030 alias rows and
  418 artifact byte sequences**. Every locally copied artifact rehashed correctly.
- `docker compose build` / `docker compose up -d --no-build` passed. An explicit
  network-disabled, non-root offline operator admitted the source network and an
  honest three-process exclusion declaration on **peykar3_peykar-data**. No synthetic
  or browser frame was imported. Authenticated reads rehashed all nineteen original
  sources, verified exact matrix/source identities and confirmed network/ownership
  replacement attempts return **409** without replacing records.
- All eight earlier vocabulary snapshots/nine annotations and three CellML models/
  four executions remain; sixteen vocabulary and twelve CellML artifacts rehashed.
  Actual reference and native COPASI submit → worker/agent → verified output,
  idempotency, eleven samples, original units and analytical checks passed. All
  prior rows and **418** artifact bytes survived; SQLite user_version stays 1 and
  all queues are idle. Original API/Worker volume and separate native agent remain.

These are engineering/provenance checks. Declared model ownership does not prove
mechanism correspondence or authorize execution; sample origin is unreviewed.
Scientific claim authority, kinetic/elemental balance, measurement approval,
feedback/bridges and runtime checkpoints remain open.

## E6 biological vocabularies and contextual annotations: 2026-10-18 local

[Observed receipt](vocabulary-verification-2026-10-18.json) and
[E6 scope](epics/E6-biological-vocabularies.md) record bounded S2.4 delivery.

- `uv run python scripts/acquire_vocabularies.py` verified all eight cached original
  files. `uv run python scripts/verify_vocabularies.py --output .peykar/e6-final`
  admitted complete basic CL/Uberon/GO releases and five identifier-entry responses:
  **67,937 terms, 4,030 aliases, 10,813 obsolete terms**, nine unreviewed contextual
  annotations and isolated synthetic concurrency/conflict fixtures. Original raw
  and normalized digests passed; gene/protein, precursor/mature ligand and obsolete
  exact-target mistakes were rejected. Production excludes synthetic fixtures.
- Real Uberon data exposed two qualified `is_obsolete: true {source=...}` flags
  initially treated as current. Boolean-token parsing now retains the complete
  original declaration/provenance, with conflicting flags rejected. Final replay
  includes both corrected obsolete terms. No provider replacement is automatic.
- `uv run pytest tests/integration/test_vocabularies.py -q`: **29 passed**.
  Final **`make check-docker`: 431 passed in 31.75 seconds, 89% statement coverage,
  strict Mypy 90 files**, Ruff, generated schemas/OpenAPI, **18 frontend tests** and
  all wheels/sdists. Host **`make build`** passed. Historical scientific schemas,
  existing OpenAPI operations/components, root `uv.lock` and **24** original project
  license hashes remain unchanged. Existing third-party notices remain intact.
- Actual Chromium (`agent-browser@0.39.1`) searched current/obsolete terms, resolved
  a disclosed UniProt alias and submitted a structured unreviewed annotation.
  Changed content under an existing identity returned immutable conflict. Reader
  mutations are disabled; local/session storage and cookies remain empty. Ten
  final axe audits include desktop/tablet/mobile/landscape, annotation conflict
  states, term detail and Persian RTL: **zero violations and no page overflow**.
  Screenshots were inspected; automated SVG/manual contrast checks remain incomplete.
- SQLite backup API/artifact copy retained `backups/pre-vocabularies-20261017T215613Z/`
  in the original volume and locally: **83 objects, 43 reference runs/129 events,
  23 native runs/72 events, 7 cohorts/38 members/57 events, 4 hybrid runs/11 nodes/20
  events, 2 experiments/4 nodes/8 events and 401 artifact byte sequences**.
  `docker compose build` and `docker compose up -d --no-build` passed. An explicit
  offline, network-disabled operator imported eight sources and nine annotations
  into the original `peykar3_peykar-data` volume; no scientific code was executed.
- Authenticated deployed reads rehashed all **16** vocabulary artifacts, confirmed
  the two qualified Uberon flags and alias disclosure, nine unreviewed annotations
  and zero production conflicts. Three earlier CellML models/four executions remain;
  all **12** CellML execution artifacts rehashed correctly. Actual reference and
  COPASI submit → worker/agent → results, idempotency, original units, eleven samples
  and analytical checks passed. All preceding rows and **401** earlier artifact
  bytes survived; SQLite user_version remains 1, queues idle, API healthy/Worker up.

These are engineering/data-provenance checks. OWL/taxon reasoning, biological
reconciliation, scientific approval and executable biological coupling remain open.

## E5 CellML admission and operator ODE solver: 2026-10-17 local

[Observed receipt](cellml-verification-2026-10-17.json) and
[E5 scope](epics/E5-cellml-models.md) distinguish original model admission,
operator execution, queue support and scientific validity.

- `uv run pytest tests/integration/test_cellml.py -q`: **26 passed**. Checks cover
  unsafe XML/imports/namespaces, malformed declarations, finite controls, immutable
  BOM/CRLF bytes, original units, output/runtime/driver lineage, corruption,
  role/body/discovery boundaries and acknowledged-container deadline cleanup.
- `docker build --platform linux/arm64 -f runtime_profiles/cellml/Dockerfile -t peykar-cellml:0.7.1 .`
  passed after the first actual wheel import exposed missing `libxml2.so.2`.
  The dedicated Dockerfile explicitly pins the required libxml2 package.
- `uv run python scripts/verify_cellml.py --output .peykar/e5-acceptance` passed
  **four actual executions**: analytic decay (maximum absolute error
  **2.94098e-13**), original Robertson Radau/BDF (maximum difference **3.29301e-9**,
  conservation error **7.77156e-16**) and original time-unit scaling (error
  **1.42109e-14**). Wrong original time unit, undefined equation and an actual
  one-second stiff-solver deadline failed without publishing success; no CellML
  container remained. Exact source/model bytes and generated output are retained.
- Final **`make check-docker` passed: 402 tests in 29.00 seconds, 88% statement
  coverage, strict Mypy 86 source files**, Ruff, schema/OpenAPI freshness,
  **18 frontend tests** and all wheels/sdists. Host **`make build` passed**.
  The built platform wheel includes the new services/routes and final browser
  assets. Root `uv.lock`, all historical scientific schemas and every old OpenAPI
  component/operation compare unchanged; six paths are additive.
- Actual Chromium (`agent-browser@0.39.1`) admitted a BOM/CRLF CellML file through
  the UI and matched its returned bytes/digest. Retained executions display original
  units, actual plots and full tables. Reader mutations are disabled; reload clears
  the session; local/session storage and cookies are empty. Final default axe audits
  for model desktop/mobile plus result **1440×1000, 768×1024, 375×812, 812×375** and
  Persian RTL mobile report **zero violations** and no page overflow. SVG/manual
  contrast remains an incomplete automated check; screenshots were inspected.
  Initial audits exposed duplicate navigation names and a non-focusable horizontal
  data table; distinct labels and keyboard-focusable regions fixed both.
- `uv run python scripts/verify_vendor.py` rehashed **26 distributions / 53,261
  files / 1,814,782,314 bytes / 37 Git links**. New libCellML source contains all
  **1,094** tracked files with its original Apache-2.0 LICENSE. All **24** existing
  project license files retain their prior hashes; previous third-party notices
  remain byte-for-byte as the prefix of the expanded attribution file.
- SQLite backup API/artifact copy created and locally retained
  `backups/pre-cellml-20261017T212945Z/`: **74 objects, 42 reference runs/126 events,
  22 native runs/69 events, 7 cohorts/38 members/57 events, 4 hybrid runs/11 nodes/20
  events, 2 experiments/4 nodes/8 events and 381 artifact byte sequences**.
  `docker compose build` / `docker compose up -d --no-build` preserved the original
  `peykar3_peykar-data` volume. All preceding rows/artifact bytes survived; schema
  user_version remains 1 and queues are idle. API is healthy; Worker runs.
- Trusted offline source/runtime/model/execution bundles were imported into that
  deployment. Authenticated reads verified **three CellML models, four executions**
  and every original model/output digest. An initial Docker copy to the read-only
  container was rejected; transfer used its writable temporary filesystem and
  explicit bundle filenames, excluding macOS archive metadata. Existing reference
  and COPASI submit/result/analytic/idempotency checks also passed after upgrade;
  retained identities are in the observed receipt. The host native agent remains
  separate, with unchanged historical profile/driver pins.

These are engineering/numerical checks. CellML managed queues/flows, multi-file
flattening, resets/DAE/NLA and context-specific biological validation remain open.

## E4 local research workbench: 2026-10-17 local

[Observed receipt](workbench-verification-2026-10-17.json) records actual browser,
solver, artifact, source and preservation evidence. Scope and operation are in
[E4](epics/E4-research-workbench.md).

- `npm --prefix apps/dashboard run check` passed strict TypeScript, formatting,
  generated OpenAPI freshness and **15 frontend tests**. Tests reject missing/
  nonfinite native quantities, preserve physical units/quoted paths, exclude PKML
  identity columns, confine token-bearing fetches to the API, reject redirects,
  clear expired sessions, retain explicit idempotency keys and prevent a download
  button from implicitly submitting its surrounding run/editor form.
- `uv run pytest tests/integration/test_workspace_api.py -q`: **10 passed**.
  New checks cover authenticated role discovery, metadata allowlists/integrity,
  bounded persisted parent-run/native-model discovery, static traversal denial,
  real packaged assets/CSP/no-store and retained frontend notices. A separate host
  attempt that included the existing Linux child-worker API tests had two platform
  failures on macOS; those actual paths pass in the Linux regression below.
- Final **`make check-docker` passed: 376 tests in 28.94 seconds, 89% coverage**,
  strict Mypy **82** source files, Ruff lint/format, scientific schema/OpenAPI
  freshness, frontend checks and all three wheel/sdist pairs. Host **`make build`
  passed**; the actual wheel contains the browser index/assets and all three
  original frontend notices. The first frontend Docker build stopped at formatter
  differences inherited from host configuration; an explicit project formatter
  configuration fixed deterministic host/container builds. Historical scientific
  schemas and `uv.lock` remain unchanged; only additive OpenAPI discovery changed.
- Pinned `agent-browser@0.39.1` ran real Chromium UI actions against a separate
  clone, with a Linux worker and an explicitly started pinned native agent. Browser
  reference run `010c82d9-cea1-4d56-83b4-22c2727948f0` and COPASI run
  `63d44249-1aed-4420-aa3f-65d6bc6af724` succeeded and displayed actual output.
  The original Aciclovir result displayed **491 samples**, minutes and µmol/l.
- The structured flow editor froze `ui.browser-flow@1.0.0` and executed reference
  → COPASI run `a16472bf-6c1b-4909-93fd-2654576496d2`. Its final A was
  **0.13533526084862807**, analytical `exp(-2)` **0.1353352832366127**, absolute
  error **2.2387985e-8** after explicit molar/millimolar transfer. This remains a
  synthetic engineering check.
- Browser CSV import retained `ui.browser-observations@1.0.0` and real lineage.
  A changed title under the same version returned HTTP 409 without altering it.
  Freezing an evaluation set, evaluating the completed reference run and retrieving
  the run-bound report passed as **synthetic_verification**. Queued cancellation
  `f9ebc0b6-041e-46c2-9323-f288f1870fe3` published no result. Stopping only the
  isolated API displayed the degraded state; Retry reconnected after restart.
- Browser checks exercised reader-disabled mutations, storage-free credentials,
  reconnect on reload, keyboard access, Escape/focus return, reduced motion,
  **1440×1000**, **768×1024**, **375×812**, **812×375**, and Persian RTL. Final
  default axe audits for these seven workspace/size combinations plus Persian
  mobile reported **zero violations**; SVG contrast/manual checks remain explicitly
  incomplete in the automated report and were visually reviewed. An initial empty
  heading-order issue, narrow plot-axis legibility and landscape navigation/route
  anchor detection were corrected and repeated. Screenshots were visually inspected.
- `uv run python scripts/verify_vendor.py` rehashed **25 distributions / 52,167
  files / 1,799,838,581 bytes / 37 materialized Git links** with no differences.
  All **24** original project license files match Git HEAD byte for byte; the
  complete previous third-party notice remains, with React/React DOM/Scheduler
  attribution appended. `npm --prefix apps/dashboard audit --omit=dev --json` reported **0 runtime
  dependency vulnerabilities**; this is an observed scan, not a permanent guarantee.
- SQLite backup API/artifact copy created
  `backups/pre-workbench-20261017T203213Z/`, also copied locally. Pre-restart checks
  found all queues idle. `docker compose build` and `docker compose up -d --no-build`
  deployed the same original `peykar3_peykar-data` volume. API is healthy and the
  Worker runs without Docker access; the original production native agent remains
  separate. Temporary browser-test containers used their own volume.
- Final live `uv run python .peykar/e4-live-verify.py` checked the deployed app/CSP,
  role and bounded discovery APIs, reference run 1e6c7c76-1c0e-4e8c-a740-2f3fa4040b06 and actual
  COPASI run 057d76e3-c55c-443f-afce-9e97a0f93f04. Both succeeded; native CSV/metadata hashes and its
  11-sample analytic endpoint passed. This private operator replay read credentials
  from `.env` without printing or placing them in arguments/assets.
- Post-upgrade comparison retained **74 object versions, 41 ordinary runs/123
  events, 2 experiments/4 nodes/8 events, 21 native runs/66 events, 7 batches/38
  members/57 events, 4 hybrid flows/11 nodes/20 events**, and all **380** previous
  artifact byte sequences. SQLite `user_version` remains **1**; all queues are idle.

The engineering workbench is delivered for the existing bounded local contracts.
Independent quality/split authority, scientific approvals, richer authoring,
spatial/feedback control and remaining specialist adapters retain their own gates.

## E3 native/reference scalar DAGs: 2026-10-17 local

[Observed hybrid-flow evidence](hybrid-flow-verification-2026-10-17.json) retains
actual node/transfer/result identities and numerical outputs.

- Targeted hybrid/cohort/native/workflow checks passed **95 tests**; after adding
  source-integrity/failure/fanout gates, **26 E3 tests** passed. Tests cover exact
  unit transfer, byte-preserving XML edits including BOM/comments, immutable parents,
  atomic derived registry/receipt/child rollback, concurrency/capacity, restart,
  cancellation/late success, cycles/duplicate writers, semantics/context, unsupported
  amount/assigned/event inputs, corrupted source manifests and API roles/idempotency.
- Final **`make check-docker` passed: 366 tests in 24.61 seconds, 88% coverage**,
  strict Mypy **80** source files, Ruff lint/format, current schemas/OpenAPI and all
  three wheel/sdist pairs. **`make build` passed** on the host; the platform wheel
  includes `005_hybrid_flows.sql` and hybrid modules. Earlier schema JSON versions
  and `uv.lock` are unchanged. An earlier Docker check stopped at lint before tests;
  the corrected final command above completed successfully.
- SQLite backup API/artifact copy created
  `backups/pre-hybrid-flows-20261017T103604Z/`, also copied locally. All existing
  queues were idle before `docker compose build` / `docker compose up -d --no-build`.
  API/ordinary Worker use the same original volume; native host image/driver pins
  and the absence of API Docker access remain unchanged.
- `uv run python scripts/verify_hybrid_flows.py --output
  .peykar/e3-live-verification` passed against the deployed services and real COPASI
  containers. It admitted the interface/reference descriptors through HTTP, froze
  both flows idempotently, executed dependencies and verified transfer/derivation
  hashes and unchanged original model bytes.
- Three-stage COPASI → reference → COPASI flow
  `95649676-962b-454f-b3de-98096e19deb8` returned terminal A
  **0.04978705189575012** against analytical `exp(-3)` = **0.049787068367863944**,
  absolute error **1.6472114e-8**. Units passed through explicit millimolar,
  micromolar and molar declarations. The downstream native model was admitted as
  a separate derivative with original license/context and parent/upstream lineage.
- Direct native → native flow `823eb118-fc25-4416-be6e-67d032fb79cd` returned
  **0.13533523846075707** against `exp(-2)` = **0.1353352832366127**,
  absolute error **4.4775856e-8**. These are authored synthetic numerical checks.
- Running native-source cancellation in flow
  `817d3c6d-4474-4a17-bb6e-a5c34b7494f2` ended cancelled and skipped both remaining
  nodes. Actual source timeout in `fe0d823b-ce94-43ab-ba30-8cd2450f1e99` ended failed
  and skipped downstream nodes. No native container remained; all queues were idle.
- Preservation comparisons retained every earlier **65 object versions, 40 ordinary
  runs/120 events, 2 experiments/4 nodes/8 events, 15 native runs/47 events and
  7 batches/38 members/57 events**. All **367** previous artifact byte sequences
  and **24** original license hashes match. SQLite `user_version` remains **1**.

E3 establishes the restricted sequential scalar initial-condition workflow. It does
not close general native mappings, PKML inputs, shared-time feedback, checkpoint
continuation, independent biological bridges or full UI gates.

## E2 cohorts, parameter sweeps and terminal statistics: 2026-10-17 local

[Actual cohort evidence](cohort-verification-2026-10-17.json) retains exact member,
manifest/result hashes, sample statistics and preservation counts.

- Targeted cohort/native/workflow checks passed **70 tests** before the final
  cleanup fix. After adding creation/cancellation and persistent-loop regressions,
  targeted cohort/native checks passed **60 tests**.
- Final **`make check-docker` passed: 340 tests in 22.04 seconds, 88% coverage**,
  strict Mypy **75** source files, Ruff lint/format, current schemas/OpenAPI and all
  three package wheel/sdist pairs. Final host **`make build` passed**. The platform
  wheel includes batch modules and `004_batches.sql`. Earlier versioned schema
  JSON and `uv.lock` are unchanged; new batch contracts have their own schemas.
- Added admission tests cover Cartesian grids, physically equivalent duplicate
  values, parameter units/ranges, deterministic seed allocation/overflow, job/sample/
  wall/snapshot-size limits and unsupported native seeds/parameter overrides.
  Lifecycle tests cover concurrent capacity, atomic rollback in both queues,
  restart, cancel/late-success fencing, continue/fail-fast, role enforcement,
  idempotency and immutable versions. Reports check stochastic replay, n-1 sample
  SD, missing units, tampered lineage and explicit incomplete/excluded members.
- SQLite backup API and artifact copy made
  `backups/pre-cohorts-20261017T091920Z/`, also copied locally. All existing queue
  families were idle. `docker compose build` and `docker compose up -d --no-build`
  deployed API/ordinary Worker on the original volume; no Docker socket was added.
- Live command `uv run python scripts/verify_batches.py --output
  .peykar/cohort-live-final-verification` passed with the real ordinary Worker and
  installed native host agent. Cohort `13402df3-3a2e-41e0-b373-42397bb252fc` ran
  **12** actual seeded parameter-grid jobs. Report statistics matched downloaded
  child values. Cohort `bfc913b8-0903-4b90-87f3-dcc13b6066e1` completed reference
  decay, actual COPASI SBML and original Aciclovir PKML with native unit metadata.
- Cancelling cohort `ea6fc927-5ded-4a4f-9aca-b69e23afbb26` during its first native
  member cancelled that member and skipped both unstarted members. Terminal report
  groups remained explicitly incomplete, with no invented measurements.
- The initial cancellation exercise exposed a real Docker create/removal race:
  cancelled native job `6543f3bb-9144-4655-a975-d33137e39832` left a container in
  `Created`. Its mounts identified the cancelled job; it was inspected and removed.
  The corrected agent acknowledges bounded creation before a separately supervised
  start, checks cancellation around creation and stops the attach client before
  final removal. The repeated actual cohort checks above left **no native container**.
- Loop mode now continues after redacted failed/cancelled attempts without retrying
  terminal jobs. Following the real cancellation, the same loop consumed native
  job `e7c4e79d-ed31-4f28-b607-0588532095cd` successfully. Creation uses a separate
  15-second RPC bound; the solver wall budget starts after creation. An unreachable
  daemon/uncertain cleanup remains an explicit retained operator warning.
- An additional actual deadline test created cohort
  `3d3224e0-456c-4f46-b30a-83e5229d79d2`: its first child failed under a 0.001-second
  budget, its independent next child succeeded, and overall status was failed.
  The failed group had no endpoint statistic; the successful group stayed complete.
- Final preservation comparisons retained all previous **61 objects, 12 ordinary
  runs/36 events, 2 experiments/4 nodes/8 events, 8 native runs/24 events**. All
  **349** earlier artifact byte sequences and **24** original license hashes match.
  SQLite `user_version` remains **1**. All queues were idle after verification.

This closes the bounded E2/S4.5 engineering workflow. Native parameter/seed control,
native DAG bindings, other engine adapters, sensitivity/identifiability, uncertainty
by source, full UI and independent biological evidence remain separate open tasks.

## Declarative SBML/PKML and native agent: 2026-10-17 local

[Actual retained outcomes](native-agent-verification-2026-10-17.json) include exact
live run/result/image/driver/artifact identities and preservation counts. This is
engineering verification of two native drivers, not completion of the platform.

- `uv run pytest -q tests/integration/test_native_jobs.py`: **32 passed** after the
  real-output correction. Covers immutable raw XML/units, hostile XML, model/profile
  compatibility, idempotency/concurrent claims, additive migration preservation,
  stale leases, cancellation, numerical envelopes, API roles/claim tokens, local
  image/driver allowlists, output symlinks and supervised cancellation/timeout.
- Final **`make check-docker` passed**: Ruff lint/format, strict Mypy **71** source
  files, **312 tests in 16.93 seconds**, **87%** statement coverage, current generated
  schemas/OpenAPI and all three wheel/sdist pairs. `make build` also passed on the
  host. The platform wheel includes `003_native_runs.sql` and native modules.
  Earlier immutable schema JSON files and the workspace uv lock were not changed.
- `scripts/verify_native_agent.py` passed against isolated API port 8012 for actual
  COPASI SBML and original OSPSuite PKML, including minute-to-second conversion.
  The temporary development server was stopped after verification.
- Before deployment, SQLite's backup API plus artifact copy created
  `backups/pre-native-agent-20261017T082908Z/`, copied to local `.peykar/backups/`.
  Ordinary/experiment queues were idle. `docker compose build` and
  `docker compose up -d --no-build` rebuilt/recreated API and Worker on the same
  `peykar3_peykar-data` volume, without adding Docker socket access.
- Offline `peykar native register-profile` admitted both reviewed profiles; offline
  `register-model` admitted original **7,428,313-byte** Aciclovir PKML from the
  vendored OSPSuite-R source. Its exact SHA-256 is retained in the model metadata.
  The existing 2 MB HTTP limit remains; declarative XML admission is capped at 10 MiB.
- The first live PKML attempt correctly failed publication because the validator
  mistook native `IndividualId` for a physical quantity. Run
  `f424b790-ba29-4e28-b85d-0e6e3daa089b` remains failed. The fix retains that original
  integer identity without assigning a unit and rejects mixed/noninteger identities;
  native quantity metadata must match every other column. Actual output replay,
  targeted tests and the final full regression above passed after this correction.
- Live command `uv run python scripts/verify_native_agent.py --output
  .peykar/native-agent-live-final-verification --pkml-model-id native.aciclovir`
  passed: HTTP SBML admission, idempotent native jobs, real Docker solvers, downloaded
  artifact hashes, event order and queued cancellation. COPASI run
  `a80fb520-715c-4604-9969-213bc1af526b` returned **11** points over **one minute**,
  max analytical error **1.5673282e-7**, conservation error **3.3306691e-16**.
  Original PKML run `e682d2cc-6404-493a-9cda-d725eafb9381` returned **491** samples,
  original **µmol/l** and **minute** metadata, with no schedule override.
- An explicit local failure check used the real installed containers: cancelling
  running PKML `5e7b7029-167d-4199-b255-2e2e9339df2c` ended cancelled; a 0.05-second
  COPASI budget `b558d682-c9f9-47aa-9392-1d1624156587` ended failed with `TimeoutError`.
  Both published no result and left no named native container behind.
- Started `peykar native agent ... --loop` separately on the host. It consumed a new
  HTTP-submitted job without the verifier executing the container itself: run
  `a68cb549-d325-4a2a-b4b4-5ce4d063f316` succeeded and output hashes matched. Anonymous
  native model access returned **401**. Agent startup is separate from Compose and
  needs host lifecycle supervision across restarts; it is not installed as an OS service.
- Existing ordinary Worker verification via `scripts/verify_api.py
  experiments/manifests/decay.json` passed again, run
  `df7b36ae-09d2-469f-a9ce-e77779254991`, **11 samples**, idempotency and hashes checked.
- Post-upgrade comparisons retained every pre-upgrade row: **57 objects, 11 ordinary
  runs, 33 events, 2 experiments, 4 nodes and 8 experiment events**. All **342** prior
  artifact hashes and **24** original license hashes match; SQLite `user_version`
  remains **1**. New records are additive, including the retained failed attempt.

CellML, other native engine queue adapters, native DAG integration, independent
biological validation, checkpoint/resume, feedback coupling and complete UI remain
open in the roadmap. Historical operator-probe evidence was not rewritten to
advertise the new, separately registered execution profiles.

## Permanent sources and installed runtimes: 2026-10-17 local

The [observed summary](native-runtime-verification-2026-10-17.json) and
[source/runtime evidence records](../engine_adapters/runtime-evidence-2026-10-17.json)
retain this continuation independently of earlier observations below.

- Explicit acquisition resolved all original recursive Git links plus nested
  nanobind/robin-map, separate VCell solvers and OSP helper dependencies. Reviewed
  relative HTTPS Git origins are supported. `scripts/vendor_sources.py` materialized
  sources as ordinary project files, preserving notices and validating Git blobs.
  `scripts/verify_vendor.py` passed: **25 source identities, 52,167 files,
  1,799,838,581 bytes, 37 materialized Git links**, zero missing dependencies/LFS.
- The separately pinned native runtime builds/pulls and explicit
  `scripts/probe_native_runtimes.py PROFILE --output NEW_DIRECTORY` executions
  passed for COPASI, NEURON, PhysiCell, Chaste, VCell, GEARS, PhysiBoSS and OSP.
  Original Chaste Delta/Notch vertex/node monolayers both passed. Existing real
  vCornea injury-crossing evidence was retained without rewriting its record.
  Exact image/artifact identities and model limitations are in the linked receipts.
- COPASI initially exposed an actual SWIG output-mask type error; its version/layout
  guarded compatibility bridge passed a real SBML integration (max error 6.09e-8,
  conservation error 2.22e-16). PhysiCell's initial ARM64 build failed on x86-only
  flags; portable C++/OpenMP builds then passed. Chaste's official 0750 home blocked
  the caller UID; a traversal-only wrapper passed original executables unchanged.
- OSP's exact release supports rSharp 1.2.3/.NET 8. The real Aciclovir PKML solver
  produced **491** times and original **µmol/l** metadata; the metadata field was
  corrected to the original API's `metaData` name and the final native run passed.
  Windows graphical applications were not executed in Linux.
- Figshare article 24049200 v3/file 44498681 was downloaded: **253,158,197 ZIP bytes**,
  provider MD5 checked, one complete H5AD member with CRC/SHA checks. The isolated
  AnnData reader reported **37,160 K562 cells / 19,264 genes / 21 conditions** and
  finite values in its bounded 256×512 sample. This was not GEARS training or its
  original published benchmark. The Harvard Dataverse metadata requests returned
  HTTP 403; that failure remains explicit in the connector guide.
- Targeted source/evidence checks passed: **21 tests**, including modified source,
  ignored dependencies, unsafe paths, LFS pointers, partial OMEX failure, timeout
  cleanup, immutable evidence, artifact corruption and API authentication.
- Final **`make check-docker` passed**: Ruff lint/format (**199 copied files**),
  strict Mypy (**65** source files), **280 tests in 15.13 seconds**, **88%** coverage,
  current schemas/OpenAPI and all three wheel/sdist pairs. **`make build` passed**
  on the host. An earlier final-check attempt stopped at two long-line lint errors
  before testing; the corrected final command above completed successfully.
- `package_native_evidence.py` retained **12 complete source receipts**, **11 native
  execution/QC receipts**, **310 artifact blobs**. Identical reimport passed; earlier
  source 1.0.0 receipts remain unchanged and complete inventories use 2.0.0.
  The first operational importer printed a summary-method error after registration;
  database and authenticated API rereads verified all 11 admitted records.
- SQLite backup API and artifact copy made
  `backups/pre-runtimes-20261016T220136Z/`, also copied locally. No jobs were active.
  `docker compose build api worker` and `docker compose up -d api worker` passed
  against the same volume. Every earlier row survived: **34 objects, 10 runs,
  30 run events, 2 experiments, 4 nodes, 8 experiment events**. All **46** earlier
  artifact byte sequences and all **24** original license file hashes match.
- Live authenticated `/v1/runtimes` returned **11** records; the OSP filter returned
  one, anonymous access returned **401**, and both historical and complete NEURON
  source receipts remained readable. Real submit→worker→result passed again with
  idempotency, exact manifest/result hashes and **11 samples**, run
  `f0944977-87c8-4b46-96f3-adac9ad42ae8`.

These observations establish local source/runtime/engineering behavior. Original
paper reproduction, independent biological validity, general native-engine queue
adapters, Windows graphical applications and the complete UI remain open.

## Whole-platform shared backend: 2026-10-16 (unreleased, on 0.3.0)

This continuation broadens delivery beyond the eye-model path. The
[whole-platform roadmap](roadmap.md) covers S0–S13 while retaining original P0–P7
scientific gates. [The observed receipt](system-continuation-2026-10-16.json)
records source counts, live workflow results and preservation checks; it is not a
claim that the entire platform or all native engine integrations are complete.

- `scripts/acquire_ecosystem_sources.py --catalog engine_adapters/ecosystem-sources.json
  --destination .peykar/external/ecosystem/sources --jobs 3` fetched all nine exact
  top-level source snapshots: CompuCell3D, PhysiCell, VCell, COPASI, Chaste, NEURON,
  PK-Sim, MoBi and GEARS. Hooks were disabled; no upstream code/build script was
  executed. Existing vCornea Git and separately licensed archive identities were
  preserved. These depth-one working trees are not complete Git histories.
- Local `LibraryService.inspect` verified **36,291 tracked entries /
  1,308,090,422 bytes**, source commits/modes, symlink targets and license bytes.
  **24 unresolved Git links** and zero top-level LFS pointers were recorded.
  MoBi's Git-declared CRLF checkout initially failed strict blob checking;
  inspection now retains actual-byte SHA and canonical blob identity while allowing
  only declared text line-ending transformations. It does not rewrite source.
- The bundled OSP license PDFs were preserved; `pdftotext` read the MoBi copy's
  GPLv2 plus Clarifying Addendum. License evidence remains component-scoped; no
  third-party code was re-licensed. This is not a legal compatibility determination.
- Reactome `R-HSA-177929` and UniProt `P01133`/`P00533` JSON were retrieved and
  hashed. Their curated human EGF/EGFR graph was imported as evidence-linked
  metadata, with precursor/ligand and tissue-kinetic limitations retained. This
  does not create a calibrated EGF mechanism or a vCornea molecular coupling.
- Targeted new-service checks passed: **17 tests** for graph bounds,
  evidence/context/cycles, source tampering/license/bundle checks, declared CRLF,
  workflow transfer, atomic rollback, concurrent dispatch/cancellation, incompatible
  bindings, failure propagation, persistence and API roles/idempotency. An earlier
  host batch included Linux-only legacy tests; its six bounded-child failures
  were the macOS limitation, and the final Linux run below passed them.
- Final **`make check-docker` passed**: Ruff lint/format (**200 files**), strict Mypy
  (**64 source files**), **259 tests** in **13.90 seconds**, **88%** observed statement
  coverage, current generated contracts/OpenAPI and all three wheel/sdist pairs.
  Git was added only to the verification stage for local source-fixture inspection.
  Test-time network and the live data volume remain absent. Host **`make build`
  passed**; wheel inspection verified new modules, additive SQL migration and
  byte-identical LICENSE text. No application code changed afterward.
- Before recreation, no queued/running user jobs existed. SQLite backup API plus
  a separate artifact tar preserved `backups/pre-system-20261016T211636Z/`, also
  copied under local ignored `.peykar/backups/`. An optional attempt to tag the old
  container image ID failed because Docker's image store did not expose that ID;
  no new rollback tag is claimed. The data backup and earlier recovery tags remain.
  `docker compose build` and `docker compose up -d --wait` succeeded; API healthy.
- Nine offline inspection bundles were imported with the deployed
  `peykar libraries import-receipt` CLI into the existing volume. Live authenticated
  library listing returned nine entries; `capability=pbpk` returned PK-Sim/MoBi;
  anonymous reads returned 401. No Git/scientific runtime dependency was added to
  the API image, and no HTTP source filesystem-import route was exposed.
- Live graph POST/GET and depth-two neighborhood succeeded for
  `biology.human-egfr@1.0.0`, returning three entities/two evidence-linked edges.
- Live HTTP model registration, experiment freezing, idempotent submission,
  separate Worker execution, result/manifest retrieval and events all passed.
  Experiment `3f2b262a-f367-485e-b2d3-c34abcd1d076` succeeded with children
  `2ec78ad7-c272-4acc-a563-3e4b04c7ed34` and
  `ac33758f-7d3e-43c5-b61f-2a933f671e71`. The second initial concentration was
  **0.3678794411714421**, and final was **0.1353352832366125 mole/liter**, matching
  `exp(-1)`/`exp(-2)` within 1e-12. Its runtime lock was nonempty and two child
  dispatch events were retained. This is synthetic engineering verification.
- Live cancellation of experiment `3fa6fdfa-4ebf-43f1-9328-a60384a06291` reached
  `cancelled` with both unsubmitted nodes skipped. Atomic dispatch left
  **zero unassociated workflow child jobs**.
- Final integrity/foreign-key checks were `ok`: **34 objects, 10 succeeded ordinary
  runs, 30 ordinary events**, two experiment runs (one succeeded, one cancelled),
  four node records and eight experiment events. Every pre-upgrade
  object/run/event row remained unchanged; all **46** live artifact hashes passed.
  All **24** original license files and **27** pinned vCornea files remained
  unchanged. New contracts/routes and additive experiment tables were introduced
  without changing existing schema/model versions or SQLite user_version 1.

Detailed ignored logs are under `.peykar/external/ecosystem/`: `inspection.log`,
`final-check.log`, `live-platform.log`, `preservation.json` and source/research
receipts. General feedback/spatial coupling, new native runtime builds, remaining
source dependencies, independent evaluator/UQ, full UI and distributed/tenant
deployment remain open in the comprehensive roadmap.

The final documentation/schema check also passed: local links across **45** current
Markdown files resolve, generated contracts remain current, source descriptors and
the portable receipt parse, all 55 source sections remain represented, and the
original Persian blueprint checksum is unchanged.

## E1 native vCornea: 2026-10-16 (unreleased, on 0.3.0)

Observed on this macOS checkout, Docker Desktop, and a separate emulated
Linux/AMD64 scientific runtime. [E1 acceptance](epics/E1-vcornea-native-runtime.md)
and its [portable receipt](../engine_adapters/compucell3d/vcornea-verification-2026-10-16.json)
distinguish engineering completion from the remaining P1 published-replay gate.

- The public Git checkout was cloned and pinned at
  `f5c03afb9c4c7d909494146c5db608389e957070`; the licensed execution source is the
  distinct exact Zenodo record **16884414**, declaring **CC-BY-4.0**. The complete
  228,842,524-byte code ZIP passed published MD5 and local SHA-256 checks.
  `scripts/prepare_vcornea.py` extracted 27 pinned reviewed files without `.git`;
  a fresh extraction and matching-source reuse were observed. The archive's Local
  paper XML differs from Git; these identities are not silently equated.
- `scripts/acquire_vcornea.py members` acquired six complete native reference
  files for three severities through checked HTTP Range/ZIP64 reads. Size/CRC32
  and local SHA-256 passed. The acquisition transferred **16,317,077 bytes** including
  ZIP indexing. Whole Data.zip MD5 is **unverified**; the stopped bulk download's
  214,036,480-byte partial file was preserved, not reported as a full download.
- `docker build --platform linux/amd64 -f engine_adapters/compucell3d/runtime/Dockerfile
  -t peykar-cc3d:4.7.0-linux64 engine_adapters/compucell3d/runtime` passed after
  repairing an observed missing `libGL.so.1` with explicit pinned GL/X11 dependencies.
  The final lock contains **424** packages; **405** package-provided notice
  directories were preserved with the full inventory. Engine/Qt imports and an
  actual PyArrow Parquet round trip passed. The runtime receipt records CC3D 4.7.0
  revision 5, Python 3.12.2, NumPy 2.2.6, pandas 3.0.6 and PyArrow 16.1.0.
  The authors' complete original environment was not recovered.
- `scripts/run_vcornea.py` first completed a 20-MCS, seed-7 native probe in
  32.013 seconds. Its private copy produced actual count, surfactant and thickness
  files. Source mechanisms were preserved; horizon and Python/NumPy/Potts seed
  controls were recorded. No API token, home directory or live data volume was
  mounted; execution was offline with resource/deadline/output bounds.
- The exact `scripts/verify_vcornea_epic.py` command in the E1 document completed
  six published imports and four real 60-MCS runs (reference/candidate seeds 7,11).
  Published count and height series have **5,041 / 4,621 / 5,041** hourly samples;
  archived seeds/runtime/per-run parameters stay unknown. Acceptance was written
  before runs and the reference protocol frozen before candidates.
- Both seven-sample Basal comparisons passed the frozen **0 count** tolerance with
  RMSE/max error **0**. All three raw native output hashes also match within each
  seed pair. Stored report retrieval passed:
  `seed-report.a4e212d71f85c7e198ab604e8bb26f9a0938b04ff9126923403b9051380f3d06`.
  Its class is `simulation_reference_comparison`, not biological validation.
- The E1 `scripts/run_vcornea.py --steps 600 --seed 7 --wall-seconds 900` command
  succeeded in **221.084646 seconds**, producing **61** count/height samples over
  **0–60 hours**. Inspection of the actual 601-row surfactant CSV found zero before
  hour 50 and first positive integral **750** at hour **50** (500 MCS), confirming
  execution crossed the injury event. Physical units of that raw integral remain
  unestablished. Its native Parquet was decoded/imported with ten lattice-coordinate
  series. No native-run containers remained after cleanup.
- Targeted importer/source/runtime/API tests passed before the final full run.
  Final **`make check-docker` passed**: Ruff lint and format (189 files), strict
  Mypy (**59 source files**), **242 tests** in 12.14 seconds, **88%** observed statement
  coverage, current generated contracts/OpenAPI and all three wheels/sdists.
  The host's subsequent **`make build`** also produced all three 0.3.0 wheel/sdist
  pairs. No application code changed after the final full regression.
- `PEYKAR_DATA_DIR=.peykar/vcornea-epic uv run peykar import-csv
  experiments/data/rho2015-scratch/control.csv --spec experiments/data/rho2015-scratch/import.json`
  stored two real experimental aggregate control means. Attribution, source XML
  hash, original eight-row means/SD transcription and percent-to-fraction transform
  are retained. The different monolayer/GM-CSF context is explicitly excluded from
  vCornea independent validation; SD remains raw metadata, not an uncertainty model.
- Before service recreation, active-job checks returned zero. SQLite's backup API
  and a separate artifact archive produced
  `backups/pre-vcornea-20261016T205315Z/`, also copied to local ignored backups.
  `docker compose build` and `docker compose up -d --wait` succeeded against the
  unchanged named volume; API healthy and the separate worker running.
- Authenticated live HTTP POST imported the three published count outputs and
  `vcornea.epic.injury600` (201). Output/lineage GETs returned 200, exact raw CRLF
  digests matched, identical reimport preserved lineage, and anonymous reads
  returned 401. Three published Parquet files and the actual injury Parquet were
  admitted by the deployed `peykar vcornea import-thickness` CLI and read through
  HTTP with ten `lattice_y_coordinate` series. The execution manifest/record were
  stored as content-addressed artifacts. Docker's read-only-root filesystem rejected
  `docker cp` into the container; streaming the same local files through `exec` into
  its existing writable `/tmp` succeeded without weakening container settings.
- A new live gap `vcornea.published-replay.evidence-gap.20261016` was registered/read;
  the historical `vcornea.integration.pending` record was preserved unchanged.
- The deployed `scripts/verify_api.py experiments/manifests/decay.json` workflow
  again passed real submission, idempotency, polling, manifest/result hashes and
  events. Run `888a8f16-7891-4c96-8874-d2ff5c8a0dec` succeeded with 11 samples.
- Final SQLite integrity was `ok`: **22 objects, 8 succeeded runs, 24 events**.
  Every object/run/event row from both the original and pre-E1 backups remained
  byte-identical; all **25** live artifact hashes passed. All **24** existing license
  files, all **27** pinned model source files and the original Persian blueprint
  checksum remained unchanged.
- A final local link/JSON/package inspection passed for **42** Markdown documents,
  generated schema JSON and the new receipt/data descriptors. All three host-built
  wheels retain byte-identical LICENSE text; the platform wheel includes native
  acquisition/runtime/import/CLI modules.

Local detailed logs are retained under ignored `.peykar/external/`, notably
`final-epic-check.log`, `epic60/`, `injury600/`, `live-native-api.json` and
`final-epic-preservation.json`. Neither bounded exact repeats nor selected published
simulation imports establish full-horizon reproduction, independent biological
validation, hostile-code isolation or a general queued vCornea adapter.

## Earlier engineering continuation: 2026-10-16 (unreleased, on 0.3.0)

Historical observations below precede E1; the native runtime/source permissions and
output-import status are superseded by the verified E1 record above.

Observed on this macOS checkout and Docker Desktop's Linux ARM64 runtime:

- `uv sync --frozen --all-packages --group dev` succeeded on the host using
  Python 3.12.2. The lockfile was not regenerated.
- Final `make check-docker` passed. It runs `make check` and `make build` in a
  separate Linux/Python 3.12.14 verification container without the live data
  volume or test-time network. Ruff lint/format passed, strict Mypy checked
  **54 source files**, **184 tests passed**, and observed statement coverage was
  **90%**. Generated contracts/examples/OpenAPI were current.
- Targeted checks preceded the complete regression. New coverage includes runtime
  lock/platform drift, result seed/grid/unit/port mismatches, cancellation/recovery
  ownership loss, offline artifact bounds/digests, explicit scalar mapping,
  equivalent physical time units, seed cohorts, concurrent cross-import provenance
  conflicts, legacy reimports, knowledge-gap immutability, exact model references,
  filters before pagination and API reader/admin roles.
- The first Docker-check attempt stopped before tests because `uv run` attempted
  to reconcile noneditable wheel installs into editable installs without a cached
  build dependency. The verification stage now sets `UV_NO_SYNC=1` and
  `UV_OFFLINE=1`; the complete final offline run above passed.
- `make build` also succeeded on the host: all three 0.3.0 wheels and source
  distributions are under `dist/`. Direct wheel inspection found the new modules,
  the current license expression and byte-identical LICENSE text in each wheel.
  This is a working-tree continuation, not a claim of a newly published release.
- `docker compose up -d --build --wait` rebuilt/recreated API and worker against
  the existing named volume. No queued/running jobs existed before recreation.
  The API became healthy; the separate worker subsequently completed real jobs.
  Previous image tags and SQLite/artifact backups were preserved for recovery.
- `scripts/verify_api.py` exercised authenticated live submission, idempotent
  resubmission, polling, manifest/result checksum verification and lifecycle events
  for both reference engines. Final deployed runs were
  `a34e51a5-dba3-484d-a1a2-5c0825fe1980` (decay) and
  `d3767d46-5c03-4842-a241-01439136b1e6` (logistic), both succeeded with 11 samples
  and a nonempty pinned lock digest.
- Authenticated live HTTP checks registered/retrieved/filtered
  `vcornea.integration.pending`; anonymous reads returned 401. The gap explicitly
  records missing integration evidence and does not claim that vCornea ran.
- Live endpoint evaluation of the decay run returned `passed: true`,
  RMSE **1.9229626863835638e-16 mole/liter**, and
  `evidence_class: synthetic_verification`. Retrieval of the run-bound record
  `report.1bd416795e14e6888ddd9ff85db18b30bb870c43f48ad1603cc725bca83059e9`
  succeeded. This is synthetic numerical verification, not biological validation.
- SQLite backup used its backup API while services were active; exact artifacts
  were archived separately. A copy of the initial backup is in ignored
  `.peykar/backups/`. Final comparison retained every original object/run/event
  row; `PRAGMA integrity_check` returned `ok`. There were 7 succeeded runs,
  5 objects and 21 events after retained verification work; every result checksum
  was readable and verified. Existing user rows were not reset or rewritten.
- All **24** license/notice/historical-license files checked against the local
  baseline remained byte-identical. Existing scientific schema/model exports were
  unchanged; only new contract files and the expanded OpenAPI were added/updated.
- The supplied full Persian architecture was archived byte for byte as
  `blueprints/universal-biological-platform-v3.fa.md`, SHA-256
  `5a65971deb58b186ae66bb6c7899f5585b35da9410304f048e5a23f0aa9d3e6d`.
  Its coverage document maps all 55 sections and separately retains the 24
  original biological architecture scenarios; neither table claims those future
  scientific scenarios passed software tests.
- A local Markdown link/count check passed for 23 current documents, all 55
  ordered source sections and all 24 ordered original scenarios. The archived
  source checksum still matched after the design/documentation updates.
- Read-only source-permission rechecks: GitHub's repository-license API returned
  404; the Zenodo web/API record could not be retrieved (web-tool failure/TLS
  timeout). Permission remains unverified. No external model code/archive was
  downloaded, installed or executed.

Child instrumentation is still excluded from the minimal child environment.
Coverage is the observed parent/test statement measurement. Docker testing here
does not validate an untrusted-code sandbox, multi-host deployment, Python 3.13,
the dashboard or any published biological reproduction. Remaining gates are in
`roadmap.md` and `status.md`; no independent approval or clinical validity is claimed.

## Earlier edition: 0.3.0

Verified locally on 2026-10-15 with Python 3.12.14 on Linux x86_64:

- `make check`: Ruff lint/format, strict Mypy (46 source files), all **114 tests**,
  and generated schema/example checks passed. Observed statement coverage: **91%**.
- New tests exercise hung-child termination/reaping, cancellation without engine
  cooperation, secret-free child environment, output bounds, invalid child results,
  bounded heartbeat lock waits, strict CSV failures, atomic rollback, frozen split
  leakage checks, immutable tolerance, run-specific report identity, API roles and CLI.
- `make build`: all three 0.3.0 wheels and source distributions built successfully.
- The three wheels installed into a fresh separate venv using locked runtime
  dependencies. The installed demo and complete CSV/freeze/submit/worker/evaluate/
  report workflow passed outside the checkout. The final rebuilt platform wheel
  was reinstalled and that workflow passed again.
- Analytical demo RMSE remained approximately 1.923e-16 mole/liter across 11 samples.
  This is synthetic numerical verification, not biological validation.
- Wheel metadata reports `LicenseRef-Peykar-Noncommercial-1.1.0`; all packages
  include current notices and historical 1.0.0/Apache texts and baseline checksums.
  Root and package licensing copies match byte for byte.
- Existing 0.2.0 model descriptors, existing ModelPackage/DatasetSpec/ValidationReport
  schemas, reference engine source and the 0.1.0 baseline record remain unchanged.
  Current licensing metadata receives new 0.3.0 reference descriptors.
- Relative links in top-level and docs Markdown resolve. Upstream vCornea branch
  and complete tree metadata were inspected read-only and recorded with an exact
  commit; its runtime and biological results were not executed.

Child processes run in integration tests, but their statement instrumentation is
not inherited into the minimal child environment. Coverage is the observed pytest
measurement, not proof of isolation, scientific validity or legal enforceability.

## Historical edition: 0.2.0

Verified locally on 2026-10-15 with Python 3.12.14 on Linux x86_64:

- `make check`: Ruff lint/format, strict Mypy, all **92 tests**, and generated
  schema/example checks passed. The observed current statement coverage was **90%**.
- `make build`: all three 0.2.0 wheels and source distributions built successfully.
- Built-wheel metadata for every package reports
  `LicenseRef-Peykar-Noncommercial-1.0.0`. Each wheel includes LICENSE, NOTICE,
  LICENSING.md, the historical Apache text and the baseline checksum record.
- The root and all three package copies of the licensing documents match byte for byte.
- Reference model/data metadata uses version 0.2.0 and the current license reference;
  the unchanged reference algorithms retain engine version 0.1.0. Historical
  scientific versions were not overwritten in place.
- Installing the three 0.2.0 wheels into the separate verification environment and
  running `peykar demo` outside the checkout succeeded, with model version 0.2.0
  and the same analytical RMSE approximately 1.923e-16 mole/liter.
- Relative Markdown links in the top-level and principal design documents resolve.

These checks verify packaging and engineering behavior, **not legal enforceability**.
The custom license draft has not been reviewed by a lawyer or tested in court.
No public GitHub release or third-party distribution is established by this record.

## Historical foundation: 0.1.0

Snapshot: Peykar 0.1.0. Verified locally on 2026-10-15 with Python **3.12.14** on
Linux x86_64. This is an observed engineering check, not biological validation.

| Check actually run | Observed result |
|---|---|
| `uv sync --frozen --all-packages --group dev` | Workspace and locked environment resolved |
| `uv run ruff check .` | Passed |
| `uv run ruff format --check .` | Passed |
| `uv run mypy` | Passed, 42 application/core/engine source files |
| `uv run pytest --cov --cov-report=term-missing` | **92 tests passed; 92% statement coverage** |
| `uv run python scripts/export_contracts.py --check` | Generated contracts and examples are current |
| `uv build --all-packages` | Three wheels and three source distributions built |
| Install the three built wheels into a separate fresh venv | Passed |
| Run installed `peykar demo` outside the source checkout | Passed |
| Compare installed engine source with checkout | Identical bytes |
| Verify installed SQL migration is packaged | Present and used successfully by the fresh demo |

The analytical decay demo compared 11 samples against `exp(-0.1*t)` and reported
RMSE approximately **1.923e-16 mole/liter**, below its **1e-12** verification tolerance.
This is synthetic numerical verification, not independent experimental validation.

Tests cover contracts, units, source-independent analytical comparison, stochastic
seed reproduction, time-aligned interventions, model immutability, dependency
references, scenario parents, concurrent queue claims, idempotency, cancellation
races, stale-worker fencing, redacted failures, corrupted data, API roles/body
limits, generated schemas, calibration split constraints and the HomBon facade.

## Checks not claimed by the historical 2026-10-15 checks

- Python 3.13 is configured in CI but was not executed locally.
- GitHub Actions jobs have not been observed running remotely.
- Docker/Compose were not executed in that earlier environment. The 2026-10-16
  continuation above records subsequent local Docker build/runtime verification.
- CompuCell3D/vCornea and specialist engines were not installed or reproduced.
- No independent biological dataset, clinical claim, generic co-simulation runtime,
  dashboard, GPU, HPC or multi-host deployment was validated.

Coverage describes which statements were executed; it is not a scientific trust
score and does not prove every branch or operational condition is covered.
