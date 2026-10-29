# Peykar continuation context
## Acuity 1.1 closed, retinal acuity 1.0 FAILED its sealed test, 2026-10-29 evening
- Acuity tranche 2 (55) reserved before triage (`e2e4d9a3`), 27 admitted blind, 11
  sealed records/23 series (`.peykar/acuity-tranche2-sealed-2026-10-29`, custody C1–C4,
  `.peykar/verify_acuity_tranche2_custody.py`): STILL SEALED AND UNREAD. Acuity 1.1
  round 1: generic eye model 4 (`eye_recovery_population4`, options
  `random_correlation="level_excess"` and `slope_groups`; defaults = model 2 bit for
  bit; harness `eye_recovery_evaluation4`, `eye_additive_evaluation2`) candidates b1
  (+0.08/+0.07/+0.00) and b2 (initial −0.12) rejected: NO 1.1 protocol; a0 stays in
  `peykar eye`.
- Retinal acuity 1.0.0, eighth component: BCVA (logMAR, letters) after macular
  treatment. Dataset experiments/data/retina-acuity-2026-10-29: 378 by the masked-table
  rule; tranche 1 (120) triaged, 62 admitted; **tranches 2–4 (258) unread**; split salt
  `peykar-retina-acuity-holdout-v1:` 41/21; 15 dev records (52 series; 2 private in
  `.peykar/retina-acuity-development-private`), 9 sealed (11 series,
  `.peykar/retina-acuity-sealed-2026-10-29`, C1–C3). Module
  `peykar.validation.eye_retina_acuity_data`, configs
  `peykar.model_learning.retina_acuity_population` (v3 adopted: pooled forecast noise),
  operator `scripts/evaluate_retina_acuity_population.py`, publisher
  `scripts/publish_retina_acuity_population.py`. Protocol SHA `34494401…`, freeze
  `f4352217`. Sealed once (18 s): ALL FOUR FAIL (coverage 0.677/0.806; next obs 6.14
  vs persistence 4.81; landmark 9.17 vs 6.49; initial 16.40 vs pooled 5.18). Cause:
  the 7-day early transient had no training visit before day 30 and was fitted at
  +1.99 logMAR at day 1; the one sealed study with a day-1 visit (rac-pmc5377272,
  CC BY-NC-ND, contexts human/other/*) carries the failure (post hoc: without it all
  four pass). NOT in `peykar eye` (only sealed-passing versions go there). Redaction
  check `.peykar/check_retina_acuity_redaction.py` (0 leaks), breakdown
  `.peykar/retina_acuity10_sealed_breakdown.py`.
- Next: a generic time-support guard (earliest training visit per family; warn before
  it) for `peykar eye`; retinal acuity 1.1 needs tranches 2–4 reserved before triage,
  a law without the unidentified transient, `treatment_fallback`, a DEX dose rule.

## Machine restart, retina 1.2 closed, acuity 1.0 tested, 2026-10-29 afternoon

- The Mac hung after 13:03 and the hardware watchdog rebooted it at 13:09 (cause
  unknown). It killed the IOP 1.3 sealed run (41/79 d1, nothing written), the retina
  e6 shards and the triage sub-agents. The IOP 1.3 sealed run was restarted from
  scratch at 13:17 with the same command/protocol (not a second test: no output had
  existed); log `.peykar/iop13-sealed-test-2026-10-29.log` (ends with `EXIT=<code>`).
  Start long runs with `python3 .peykar/detach.py <log> <pidfile> -- <cmd...>` (own
  session + caffeinate). Docker Desktop and the stopped `peykar3-api-1`/`peykar3-worker-1`
  containers were restarted unchanged on `peykar3_peykar-data` (`/healthz` ok).
- Retina 1.2 round 2 (e6 regimen level) rejected (gain 0.012 pp, initial +0.030): NO
  1.2 protocol, tranche 3 stays sealed/unread; `check_frozen_protocols.py` still prints
  "retina 1.2.0: no protocol yet" (expected).
- Acuity 1.0.0, seventh eye component: corrected distance acuity (logMAR) after PK,
  DALK, DSAEK, DMEK, CXL. Dataset experiments/data/cornea-acuity-2026-10-29: 1,994
  screened; `screening/reserved.py` drops retina t3–4 and IOP t4–5 articles before
  triage; 175 by the masked-table rule; tranche 1 (120) triaged blind, 67 admitted;
  **tranche 2 (55) unread** for 1.1; split salt `peykar-acuity-holdout-v1:` 47/20; 27
  dev records (62 series; private in `.peykar/acuity-development-private`), 15 sealed
  (25 series, custodians C1–C3, `.peykar/acuity-sealed-2026-10-29`). Brief +
  `.peykar/acuity-extraction-brief-addendum-1.md` (supplements, complete-case rule,
  combined flag). Module `peykar.validation.cornea_acuity_data` (letters → logMAR =
  (85 − L)/50; `{"cell":..., "take": "parenthesized"}`), configs
  `peykar.model_learning.acuity_population` (a0 adopted; a1, a2, a4 rejected), operator
  `scripts/evaluate_acuity_population.py`, publisher `scripts/publish_acuity_population.py`
  (IOP 1.0 H1–H4, letters). Protocol SHA `0c60925c…`, freeze `baafe4cd`. Sealed once
  (52 s): ALL FOUR pass (coverage 0.829/0.927; next obs 5.00 vs persistence 5.06
  letters; landmark 5.24 vs 7.27; initial 9.04 vs pooled curve 9.79), but on
  development a0 failed H2/H3 to the pooled-curve increment; sealed set has no PK; one
  DSAEK group after glaucoma surgery missed by ~24 letters. `peykar eye` forecasts
  `acuity` (AdditiveComponent now takes `scenario_series`).
- Acuity results/docs/CLI committed and pushed as `f6c74736`. Pending: IOP 1.3
  publication when its restarted sealed run ends (45/79 d1 studies at 15:06, then 79 d0;
  about 2.4 min per study); acuity 1.1 starts with the tranche-2 reservation BEFORE triage.

## IOP 1.3 frozen, refraction 1.0 tested, retina 1.2 round 2 running, 2026-10-29

- IOP 1.3 (generic eye model 3, `scripts/evaluate_iop_population_1_3.py`): candidate d1
  (sub-procedure level, contexts `human/<sub>/<procedure>`) adopted on LOSO over the 229
  training studies: 6.382/8.061/10.066 pp against d0 = 1.2.0 6.464/8.348/10.209 (next
  obs/landmark/initial). d2 (lognormal-mean point) rejected (+0.064/+0.073). Protocol
  experiments/eye/iop/model-1.3/protocol-1.3.0.json SHA `0cac4daf…`, freeze `38f4f874`.
  The ONE sealed run on tranche 4 started 09:37 CET 10-29 (PID 68525, about 6 min per
  study, d1 then the d0 reference; log `.peykar/iop13-sealed-test-2026-10-29.log`,
  output `.peykar/iop13-sealed-test-2026-10-29/`). Never stop or restart it. Publisher
  `scripts/publish_iop_population_1_3.py` tests human series only (the one rabbit
  training study has individual series only) and allow-lists initial-state fields of
  redacted studies; then run `.peykar/check_iop_redaction_1_3.py` and
  `.peykar/iop13_sealed_breakdown.py`.
- Refraction 1.0.0, sixth eye component: spherical equivalent after PRK/surface
  ablation, LASIK or SMILE. Dataset experiments/data/cornea-refraction-2026-10-29:
  2,169 screened, 89 by the masked-table rule, 39 admitted blind, split salt
  `peykar-refraction-holdout-v1:` 31 development / 8 sealed; 24 development records
  (43 series; private ones in `.peykar/refraction-development-private`) and 8 sealed
  (14 series, custodians C1–C2, `.peykar/refraction-sealed-2026-10-29`). Additive
  scale: the change enters generic model 2 as the exact pseudo-fraction exp(change/5 D);
  `peykar.validation.eye_additive_evaluation` rescores in report units of 0.1 D (1 pp =
  0.1 D) and adds the plano-target comparator. f1 (procedure → refractive error)
  adopted over f0; slope b −0.90. Protocol SHA `8634f5a2…`, freeze `a2eccf2c`. Sealed
  once (39.9 s): ALL FIVE pass: coverage 0.821/0.857, next obs 0.293 vs persistence
  0.370 D, landmark 0.468 vs 0.780, initial 0.397 vs plano 0.501 and f0 0.413. A public
  severe-myopia LASIK group regressed to −1.9 D, outside the 90% landmark interval.
  Every sealed series is myopic. Both refraction sets are consumed. `peykar eye` lists
  and forecasts `refraction` (`ADDITIVE_COMPONENTS`; needs `--records`; output in D).
- Retina 1.2 round 1 (LOSO, 128 studies, Gaussian primary): e3 (prior treatment) and
  e4 (OCT device) rejected (no 0.10 pp gain). Round 2: e6 (regimen level,
  `eye_retina_data_1_2.to_retina_series_1_2_regimen`) declared from the e0 regimen
  bias diagnostic, running in 8 shards `.peykar/retina-1.2-dev/round2-e6-shard{0..7}`
  since 12:21 CET. Merge with `--merge`, compare with `.peykar/compare_iop_12.py`
  against round1-e0. If e6 is rejected: no 1.2 protocol, tranche 3 stays sealed.
- Next component candidate: BCVA (logMAR, 818 OA hits) on the additive scale; drop
  retina tranche-4 and IOP tranche-5 articles before triage so they stay unread.
  Corneal nerves had only 47 hits.
- `.peykar/check_frozen_protocols.py` checks the pins of all eleven frozen protocols
  (and reports "retina 1.2.0: no protocol yet"); run it before every commit.

## Retina 1.1 and IOP 1.2 tested; next tranches extracted, 2026-10-28/29

- Fresh sealed sets, reserved before triage and extracted blind: retina tranche 2
  (63 records, `.peykar/retina-tranche2-sealed-2026-10-28`) and IOP tranche 3 (83
  records, `.peykar/iop-tranche3-sealed-2026-10-28`); custody checks
  `.peykar/verify_retina_tranche2_custody.py` and `verify_iop_tranche3_custody.py`.
- Generic model `eye_recovery_population2` (PINNED by both protocols; defaults = model
  1 bit for bit): wear-off `L(t)(1 - rho W(t))` from series metadata
  `effect_duration_months`, treatment fallback, early noise (`early_noise_in_likelihood`),
  `noise_pooling` median/pooled, `forecast_noise_pooling`, `study_effects`
  shared/per_group. Harness `eye_recovery_evaluation2` (pinned).
- Retina 1.1.0 = s5 (`retina_thickness_population_1_1`, dose rule: one dexamethasone
  implant D 3.5 mo, w 0.5, rho 0.8; forecast-only pooled noise). Protocol `fd14a1c5`,
  freeze `84ee1e9c`. Sealed once (2,382 s): H1–H4 pass (coverage 0.844/0.896, next obs
  7.91 vs persistence 8.81, landmark 9.90 vs 11.55, initial 13.00 vs 18.51); H5 FAILS
  (1.0.0 same training 7.79/9.73/13.25). 1.0.0 coverage there 0.660/0.772. One
  vitrectomy+implant group (ret-pmc6899066) did not rebound. Tranche 2 consumed;
  tranche 3 reserved and extracted for 1.2 (below). Next: per-series duration learned
  from own visits.
- IOP 1.2.0 = c1 (`iop_population_1_2`: 1.1.0 law in generic 2 with per_group study
  effects, forecast-only early noise k 8, T 1 month). Protocol `4169750d`, freeze
  `df8f389a`. Sealed once (7,260 s, `.peykar/iop12-sealed-test-2026-10-29`): ALL FIVE
  pass on 130 group means (coverage 0.835/0.931, next obs 5.89 vs persistence 7.21,
  landmark 7.76 vs 11.89, initial 10.17 vs 18.06, first-month coverage 0.824/0.939 vs
  1.1.0 0.679/0.817 at 5.885 vs 5.877). Ahmed month-1 rise missed; coverage uneven by
  procedure (tube/cyclo/non-penetrating 0.71–0.74). The frozen 1.2 publisher keeps
  `reference_value` of redacted studies: ALWAYS run
  `experiments/eye/iop/model-1.2/redact-reference-values.py <folder>` and
  `.peykar/check_iop_redaction_1_2.py` after publishing with it.
- Next sealed sets, reserved before triage, triaged blind and extracted blind (records
  private, read NO value before the next protocol is frozen): retina tranche 3 (88
  admitted, 67 records, 134 series, `.peykar/retina-tranche3-sealed-2026-10-29`) for
  retina 1.2 trained on tranches 1–2 (128 studies); IOP tranche 4 (94 admitted, 79
  records, 222 series, `.peykar/iop-tranche4-sealed-2026-10-29`) for IOP 1.3 trained
  on tranches 1–3 (229 studies). Custody checks `.peykar/verify_retina_tranche3_custody.py`
  and `.peykar/verify_iop_tranche4_custody.py`. Retina tranche 4 (35) and IOP tranche 5
  (19) stay unread. Retina has no owner override rule; IOP applies the tranche-1
  percent rule to reviewer excludes whose reason is a percent reduction from baseline.
- `peykar eye` CLI (`peykar/eye_simulation.py`, `peykar/eye_cli.py`,
  `model_learning/eye_forecast.py`, guide `docs/eye-simulation.md`): retina-thickness
  1.1.0, iop 1.2.0 (stored JSON fits), corneal-endothelium and corneal-thickness 1.0.0
  (fitted at forecast time). When a newer version passes its sealed test, add it to
  `COMPONENTS` with its sealed result.
- IOP operator `--shard i/n` + `--merge` for development LOSO. Always run LOSO with
  `OMP_NUM_THREADS=1` (BLAS threads oversubscribe 16 cores otherwise).
- The image viewer does not see files created after the session snapshot, and long
  `sleep` calls may be cut short: poll in a loop that checks `date +%s`.

## Retinal-thickness model 1.0.0 and its sealed test, 2026-10-28

- Fifth eye component, chosen by data availability. Dataset
  experiments/data/retina-cst-2026-10-28: 3,128 screened OA papers; 395 passed the
  masked-table rule; salted tranches (`peykar-retina-tranche-v1:`, size 120); tranche
  1 triaged blind (78 admitted); **tranches 2–4 (275 articles) stay unread** for the
  next version. Split salt `peykar-retina-holdout-v1:`: 54 development, 24 sealed.
  46 development records (26 CC BY public, 20 private in
  `.peykar/retina-development-private`) and 19 sealed records (private,
  `.peykar/retina-sealed-2026-10-28`, custody reports C1–C3). Schema
  `eye.retina-thickness-record-1.0.0`; `thickness_number` skips a leading Δ sign.
- New GENERIC model `peykar.model_learning.eye_recovery_population` (IOP 1.1 law
  and hierarchy with a `RecoveryConfig`; equals `iop_population_1_1` to 1e-9 on
  synthetic data) and harness `peykar.validation.eye_recovery_evaluation`. Reuse
  them for later components instead of copying. Options: grouping
  `("treatment", "context")`, Ornstein–Uhlenbeck deviations
  (`deviation_fraction`, `deviation_months`, `deviation_in_population_fit`).
- Retina configurations: `retina_thickness_population.DEVELOPMENT_CANDIDATES`
  (never change a declared candidate). Adopted r3: 14-day transient, covariate
  ln(CST0/300 µm), deviations 0.8 / 12 months in forecasting only. Rejected by the
  declared rule: r1 (treatment grouping, initial state +0.07 pp) and r2d
  (fit-coupled deviations, +0.36 pp).
- Protocol experiments/eye/retina/model-1.0/protocol-1.0.0.json, SHA 64b0ad17…,
  freeze commit `d40b6ed2`. Sealed run once (146.6 s), r0 scored as reference. All
  five hypotheses passed on 40 group means: next observation 5.75 vs persistence
  6.34 and r0 6.14; coverage 0.816/0.918; landmark 7.50 vs 9.50; initial state
  11.35 vs 17.09. Anti-VEGF 1.1–3.7 pp; gain only before month 3; single
  dexamethasone implant wear-off rebound missed (15–30 pp); 5/40 meet every
  per-series criterion. Pre-freeze make check-docker: 870 tests, 89% coverage, mypy
  134 files.
- Tool quirk: an identical execute_bash command string can return a stale earlier
  result; vary the command text when re-running a check.

## IOP model 1.1 and its tranche-2 sealed test, 2026-10-28

- Tranche 2 (ranks 120–239) was reserved for the 1.1 test BEFORE triage
  (`ac2fad66`; tranche-2/README.md). `screening/triage_chunks.py` rebuilds the
  tranche-1 chunks byte for byte. Blind triage admitted 87 articles; the reservation
  lists the 72 tranche-1 record studies as training and the 87 as sealed. Seven
  custodians built 74 records with 192 series (`.peykar/iop-tranche2-sealed-2026-10-28`,
  private). Sub-agent runs can hit usage limits; a resumed custodian must verify its
  earlier outputs by rebuild hash.
- Model `iop_population_1_1` 1.1.0 adds an early transient s·exp(−(t − 1 d)/7 d) and
  group-mean noise scaled by (n/40)^−g. Harness `eye_iop_evaluation_1_1` imports the
  model-independent parts of the frozen 1.0.0 harness. The operator
  `scripts/evaluate_iop_population_1_1.py` has `--model 1.0.0` for the reference and
  scores 1.0.0 in the same sealed run. The publisher
  `scripts/publish_iop_population_1_1.py` loads the frozen 1.0.0 publisher as a
  library. The primary is the Gaussian variant, selected on group means.
- Protocol experiments/eye/iop/model-1.1/protocol-1.1.0.json, SHA 33dff220…, frozen at
  commit `ac8abf83`. The sealed set was run once (996 s). All five hypotheses passed
  on 113 human group means:
  - next observation 6.69 vs persistence 8.06 pp;
  - coverage 0.802/0.896;
  - landmark 8.95 vs 15.10;
  - initial state 9.71 vs 18.06;
  - H5 vs 1.0.0 with the same training: 6.69 vs 6.92 and 8.95 vs 9.40.
- The gain is only in the first month. First-month coverage was 0.585/0.734 and
  landmark 80% coverage 0.671; 58/113 within 5/10 pp. Tranches 3–5 (259 unread
  articles) are the next fresh test data. Pre-freeze make check-docker: 858 tests,
  89% coverage, mypy 130 files.

## IOP after glaucoma procedures model 1.0.0 and its sealed test, 2026-10-28

- Fourth eye component, first outside the cornea. Dataset
  experiments/data/iop-glaucoma-surgery-2026-10-28: 2,572 screened OA papers;
  499 passed the masked-table rule; salted tranches (`peykar-iop-tranche-v1:`,
  size 120). Only tranche 1 was triaged (86 admitted). **379 articles in tranches
  2–5 stay untriaged and unread**: they are the fresh test data for IOP 1.1. Split
  salt `peykar-iop-glaucoma-holdout-v1:` gave 54 development and 32 sealed studies.
  There are 47 development records (23 CC BY public, 24 private in
  .peykar/iop-development-private) and 25 sealed records by three custodians
  (.peykar/iop-sealed-2026-10-28). Commits `96d7b828`, `f5c5b535`, `c125cf64`.
- Model `peykar.model_learning.iop_population` 1.0.0:
  - law: ln f = ell + e·exp(−(t − t1)/tau), t1 = 1 day;
  - ell = context level + b·ln(IOP0/20), with b shared and estimated inside REML
    (about −0.74);
  - one hierarchy per species (`species/all`), no mechanism families;
  - point forecast = predictive median (round 2), primary
    population_assimilation_robust.
- Code: harness validation/eye_iop_evaluation, operator
  scripts/evaluate_iop_population.py, publisher scripts/publish_iop_population.py
  (strict licence classifier: CC BY/CC0 statement at the start, open URL, no NC/ND/SA,
  "not CC" or "unclear"), tests tests/unit/test_iop_population.py, log
  experiments/eye/iop/development-log.md.
- Protocol experiments/eye/iop/protocol-1.0.0.json, SHA f13238a2…, frozen at commit
  `ff470345` (pushed). Tested hypotheses cover group means only. The sealed set was
  run once (88.8 s, report 9c62c0f4…). Results, all four passing:
  - next observation 7.74 vs persistence 8.64 pp (also below pooled ratio 8.98 and
    analog 9.77);
  - coverage 0.763/0.894, inside both windows;
  - landmark 9.92 vs 12.89;
  - initial state 10.96 vs 17.99.
- Limits: only 21/43 group means within 5/10 pp; landmark 80% coverage 0.652.
  Failures: week-1 pressure rises after GATT and canaloplasty (the law is monotone
  from day 1) and waning micropulse-laser effect. Both IOP sets are now consumed.
  Pre-freeze make check-docker: 846 tests, 99.67 s, 88% coverage, mypy 128 files.
  Heartbeat stays PAUSED.

## Corneal thickness (CCT) model 1.0.0 and its sealed test, 2026-10-28

- Third corneal component: CCT after surgery (hydration). Dataset
  experiments/data/cornea-thickness-2026-10-28. It was built from 1,179 screened
  OA papers with masked triage (56 admitted) and a salted split (37/19) at
  `b34838cf`. There are 33 development records: CC BY public in
  development/records, NC/ND/SA private in .peykar/thickness-development-private.
  Two outcome-selected DMEK analysis sets were excluded by the rule the custodians
  also applied. Two custodians produced 15 sealed records
  (.peykar/thickness-sealed-2026-10-28). Extraction commit `dc59a42c`.
- Generic `scripts/link_study_authors.py` wrote author-overlap-1.0.0.json. Never
  edit the pinned `link_endothelium_authors.py`.
- Model `peykar.model_learning.thickness_population` 1.0.0:
  - law: ln f = ell + e·r(t), r = (1+t1/tau)/(1+t/tau), t1 = 1 day;
  - host family is relative to the preoperative CCT;
  - graft family pools the long-term level as absolute thickness,
    ell = L − ln(CCT0/550);
  - posteriors are exact Gaussian mixtures on an 80-point tau grid;
  - REML runs on likelihood-information summaries (round 2);
  - primary forecaster is population_assimilation_robust.
- Code and records:
  - harness: validation/cornea_thickness_evaluation;
  - operator: scripts/evaluate_thickness_population.py;
  - publisher: scripts/publish_thickness_population.py;
  - log: experiments/vcornea/thickness/development-log.md.
- Protocol experiments/vcornea/thickness/protocol-1.0.0.json, SHA a97db853…,
  frozen at commit `deef7664`. The sealed set was run once (28 s). Results:
  - next observation 1.60 vs persistence 2.06 pp;
  - coverage 0.892/0.969, inside both windows near the upper edge;
  - landmark 2.31 vs 4.01;
  - 28/29 series within 5/10 pp;
  - initial state 18/21.
- Failures:
  - YAG capsulotomy, extracted as `other`, got the graft-family prior (17–19 pp);
  - late DMEK thickening is not modelled. The same cohort (PMC9156484) shows late
    ECD loss, so coupling ECD and CCT is the next idea.
- Both thickness sets are now consumed. Pre-freeze make check-docker: 825 tests,
  94.87 s, 88% coverage, mypy 125 files. Heartbeat stays PAUSED.

## Corneal endothelium model 1.0.0 and its sealed test, 2026-10-28

- Second eye component: ECD after surgery. Dataset
  experiments/data/cornea-endothelium-2026-10-28 (843 screened OA papers, masked
  triage, salted split 66/21 at `b0c2b043`), 59 development records (public CC BY
  in development/records, NC/ND/SA in .peykar/endothelium-development-private),
  17 sealed records by three custodians (.peykar/endothelium-sealed-2026-10-28).
  author-overlap-1.0.0.json (>=2 shared Europe PMC authors) clusters development
  studies and removes linked studies from sealed training.
- Model `peykar.model_learning.endothelial_population` 1.0.0: f=(1-A)(1+t/tau)^-p
  on a (ln(1-A), ln tau, ln p) grid, per tissue family (host/graft), REML hierarchy,
  family fallback, recipient-referenced grafts separate, noise log-SD 0.5, primary
  forecaster population_anchored_t4. Harness validation/cornea_endothelium_evaluation;
  operator scripts/evaluate_endothelial_population.py; publisher
  scripts/publish_endothelial_population.py. Development rounds r1-r3 in .peykar.
- Protocol experiments/vcornea/endothelium/protocol-1.0.0.json SHA fdec8d89…,
  frozen commit `00d2cd09` (pushed, remote equal). Sealed run once (141.1 s):
  next observation 5.60 vs persistence 7.36 pp, coverage 0.711/0.829 inside both
  windows, landmark 6.85 vs 12.41, 23/30 within 5/10 pp, initial state 13/29.
  Worst public series: 10-year DMEK late drop (constant-offset anchor lags).
  Both endothelium sets are consumed; a new version needs newly reserved studies.
- Final make check-docker before freeze: 807 tests 97.70 s, 87%, mypy 122 files.
  Heartbeat stays PAUSED; volume peykar3_peykar-data untouched.

## Corneal model 2.0.0, dataset 1.1.0 and second sealed test, 2026-10-28

- User: "continue and do not stop", may search/acquire reliable data. Heartbeat
  stays PAUSED. Main after this milestone: freeze commit `2cee1b01` (pushed,
  remote SHA equal) plus a results commit; check `git log` before acting.
- Data: rounds 2-3 screens (2,015 + 772 new OA full texts), no supplement
  admissions; new masked article-table route (`scripts/screen_corneal_tables.py`,
  `show_masked_previews.py` hides decimals/±/%/n-N/equations). triage-1.1.0 (10
  admitted) + split-reservation-1.1.0 (same salted rule; 1 sealed) + triage-1.2.0
  (none). Cell-citing records via `peykar.validation.cornea_table_records` and
  `scripts/build_corneal_table_record.py` (`table!RrCc`, definitional baseline,
  `transform:100-minus`). Extraction protocols 1.1.0 and 1.1.1 (1.1.1 written AFTER
  PJE dev values were read). FL2 rat (CC BY-NC-ND) record private at
  `.peykar/cornea-development-1.1.0-private/`; its triage key wrongly says mouse.
- Custodian: i-PRF human CXL sealed (2 group means) + IL-10 mice re-admitted
  under 1.1.1 definitional baseline (55 per-mouse series); norepinephrine still
  excluded. Records `.peykar/cornea-sealed-test-2026-10-28/records/`; now consumed
  and published (CC BY) under dataset `consumed-sealed/`.
- Model 2.0.0 (`corneal_population2`, harness `cornea_population_evaluation2`,
  operator `evaluate_corneal_population2.py`, publisher `publish_corneal_population2.py`):
  closure tail r_c per context. Dev LOSO 16 studies: cohort 9.23 pp, coverage
  82.9/90.8%. Protocol-2.0.0 SHA 68ef4771…; sealed once: cohort coverage 70.3/82.4%
  (both windows, 80% by ONE target; 1.0.0 69.7% misses), cohort 13.1 vs last-two
  16.3 pp, 3/55 series pass 5/10, initial-state IL-10 18.0 and i-PRF 25.0 pp fail.
  Alkali burns are not fitted even in-sample. Both sealed sets are CONSUMED.
- Frozen files must not change (v1 and v2 protocols list SHA-256 pins). Editor
  re-wraps edited files; always run `ruff format` before `make check-docker`.
- `make check-docker` exit 0 on 2026-10-28: 794 tests, 87%, mypy 119 files.

## Cross-study corneal population model and sealed test, 2026-10-27 (late)

- User asked for smarter, non-brittle methods, a frank audit and continued work.
  Read docs/corneal-work-audit-2026-10-27.fa.md and the new continuation-state.
  vCornea replays are PAUSED until original run inputs/seeds are obtained; keep it
  as an executable archived reference. Heartbeat/scheduling remains PAUSED.
- New outcome-blind data pipeline: scripts/screen_corneal_wound_literature.py
  (Europe PMC, 1,993 OA full texts), acquire_corneal_wound_supplements.py (data
  members only, masked previews hide all numbers), triage-1.0.0, hash split
  reserve_corneal_population_split.py (sha256(salt+doi) mod3==0 sealed). Dataset:
  experiments/data/cornea-wound-population-2026-10-27. Dev new: DIO mouse (CC0,
  duplicate 18h values flagged), ketamine rat (CC BY, diameter 5mm ASSUMED).
  Sealed (custodian sub-agent, private .peykar/cornea-sealed-test-2026-10-27):
  Zeb1 2023 (CC BY) and LSC 2026 (CC BY-NC-ND: NEVER publish data, metrics only);
  norepinephrine/IL-10 excluded (no times / no hour-0 rows).
- Model: peykar.model_learning.corneal_population (radius-lag grid, censored zeros
  125µm, learned study noise, penalized REML, cohort_posteriors) and corneal_analog
  (k-NN ML comparator); evaluation cornea_population_evaluation/data/sources;
  operator scripts/evaluate_corneal_population.py; xls via runtime_profiles/xls-reader
  (.peykar/xls-reader-venv, xlrd 2.0.1 hash-pinned, not in uv.lock).
- Protocol-1.0.0 SHA de3c010e... pushed in commit de9762a0 BEFORE the one-shot
  sealed run. Sealed: initial-state control means 2/2 pass (2.26/2.83, 3.60/6.34pp);
  next observation 52/64 series (last-two 42/64), cohort RMSE 6.12 vs 6.73pp,
  k-NN 5.86; 90% coverage 79.6% FAILS 80% floor. Dev LOSO mostly fails; rabbit/rat
  cross-study prediction fails. Scope: mouse 2mm central debridement controls only.
  Frozen files must not change or the sealed replay check refuses; the sealed set
  is consumed — use NEW sealed studies for any revised version.
- Next: calibration fix in a new version, rabbit lag covariates and more rabbit/
  human data, then the next eye component (stroma haze, nerves, then retina) with
  the same search→blind split→baseline protocol. RL not justified (no actions).

## Observation-update milestone and user handoff, 2026-10-27

- Latest user wants both continued real work and a complete reusable prompt for
  Kiro/another agent. Read docs/continuation-prompt.fa.md and the current Persian
  docs/continuation-state.fa.md; retain whole-platform intent and corneal priority.
  Heartbeat remains PAUSED. No live deployment or model promotion occurred.
- Base main0001dc29 was clean. New pure corneal_online.py estimates a signed
  normalized-radius slope from exactly the last two actual prefix observations;
  cornea_online_evidence.py retains16 original animal/aggregate curves separately.
  No cross-series training/future-value fitting, no default engine registration.
  Expansion1.02 and later observed reopening are preserved;2021derived0h cannot
  provide a conditioning point. First-to-current comparator starts at actual4h.
- Frozen protocol1.0.1 SHA92d7bb2f717b1122cdd23bd33a4bdc8db3e37fab5d037c793113ee1c57c2a9d5
  retains original1.0.0/import failure before any prediction; revision only allows
  Python module-directory listing. Original source/method/criteria are unchanged.
- Actual .peykar/cornea-online-forecast-retry1-20261027 completed98 children exit0
  in16.2691s with10s/300s bounds; five denied-read probes plus network/write denial
  per child. Outcomes entered each job directory only after exit0. Read-only
  analysis replays frozen arithmetic and rejects changed/failed complete-looking
  outputs. Full actual archive784members and original sources/hashes retained at
  experiments/vcornea/repair-front/online-forecast/observed-2026-10-27/.
- Actual comparison EXIT2/rejected:96numeric targets/2categorical unscored/32warmup
  rows/16series. All16 beat persistence;5pass accuracy+improvement criteria.
  RMSE1.8573–18.5612pp/max25.1600pp. No uniformly better result than other simple
  baselines; no pooling or outcome-driven window/cap change. Same three consumed
  studies, so retrospective prequential replay is NOT new independent validation.
  Single-target Ocu/Amo metrics cannot grant a broad group-validity claim.
- Next dependency is independent reserved matched longitudinal data with stable
  identity, calibrated wound edge/shape/time and measurement quality; freeze QC
  before extracting partial2025images. Four2019curves err most at36→48h,Blade2021
  at24→50h; these are trajectory findings, not evidence of a specific mechanism.
  Do not select acceleration/ML/RL on these errors and relabel tests untouched.
- Targeted47tests passed1.58s; Ruff and strictmypy two new modules passed. Final
  makecheck-docker exit0:766Python tests47.69s/87%, mypy110,18frontend tests,
  Ruff/format/contracts/allbuilds passed; log cornea-online-final-check-docker-20261027.log.
  Hostmakebuild exit0; two modules/two domainREADMEs match actual platformwheel.
  All130old/31new retained inventory entries hash-match. APIhealth200/version0.3.0;
  original peykar3_peykar-data mount verified, no live service/data reset.
  Preserve old scientific/module/protocol pins; use new versions for later work.

## Latest user priority and actual corneal work, 2026-10-27

- User permits moving to another eye component only if current corneal scope is
  acceptably useful; otherwise continue sources/data/methods, ML/RL if warranted.
  Current cross-study failures remain material, so cornea continues. Do not
  relabel a narrow agreement as universal biology; heartbeat remains PAUSED.
- Main4904d9bd was clean at continuation. Read the new conditioned-forecast
  experiment and cornea-evidence-20261027 README. Original Suárez-Barrio2019
  XML/PDF match primaryNCBI MD5; CC BY4 terms preserved. Reader retains28Table1
  mean/SD/mm² entries and separates only Control0/24h from future36/48/60/72/84h.
  PBS also receivesDex/chloramphenicol/diclofenac.33eyes/17rabbits, perarmn/IDs
  unknown; daily-method versus halfday-table conflict retained, no inventedn.
- Single no-lag conditioned radius method frozen BEFORE actual candidate/future
  evaluation: protocolSHA2c8d7f806b01a440c5af4831d7b67d7386301436b5e89e4c73cc37c2124c72cd.
  Input72.32mm²at0h/47.02mm²at24h implies effective38.7176µm/h, NOT instantaneous
  measured cell velocity. Actual .peykar/cornea-conditioned-forecast-20261027
  child0/0.005393s, parent0.504081s, deniedfutureXML/PDF/JSON reads, full241samples
  from elapsed0..60h, explicittime_since_injury24..84h. Future-onlycomparison
  FAIL14.9882ppRMSE/19.6560ppmax vs fixed5/10pp. All oldfailedmodels/thresholds
  preserved; optional engine1.0.0/model1.0.0 unvalidated and defaultcatalogabsent.
- Prefix-only structural diagnostic (explicitly RETROSPECTIVE afterfailure) under
  engineeringlag0..24h/rate0..200µm/h yields compatible38.7176..200µm/h and
  lag0..19.3539h. OptimisticminimaxRMSE14.5104pp/max20.4837pp: abstainfromclaiming
  targetprecision. No futureoutcomeread/fit/statisticalCI/biologicalguarantee.
  Retainedprotocol/actualreport under conditioned-forecast; never treat midpoint
  envelope as validated forwardtrajectory. Need additional motion/independent
  kinetics plus separately tested futuredata, not refit consumed2019holdout.
- New2025AungSI originalCC BY-NC4 source retained unsegmented:4unlabeledrows vs
  PBSn6EYES, unknownIDs/calibration/pairedanimals, ~242x182JPEGs. No completecohort
  accuracy claim.2021Ankara12eyes/6rabbits saline0/72/96h is a separateunfitlead,
  reportedSEnotSD/depthunknown.2019OCTDOI10.1111/aos.14053 reuses Fischak2017
  rabbitcohort; do not doublecount independent evidence. Rawcomplete data needed.
- No liveAPI/Worker redeploy, data-volume reset, upstream/license modification,
  remote model-codeexecution, authorscontact or automationresume.
- Targeted87tests0.19s. Finalmakecheck-docker719tests41.03s87%/mypy108/
 18frontend/contracts/allbuilds and hostmakebuild pass; three new modules match
  exactwheelbytes. APIhealth200/version0.3.0. Logs cornea-conditioned-final-*;
  no repeatfullsuite without newcode/failure. Pending work is scientific evidence,
  not another unchanged simulation or threshold adjustment.

## Latest corneal learned-candidate evidence, 2026-10-24

- The user explicitly authorized new learned models and multiple agents. A small
  four-parameter corneal learner now actually executes offline; no unrelated model
  work, automated scheduling or default API registration is authorized by this.
- Read experiments/vcornea/repair-front/learned-front/README.md and results.fa.md.
  Protocol1.0.0 SHA df8a95a48f8a43947720e0c7267f6b49a3ff7fc8d46fc0e58bb60a2d4ac7339b
  froze exact algorithm/source/input hashes and criteria BEFORE first real fit.
  Train6actual Control/2%SDP rabbits,60rows/54nonbaseline values; exclude entire
  0.5%SDP group3animals/30rows. Previously inspected data: retrospective, not blind.
- Actual .peykar/cornea-learned-front-20261024 child exit0/0.2256s,41640evaluations,
  full54/50/72h outputs. Seatbelt network/write/heldout-read denied; actual forbidden
  read probes recorded. HeldoutRMSE5.9583pp/max12.2600pp vs baseline10.2604/17.0329:
  improved but FAILS frozen5/10pp targets. Independent2021 all3groups fail28.28–29.46pp.
  New independent Fischak2017 CC BY originalPDF/JATS/figure retained; only conservative
  one-sided graph constraints recovered, no animal IDs/n/means invented. Optimistic
  RMSE lower bound5.5674pp rejects candidate; this is NOT actual measured-studyRMSE.
- Immutable learned-front model1.0.0 descriptors and exact calibration/outputs are
  retained; no biological promotion. Do not refit consumed tests and call them new
  holdouts. Source/engine/protocol hashes must remain frozen; version changes.
- Shared-front inverse audit at exact18/24h rejects common deterministic displacement
  within10pp; optimistic maximum-error bound21.2858pp under nominal circulargeometry.
  This is not a universal stochastic/hierarchical impossibility result or causal finding.
- Next corneal dependency: matched raw longitudinal geometry/edge/substrate/injury/care
  data plus another entire study reserved BEFORE model selection. Dale/EGF primary
  sources screened, unresolved printed time-scale/coefficient differences documented;
  do not silently transplant them as a known biological correction. Retinal reuse
  remains backlog. Existing vCornea publication and biological gates remain open.
- Live API/Worker and peykar3_peykar-data were not changed. Heartbeat remains PAUSED.
- Final targeted52tests0.55s; make check-docker632tests35.41s87%/mypy105/18frontend/
  contracts/allbuilds and hostmakebuild pass. New four module bytes match builtwheels.
  Logs cornea-learning-final-*. Do not repeat fullsuite without new code/failure.

No `.kiro` directory existed in the delivered checkout. Added on 2026-10-16 to
retain the requested project context. Root `AGENTS.md`, `README.md`,
`docs/status.md` and `docs/roadmap.md` remain the authoritative rules/inventory.

- The user's objective is the whole platform, not an eye-model-only product.
  Roadmap S0–S13 now covers all shared domains and specialist/biological gates;
  Latest user priority (2026-10-18) supersedes the earlier parallel-work sequence:
  finish full corneal publication reproduction and analysis before another model.
- All named specialist source trees are now permanent ordinary files under `vendor/`,
  not only ignored acquisition caches: 29 source identities, 53,387 files,
  1,824,257,046 bytes, 37 fully materialized recursive Git dependencies, no LFS pointers.
  The separately pinned VCell solvers, MaBoSS and OSP R packages are included.
  Historical source receipts 1.0.0 remain; 12 complete inventories use version 2.0.0.
  Source availability, dependency completion, runtime, adapter and validity are
  separate states. Preserve pristine upstream trees and version patch overlays.
- Shared backends now include `model_import/libraries.py`,
  `biology/persistent_graph.py` and `composition/experiments.py` with API/CLI access.
  The workflow mode is sequential scalar terminal-to-initial transfer, not continuous
  feedback coupling. Child enqueue and association share one SQLite transaction.
  Additive `002_experiments.sql` preserves schema version 1 and existing rows.
- Sourced human EGF/EGFR graph is evidence-linked metadata, not a kinetic/tissue bridge.
  The six-workspace UI remains open on S12; build complete workflows on actual APIs.
- Pre-system upgrade backup: `backups/pre-system-20261016T211636Z/`, copied locally.
  Full final Linux regression: 259 tests, 88% observed coverage; live two-stage
  experiment and cancellation pass. All old rows/license/source bytes are retained.

- Git metadata now exists in this checkout. Preserve all existing local/index work,
  license notices and immutable schema/model versions; never reset to a presumed baseline.
- The user authorized committing and pushing `main` after meaningful completed,
  verified milestones, with useful history; do not create a commit for every small edit.
  Serialize Git index mutations with app Git operations. Never delete an active
  index lock; first establish that its owning Git process has ended.
- Local Docker Compose API and worker share the `peykar3_peykar-data` volume.
  Never reset it to test or upgrade; check active jobs before recreating workers.
- Back up active SQLite with its backup API, and preserve referenced artifacts.
  Continuation backups live under the volume's `backups/` and local ignored
  `.peykar/backups/`; do not put credentials or scientific data in documentation.
  E1's pre-deployment backup is `backups/pre-vcornea-20261016T205315Z/` with a local
  copy; final verification retained all rows from that and the original backup.
  API/worker were rebuilt against the same volume and the live submission/result
  smoke test passed. Native output imports are additive to existing user records.
- Use the frozen uv lock. Linux child integration requires Docker on this macOS
  host. `make check-docker` runs the complete checks/build without live data.
- The user explicitly authorized reviewed external model/source/dataset acquisition
  and execution. EPIC E1 admits exact Zenodo record 16884414 under its CC-BY-4.0
  declaration, extracts only 27 pinned files (no .git), builds a separate locked
  CompuCell3D 4.7.0 Linux/AMD64 image and runs private seeded/horizon-controlled copies.
  It never fetches/executes code through the management API or general worker queue.
- E1 has real native CSV/Parquet imports, six published reference files, a frozen
  seed-7/11 repeat comparison and a successful 600-MCS injury-crossing run. Raw Time
  is already hours; height stays lattice_y_coordinate. Archived seeds/per-run
  parameters are unknown. P1 full published reproduction and independent biological
  validation remain open. Read docs/epics/E1-vcornea-native-runtime.md before extension.
- The 6.75 GB Data.zip was acquired only as selected complete ZIP members with
  size/CRC/local SHA checks. Whole-archive MD5 is unverified. The separate partial
  bulk download is preserved but stopped; do not restart automatically or call it complete.
- Rho2015 scratch-control experimental aggregate data is attributed and ingested in
  isolated .peykar/vcornea-epic state. Its monolayer/GM-CSF assay is not vCornea tissue
  validation. COPASI/BioModels candidates remain deferred; EGF already exists in vCornea.
- The complete original Persian v3.0 blueprint is preserved verbatim under
  `docs/blueprints/`; `docs/blueprint-v3-coverage.md` maps all 55 sections and the
  original 24 biological scenarios. Preserve its ten intervention categories and
  entity/model/instance distinctions in future design work. These target scenarios
  are separate from the executable software failure matrix in `docs/scenarios.md`.
- Persistent knowledge gaps are immutable manual declarations, not approvals,
  automatic resolution or scheduler enforcement. Scientific claims, conflicts and
  independent L0-L3 authority remain separate design requirements.

Update this context when these operational facts change; record only observed
commands/outcomes in `docs/verification.md`.

## Native ecosystem continuation, 2026-10-17 local

- Reviewed sources and native dependencies were explicitly authorized for installation
  and execution. Native CPU/headless profiles now actually run COPASI, NEURON,
  PhysiCell/PhysiBoSS, Chaste, VCell, GEARS and OSP PK-Sim/MoBi, alongside CC3D/vCornea.
  Read `runtime_profiles/README.md`; do not mistake the installed PKML computation
  profile for installed Windows graphical authoring applications.
- Compatibility overlays preserve pristine scientific source. COPASI SWIG flags,
  portable ARM64 C++ compiler flags and Chaste non-root directory traversal are fixed.
  Exact overlay/source/image hashes and actual outputs are retained.
- Real Chaste Delta/Notch vertex/node monolayers pass. Original timestep warning
  remains recorded; independent validation/refinement is still open. PhysiBoSS's
  tested six-cell-line Boolean fixture is an engineering example, not corneal biology.
- Actual Dixit/scFoundation H5AD is checksum-verified and read: 37,160 K562 cells,
  19,264 genes, 21 conditions. Only bounded matrix QC ran; no GEARS original split,
  trained predictor score, donor independence or cross-cell-type validity is claimed.
- `NativeRuntimeService` admits offline operator evidence with source/image/artifact
  checks; `/v1/runtimes` and `peykar runtimes` expose 11 immutable historical probe
  records. API/Worker have no Docker socket. The subsequent native-agent milestone
  below implements S4.6 for two explicit driver kinds.
- Final full regression passed: 280 tests, 88% coverage, strict mypy 65 files and all
  package builds. Deployed API/Worker share the original volume; real submit/result
  smoke passed. All 34 old objects, 10 old runs, all related events/workflows and 46
  old artifact byte sequences survived. All 24 original project license hashes match.
- Pre-runtime backup `backups/pre-runtimes-20261016T220136Z/` is also copied locally.
  Keep older backups, model/schema versions, raw datasets and partial downloads.

## Declarative native model/agent milestone, 2026-10-17

- `NativeModelService` preserves exact bounded SBML core L2/L3 and PKML XML,
  context/license and original units. DTD/entities/includes are rejected; model
  admission never executes code. Original Aciclovir PKML is 7,428,313 bytes and
  uses offline admission under the 10 MiB limit; keep the existing 2 MB HTTP cap.
- `NativeExecutionProfile` separately pins a historical runtime and reviewed
  driver. Do not rewrite the immutable probe's `managed_queue_supported=false`;
  new `/v1/native-profiles` describes queue capability. The host's local catalog
  must independently match image/driver hashes; no automatic image pulls.
- S4.6 now has a dedicated additive SQLite queue/events migration, physical controls,
  idempotency, 30-second fenced leases, cancellation and checked numerical outputs.
  API/ordinary Worker never run Docker. The explicitly started host agent supports
  COPASI SBML and OSPSuite PKML; other native adapters, CellML, native DAG nodes and
  feedback/checkpoints remain open. Read docs/native-agent.md before extension.
- OSPSuite retains the original `IndividualId` identity column, with no invented
  physical unit. This driver admits one individual; quantity metadata must match
  the remaining columns. A rejected first live attempt remains as a failed record.
- Final Linux regression after that actual-output fix: 312 tests passed, strict
  mypy 71 source files, current schemas and all package wheel/sdist builds. The
  operator replay script is scripts/verify_native_agent.py; it retains every run.
- Pre-upgrade backup `backups/pre-native-agent-20261017T082908Z/` is copied locally;
  it contains 57 objects, 11 ordinary runs, 33 events, two workflows/four nodes/eight
  workflow events and 342 artifact byte sequences. Preserve it and older backups.
- Final deployed checks retained all those rows/artifact bytes and 24 original
  license hashes. API/ordinary Worker are healthy; native COPASI (11 samples) and
  original PKML (491 samples) passed actual submit/result verification. Real
  running cancellation/timeout removed their containers and published no results.
  The separately started host `native agent --loop` consumed another actual job;
  it is not an installed OS service and needs supervision across host restarts.
  See docs/native-agent-verification-2026-10-17.json for exact retained run identities.
- Git milestone d5b7f25a successfully pushed complete upstream source distribution
  to main. Continue committing/pushing only completed, verified meaningful steps;
  serialize index mutations with the app and never remove a live index lock.

## E2 reproducible cohorts, 2026-10-17

- Previous native-agent milestone 9781dde0 was committed/pushed to main and verified
  against the remote SHA. New work implements S4.5 in composition/batches.py and
  batch_results.py, with separate API/CLI routes and additive 004_batches.sql.
- Freeze exact plans/models/dependencies/runtime identities, Cartesian reference
  parameter grids and seed=base_seed+expanded ordinal. Enforce job/sample/wall/
  snapshot-size budgets before admission and per-cohort queued/running capacity
  during atomic dispatch into either reference or native queue.
- Native drivers still lack parameter override/stochastic seed support; reject
  sweeps/replicas for native cases rather than inventing capabilities. PKML's
  original schedule uses a conservative 100001 returned-sample bound for admission.
- Terminal reports keep each case/grid separate, disclose failures/cancellations/
  skips and preserve child hashes. Sample SD uses n-1, absent for n<2; this is model
  sample variability, not a confidence interval or biological uncertainty claim.
- Ordinary Worker reconciles cohorts every tick, including native-only cohorts.
  Native host agent remains separate. No Docker socket is added to API/Worker.
- Targeted checks passed 70 tests. Final make check-docker passed 338 tests in
  20.95 seconds, 88% coverage, strict mypy 75 source files, schemas and all builds;
  host make build passed. Existing versioned schema JSON and uv.lock are unchanged.
- Pre-upgrade backup backups/pre-cohorts-20261017T091920Z/ is copied locally. It
  retains 61 objects, 12 runs/36 events, two experiments/four nodes/eight events,
  eight native runs/24 events and 349 artifact byte sequences. Never reset volume.
- Final actual replay passed 12 seeded grid jobs and a mixed reference/COPASI/OSP
  cohort. Running cohort cancellation skipped unstarted children; an actual tiny
  child deadline demonstrated partial failure with continuation and explicit gaps.
- Cancellation exposed an original Docker run/create cleanup race (a Created
  container survived). The agent now acknowledges create before start, checks
  cancellation around creation and removes after stopping its attach client.
  Uncertain daemon cleanup stays explicit. Loop mode continues to later jobs after
  redacted errors/cancellation, without retrying failed scientific work.
- Final regression after this fix: 340 tests in 22.04 seconds, 88% coverage, strict
  mypy 75 files, schemas and builds pass. All pre-cohort rows, 349 artifact byte sequences
  and 24 original license hashes are preserved. See docs/cohort-verification-2026-10-17.json.

## E3 native/reference scalar flows, 2026-10-17

- Previous E2 milestone 299a49a5 was pushed to main with a clean working tree.
  E3 adds NativeInterfaceSpec/HybridFlowSpec and composition/hybrid_flows.py,
  model_import/native_interfaces.py, API/CLI and additive 005_hybrid_flows.sql.
- Scalar DAG bindings require exact context, semantic/scale/frame/unit agreement,
  explicit destination write ownership and one writer per target. Ordinary Worker
  schedules both queues; native host agent remains separate with unchanged image/
  driver pins. Native interface registration starts no solver.
- Native inputs currently support only explicit SBML L3 dynamic molar initial
  concentrations in fixed 3D litre compartments. Unknown/amount-only/assigned/event
  overridden inputs are rejected; PKML exposes output paths but no input rewriting.
- Derived models are new immutable versions, not edits to the original. Only the
  numeric initialConcentration byte spans change, preserving all other bytes,
  notices, units and namespaces. Receipts pin parent/interface and upstream child
  result/manifest/quantity identities. Derivation/model registration, child enqueue
  and association share one SQLite transaction. CAS blobs may remain after rollback.
- Shared simulation/terminal_values.py verifies child/result identities for E2/E3.
  Database.register_in_transaction preserves the existing immutable-object checks
  inside atomic dispatch. Existing model/schema contracts and uv.lock are unchanged.
- Targeted native/cohort/workflow/E3 checks passed 95 tests, then 26 E3 tests passed.
  Final make check-docker: 366 tests in 24.61 seconds, 88% coverage, strict mypy 80
  files, schemas/OpenAPI and all three wheel/sdist pairs pass. Host make build passed.
- Backup backups/pre-hybrid-flows-20261017T103604Z/ is copied locally: 65 objects,
  40 ordinary runs/120 events, 2 experiments/4 nodes/8 events, 15 native runs/47 events,
  7 batches/38 members/57 events and 367 artifact byte sequences. Preserve it and
  older backups; never reset the shared volume.
- Actual deployed COPASI→reference→COPASI and COPASI→COPASI flows passed analytical
  endpoint checks (absolute errors 1.65e-8 / 4.48e-8). Original XML bytes, source
  notices/context and units survived; downstream derivatives/receipts are separately
  admitted and readable. Actual running cancellation and source timeout skipped
  dependent nodes, with no remaining native container and all queues idle.
- All pre-E3 rows and 367 earlier artifact byte sequences plus 24 original license
  hashes survived the upgrade. Exact retained identities are in
  docs/hybrid-flow-verification-2026-10-17.json. E3 is sequential initial-state
  transfer; PKML inputs, general native mappings, feedback and biological bridges
  remain open. Continue using meaningful verified main commits and normal pushes.

## E4 local six-workspace research workbench, 2026-10-17

- E3 commit 8da671e2 was pushed to main. E4 adds the actual React/TypeScript client
  in apps/dashboard, compiled with Vite and packaged in the platform wheel/sdist
  at /app/. The prior Next.js frontend was designed only; a static build retains
  same-origin FastAPI authorization without another production Node service.
- Biology, model/native/source/runtime inspection, scalar datasets/CSV lineage,
  reference/native run forms, structured mixed scalar flow editor, frozen cohorts/
  DAGs, persisted all-queue Viewer and evaluation/report/gap workflows now operate
  on actual APIs. Advanced metadata/definitions use JSON editors. No spatial views,
  independent quality/split authority, feedback or new native driver is claimed.
- New bounded workspace catalog/session and experiment/hybrid run discovery keep
  old scientific contracts and SQLite schema version 1. Original model/source/raw
  bytes remain immutable; no remote scientific code installs through the UI.
- Tokens stay only in React memory, never browser storage/cookies/URLs/bundles.
  Reload/disconnect clears credentials. Reader controls and server roles agree;
  same-origin fetch rejects redirects. Packaged assets use strict CSP/no-store/
  denied framing. Native agent and original Docker volume remain separate/preserved.
- Node/npm lock and original React/React DOM/Scheduler MIT notices are retained;
  make install/check/build and Docker/CI now build/check the client. uv.lock and
  all historical scientific JSON schemas remain unchanged. Use make check-docker
  on macOS for the Linux child-worker paths, not unrestricted host execution.
- Final checks: 376 Linux tests, 89% coverage, strict mypy 82 files, 15 frontend
  tests and all wheels/sdists; host make build passes. Real browser reference and
  COPASI runs, a designed reference→native flow, CSV immutable conflict, frozen
  evaluation, cancellation, degraded/retry and role/storage checks pass. Chromium
  desktop/tablet/mobile/landscape/Persian RTL audits have zero final violations.
- Actual UI testing found/fixed COPASI time_unit metadata handling, stale evaluation
  selectors after freeze, accidental form submission from download buttons, empty
  heading order, narrow chart legibility and landscape navigation. Native plotting
  errors retain artifact access, never replace missing values with zero.
- Pre-upgrade backup backups/pre-workbench-20261017T203213Z is copied locally.
  All 74 original objects, 41 reference/21 native/7 cohort/4 hybrid/2 experiment
  runs, related nodes/events, 380 artifact byte sequences and 24 project license
  files survive deployment. Permanent vendor rehash matches all 52,167 files and
  37 Git links. API/Worker are healthy on the original volume; real deployed
  reference/native smoke checks pass. Exact receipts are in
  docs/workbench-verification-2026-10-17.json. Continue meaningful verified main
  commits/pushes; richer scientific authoring and remaining roadmap EPICs stay open.

## E5 CellML admission and reviewed operator solver, 2026-10-17

- E4 milestone 30fc035a was pushed to main before this continuation. E5 closes
  bounded S3.4b with separate CellML contracts, model admission/result reads and
  explicit operator CLI, without expanding old NativeModelSpec/Profile literals.
- Complete libCellML v0.7.1 source commit bc71bfa5816cd3e1e1a28430366eb5688fe9e501
  is pristine under vendor/dependencies/libcellml (1,094 files, Apache-2.0 LICENSE).
  Total vendor rehash: 26 distributions/53,261 files/1,814,782,314 bytes/37 Git links.
- Structural CellML 2 inspection starts no engine. Full parser/validator/analyser/
  generated ODE execution runs in a separately locked offline non-root Docker
  profile; explicit original-time-unit controls, finite output grids and exact
  model/runtime/image/driver/variable/unit/artifact identities are checked.
- E5 operator supports RK45/Radau/BDF, self-contained ODEs only; no imports, resets,
  external variables, DAE/NLA, checkpoint, API queue or native flow support. Keep
  these limits truthful. Do not silently add formats to historical native schemas.
- Model Studio now admits/inspects CellML and plots retained actual operator results.
  Binary download preserves BOM/line endings; tokens stay in page memory. Distinct
  tab navigation labels and keyboard-focusable horizontal data regions fix actual
  desktop/mobile axe findings. Seven final browser/size/RTL audits have no violations.
- Actual four-run replay passed analytic decay, stiff Robertson Radau/BDF/conservation
  and original unit scaling; wrong units/equations and a real deadline publish no
  success and leave no solver container. This does not establish biological validity.
- Final make check-docker: 402 tests/29.00 seconds/88% coverage, strict mypy 86 files,
  18 frontend tests, current schemas/OpenAPI and all package builds. Host make build
  passed; old schemas/OpenAPI operations/root uv.lock and 24 original license hashes
  remain unchanged. See docs/cellml-verification-2026-10-17.json and the E5 guide.
- Backup backups/pre-cellml-20261017T212945Z is copied locally. All 74 earlier objects,
  42 reference/22 native/7 cohort/4 hybrid/2 experiment runs, related events/nodes and
  381 artifact byte sequences survived deployment. Original volume preserved; API
  healthy/Worker running; new CellML reads and old reference/COPASI live runs pass.
- Continue meaningful verified main commits/pushes, then dependency-ready roadmap
  work. Ontology/data admission and remaining native adapters/benchmarks stay open;
  source-only or numerically verified work must not be called biological validation.

## E6 vocabulary snapshots and annotations, 2026-10-18

- E5 commit 464e3467 was pushed to main. Bounded S2.4 now has offline OBO/HGNC/
  UniProt/Reactome admission, exact source/term/alias provenance and indexed reads,
  unreviewed graph-entity annotations and atomic competing-exact-mapping records.
- Eight pinned sources retain 67,937 terms/4,030 aliases/10,813 obsolete terms.
  CL/Uberon/GO basic releases are complete; HGNC/UniProt/Reactome are explicit entry
  subsets. Preserve exact raw bytes, original licenses and qualified obsolete flags.
  No OWL/taxon reasoning, scientific approval or executable bridge is implemented.
- Biological Explorer exposes source/term/alias/obsolete inspection, binary original
  downloads and structured annotations. Ten final browser/size/RTL audits show no
  axe violations or page overflow; reader writes disabled, credentials memory only.
- Final make check-docker: 431 tests/31.75 seconds/89% coverage, strict mypy90 files,
  18 frontend tests/current schemas/all packages; host make build passed. Historical
  contracts/OpenAPI operations/uv.lock and 24 original license hashes are unchanged.
- Backup pre-vocabularies-20261017T215613Z is retained in the original volume and
  locally. All 83 old objects, 43 reference/23 native/7 cohort/4 hybrid/2 experiment
  runs, related rows and 401 artifact byte sequences survive. Deployed API source/
  alias/obsolete/annotation reads and old reference/COPASI pipelines pass; three
  CellML models/four executions and their 12 artifacts remain verified. Queues idle.
- Read docs/epics/E6-biological-vocabularies.md before extending. Next dependency-
  ready biological milestone is S2.5 multiparty reactions, stoichiometry, contextual
  state instances and process ownership. Keep S2.6/S5 scientific/kinetic gates open.

## E7 multiparty processes and instance frames, 2026-10-18

- E6 commit 36c66d37 was pushed/verified on main. E7 adds separate process/state
  contracts and biology/processes.py + reactome_processes.py, API/CLI/workbench.
  Old graph/edge/schema semantics stay immutable. Bounded S2.5 is delivered;
  S2.6/S5/S11 scientific model correspondence/approval/bridge authority stay open.
- Nineteen exact original Reactome query JSON files (138,226 bytes, CC0 attribution)
  are committed under biological_libraries/processes/reactome-egfr. Projection:
  fifteen states/three reactions, dimer input2, ATP/ADP12, separate catalyst,
  compartments, original coordinates/six phosphorylation sites, complexes and
  seven alternative ligands. Numeric repeated references are multiplicities.
- Exact reduced rational matrices infer no kinetics or elemental balance. Each
  reaction in a frozen ownership plan needs one same-context exact model owner or
  reasoned exclusion. Plans remain unreviewed and do not authorize execution.
  Sample-instance frames retain original physical units/origin/evidence, unknown
  states stay unknown, unresolved sets cannot receive quantities. Frames are not
  shared runtime states, verified outputs, checkpoint restoration or measurement approval.
- Final make check-docker: 469 tests/35.56 seconds/89% coverage, strict mypy94 files,
  18 frontend tests/current schemas/all builds; host make build passed. Fifteen
  browser desktop/tablet/mobile/RTL audits have zero axe violations/overflow.
  Actual ownership/sample form writes and immutable conflict passed; reader writes
  disabled, tokens memory only, reload clears session. Temporary sessions closed.
- Backup pre-processes-20261018T110527Z is retained in the original volume and locally:
  101 old objects, 44 reference/24 native/7 cohort/4 hybrid/2 experiment runs, all
  related rows, 67,937 terms/4,030 alias rows and 418 artifact byte sequences survive.
  All 24 original license hashes, historical schemas/OpenAPI operations/uv.lock stay.
- Deployed reads verify all nineteen source hashes/matrix and three honest process
  exclusions. No synthetic/browser frames imported. Earlier vocabulary/CellML data
  and artifacts plus actual reference/COPASI pipelines pass; queues idle, user_version1.
  See docs/process-verification-2026-10-18.json and E7 guide before extension.

## Current user priority: complete corneal reproduction before switching, 2026-10-18

- Finish the two principal published configurations (tissue self-organization and
  injury/recovery), full original horizon and original analysis/comparison. Recover
  exact archived scenario parameters and distinguish unknown stochastic seeds from
  a reproducible selected new seed. Do not claim biological validity from software tests.
- Stay on corneal investigation/runs/analysis when evidence is missing; do not
  substitute unrelated feature milestones. Hourly continuation prompt was updated
  to enforce this priority and avoid duplicate long runs.
- Retain potential reuse of installed sources for retina/other tissues as a roadmap
  backlog with context/evidence gates, not as installed biological functionality.

## Full corneal publication work in progress, 2026-10-18

- Read experiments/vcornea/publication/README.md and
  runtime_profiles/vcornea-publication/README.md before continuing this work.
  174 scalar files/65 original replicates, two unchanged analysis files and17
  final-paper supplements were actually acquired. Original analysis ran on all65;
  its14 output identities and replicate summaries are retained as a receipt.
- Historical PRIMARY series: `.peykar/cornea-hpc46-full-20261018`, former supervisor
  PID79137, exec session56885 (gone by October23; see continuation below).
  Four jobs at a time, seeds7/11, formation43200MCS
  and all three injury severities50400MCS (pulse7200MCS), 24-hour per-job deadline.
  Inspect current PID/records/progress and plan before acting; these PIDs are
  historical locators, not permission to signal an unrelated reused PID.
  Do not launch duplicates, alter private inputs/profiles or restart healthy jobs.
- This primary series uses `hpc-2.0.0`, actual official CC3D4.6.0 revision2
  (397f8b1), separate400-package lock/profile, and native-lifetime overlay2.0.0.
  HPC formation source supplies40/8/12 versus Local/final-table25/7/20. Injury Y65
  differs from final S10 Y75. Unknown per-run parameters/seeds stay unknown.
  Do not silently merge these source contexts or fit parameters to candidates.
- Frozen comparison SHA256 remains
  1189fd62ece9d90113e6096c51fa4490e6385901b0390e0a8520cb106c40bb43.
  Preserve comparison-1.0.0.json exactly; it predates full candidate execution.
  After all8 actual jobs succeed, run compare.py, analyze_candidates.py and
  analyze_spatial.py as documented, inspect images and retain every negative result.
  Execution completion alone does not close P1 or establish biological validation.
- Earlier `.peykar/cornea-full-20261018` 4.7.0/Local diagnostic series was explicitly
  superseded after discovering the HPC preset, original engine version and a second
  invalid native lifetime. Its8 failed records, inputs/progress/logs and operator
  supersession receipt remain. Do not resume it or classify it as a full reproduction.
- Source repairs reacquire deleted FPP links and stop a membrane pixel iterator
  after deleting its cell. Upstream/vendor bytes remain unchanged. Actual4.6.0
  observer/unobserved20MCS raw outputs are byte-identical. The600MCS moderate
  injury probe passed in127.76s and removed200/990 membrane lattice sites; exact
  lattice/cell-volume consistency passed. Both are engineering checks only.
- New observations preserve actual cell centers,200x90 cell-type lattices and
  EGF/SLS fields at formation/injury/recovery times; no RNG draws/state writes.
  Fluid access to a fixed original basal surface is a descriptive2D proxy, never
  clinical wound closure or whole-eye defect area. Independent data gate stays open.
- No production service/volume upgrade was performed for this offline work.
  Preserve the original live volume and E7 backup/objects. Meaningful engineering
  changes should be tested/committed/pushed while full scientific results stay pending.

## Corneal recovery and source correspondence, 2026-10-23

- Old formation2 jobs genuinely completed with captured returncode0. Both fail the
  frozen comparison:2/4 stable count means and1/10 thickness bins pass; hourly
  coverage3/4 (seed7) and4/4 (seed11). Never present these as successful reproduction.
- Old slight/mild4 jobs have complete-looking files but no captured exit; preserve
  interrupted-output-inventory-2026-10-23.json. Moderate2 never started. No success
  was inferred. Fresh resume series `.peykar/cornea-hpc46-resumed-20261023` reuses
  only formation2, reruns missing-evidence4 and starts moderate2, maxparallel6.
  Detached supervisor PID32354; original-analysis/comparison/spatial follow-on
  PID35547, output `.peykar/cornea-hpc46-analysis-20261023`. Check actual ownership
  and current progress before acting, never signal by historical PID alone.
- New copied completion_guard.py records the actual native child's exit, manifest,
  runtime and output hashes independently of the outer supervisor. Explicit recovery
  can reconcile that proof; it cannot repair old unguarded missing exits. Actual
  guarded20MCS probe passed with byte-identical original numeric outputs.
- Original data DML directories are dated July15/16 2024. Recovered commit
  b6123adf992e5b48442c0ebedd34ba7cd3162ef2 (July14) directly from licensed Code ZIP
  embedded Git objects, without loading Git config/hooks. All50 original files/modes
  now retained as vendor/models/vcornea-history-b6123adf with independent pins.
  Reacquisition script actually verified50 identical files. Current vendor totals:
  27 distributions,53,311 files,1,817,625,830bytes,37 materialized Git links.
- Separate historical-hpc-3.0.0 formation candidate uses the complete old HPC source,
  its95 literal batch controls (40/8/10,43200MCS), original Temperature10/geometry,
  lifetime overlay2 and a declared read-only five→ten output-bin overlay. No fitting.
  Two20MCS native grid probes gave identical counts/cells/fields. Full seeds7/11
  completed in `.peykar/cornea-historical-formation46-20261023`;
  diagnostic-only follow-on in `.peykar/cornea-historical-formation-analysis-20261023`
  passed all formation checks (see completed subset below). Historical PIDs41677/43430
  are no longer active; never signal a reused PID from a document.
  A diagnostic subset can never satisfy the full four-group publication gate.
- Old injury PIFF3/35/132/39 differs from archived injury4/32/127/46; historical
  injury execution is deliberately unsupported pending actual state correspondence.
  Do not cherry-pick a later PIFF/parameters and label them recovered old controls.
- Acquired10 original VTK files/three descriptors, checked required fields and rendered actual
  cell/EGF/SLS figures. These spatial groups differ from selected scalar groups;
  exact cohort/seed correspondence is not invented. DML Version1.0 is a file schema,
  not the CompuCell3D version. Original clinical/experimental validation stays open.
- Dated portable negative results, source/field observations and figures are under
  experiments/vcornea/publication/observed-2026-10-23. Current comparison SHA256 is
  still1189fd62ece9d90113e6096c51fa4490e6385901b0390e0a8520cb106c40bb43.
- No API/Worker deployment or live data-volume mutation occurred for these offline
  experiments. Keep the E7 deployment, original licenses/backups/raw datasets intact.
- Native thread correction: BLAS/OpenMP environment1 does not override CC3D XML
  Metadata NumberOfProcessors. Both formation sources retain original4 workers;
  later-source injury retains1. No hard native CPU cap or cross-thread bitwise
  stochastic equivalence is claimed; never rewrite executed manifests to hide this.

## Derived injury candidate from original observed constraints, 2026-10-23

- source-state-correspondence-1.0.0.json SHA256
  08b485a996d7ee5a636fa950f6b64d8c1ba05f18f7d7bf3319e70573c95a2d05 retains
  original per-cell pressure/volume under known source lambdas: unique initial
  targets40/40/25/25 for stem/basal/wing/superficial. Lambda itself is assumed from
  source, not recovered. Every original VTK cell ID/type matches the separately
  archived PIFF90690242...; pixel geometry afterMCS0 differs, so no exact checkpoint.
  Original lineage fields support the older initializer, and actual MildComb0 pulse
  at3000MCS is exactly one[100,75] site/1500 units. Its group/time remains separate.
- New observed-injury-4.0.0 is explicitly derived: complete July HPC mechanisms,
  original batch defaults40/8/10/Temperature10/Y75, separately evidenced original
  PIFF, declared7200/50400MCS/dose overrides. It does not weaken version3's rejection
  of silent injury-state replacement or assert original historical seeds/environment.
- Actual600MCS moderate probe exit0/130.00s, pulse deliberately20MCS at[100,75]/2500;
  count61/hourly, height610/ten bins, SLS601/per-MCS rows pass. Membrane990→869,
  epithelial sites5013→2758; actual lattice/cell-volume checks pass. Engineering only.
- Six full derived injury jobs prepared/frozen before new results in
  .peykar/cornea-observed-injury46-20261023, detached supervisor53938. It waits24h
  for BOTH previous series to genuinely complete/verify before at most4 simultaneous
  jobs (original4 CC3D workers each),24h per-job. Do not duplicate/change these inputs.
- Full combined follow-on PID54075 at
  .peykar/cornea-reconstructed-combined-analysis-retry1-20261023 waits at most48h,
  then combines actual injury6 with historical formation2, retaining separate input
  plans/source identities and unchanged protocol. Default full gate still needs4
  groups×2 seeds. Original first combined follow-on failed before plan publication;
  retained logs/state and a targeted preparation-wait regression document the fix.
- Main60884638 already pushed the prior verified497-test source/recovery milestone.
  New source-state/orchestration final make check-docker passed510 tests/44.46s/86%,
  mypy98 files,18 frontend tests/contracts/all builds; host make build passed.
  Commit/push the verified increment; do not present pending runs as completed.

## Completed formation subset and original field integrity, 2026-10-23

- Main39bda9a7 was already pushed. The two historical formation jobs actually
  completed exit0 in5963.56/5962.51s,43200MCS each. Their bounded follow-on completed
  all three stages, diagnostic_comparison_passed: each seed passes4/4 stable count
  means,4/4 full hourly trajectories and10/10 thickness bins under the unchanged
  protocol1189fd62... . Both original-analysis figures, count comparison and both
  actual layer figures were visually inspected. Formation alone is not the full gate.
- Portable positive receipts/figures have formation-historical3-* names under
  experiments/vcornea/publication/observed-2026-10-23. Previous negatives stay intact.
  Six earlier injury jobs remain active; the prepared derived6 and combined analysis
  still use their existing prerequisite gates. Inspect live records before acting.
- Actual original CONTROL/Images finalVTK ends in Pressure:9597/18000 values, with
  three additional declared fields absent. Archive size/CRC/SHA remain correct.
  A new checked ASCII reader rejects incomplete required fields, records optional
  gaps and never fills missing values. All10 earlier cell/EGF/SLS figure hashes are
  unchanged when reading the complete original values directly. The earlier VTK
  sensing attempt failed on a missing field; its failure and later attempts remain.
- Original EGF sensing audit retains1218 positive epithelial cells across six
  snapshots. Median reported/summed field ratio is approximately1; all except one
  have relative discrepancies at most2.611e-7. Mild46200MCS stemID8002 differs
  by1.807958 (relative0.893025); retain it, do not discard/invent a confirmed cause.
  The finalCONTROL sensing field is unavailable, not zero. No candidate criteria change.
- Source-history audit retains56 original commits and the January2025 stash50-file
  formation fingerprint: only plot methods differ from July, while temperature8/
  diffusion12 and ten bins differ. Stored local notebook stdout4.5.0 has a failed
  simulation cell; setup scripts request4.6.0. Neither identifies HPC run binaries.
- The user explicitly requested that the Peykar heartbeat remain PAUSED. Do not
  reactivate/create a schedule. Existing explicit native operators/analysis continue.
- Final source-quality/formation increment verification:15 targeted tests, then
  make check-docker518 tests/37.63s/86%, mypy98 files,18 frontend tests, contracts
  and all package builds. Host make build passed. API healthy/Worker up5days;
  original locks/schemas/models/licenses/vendor bytes have no Git changes.
  Preserve the per-cell CSV's actual CRLF bytes via its scientific-output attribute.

## Complete baseline and derived execution, 2026-10-23

- Mainc11c5047 was pushed/remote-SHA verified with a clean tree after the518-test
  source-quality/formation milestone. Never recreate that completed work.
- All six resumed injury jobs genuinely completed50400MCS, exit0,11262.71–11443.62s.
  Whole source2 baseline8 comparison is not_reproduced: formation2 fail, slight2
  have9/10 height bins, mild2 have9/10 (seed11 also3/4 count means), moderate2 pass
  all frozen scalar checks. Original math/spatial stages exit0; overall completion
  at17:29UTC. Old supervisor32354 and follow-on35547 are complete, not live operators.
- Explicit read-only chemical audit verified each full50401-sample0.1h trace,
  finite/nonnegative data and actual[100,65] pulse field matching its source controls.
  Do not relabel these as[100,75] or promote passing moderate alone to a full gate.
- Derived supervisor53938 is now running first four slight/mild jobs; moderate2
  wait for its maxparallel4 capacity. Original XML requests4 CC3D workers/job.
  Combined follow-on54075 still waits for six real completions then combines them
  with the two completed historical formation records. Inspect current ownership/
  progress before acting. Full reconstructed publication gate remains pending.
- Original44 scalar SLS traces all confirm pulse720h/doses750/1500/2500 and50401
  rows. Descriptive original count-return observations retain all missing events;
  they are not clinical healing or new acceptance criteria. Same structural proxy
  on original MildComb0 finds52µm exposure at24h,0 at120h; group/time differs from
  selected MildComb1. Source/receipt files are linked from completed-baseline-audit.
- Original65 Parquets identify fastparquet2024.5.0 /pandas2.2.2, metadata keypandas
  only. Current writer is PyArrow16.1.0 /pandas2.3.3; do not mutate active environments
  or infer unknown original engine/seed/date/full dependencies from writer versions.
- Actual original-analysis figures had clipped titles/axis labels. Wrapper-only
  tight layout/bounds correction retains exact original code/math. Bounds-only
  attempts failed visually and remain; final native8/original65 replays have equal
  numerical summaries and all eight full-horizon mean CSV byte sequences.
  Current wrapper SHA d0c09adc23322eace61fe13a0212e13e7161efe729b1c050b9c2d20d4ad26a97;
  comparison script302b06bc... and protocol1189fd62... remain unchanged.
- Final make check-docker passed518 tests/47.09s/86%, mypy98 source files,18 frontend
  tests, current contracts and all package builds. All130 fresh original-analysis
  input hashes exactly match the frozen protocol. Retain completed baseline/export
  receipts and commit/push this verified meaningful increment normally.
  Host make build also passed; existing locks/schemas/model/vendor/license bytes
  remain unchanged. Do not confuse these software checks with pending scientific acceptance.
- Interim field observation `.peykar/cornea-derived-first-pulse-observation-20261023.json`
  verified the four running jobs' actual7200MCS pulses at[100,75], doses750/1500.
  It explicitly remains ineligible for final comparison until native completion;
  retain those hashes for checking against the later durable completion proofs.
  Heartbeat remains PAUSED by explicit user request; keep current native work active.

## Complete January family snapshot and reviewed capacity, 2026-10-23

- Main3a391e7e was already pushed/remote-verified after the518-test
  baseline/layout increment; preserve its complete negative baseline and old logs.
- Complete January23,2025 stashc7f95d28dfa2329abb7353caab724c52f9341dee now retained
  as50 pristine files/2783833 bytes. Treecf708f13..., independent pins/inventory;
  explicit objects-only recovery verified identical files. Vendor totals28/53361/
  1820409663bytes/37 links. Formation keeps old mechanics40/8/12/T8/4workers;
  injury keeps its DIFFERENT normalized mechanisms25/7/20/T8/1worker. Original
  injury PIFF is identical to later archive after CRLF→LF only; never substitute bytes.
- New stash-hpc-5.0.0 retains both original family projects/defaults and original
  ten bins. Injury only explicitly overrides activationTrue/pulse7200/horizon50400/
  doses750,1500,2500. New seeds7/11 and reconstructed4.6 runtime remain disclosed.
  This January source postdates selected2024 data; no exact-cohort identity claim.
- Actual formation20 observer/unobserved exit0, five numeric files byte-identical;
  moderate600 early20 pulse[100,75]/2500 exit0/139.91s,61/610/601rows, exact lattice/
  count/volume consistency. Basement990→901, epithelium5015→4204; engineering only.
  Initial wrapper driver-filename failure happened before its next native runs;
  preserve the failed preparation and successful fresh probe evidence.
- Explicit native operator checks XML worker counts: up to4 multiworker jobs/16
  requested workers, or6 only when ALL request1. Per-series bound is not hardCPUcap.
- Capacity-view operator73782 watches FOUR source4 slight/mild actual completions,
  pins their job definitions and verifies outputs/exits. It then admits six source5
  injury jobs (supervisor73824) with at most6workers alongside only source4moderate2
  at most8workers. This read-only prerequisite view is not another simulation.
- Source5formation supervisor73826 waits for whole source4series before two4worker
  jobs; together with injury6 the maximum request remains14workers. All8 source5
  inputs are already frozen/prepared, no healthy native job duplicated or stopped.
- Source5combined follow-on73926 waits for BOTH complete/verified families before
  running full comparison/original math/spatial analysis. Each native job24h bounded;
  waits24/48h. Check live ownership/status, never signal a historicalPID blindly.
- Final make check-docker530 tests/40.18s/86%, mypy99,18 frontend/contracts/all builds;
  host make build passed. No live API/Worker deployment or volume mutation.
  Protocol1189fd62... and compare302b06bc... unchanged. Heartbeat remains PAUSED.

## Full-horizon observation and original inverse evidence, 2026-10-23

- Main490ec07caad51702e0551dfaf8b47e71390377c4 was pushed/remote-SHA verified,
  with79 exact-byte staged files,530 tests/40.18s/86%, host build and clean tree.
  Original API/Worker remain on the same volume, /healthz200/ok; no deployment.
- Spatial diagnostics1.1.0 now includes actual initial and final180-day post-pulse
  structure and final4320h EGF. Actual source2 baseline8 re-analysis passed every
  lattice/cell/count check; ALL earlier numeric snapshot rows are identical. Both
  moderate final structure/EGF exports visually inspected. Final widths0/4um and
  membrane842/784 remain descriptive; full source2 comparison is still negative.
- Original56 commit object inventory has368 blobs/408trees. Bounded named-parameter
  AST audit found32 unique Parameters.py blobs,16 under output paths, horizons100/
  14600MCS, no top-level seed assignments.78574 DataZIP pathnames have no parameter/
  seed/manifest/config/batch-named member.2023 valid_combinations is unrelated LHS
  exploration. No selected2024 per-run parameters identified; do not treat scope as
  arbitrary-blob content search or proof that source never seeds RNG.
- New exact inverse uses ALL418 original initial pressure/volume cells, not a known
  lambda/target prior or candidate fitting. Under stated common-per-type parameters
  and pressure timing/formula, targets40/40/25/25 and lambdas2/2/2/5 are unique in
  both original spatial groups. Every sign branch checked with exact rational
  arithmetic; ambiguity/inconsistency self-checks retained. Original spatial groups
  still differ from selected scalar cohorts. Existing08b485... state receipt unchanged.
- Native pressure getter was checked at volume0 only, matching reviewed4.6 source
 397f8b1; original nonzero-volume inverse remains conditional on the reviewed formula.
  Native cell.volume setter rejected initial probe; missing probe receipt stopped
  first inverse attempt, then corrected fresh proof succeeded. No model state changed.
- Official4.6 release chronology dates differJune8/July6 2024, both before cohort
  directory dates. Neither recovers HPC binaries; active runtime/controls unchanged.
- Source4 native four slight/mild jobs still progress; moderate2 wait for capacity.
  Source5 injury6/formation2 stay prepared behind existing explicit gates and bounded
  combined follow-on. Never duplicate or alter these running/prepared identities.
- Final make check-docker531 tests/39.40s/86%, mypy99,18 frontend/current contracts/all
  builds; host make build passed. Logs cornea-final-state-*. Heartbeat stays PAUSED.

## Four actual completions and source5 admission, 2026-10-23 19:20UTC

- Mainc42ba24035b14f90adc2e660a0514d9d97181d77 was pushed/remote-SHA verified,
  exact20-file stage and clean tree after531-test/full-horizon/original-inverse increment.
- ALL FOUR source4 slight/mild jobs genuinely completed50400MCS/exit0, elapsed
  6574.92–6592.11s. Each earlier7200MCS pulse hash exactly matches its actual
  completion record; no interim observation was promoted without the real exit.
- Source4moderate2 now run under the existing53938 supervisor. Full combined54075
  still waits for these two; no full reconstructed comparison has been declared.
- Read-only capacity-view73782 genuinely completed after all FOUR verified exits.
  Source5injury73824 is running all six one-worker jobs (guards79021/79022/79023/
  79027/79028/79029 observed19:20; always recheck ownership). First native snapshots
  show original4/32/127/46 counts; no parameter/protocol/source identity changed.
- Current requested worker maximum is8 source4moderate +6 source5injury =14.
  Source5formation73826 waits for WHOLE source4series; combined73926 still waits
  for BOTH source5families. Do not duplicate or relax gates. Heartbeat stays PAUSED.
- `.peykar/cornea-observed-first-four-completion-verification-20261023.json`
  retains real elapsed/exit/manifest/proof and pulse-hash continuity. Source4full
  gate, source5full gate and independent biology remain pending.

## Prior published source and terminal injury observations, October24 local

- Earlier official16764319 CodeZIP is fully MD5/SHA verified;26 pristine source
  files3847383bytes added as29th distribution. Total53387files1824257046bytes,
  37 links.23files same as16884414;2launchers/1newline-only Parameters differ.
  No numeric/Boolean/mechanism change; no new scientific profile/run claimed.
- All238 older original object files match current. Of832 inspected objects,
  808 reachable from56commits;22trees+2unresolved XML merge artifacts extra.
  Preserve original conflicts as data; never install/resolve them as configs.
  Six extra-tree named parameter blobs already among32. No selected run metadata.
- Final source increment check-docker531tests39.04s86%/mypy99/18frontend/builds;
  host make build passed. Source pins/licenses/uv.lock/protocol remain preserved.
- At23:04UTC source4terminal:5verifiedfull exits,moderate7actual SIGABRT/-6,
  outer134,lastobserved25000MCS. Mac allocator detects guard corruption during
  BoundaryPixelTracker allocation in parallel Potts worker; origin UNDETERMINED.
  Failed evidence and blocked derived combined analysis retained. Never infer success.
- Source5injury6 all50400MCS/exit0,11311.59–11557.02s. Originalmath/spatial0,
  frozen injury diagnostic2:moderate2+mild11passall,slight2+mild7fail. All50401
  chemical samples and actual720h[100,75]doses checked; interim hashes matchproofs.
  Portable receipts/figures observed-2026-10-24,completion-audit-2026-10-24.md.
- Old source5formation plan failed success prerequisite before ANY job started.
  Explicit reviewed private operator95506 reuses SAME never-started prepared inputs
  in new series .peykar/cornea-stash-prepared-formation46-20261024,2jobs×4workers.
  Original failed plan preserved, verified no active native solver before release.
  Do not duplicate/reconfigure these jobs. New bounded full follow-on95617 uses
  formation as waiting primary and completedinjury6 as additional; directory
  .peykar/cornea-stash-full-analysis-20261024. Recheck ownership/status now.
- Full publication gate and independent biology still open. Heartbeat stays PAUSED.

## Failed-child diagnosis and original-type legends, October24 local

- Maince4be39afcb103b78660002efd0945bbeaba4ec6 pushed/remote-SHA verified after
  90exact-byte staged files:older26-file original source +6actualJanuaryinjury
  outcomes and5success/1abort derived terminal receipt. Do not recreate that work.
- execute_native now preserves verified failed guard native_returncode/signal,
  completion hash and partial_outputs separately; failed outputs stay unadmitted.
  Actual oldabort read-only extraction verified-6/SIGABRT,outer134,25partialhashes,
  oldrecordbyteunchanged. Diagnosis improvement is NOT a native heapcorruption fix.
- New prepared driver copies enable faulthandler(all_threads); current running
  formation copies unchanged. Resume now validates actualXML retryworkerbudget,
  onlynewpreparedchildren, atmost4multiworker/16requested or6singleworkerjobs.
- Spatialdiagnostics1.2 includes ALLplotted originalXMLtype names inlegend.
  Reanalysis6Januaryinjury numericrows/6EGFPNGs EXACTLYunchanged, finalmoderate11
  updatedfigure visuallychecked. No physical/acceptancechange. Oldfiguresretained.
- Source4completedfirst4 view got three realstages:comparison2/original0/spatial0;
  oldnegativecomparisonrowsidentical. One realfailedmoderatejob remains excluded.
- Targeted29tests0.63s/Ruff/mypy2pass. Finalcheck-docker534tests38.00s86%,mypy99,
  18frontend/contracts/allbuilds;hostmakebuildpass. Logs cornea-terminal-failure-*.
  Heartbeat staysPAUSED. Formation95506/fullfollowon95617 continue; recheckactualPIDs.

## Serial derivative after actual native stress failure, October24 local

- Main383fae9499f6c195615990228969956a35270598 pushed/remote-SHA verified after
  23exact-byte staged files,534tests38.00s/86% and hostbuild. Do not recreate work.
- Bounded synthetic native4.6 test: two giant cells/200x90/seed7/10000MCS.
  1worker exit0/25.06s (last9900,totalvolume18000);4withtrackersSIGSEGV1.85s.
  4pixel-onlySIGABRT1.91s,boundary-onlySIGSEGV1.89s,neitherSIGABRT2.09s.
  Thus do NOT claimBoundary-onlycause or installspeculativemutexpatch. Original
  cornealcorruptionorigin stillunknown; reducedworkers is explicitworkaround.
  Bothinitialsetupproberrorsretained; correctedcontrols arenotbiologicalvalidation.
- New observed-injury-serial-6.0.0 changes ONLYsource4workerXML4to1, retains
  biologicalparameters/state/lifetime2/10bins/protocol; newmanifestversion6/control
  overlaylabelsunknownparallelstochasticequivalence. Existingprofiles/copiesintact.
- Explicit fullmoderate7/11 newnativeoperator98682, .peykar/cornea-serial-derived-
  moderate46-20261024.2requestedworkers,24h/job. Boundedfollowon98752 at
  .peykar/cornea-serial-derived-moderate-analysis-20261024 isdiagnosticmoderateonly.
  CurrentJanuaryformation95506/fullfollowon95617 remainactiveunchanged,8workers;
  totalnative requestatmost10. Recheckactualstate/PIDs; NEVER duplicatehealthyjobs.
- Targeted37tests1.01s/Ruff/mypy2pass. Final537-testcheck-docker and hostbuildpass;
  logs cornea-serial-execution-*. Publication familygate and independentbiologyopen.
- Exhaustive78574DataZIPmemberkinds show onlyknownoutputformats/oneplotscript;
  noextraidentifiedconfigurationfile. FullzipMD5/arbitraryembeddedcontentsunknown.
  original-run-metadata-needed.md islocalconcreteevidenceinventory; noauthorscontacted.
- Heartbeat staysPAUSED. Finish/analyze these existingnative jobs beforeanynewmodel.

## Actual full January rejection and serial formation7, 2026-10-24 00:28UTC

- Main6163143d22395520bf98f19402728cd25a4fef99 already pushed/remote verified,
  clean after28exact-byte files/537tests39.28s86%+hostbuild. No source/licensechange.
- Formation5 native2 completed43200MCS/exit0,4438.35/4461.59s. FullJan8combined
  .peykar/cornea-stash-full-analysis-20261024:comparison2/not_reproduced,original0,
  spatial1,overallanalysis_failed. F7means1/4traj3/4height2/10,F11sameheight1/10;
  oldinjury6:3pass3fail. No full reproduction. Actual raw/receipts nowretained.
- Actual finalF7tearvolume1974vs1973latticesites (MCS43200/type10); read-onlyall
  frames/types scan BOTHformationseeds foundonlythisone. Nativeexit0notphysical
  validity. NEVER repair array/relaxintegercheck. CauseUNDETERMINED; do notclaim
  scalarfailuresarecausedbyonepixel. Diagnostics1.2.1 adds exacterrorcontextonly.
- Added stash-formation-serial-7.0.0, originalJanbiology/PIFF/T8/diff12/43200,
  tenbins/lifetime2unchanged; explicitworkerXML4to1newrecipe7/overlay. Injurycase
  rejectedfor7; oldprofile5unchanged. Two fulljobs explicitlystartedsupervisor5613
  .peykar/cornea-stash-serial-formation46-20261024. New fullfollowon5614 at
  .peykar/cornea-stash-serial-full-analysis-20261024 waitsformation thenreuses
  ALREADYcomplete/verifiedone-workerJanuaryinjury6 (do NOT duplicate them).
- Existing source6moderate2 operator98682/diagnosticfollowon98752 stillrunning,
  .peykar/cornea-serial-derived-moderate46-20261024 (~15000MCSat00:26UTC).
  Jan7formationjuststarted; totalnative request4workers. Preserveallhealthyjobs.
- Interim source6fullpulse actual7200MCS[100,75]/2500 verifiedbothseeds:finite
  fields/lattice-cellvolumes/countsagree. Private cornea-serial-full-pulse-audit-
  20261024.json hashes awaitmatchingactualcompletion; stillineligibleforfinalgate.
- Resumer bug fixed: afterverifiedchildexits, newview usesrecordstatussuccess/
  elapsed, dropsstaleparentfailure; oldparentrawpreserved. Actual OLDfailedJan
  preparedplan resumed to .peykar/cornea-stash-formation-verified-reuse-20261024,
  bothactualexitsreused/ZEROnewsolverchildren,oldplanunchanged. Notphysicalapproval.
- Targeted22tests0.58s +33tests0.83s/Ruff/mypy2pass. Intermediate540tests34.30s
  beforeactualqualityfailure. FINAL541tests37.66s86%/mypy99/18frontend/contracts/
  allbuilds,hostbuildpass;logs cornea-serial-formation-*. No furtherfullsuiteunless
  managedcodechanges/newfailurejustifyit. HeartbeatremainsPAUSED.
- Read january-full-family-audit-2026-10-24.md, original-run-metadata-needed.md.
  Finish/analyze current source6/7 nativejobs beforeothermodels. Mainuser wants
  actual full reconstruction; do NOT call matchedsubsets or softwaretestscompletion.
  Old read-only watch session95283 covers oldJan5terminal +source6only, NOTnewJan7.

## All outstanding serial jobs terminal, 2026-10-24 08:00UTC

- Main4a81ef338a3ec9e0f7d59a7f0e93caabe8e48780 was pushed/remote verified;
  managed code unchanged since its541-test/37.66s86%/mypy99/18frontend/build gate.
- Source6moderate7/11 actual50400MCS/exit0,10715.82/11179.29s. Follow-on
  cornea-serial-derived-moderate-analysis-20261024 finished02:33UTC:
  comparison2/diagnostic_not_reproduced, original0/spatial0. Both4/4means,
  3/4trajectories,10/10height; Wing coverage.899226/.889308 failsfrozen.90.
- Source7formation7/11 actual43200MCS/exit0,8933.22/8964.58s. Fullfollowon
  cornea-stash-serial-full-analysis-20261024 finished02:54UTC, reusing existing
  Januaryinjury6 without anynewsolver: comparison2/not_reproduced,original0/
  spatial0. F7means1/4,traj3/4,height1/10;F11means2/4,traj3/4,height1/10.
  Reused6comparisonrows exactsameprior; fullfamilyfivefail/threepass. No fullmatch.
- Read-only retentionoperator verifiedall10native guards/source/runtime/outputs
  andbothstagefileinventories;61exactbyteanalysisfiles+receipts/operators/matrix
  retainedobserved-2026-10-24/serial-*. Originalarrays/failedattempts preserved.
- Fullnewinjurychemicalaudit:50401finite/nonnegative0.1h samples/5040h,actual
  720h[100,75]/2500pulse. Bothpriorinterimpulsehashes matchactualcompletion.
  Allnewspatialsavedframes passstrictcellvolume/lattice/count/finitefieldchecks.
- Actualfinalmoderate11Wingredistribution belowfixedinitialsurface:3150/5194
  eligiblesites over176columns;7has45/1333 over12. Initial/pulsezero both.
  Read-only descriptiveaudit/fighashesretained;NOnewacceptance/invasiondiagnosis.
  Zeroexposedcolumns doesnotmean normalrepair. Originalmath/count/EGF/structureviewed.
- No native jobs active; previousread-onlywatchers ended. API/Workerhealthy,
  /healthz200/version0.3.0,livepeykar3_peykar-datauntouched. No new simulations,
  modelwork,remote scientificexecution,scheduleror contactmessages. HeartbeatPAUSED.
- Read serial-final-results-2026-10-24.md and results-2026-10-24.fa.md. Next
  scientificdependency isoriginal-run-metadata-needed.md: exactexpandedoriginal
  controls/state/RNG/replicatemapping/nativebuild. Existing releases/history/member
  inventorywithinrecordedscopehasnotidentifiedit. Do NOTguess/fitorrelaxcriteria,
  cherry-pickmixedsourcepasses orrestartunchangedcompletedruns. Fullpublication
  andindependentbiology gatesremainOPEN; no othermodelwork untilpriorityresolved.

## Real experimental evidence after user's HF direction, 2026-10-24

- Latest user instruction explicitly advances corneal experimental-data search,
  sample inspection and useful comparison-code updates, including Hugging Face.
  It expands work beyond waiting for original paper run metadata, while preserving
  corneal priority and separate publication/biological gates. Do not ask for the
  same dataset-acquisition permission again or begin another model.
- Mainaba2afae0aaa441c7aa714f771bab83a3841c582 was pushed/verified/clean.
  New work: experiments/data/cornea-evidence-20261024 and vcornea/experimental.
 23admitted source/normalized/plan files hash-verified, original terms retained.
- HF three candidate identities pinned; actual human metadata22972rows/12prefixes/
 3columns, LFS SHA verified. No cell-type, injury-time or physical-thickness columns.
  Card~19472/viewer141k differs. Actual202eye biomechanics CSV/Git object checked;
  CCT is whole cornea, not epithelium. Undocumented37.7GBH5 not downloaded.
- Original Abdel-Naby2017 S1ZIP97548bytes/Fig2+3XLSX retained;90measured wound
  rows normalized:3groups×3actualrabbits×10times0–54h. Caption n4 discrepancy
  retained; rawareaunitunreported, no mm² conversion/fourthrabbit invented.
- Buffault2020 original publisher JATS/Normal group22subjects/7aggregates retained;
  central52.5±3.6µm. No synthetic human rows or untouched-holdout assertion.
  Dryad467979 all47file metadata acquired;6small-file data downloads failed
  API401/publicUI403. No porcine experimental data or1.27GBimages acquired.
- peykar.validation.cornea_evidence adds inert bounded source readers and
  cornea.vertical-epithelial-span-1.0.0:actualtypes1/3/4/5 span/occupancy/gaps/nulls,
  nominal2µm/site. New explicit scripts/compare_corneal_evidence.py reads verified
  completed final43200MCS arrays only; no model code or original acceptance changed.
  Pydantic dependency found absent in nativeenv; use stdlib SHA instead, NO package
  installed/updated in immutable native runtime. Earlier failed attempt retained.
- Four actual mean spans:July3seeds7/11=54.01010/55.62626µm;Jan7=61.68687/61.40404.
  Human central52.5±3.6. Exploratory plan declared before newmetric extraction but
  AFTER existing simulations; NOT preregistered validation. Region/measurement
  mismatch blocks independent pass; raw outputs/failedpublicationresults unchanged.
  Rabbit7mm mechanical fluorescein area is not genericchemical2D fluid access.
-18targetedtests0.05s/Ruff/format/mypy1pass. First fullcheck stopped on original
  HFREADME formatting; newrawdirectory excluded, upstream bytes unchanged. Final
 559tests34.35s86%/mypy100/18frontend/contracts/allbuilds,hostbuildpassed.
  Logs cornea-experimental-evidence-final-check-docker/build-20261024 under.peykar.

## Corneal projected repair: actual narrow agreement and transfer failure (2026-10-24)

- User requires actual small-error experimental comparison and model work, not
  another data-discovery-only report. Stay within cornea; recurring scheduling
  remains paused. Preserve all original cellular runs/publication criteria.
- Added optional cornea.projected_front1.0.0 with source digest
  af1f1bf02200906a211b279ff7dcd693013787d71f09d75ccdf82bfaeda3e688.
  Crosson1986 PMID3957565 point values: lag5.5h, radius motion64µm/h; fulltext403,
  full supportive-care equivalence and reported± definition unresolved. Planar
  circular projection is explicit, not a curved/cellular/chemical model. No
  default API/Worker registration or global validity promotion.
- Frozen2017 protocol865f9672… before actual child; original7mm diameter supplies
  initial radius3500µm. Actual .peykar/cornea-repair-front-cross-study-retry1-
  20261024: child0/0.335289s,54h/217samples, all3Control rabbits/all10times.
  Nonbaseline cohort mean RMSE.04021733/max.05862686 passes fixed.05/.10 targets.
  Rabbit2 RMSE.06646529/max.10314309 remains; no per-animal accuracy guarantee.
  Retrospective external-study check, no blinded/preregistered clinical claim.
- Acquired Chung2021 original XML PMC8828349,65161bytes/SHA4e4cf5f5…; Table1
 10numeric aggregates/2categorical50h statements across Blade/Ocu/Amo. Preserve
 1.02 ratio, unknown± definition/per-time n, no invented measured0 or animals.
  Original CC Attribution statement/version-unspecified terms retained.
- Separate6mm model/frozen challenge1.0.1 SHA9c70007c…; unexecuted1.0.0 retained
  after pre-execution formatting/type-only parser pin corrections. Actual
  .peykar/cornea-repair-front-independent-challenge-20261024 child0/0.324017s,
 50h/201samples; comparison2/challenge_failed, all three group RMSEs~.248–.264.
  No fitting/criteria relaxation. Substrate, edge, inflammation/care differences
  are possible gaps, not proven causes. Need new independently evidenced dynamics
  and new study-level test data before any fitted variant is called validated.
- Both bounded children Seatbelt network-denied. Failed setup before any first
  child remains. Portable exact outputs, original executed operators, manifests,
  plots and plain Persian report: experiments/vcornea/repair-front. Current
  operators add actual exit/artifact/protocol/grid checks; targeted39tests pass,
  strict mypy2files. Final make check-docker:580tests/34.05s/86%, mypy101,
 18frontend/current contracts/package builds pass. Host make build passes.
  Logs cornea-repair-front-final-check-docker/build-20261024 under.peykar.
- Old vCornea is400µm limbal/peripheral radial sagittal slice, not central7mm
  en-face defect. Do not rename whole-slice thickness as central human OCT or
  equate the new narrow kinetic result with full publication/cell-layer validation.
- Read experimental/README.md/results.fa.md and datasetREADME/catalog. Next:
  homologous anatomical ROI/OCT observation mapping, reviewed independent protocol;
  rabbit n3/n4/rawunit context and matched separatelyversioneddebridement scenario.
  Do NOTfit missing chemical kinetics or invent woundarea from2D width/countreturn.
  Existing native runs terminal, no duplicates. Heartbeat staysPAUSED, data preserved.
