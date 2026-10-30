# Peykar whole-platform delivery roadmap

Updated 2026-10-27 against the complete [55-section architecture and 24 scenarios](blueprint-v3-coverage.md).
Peykar is a general biological simulation platform. vCornea is one admitted test
case; completion of its operator workflow is not completion of this product.
This roadmap supersedes the earlier eye-centered ordering, retained in
[history](history/roadmap-before-system-continuation-2026-10-16.md).

The implementation inventory is [status.md](status.md). A source checkout, an
installed runtime, a working adapter, numerical verification, published reproduction
and context-specific biological validation are separate states. No percentage
combines them into a misleading overall completion score.

## Delivery order and dependencies

| EPIC | System outcome | Dependencies | Current delivery state |
|---|---|---|---|
| S0 | Preserve data, licenses and reliable local execution | None | Working local foundation; production deployment open |
| S1 | Managed ecosystem of reusable source libraries | S0 | Complete permanent vendor trees and 37 recursive dependencies; native operator profiles installed/executed; original benchmark coverage remains explicit |
| S2 | Persistent biological entities, relations and evidence | S0 | Versioned graphs, pinned OBO/identifier snapshots and unreviewed contextual annotations/conflicts working; reaction/claim authority open |
| S3 | Standard model/data package import | S1,S2 | JSON, scalar/native CSV/Parquet, SBML/PKML and CellML 2 working; reviewed CellML operator ODE solver; omics/image admission open |
| S4 | General experiment orchestration | S0,S3 | Durable reference/native scalar DAGs, COPASI/OSP agent and bounded cohorts/sweeps; remaining native adapters and richer bindings open |
| S5 | Numerical and biological coupling | S2,S3,S4 | Port/unit checks and exact exchange primitive; continuous feedback/spatial solver open |
| S6 | Evidence, measurement and independent validation | S2,S3,S4 | Frozen scalar evaluation and declared gaps; independent authority/UQ incomplete |
| S7 | Controlled model optimization and improvement | S1,S4,S6 | Decay calibration and explicit bounded corneal empirical learner execute; frozen animal/dose split and independent failures retained; general challenger/promotion workflow open |
| S8 | Molecular and gene-response execution | S1,S3,S6 | COPASI and GEARS runtimes execute; real perturbation H5AD acquired/read; original learned benchmark and biological bridges open |
| S9 | Multicellular/tissue execution across engines | S1,S4,S6 | vCornea, PhysiCell/PhysiBoSS, VCell and Chaste native operator paths execute; general queue adapters and validated bridges open |
| S10 | Organ and multi-organ composition | S5,S6,S8,S9 | NEURON and OSP PK-Sim/MoBi headless models execute; validated organ/system bridges and Windows GUI profile open |
| S11 | Governance and research-system integration | S0,S2,S6,S7 | Local roles/provenance/HomBon facade/gaps working; scientific approvals/claims/conflicts open |
| S12 | Interactive six-workspace application | S2,S3,S4,S6,S11 | E4 local six-workspace application implemented on current APIs; richer scientific authoring/quality authority/spatial workflows remain open |
| S13 | Scalable operation and release | S0,S4,S11,S12 | Single-host Docker working; PostgreSQL/object storage/HPC/multi-host open |

The latest2026-10-27 user instruction allows another eye component once the
present corneal scope is acceptably useful; otherwise continue corneal evidence,
sources and model work, including ML/RL only when supported by the task/data.
After the sealed population-model tests two scoped uses are defensible: the 1.0.0
initial-state prediction for mouse 2 mm central debridement controls, and the
2.0.0 calibrated online forecast intervals (sealed IL-10 coverage within both
windows). Cross-study initial-state transfer (rabbit, rat, human, IL-10 early
hours) and 5/10 pp accuracy on noisy data remain open, so further corneal work
waits for new data while the next eye component starts with the same protocol.
RL is not used: these observational data contain no actions or rewards.
Original publication reproduction remains a separate open gate. Scientific gates
remain mandatory before validity or biological coupling is claimed. UI work
depends on concrete operations and states; an illustration cannot replace execution.

The [terminal serial results](../experiments/vcornea/publication/serial-final-results-2026-10-24.md)
retain all four new full-horizon exits and completed analyses. Full January serial
formation2 + reused injury6 remains negative (five failures/three scalar passes);
the separate July serial moderate2 fails its trajectory criterion. Both new
original-math/spatial stages pass. Earlier native aborts, the inconsistent
four-worker output and every negative candidate remain. No native run is active.
The next scientific dependency is [exact original run correspondence](../experiments/vcornea/publication/original-run-metadata-needed.md),
not another unchanged simulation. The four-group gate stays open; no other model
work starts and recurring scheduling stays paused.

Latest user direction also requires active search for applicable real experimental
data, including Hugging Face, and useful code to compare it. The completed
[corneal evidence increment](../experiments/vcornea/experimental/README.md) retains
real human aggregates/rabbit observations and inspected HF samples, adds an actual
lattice-thickness operator and records four exploratory clinical comparisons.
This advances corneal empirical work while preserving the separate unresolved
publication gate; it authorizes no unrelated model or biological pass.

The separately versioned [projected wound-repair component](../experiments/vcornea/repair-front/README.md)
now has a frozen external-study predictive check: independent 1986 parameters give
2017 PBS cohort-mean RMSE4.02pp/max5.86pp without fitting. A second 2021 6mm
removal-method challenge fails all three groups (24.81–26.45pp RMSE). This is an
observed applicability boundary, not universal repair validity. Preserve both
results; do not fit the failed cohort and relabel it an untouched validation set.
The next corneal model dependency is an evidenced initial substrate/edge/injury
context and separately versioned dynamics that can explain the slower early
closure, with a new independent test cohort selected before fitting. Aggregate
ratios alone do not identify whether lag, migration, inflammation or geometry is
responsible. Full publication reproduction and cellular/chemical validation remain open.

The user's subsequent authorization includes new learned models. The completed
[corneal learning experiment](../experiments/vcornea/repair-front/learned-front/README.md)
fits four constrained parameters on six actual endpoint-dose animals, reserving
all three intermediate-dose animals from the optimizer. Their retrospective
mean RMSE improves10.26→5.96pp, but the candidate fails frozen5/10pp targets.
The2021 transfer also fails; newly acquired Fischak2017 PBS curves reject the
candidate using one-sided conservative constraints over the full72h horizon.
All failures are mandatory regression evidence, not datasets for unreported
test-driven selection. The shared deterministic-front audit has an optimistic
maximum-error lower bound21.29pp at common times, under declared nominal geometry;
it does not rule out stochastic/hierarchical alternatives or identify a cause.

Next corneal work, before further model selection:

- [x] Isolate complete calibration/heldout animals; freeze algorithm/criteria and
  retain actual bounded fitting, predictions and independent negative comparisons.
- [x] Recover another independent original study, preserve CC BY source/metadata,
  separate projected fluorescence from curvature-corrected OCT, and quantify
  conservative graph constraints without invented animal tables.
- [ ] Acquire matched raw longitudinal wound/edge images or measured initial
  geometry/substrate/injury/care covariates and an additional **entire independent
  study** reserved before selection. No identity/time split leakage or post-result
  threshold relaxation. Existing holdouts are now consumed evaluation evidence.
- [ ] Resolve original cell-density/EGF model time-scale/source inconsistencies
  recorded in the learning experiment's mechanism screening before any new
  mechanistic implementation. Do not infer coefficients from unrelated rat data.
- [ ] Implement a separately versioned, evidence-supported correction; meet the
  same small-error targets on selected independent measurements and retain all
  previous mandatory challenges. No automatic production promotion from fitting.

Original vCornea publication input correspondence remains a separate open
dependency. No other tissue/model work or automatic scheduling starts here.

The [2026-10-27 conditioned-forecast experiment](../experiments/vcornea/repair-front/conditioned-forecast/README.md)
now tests another useful task: input real0h/24h areas, predict all later36–84h
means with one preselected no-lag radius law. Actual complete output fails
14.99ppRMSE/19.66ppmax; no future-dependent lag/window/architecture retuning.
A separate retrospective prefix-only structural audit reports an optimistic
minimax14.51ppRMSE/20.48ppmax across its explicitly assumed lag/rate family, so
the requested5/10pp precision claim is withheld. This is not statistical coverage
or a proof against every possible biological mechanism. Latest source search:
[admission/screening](../experiments/data/cornea-evidence-20261027/README.md).

- [x] Preserve a new CC BY4.0 original study and all28 table aggregates; separate
  only0/24h conditioning from every future outcome and preserve the actual failed
  full84h comparison, paired-eye/time/care inconsistencies and source identities.
- [x] Implement exact lag/rate compatibility envelopes and precision abstention
  from the prefix alone; prove it does not read future outcome files. No midpoint
  envelope, SD or inferred trajectory is relabeled clinical confidence.
- [x] Acquire2025 original PBS image supplement with separate CC BY-NC4 terms;
  preserve four unlabeled rows versus six reported eyes, resolution/calibration
  limits and absence of segmentation. Retain2021 saline aggregate lead separately.
- [x] Flag reanalysis of the same2017 rabbit cohort by the2019 OCT paper so it is
  not counted as a new independent validation cohort.
- [ ] Recover complete calibrated longitudinal areas/images, stable eye/animal
  identities, exact acquisition times and injury/supportive-care context. More
  post-injury motion observations or independently measured kinetic constraints
  are needed before claiming precise conditioned forecasts. The consumed2019
  future table cannot become a fresh holdout after changing assumptions.
- [ ] Freeze any image observation/quality protocol before extracting the reserved
  2025 partial cohort; quantify visibility/zoom/glare uncertainty and resolve
  missing eyes/row identity before a complete-cohort accuracy claim.

The next [retrospective online replay](../experiments/vcornea/repair-front/online-forecast/README.md)
now tests causal updates after every available observation. It is a different
task from initial-state simulation and uses previously consumed studies. All98
actual isolated children completed;96 numeric targets across16 separate series
show improvement over persistence, but only5 series meet the fixed5/10pp accuracy
and0.5pp improvement gates. Its largest error is25.16pp; it is not uniformly better
than first-to-current trend or the original initial-only formula. Complexity and
extra observations have not established general precision.

- [x] Implement a pure prefix-only signed radius state estimator and source adapter
  preserving individual versus aggregate statistics, expansion, derived baselines,
  actual irregular horizons and categorical closure.
- [x] Freeze one method/criteria before its new replay; retain all98 child exits,
  denied-read/network/write probes,96 scores,2 unscored categorical outcomes and
  the import-only setup failure. Reject the general claim without threshold changes.
- [x] Deliver a reusable [Persian transfer prompt](continuation-prompt.fa.md) and
  [current handoff](continuation-state.fa.md) covering the whole-platform intent,
  actual scientific results, verification and required documentation updates.
- [ ] Acquire a new independently reserved, matched longitudinal measurement set
  with stable eye/animal identity and calibrated geometry/timing; freeze image/
  measurement quality rules before extraction. Existing2025 partial unlabeled
  images are not a complete substitute and consumed targets are not new holdouts.
- [ ] Quantify measurement repeatability and independent dynamics/substrate/care
  covariates before selecting a new mechanistic, filtered or learned correction.
  Retain all existing baselines and long-interval failures; evaluate per series
  and horizon on fresh reserved evidence before a scoped accuracy claim.

### 2026-10-27 cross-study population model and decisions

The [audit](corneal-work-audit-2026-10-27.fa.md) found the earlier corneal work
rigorous but narrow: single-study rules, the same three consumed studies, no
noise/between-study model and keyword-only repository searches. The new
[population model](../experiments/vcornea/repair-front/population-model/README.md)
was frozen (commit `de9762a0`) before a one-shot sealed test.

- [x] Screen 1,993 open-access full texts, acquire data-like supplements, triage
  with all numbers hidden, freeze an outcome-blind hash split and let a separate
  custodian extract sealed records (pinned by SHA-256 only).
- [x] Implement the hierarchical radius–lag model with censored zeros, learned
  study noise, penalized REML pooling and cohort assimilation; add the
  nearest-trajectory ML comparator and an in-sample floor diagnostic.
- [x] Retain the failing development LOSO and the sealed result: initial-state
  mouse control curves 2/2 pass; next observation 52/64 series; 90% coverage 79.6%
  (fails its 80% floor). Scope is mouse 2 mm central debridement controls.
- [x] Pause unchanged vCornea replays until original run inputs are obtained; keep
  it as an executable archived reference.
- [x] Fix interval calibration in a new version and test it on **new** sealed
  studies: model 2.0.0 (closure tail) with dataset 1.1.0 (article tables, 16
  development studies) was frozen at `2cee1b01`; on sealed IL-10/i-PRF data its
  cohort intervals met both windows (70.3%/82.4%, the 80% one by one target) and
  beat the last-two trend (13.1 vs 16.3 pp); accuracy equalled 1.0.0 and both
  initial-state groups failed. Both sealed sets are now consumed.
- [ ] Initial-state transfer needs per-context data with covariates (onset time
  of measurements, inoculation/anaesthesia, measurement method) and more human
  surgical studies; the human/other context has one 4-patient training study.
- [ ] Alkali burns: the radius–lag kinematics fails even in-sample (0/13
  series); a separate limbal-damage/re-erosion model would be needed.
- [ ] Explain the rabbit latent-phase spread (≈1–17 h) with recorded injury-method,
  care and measurement covariates; acquire more rabbit and human (e.g. post-PRK
  epithelial closure) longitudinal data with the same outcome-blind protocol.
- [ ] Extend the screen beyond fluorescein full-text matches (figure-only data
  needs a validated digitization protocol; author data requests need the user).
- [x] Next eye component, per the user's rule now that one corneal use is
  defensible: chosen by data availability with the same search → blind split →
  simple-baseline protocol. Endothelial cell density after surgery had the most
  open-access tables (843 papers screened; nerve, haze and post-cataract
  thickness had fewer); see the section below.
### 2026-10-28 corneal endothelial cell density (second eye component)
[Model 1.0.0](../experiments/vcornea/endothelium/README.md) was frozen (commit
`00d2cd09`, protocol `fdec8d89`) before a one-shot sealed test.
- [x] Screen 843 open-access full texts and 679 tables with masked previews,
  triage 87 studies, freeze an outcome-blind split (66 development/21 sealed) and
  let separate custodians extract 17 sealed records (4 excluded with reasons).
- [x] Hierarchical power-law model per tissue family (host/graft), recipient-
  referenced grafts as their own context, family fallback for unseen procedures,
  offset-anchored Student-t forecasting; author-overlap rule for possibly shared
  cohorts; NC/ND/SA records kept private and redacted from public reports.
- [x] Sealed result: next observation 5.60 vs persistence 7.36 pp (H2 pass),
  coverage 71.1%/82.9% inside both windows (H1 pass), landmark 6.85 vs 12.41 pp
  (H3 pass); 23/30 series within 5/10 pp; initial-state 13/29 control groups.
  Development and sealed sets are now consumed.
- [ ] Handle late level shifts (a 10-year DMEK group dropped between 60 and 84
  months and was followed slowly): test a local-level/random-walk offset in a new
  version on newly reserved studies.
- [ ] Improve initial-state transfer with covariates that the dataset already
  records but the model ignores (indication such as Fuchs dystrophy, donor age or
  storage, device, nucleus grade, tamponade such as silicone oil).
- [ ] Single-eye prediction needs per-eye data and its own calibration; it is not
  claimed. Rabbit/other species need their own studies.
- [x] Next candidate after endothelium: corneal thickness (hydration) after
  surgery, chosen as the third corneal component; see the section below.
### 2026-10-28 central corneal thickness (third corneal component)
[Model 1.0.0](../experiments/vcornea/thickness/README.md) was frozen (commit
`deef7664`, protocol `a97db853`) before a one-shot sealed test.
- [x] Screen 1,179 open-access full texts and 631 tables with masked previews,
  triage 56 studies, freeze an outcome-blind split (37 development/19 sealed);
  development extraction (33 records) and two custodians (15 sealed records);
  outcome-selected analysis sets excluded by a rule shared with the custodians.
- [x] Generic author-overlap operator (`scripts/link_study_authors.py`); the
  endothelium operator stays pinned and unchanged.
- [x] Hyperbolic return from the day-1 excess; host family relative to the
  preoperative CCT, graft family pooling the long-term level as absolute thickness;
  exact Gaussian-mixture posteriors; REML on likelihood-information summaries;
  development log with every declaration written before its run.
- [x] Sealed result: next observation 1.60 vs persistence 2.06 pp (H2 pass),
  coverage 89.2%/96.9% inside both windows (H1 pass, near the upper edges), landmark
  2.31 vs 4.01 pp (H3 pass); 28/29 series within 5/10 pp; initial state 18/21 control
  groups. Development and sealed sets are now consumed.
- [ ] Assign `other` procedures to a family by mechanism (endothelial replacement or
  edema clearing versus minor laser procedures such as YAG capsulotomy) or by a
  preoperative-CCT rule; test on newly reserved studies.
- [ ] Couple thickness to endothelial density: late endothelial loss after DMEK
  (seen in the same 10-year cohort in both components) should raise CCT.
- [ ] Graft forecasting and single eyes remain too sparse to claim; more keratoplasty
  studies with early follow-ups are needed.
- [x] Next candidate after thickness: intraocular pressure after glaucoma
  procedures, chosen as the fourth eye component; see the section below.
### 2026-10-28 intraocular pressure after glaucoma procedures (fourth eye component)
[Model 1.0.0](../experiments/eye/iop/README.md) was frozen (commit `ff470345`,
protocol `f13238a2`) before a one-shot sealed test.
- [x] Screen 2,572 open-access full texts and 3,040 tables with masked previews; a
  mechanical rule kept 499 articles; salted tranches keep 379 of them untriaged and
  unread for later versions; triage of tranche 1 (86 admitted); outcome-blind split
  (54 development/32 sealed); four development extractors (47 records) and three
  custodians (25 sealed records); censoring at reoperation treated as outcome-selected.
- [x] Exponential return from the day-1 deviation; long-term level regressed on the
  preoperative pressure with one shared slope; one hierarchy per species with
  procedure contexts (no mechanism families); exact Gaussian-mixture posteriors;
  predictive-median point forecasts (round 2, declared before the run); stricter
  licence classifier in the publisher.
- [x] Sealed result on group means: next observation 7.74 vs persistence 8.64 pp
  (H2 pass, also below the pooled-curve ratio and analog kNN), coverage 76.3%/89.4%
  inside both windows (H1 pass), landmark 9.92 vs 12.89 pp (H3 pass), initial state
  10.96 vs 17.99 pp (H4 pass); 21/43 group means within 5/10 pp. Development and
  sealed sets are now consumed.
- [x] Represent early non-monotone courses (week-1 rises after GATT, rebound after a
  very low day 1 after canaloplasty) with a second early component: model 1.1 (fixed
  7-day transient, group-size noise) was tested on tranche 2, reserved before triage.
  All five declared hypotheses passed: next observation 6.69 vs 8.06 pp for
  persistence and vs 6.92 for 1.0.0 trained on the same records; landmark 8.95 vs
  15.10 and 9.40.
- [x] Calibrate early and long-horizon intervals. Tranche 2 showed first-month
  next-observation coverage of 58.5%/73.4% and landmark 80% coverage of 67.1%. Model
  1.2 (forecast-only early noise) raised first-month coverage to 82.4%/93.9% on
  tranche 3 at the same accuracy; landmark coverage was 79.5%/91.0% (see the 1.2
  section below).
- [ ] Early rises far above the context prior (KDB goniotomy 42% → 89% at week 1)
  can only be followed after week 1; no fixed-prior model predicts them from day 1.
- [ ] Represent a waning effect (micropulse cyclophotocoagulation) or late failure
  with a late drift term; recalibrate long-horizon intervals (landmark 80% coverage
  0.652 in the sealed set).
- [ ] Use postoperative medication and medicated/unmedicated references as
  covariates; surgical success/failure endpoints are not claimed.
- [x] Next candidate chosen by data availability: central retinal thickness after
  treatment (1,938 and 2,250 query hits against 67 for corneal nerves and 181 for
  retinal nerve fibre layer); see the section below. Corneal nerves and stromal haze
  remain candidates.

### 2026-10-28 central retinal thickness after treatment (fifth eye component)
[Model 1.0.0](../experiments/eye/retina/model-1.0/README.md) was frozen (commit
`d40b6ed2`, protocol `64b0ad17`) before a one-shot sealed test.
- [x] Screen 3,128 open-access articles and 2,548 masked tables; a mechanical rule
  kept 395; salted tranches keep 275 untriaged and unread; blind triage of tranche 1
  (78 admitted); outcome-blind split (54/24); four development extractors (46 records,
  98 series) and three custodians (19 sealed records, 82 series).
- [x] Generic eye recovery population model and harness (`eye_recovery_population`,
  `eye_recovery_evaluation`): the IOP 1.1 law and hierarchy in a `RecoveryConfig`,
  equal to `iop_population_1_1` to 1e-9 on synthetic data, so later components reuse
  one implementation.
- [x] Persistent Ornstein–Uhlenbeck deviations in forecasting (exact, checked against
  the joint Gaussian); three declared LOSO rounds, two candidates rejected by the
  adoption rule.
- [x] Sealed result on group means: all five hypotheses pass; next observation 5.75
  vs 6.34 pp for persistence and 6.14 for the same model without deviations;
  coverage 81.6%/91.8%; landmark 7.50 vs 9.50; initial state 11.35 vs 17.09.
- [ ] Represent drug wear-off after a single implant or injection (dexamethasone
  rebound at 3–4 months, missed by 15–30 pp), for example with a dose-response term
  whose duration depends on the agent; test on tranches 2–4 (275 unread articles).
- [ ] Contexts without development studies (postoperative cystoid edema, uveitic
  edema, observation arms) fall back to the species mean (initial state 17.2 vs
  10.3 pp in trained contexts); later tranches should add them.
- [ ] Single-eye noise: no single-eye series has five follow-ups, so single-eye
  intervals were too narrow in development (80% coverage 0.49); reported only.
- [ ] Late follow-ups (after month 3) tie with persistence; retreatment timing is
  not an input.

### 2026-10-28/29 retinal-thickness model 1.1 (tranche-2 sealed test)
[Model 1.1.0](../experiments/eye/retina/model-1.1/README.md) was frozen (commit
`84ee1e9c`, protocol `fd14a1c5`) before a one-shot test on tranche 2, which was
reserved before triage.
- [x] Tranche 2: blind triage (85 admitted), seven custodians (63 records, 135
  series), every record rebuilt byte for byte.
- [x] Generic eye model version 2 (`eye_recovery_population2`, harness
  `eye_recovery_evaluation2`): wear-off of single doses, treatment fallback, early
  noise, noise pooling, per-group study effects; defaults reproduce version 1 bit for
  bit.
- [x] Declared dose rule for one dexamethasone implant (duration 3.5 months, 80% of
  the effect lost) and forecast noise from the pooled study noise; three declared LOSO
  rounds on 65 studies (fallback and full pooled noise rejected).
- [x] Sealed result on 92 group means: H1–H4 pass (coverage 84.4%/89.6%, next
  observation 7.91 vs persistence 8.81, landmark 9.90 vs 11.55, initial state 13.00 vs
  18.51). H5 fails: model 1.0.0 with the same training forecast slightly better (7.79,
  9.73) but would have failed H1 (coverage 66.0%/77.2%).
- [ ] Let the implant's effect duration vary per series and be learned from its own
  visits (a fixed 3.5 months predicted a rebound that did not occur after vitrectomy
  with an implant); test on tranches 3–4 (155 unread articles).
- [ ] Contexts without training series remain worse than persistence (11.27 vs 10.21
  pp); add data rather than a fallback rule.

### 2026-10-29 IOP model 1.2 (tranche-3 sealed test)
[Model 1.2.0](../experiments/eye/iop/model-1.2/README.md) was frozen (commit
`df8f389a`, protocol `4169750d`) before a one-shot test on tranche 3, which was
reserved before triage.
- [x] Tranche 3: blind triage (97 admitted), eight custodians (83 records, 196
  series), every record rebuilt byte for byte.
- [x] The 1.1.0 law and fit in generic eye model version 2 (identical in all 195,431
  development numbers) with forecast-only early noise `k = 8`, `T = 1 month`, chosen
  by a declared calibration on development rows; persistent deviations rejected.
- [x] Sealed result on 130 group means: all five hypotheses pass (coverage
  83.5%/93.1%, next observation 5.89 vs persistence 7.21, landmark 7.76 vs 11.89,
  initial state 10.17 vs 18.06, first-month coverage 82.4%/93.9% vs 1.1.0's
  67.9%/81.7% at 5.885 vs 5.877 pp).
- [x] Tranche 4 reserved before triage, triaged blind (94 admitted) and extracted
  blind (79 records, 222 series) for model 1.3; tranche 5 (19 articles) stays unread.
- [ ] Procedure-specific early courses: the month-1 rise after Ahmed valves and the
  week-1 fall then month-1 rise after repeat cyclophotocoagulation are missed; the
  first-month error (9.09 pp) is twice the later one.
- [ ] Procedure-specific noise and offsets: 80% coverage 0.71–0.74 for tubes,
  cyclodestruction and non-penetrating surgery against 0.88 for angle MIGS; landmark
  forecasts 5–9 pp too high after subconjunctival MIGS, cataract and non-penetrating
  surgery. Model 1.3 is to be developed on tranches 1–3 (229 studies) and tested on
  tranche 4.
### 2026-10-29 IOP model 1.3 (tranche-4 sealed test)
[Model 1.3.0](../experiments/eye/iop/model-1.3/README.md) was frozen (commit
`38f4f874`, protocol `0cac4daf`) before a one-shot test on tranche 4.
- [x] A sub-procedure (device or technique) read from each arm's description and
  nested under the procedure; adopted on 229 training studies (6.38/8.06/10.07 against
  1.2.0's 6.46/8.35/10.21 pp); a lognormal-mean point rule rejected.
- [x] Sealed result on 133 group means: H1–H4 pass (coverage 77.4%/88.3%, next
  observation 6.64 vs persistence 8.27, landmark 7.92 vs 15.02, initial state 10.94 vs
  15.94); **H5 fails** (initial state 10.94 vs 1.2.0's 10.69 pp on the same training).
  `peykar eye` keeps 1.2.0, which met H1–H4 on tranche 4 as well (reported).
- [ ] A sub-procedure level only where the training data show a bias (cyclodestruction,
  subconjunctival MIGS; tubes and stents lost), the hypertensive phase of valved tubes
  and early spikes after cataract surgery. Needs a new screen: only tranche 5 (19
  articles) is unread.

### 2026-10-29 refraction after corneal laser refractive surgery (sixth eye component)
[Model 1.0.0](../experiments/eye/refraction/model-1.0/README.md) was frozen (commit
`a2eccf2c`, protocol `8634f5a2`) before a one-shot sealed test.
- [x] Screen 2,169 open-access articles and 1,215 masked tables; a mechanical rule
  kept 89 (one tranche); blind triage (39 admitted); outcome-blind split (31/8); three
  development extractors (24 records, 43 series) and two custodians (8 sealed records,
  14 series), every sealed record rebuilt byte for byte.
- [x] Additive outcomes on the generic model: an exact pseudo-fraction of the change
  from baseline (`cornea_refraction_data`) and an additive evaluation that rescores
  the unchanged harness in the outcome's unit with a target comparator
  (`eye_additive_evaluation`). The same pattern can serve visual acuity (letters or
  logMAR) and visual-field indices.
- [x] Sealed result on 13 group-mean forecast series: all five hypotheses pass
  (coverage 82.1%/85.7%, next observation 0.29 vs persistence 0.37 D, landmark 0.47
  vs 0.78, initial state 0.40 vs 0.50 D for full correction, refractive-error level
  better than procedure only).
- [x] `peykar eye forecast --component refraction` (fitted from record folders,
  forecasts in dioptres).
- [ ] Hyperopic and mixed-astigmatism corrections: six training series, none tested.
- [ ] The initial state is limited by between-study differences (SD about 0.48 D);
  laser platform, nomogram and optical zone are not inputs.
- [ ] A later version needs a new screen reserved before triage (no unread tranche).

### 2026-10-29 retinal-thickness model 1.2 development (no change adopted)
- [x] Round 1 on the 128 training studies of tranches 1–2: a prior-treatment level and
  an OCT-device level gained at most 0.074 pp and were rejected by the declared rule.
- [x] Round 2 (regimen level, declared from an out-of-sample regimen bias diagnostic)
  gained at most 0.012 pp and was rejected; the regimen biases stayed almost unchanged.
  No 1.2 protocol was frozen; tranche 3 (67 extracted sealed records) stays sealed and
  unread for a later version.
- [ ] Untested: an implant-duration mixture learned from each series' own visits.

### 2026-10-29 corrected visual acuity after keratoplasty and cross-linking (seventh eye component)
[Model 1.0.0](../experiments/eye/acuity/model-1.0/README.md) was frozen (commit
`baafe4cd`, protocol `0c60925c`) before a one-shot sealed test.
- [x] Screen 1,994 open-access articles and 1,185 masked tables; articles kept by
  other components' untested tranches dropped first; a mechanical rule kept 175 in two
  tranches; blind triage of tranche 1 (67 admitted); outcome-blind split (47/20); five
  development extractors (27 records, 62 series) and three custodians (15 sealed
  records, 25 series), every sealed record rebuilt byte for byte.
- [x] The additive pattern of refraction reused for logMAR, scored in ETDRS letters;
  three development rounds (indication level, per-family hierarchies, forecast-only
  persistent deviations) rejected by the declared rule.
- [x] Sealed result on 21 group-mean forecast series: all four hypotheses pass
  (coverage 82.9%/92.7%, next observation 5.00 vs persistence 5.06 letters, landmark
  5.24 vs 7.27, initial state 9.04 vs pooled curve 9.79), although development had
  failed H2/H3 against the pooled-curve increment.
- [x] `peykar eye forecast --component acuity` (fitted from record folders, forecasts
  in logMAR).
- [ ] Poorly recovering keratoplasty groups (comorbidity, very poor baseline) are
  forecast too optimistically; PK was not in the sealed set. Next version: keep an
  early shortfall in the forecast and test on tranche 2 (55 unread articles).

### 2026-10-29 visual-acuity model 1.1 development (no change adopted)
- [x] Tranche 2 of the corneal acuity dataset (55 articles) reserved before triage;
  blind triage (27 admitted), four custodians (11 sealed records, 23 series).
- [x] Generic eye model 4 (`eye_recovery_population4`): an optional fitted correlation
  of the long-term level and the transient amplitude, and separate level slopes for
  named treatment groups; defaults reproduce version 2 bit for bit.
- [x] Round 1 on the 42 tranche-1 studies: both candidates rejected by the 0.10-letter
  rule. No 1.1 protocol; tranche 2 stays sealed and unread.
### 2026-10-29 best-corrected visual acuity after macular treatment (eighth eye component)
[Model 1.0.0](../experiments/eye/retina-acuity/model-1.0/README.md) was frozen (commit
`f4352217`, protocol `34494401`) before a one-shot sealed test.
- [x] Screen 3,655 open-access full texts and 3,623 masked tables; articles kept by
  other components' untested tranches dropped first; a mechanical rule kept 378 in four
  tranches; blind triage of tranche 1 (62 admitted); outcome-blind split (41/21); five
  development extractors (15 records, 52 series) and three custodians (9 sealed
  records, 11 series), every record rebuilt byte for byte.
- [x] The additive acuity pattern reused for macular disease; treatment nesting and
  forecast-only persistent deviations rejected, pooled forecast noise adopted under a
  declared calibration clause.
- [x] Sealed result on 11 group-mean forecast series: **all four hypotheses fail**
  (coverage 67.7%/80.6%, next observation 6.14 vs persistence 4.81 letters, landmark
  9.17 vs 6.49, initial state 16.40 vs pooled curve 5.18). Not added to `peykar eye`.
- [x] Cause found after unsealing: the 7-day early transient had no training visit
  before day 30 and was fitted at about +2 logMAR at day 1; one sealed study with a
  day-1 visit carried the failure.
- [x] `peykar eye` reports each forecast's training-time support (earliest and latest
  training visit, stored with each fit) and warns for observed or forecast months
  outside it; forecast values are unchanged.
- [x] Tranche 2 (120 articles) reserved before triage as the sealed set of version 1.1:
  blind triage (64 admitted), eight custodians (29 records, 81 series).
- [x] Version 1.1 development on the 24 tranche-1 record studies: the law without the
  early transient (w1) adopted (3.46/3.97/4.87 against 4.33/5.83/7.62 letters); a
  treatment fallback and dexamethasone wear-off rejected. Protocol `c0c37c7e` frozen
  with H5 against 1.0.0 trained on the same records.
- [x] One-shot sealed test of version 1.1 on tranche 2: H1 passes, H2–H5 fail (next
  observation 3.95 vs persistence 3.00 and the pooled-curve increment 2.80 letters;
  landmark 6.24 vs 3.56 and 3.06; initial state 6.65 vs 5.87; worse than 1.0.0 with the
  same training). The recovery term (time constant 7.5–13 days) took the transient's
  place before each context's first training visit; two linked dexamethasone-implant
  studies with day-3 visits carried most of the error.
- [ ] A pre-freeze check of which terms of a law the training visits identify in each
  context, and a rule for times before a context's first training visit (pooled-curve
  shape there, or interpolation from the baseline).
- [x] Tranches 3–4 (138 unread articles) reserved before triage as the sealed set of
  version 1.2 ([declaration](../experiments/data/retina-acuity-2026-10-29/tranche-3/README.md));
  development on the 53 record studies of tranches 1–2.
- [x] Blind triage of tranches 3–4 (70 admitted) and author overlap.
- [x] Custody of tranches 3–4 (27 records, 143 series, 34 group means).
- [x] Generic support hold (`eye_support`, harness 5) and version 1.2 development on the
  53 record studies of tranches 1–2: x3 (hold + dexamethasone wear-off) chosen,
  3.228/3.782/5.216 letters against 1.1.0's 3.628/4.921/5.556.
- [x] Freeze of version 1.2 (protocol `a0618bbf`).
- [x] One-shot sealed test of version 1.2: all five hypotheses fail (next observation
  4.51 vs the pooled-curve increment 3.82 letters; landmark 5.85 vs 4.52; 80% coverage
  0.59). Third failure with the same pattern.
- [ ] A model that starts from the pooled training curve's increments (flexible shared
  time course, context offsets only where supported) instead of a fast exponential to a
  level; it needs a new literature screen with a test part reserved before triage.
### 2026-10-30 forecast stacking (corneal acuity 1.2, retinal thickness 1.3)
[Acuity 1.2.0](../experiments/eye/acuity/model-1.2/README.md) and
[retinal thickness 1.3.0](../experiments/eye/retina/model-1.3/README.md) were frozen
together (commit `fc3590ac`; protocols `484c1694` and `9fe7bcff`) before their one-shot
sealed tests on the two unread sets (acuity tranche 2, retina tranche 3).
- [x] Generic `eye_stacking` 1.0.0 and `eye_stacking_evaluation` 1.0.0: convex
  least-squares weights on leave-one-study-out forecasts, nested LOSO for development,
  stacked intervals moved from the model's; harness 6 and `eye_additive_evaluation4`
  keep initial-state intervals and reproduce harness 2 with 0 differences.
- [x] Development: both stacked candidates adopted by the declared 0.10 rule (acuity
  5.11/6.86/12.92 against 5.41/7.14/13.01 letters; retina 7.154/9.228/12.854 against
  7.193/9.376/12.892 pp).
- [x] Sealed acuity: H1–H4 pass, H5 fails (forecasts 3.72 vs 4.06 and 5.74 vs 6.66
  letters better, initial state 9.83 vs 9.01 worse). Sealed retina: H2–H4 pass, H1 fails
  (80% coverage 0.911), H5 fails (initial state 11.62 vs 11.61; forecasts 5.70 vs 6.08
  and 7.72 vs 7.98 better). `peykar eye` keeps acuity 1.0.0 and retina 1.1.0.
- [x] Lesson from both tests: stacking the forecasts from follow-up transfers; stacking
  initial states with a pooled curve that ignores the baseline does not; an interval
  moved to a better point must also be narrowed.
- [x] A stacked version that combines only the forecasts from follow-up and scales its
  intervals to the stacked errors on development: retinal thickness 1.4.0 (below).

### 2026-10-30 history-only stacking with scaled intervals (retinal thickness 1.4)

[Retinal thickness 1.4.0](../experiments/eye/retina/model-1.4/README.md) was frozen
(commit `c1dbd266`; protocol `ea054039`) before its one-shot sealed test on retina
tranche 4, reserved before triage (22 admitted, 17 custodian records).

- [x] Generic `eye_stacking2` 2.0.0 and `eye_stacking_evaluation2` 2.0.0: stacking
  1.0.0's weights for forecasts from follow-up only, initial states left to the model,
  stacked intervals scaled around the stacked point by a factor chosen on development
  (one factor, or one per forecast task).
- [x] Development on the 195 studies of tranches 1–3: one scale (k2) under-covered the
  landmark intervals (0.692); one scale per task (k3, 0.79/1.00) adopted: 6.623/8.673 pp
  against 6.752/8.850, coverage 0.797/0.877 and landmark 0.791/0.897.
- [x] Sealed: H1 (0.793/0.839), H2 (5.84 vs persistence 7.00) and H4 pass; H3 fails
  (landmark 10.58 vs the pooled-curve increment 9.80) and H5 fails (landmark 10.58 vs
  1.1.0's 9.99). `peykar eye` keeps retina 1.1.0, whose own H3 also fails there.
- [x] Lesson from three stacking tests: forecasts from follow-up gained twice and lost
  once; a fixed persistence weight hurts when new trajectories change more after the
  first visit than the development ones did. Scaling the next-observation interval on
  development worked (80% coverage 0.793).
- [ ] A forecast that adapts how much it trusts the last observation to the series' own
  observed change, rather than a fixed persistence weight; it needs a new retina screen
  with a test part reserved before triage (no unread retina article remains).
### 2026-10-30 maximum keratometry after cross-linking or ring segments (Kmax 1.0)
[Kmax 1.0.0](../experiments/eye/kmax/model-1.0/README.md), the ninth eye component, was
frozen (commit `9fe023ae`; protocol `9a20b859`) before its one-shot sealed test on 17
custodian records (33 series) reserved before any value was read.
- [x] Dataset `cornea-kmax-2026-10-30`: 1,526 screened full texts, 102 articles by the
  masked-table rule with one follow-up time, blind triage (62 admitted), salted split
  40/22, 25 development records (35 series) and 17 sealed records.
- [x] Development: the generic 7-day transient was read from almost no data and the
  intervals were too narrow; k6 (no transient, forecast-only pooled noise) adopted in
  round 2 (6.145/11.113/15.412 pp against 7.053/11.433/15.574); development H4 fails.
- [x] Sealed: H1 (0.745/0.855) and H2 (10.20 pp against persistence 11.91 and the
  pooled-curve increment 10.73) pass; H3 (landmark 12.60 against 12.35) and H4 (initial
  state 14.61 against 14.17) fail. One study of eyes treated under 18 years, which kept
  flattening for five years, carries both failures. Not offered by `peykar eye`.
- [ ] A Kmax law with a slow long-term component (or a law in log time) learned from the
  long series, with age as a covariate, tested on a new screen with a test part reserved
  before triage (every article of this dataset is consumed).
### 2026-10-30 selective sub-procedure level (IOP 1.4)
[IOP 1.4.0](../experiments/eye/iop/model-1.4/README.md) was frozen (commit `dfd88dbb`;
protocol `a0499d76`) before its one-shot sealed test on IOP tranche 5, reserved before
triage (17 admitted, 14 custodian records).
- [x] Development on the 308 studies of tranches 1–4: a selection rule declared before
  the diagnostics kept model 1.3's sub-procedure level for cyclodestruction and
  subconjunctival MIGS only; d1s adopted (6.512/8.087/10.054 pp against 1.2.0
  6.553/8.232/10.268).
- [x] Sealed: H1–H4 pass (coverage 0.764/0.903; next observation 8.34 pp against
  persistence 10.86; landmark 9.14 against 18.06; initial state 9.46 against 15.59);
  H5 fails (next observation +0.057 pp and landmark +0.182 pp against 1.2.0, allowed
  +0.05). One study with the only sealed endoscopic cyclophotocoagulation arm decides
  H5. `peykar eye` keeps IOP 1.2.0, which met H1–H4 on tranches 3, 4 and 5.
- [ ] A sub-procedure reader tested on device names with numerals (the frozen rule
  read XEN45 as `other`), and a new IOP screen with a test part reserved before triage
  (every article of the 2026-10-28 screen is consumed).
## S0 — preservation and platform contracts

- [x] S0.1 Preserve original licenses, historical model/schema versions and source blueprint.
- [x] S0.2 Exercise authenticated submit → worker → checksum-verified result; fix lifecycle races.
- [x] S0.3 Back up SQLite through its backup API and retain artifact bytes before upgrades.
- [x] S0.4 Bound trusted child execution; pin result/runtime contracts and redact failures.
- [ ] S0.5 Establish documented recovery drill, deployment permissions, tenant isolation and threat model for a multi-user release.

Exit: existing records survive an upgrade; cancellation/retries cannot publish a
stale result; the configured deployment's auth/isolation guarantees are tested.
Local single-user verification does not close S0.5.

## S1 — library/source ecosystem and dependency ownership

- [x] S1.1 Retrieve all eight named specialist repositories plus CompuCell3D at exact commits; preserve the existing vCornea Git/archive identities separately.
- [x] S1.2 Hash every tracked file and preserve license evidence; explicitly inventory Git links/LFS pointers and declared CRLF checkout transformations.
- [x] S1.3 Persist immutable source inspection receipts; expose discovery and offline operator bundle import without installing code in the API.
- [x] S1.4 Acquire/review recursive source dependencies: 37 Git links materialized, including nested NEURON, OSP and separate VCell solver dependencies; no remaining links/LFS pointers in the verified distribution.
- [ ] S1.5 Complete original benchmark coverage for every engine. Isolated native CPU/headless profiles are installed and executed for all named computational families; GEARS's full original screen/split/score and Windows graphical profiles remain separate unclosed requirements.
- [x] S1.6 Version compatibility overlays with upstream bases, exact file hashes and actual native execution results; preserve original scientific source.
- [x] S1.7 Materialize all named sources as permanent project files under `vendor/`, preserving both vCornea identities and all original notices.
- [x] S1.8 Admit native execution evidence through an offline operator bundle, immutable source references and authenticated API/CLI discovery; preserve earlier receipts.

Source catalogue: [engine_adapters/ecosystem-sources.json](../engine_adapters/ecosystem-sources.json).
Actual receipts: [source inspection](../engine_adapters/ecosystem-source-receipts-2026-10-16.json).
The permanent distribution now contains **29 source identities / 53,387 files /
1,824,257,046 bytes**, excluding Git administrative data, with **37 materialized
Git links**. See [vendor manifest](../vendor/manifest.json), [original notices](../THIRD_PARTY_NOTICES.md),
[native runtime profiles](../runtime_profiles/README.md) and [actual new receipts](../engine_adapters/runtime-evidence-2026-10-17.json).
Earlier 1.0.0 source inspections remain immutable historical observations; complete
recursive inventories are admitted as 2.0.0. Runtime evidence does not establish validity.

Exit per engine: exact source/dependencies/license inventory, reproducible runtime,
native benchmark, adapter capabilities, failure/cancellation/artifact tests and an
explicit scientific applicability statement. A source-only entry fails this gate.

## S2 — biological representation and evidence graph

- [x] S2.1 Persist immutable entity/relationship snapshots with context, species and evidence.
- [x] S2.2 Reject unknown endpoints, duplicate identities and hierarchy cycles; query bounded neighborhoods with truncation disclosure.
- [x] S2.3 Import a small sourced human EGF/EGFR graph from inspected Reactome/UniProt evidence; keep it separate from a kinetic mechanism.
- [x] S2.4 Add pinned ontology/identifier connectors with mapping/conflict provenance. E6 admits complete basic CL/Uberon/GO releases plus explicit HGNC/UniProt/Reactome entry subsets, exact raw/normalized data, indexed queries and unreviewed contextual annotations. Full database ingestion, OWL/taxon reasoning and scientific reconciliation are not inferred.
- [x] S2.5 Represent multiparty reactions, stoichiometry, process ownership, state instances and contextual declarations. E7 adds immutable process networks, exact rational matrices, source-defined molecular states/complexes/alternative sets, one-owner-or-exclusion plans and original-unit sample frames. These remain unreviewed declarations; scientific authority and executable bridge enforcement remain S2.6/S5/S11.
- [ ] S2.6 Add scientific claim/conflict authority and evidence review; connect executable model versions to represented biological entities. E6 graph-entity identifier annotations are unreviewed declarations and do not close this gate.

Exit: entity, model and runtime instance remain distinct; queries retain evidence
and context; a graph relation never silently authorizes an executable coupling.

## S3 — model and dataset admission

- [x] S3.1 Admit immutable JSON packages and digest-pinned local external inventories.
- [x] S3.2 Preserve raw/normalized CSV and native vCornea count/height artifacts with units and lineage.
- [x] S3.3 Acquire three severities of real published simulation references and a separate attributed experimental aggregate.
- [x] S3.4a Admit exact declarative SBML core L2/L3 and original PKML with context/license and unit metadata; execute with isolated pinned COPASI/OSPSuite drivers through the native agent.
- [x] S3.4b Add bounded self-contained CellML 2 admission and a reviewed libCellML/SciPy operator ODE solver, retaining original units, immutable raw bytes and separate execution receipts. E5 includes numerical/failure/artifact checks and workbench inspection; general CellML queue/flow support remains under S4.6b.2.
- [ ] S3.5 Add AnnData/SpatialData and image/geometry readers in separate runtimes with donor/assay/license metadata.
- [ ] S3.6 Add explicit GEO, CELLxGENE, HuBMAP, BioModels, Physiome/ModelDB and Hugging Face connectors only where source terms and a concrete experiment support them.
- [ ] S3.7 Add donor/experiment-level split authority, data quality decisions and dataset-to-model impact tracking.

Exit per format: malformed/adversarial inputs rejected, exact raw bytes retained,
units/context/transformations testable and no automatic execution from data import.

## S4 — reusable experiment execution

- [x] S4.1 Freeze multi-model DAG specifications, exact model snapshots and explicit endpoint bindings.
- [x] S4.2 Dispatch eligible child runs through the existing bounded Worker; map final scalar output to a declared downstream initial parameter.
- [x] S4.3 Enqueue child and attach it to its experiment atomically; test concurrency, retries, rollback, cancellation and failure propagation.
- [x] S4.4 Expose register/submit/status/cancel/results/events through authenticated API/CLI; retain each child's manifest/result provenance.
- [x] S4.5 Add frozen batch/ensemble/sweep plans with aggregate budgets, deterministic reference seed allocation and capacity-bounded atomic dispatch across reference/native queues. E2 includes terminal statistics and explicit incomplete groups; native seed/parameter overrides are rejected.
- [x] S4.6 Add a durable external-engine agent protocol consuming admitted runtime/source identities, with no API Docker socket or host credentials. Actual COPASI SBML and OSPSuite PKML jobs pass model admission → claim → solver → verified API output.
- [x] S4.6b.1 Implement explicit native/reference scalar DAGs with declared interfaces, unit/context/semantic checks and atomic initial-condition derivation/dispatch. E3 executes COPASI/reference chains; PKML provides output-only interfaces.
- [ ] S4.6b.2 Extend the reviewed agent/flow interfaces to remaining native engines, PKML inputs and richer mappings; runtime probes alone do not provide queue adapters.
- [ ] S4.7 Add checkpoint/restart and engine capability negotiation; never claim unsupported state restoration.

Exit for the delivered subset: a real upstream result initializes the downstream
run exactly once and survives process/service reconstruction. The current mode is
`sequential_endpoint_transfer`; it is not simultaneous or feedback co-simulation.

## S5 — numerical coupling and biological bridges

- [x] S5.1 Check direction, semantics, species, units, shape and coordinates; reject unsupported feedback scheduling.
- [x] S5.2 Preserve an exact conserved two-compartment numerical primitive.
- [ ] S5.3 Define a stateful engine step/checkpoint contract, time ownership, event ordering and rollback across engines.
- [ ] S5.4 Implement feed-forward and feedback coupling with convergence/error control, stiffness handling, conservation and positivity checks.
- [ ] S5.5 Add scalar↔field↔agent mappings with lattice/geometry/time metadata and mass/volume normalization.
- [ ] S5.6 Define a separately versioned biological bridge with applicable cell/context, parameter evidence, process ownership and independent endpoints.
- [ ] S5.7 Verify coupling against an uncoupled/simple analytical baseline and step-size refinement before a biological test.

Exit: numerical tests plus held-out coupled endpoint evidence for the stated
context. EGF uptake/endocytosis already abstracted by vCornea must be replaced or
partitioned explicitly before adding EGFR kinetics; a matching unit is insufficient.

## S6 — scientific validation, measurement and uncertainty

- [x] S6.1 Distinguish synthetic verification, simulation comparison and experimental origin.
- [x] S6.2 Freeze datasets/groups/endpoints/tolerances and repeated-seed scalar protocols.
- [x] S6.3 Persist missing mechanisms/evidence as queryable knowledge gaps.
- [ ] S6.4 Implement versioned observation operators with sampling/noise assumptions and original-analysis comparison.
- [ ] S6.5 Establish independent evaluator identity and prevent author/candidate-controlled holdout changes.
- [ ] S6.6 Extend delivered within-case terminal mean/sample SD with sensitivity/identifiability and uncertainty by source; no confidence interval or parameter uncertainty is inferred by E2.
- [ ] S6.7 Expand trust into separate execution, numerical, biological, applicability and evidence axes.

For vCornea, [E1](epics/E1-vcornea-native-runtime.md) closes native operator engineering.
Published replay still needs exact scenario/parameters, original analysis, seed
policy, full horizon and frozen stochastic acceptance. Experimental scratch-control
means have a different context and do not close independent tissue validation.

## S7 — model improvement and optimization

- [x] S7.1 Implement a limited analytical decay-rate calibration proposal.
- [ ] S7.2 Register baseline/challenger versions, measured weakness and a frozen optimization objective.
- [ ] S7.3 Separate runtime performance patches from scientific mechanism/parameter changes.
- [ ] S7.4 Run historical benchmarks and held-out endpoints; retain negative results, computational cost and applicability limits.
- [ ] S7.5 Require independent promotion approval with rollback/revalidation impact; never let a candidate certify itself.

Corneal subset delivered: four-parameter bounded calibration, immutable candidate
descriptors, complete animal/dose exclusion, frozen objective/criteria, actual
isolated fit/full predictions and negative heldout/independent results. S7.2–S7.5
remain open for a general registry/promotion workflow; this failed candidate does
not approve itself or satisfy independent biological accuracy.

Exit: reproducible supported improvement or an interpretable negative result.
Changing pristine upstream source solely to make it look “optimized” does not pass.

## S8–S10 — specialist engines and biological scales

| Engine/model | Source state | Next execution task | Interaction gate |
|---|---|---|---|
| vCornea / CC3D 4.7.0 | Git and licensed archive preserved; native runtime executed | Full published scenario and original-analysis correspondence; general external agent | Existing EGF process ownership, lattice/time/observation mapping |
| COPASI | Complete Build-309 source | Installed 4.47.309 runtime; actual admitted SBML API/agent jobs and analytical checks pass; biological model benchmarks next | Compartment volumes, species units, reaction ownership |
| VCell | Complete 8.2.1.01 plus eight native solver sources | Original BioModel1 OMEX runs with real HDF5; general model/queue adapter next | Intracellular geometry and PDE/ODE boundaries |
| PhysiCell / BioFVM | Complete 1.14.2 and source-built MaBoSS | Native template and original 486-cell Boolean fixture execute; general agent/field exchange next | Agent state, substrate flux and phenotype ownership |
| Chaste | Complete 2026.1 source; matching official native build | Original ODE and both Delta/Notch monolayers pass; refinement/context-specific tissue evidence next | Mesh, mechanics/electrophysiology and conservation |
| NEURON | Complete 9.0.2 with recursive submodules | Actual native HH current clamp passes; general model/morphology admission next | Voltage/current/ion semantics and morphology |
| PK-Sim | Complete v12.3.173 source/dependencies | Original PKML executes through the durable API/agent queue; Windows project-authoring GUI remains separate | Exposure/PBPK compartments, populations and physiological context |
| MoBi | Complete v12.3.136 source/dependencies | Original Aciclovir PKML runs through API/agent; original schedule/units retained; building-block admission next | Building-block ownership and kinetic/physiological interfaces |
| GEARS | Complete pinned source, own CPU lock | Network forward/backward and real Dixit H5AD reader pass; full original screen/split/benchmark next | Cell type/training context; transcript changes do not directly define tissue effects |

These workstreams can progress independently. No engine must be forced into an
incompatible host environment, re-licensed under Peykar or marketed as a universal
biological library. Gene → protein → cell → tissue → organ links require evidence
at every bridge; source availability does not establish those links.

## S11 — governance and research integration

- [x] S11.1 Keep HomBon-specific behavior in the transport-neutral facade.
- [x] S11.2 Preserve local roles, append-only execution events and checksum provenance.
- [ ] S11.3 Implement scientific claim/conflict registries, five histories and L0–L3 change authority.
- [ ] S11.4 Add independent approval, signatures/audit retention and reproducible research export (RO-Crate/SED-ML where supported).
- [ ] S11.5 Expose capability/limitation-aware model search and experiment collection to research agents without delegating scientific approval to them.

## S12 — interactive product, backed by real operations

The complete product needs an interactive UI. Build each workspace once its
required operations and error states exist; do not wait for every biological engine.

- [x] S12.1 Shared application shell, accessible design system, authentication, localization/RTL and responsive layouts.
- [x] S12.2 Biological Explorer using versioned graph/neighborhood/evidence APIs; distinguish evidence links from runnable bridges.
- [x] S12.3 Model Studio with source/runtime/adapter/validity states and a port-compatible experiment editor.
- [x] S12.4a Dataset Studio with origin/license/lineage, declared groups and original-unit scalar observations/CSV imports.
- [ ] S12.4b Add quality adjudication and independent split authority after their backend contracts exist.
- [x] S12.5 Experiment Studio with real submit/estimate/cancel/dependency states; resume persisted experiments.
- [x] S12.6 Simulation Viewer with computed series/artifacts and explicit geometry limits; no invented cell motion.
- [x] S12.7 Validation Center with frozen benchmarks, failure explanations, uncertainty and missing evidence.
- [x] S12.8 Browser-test desktop/tablet/mobile, keyboard access, loading/empty/error/forbidden/degraded/conflict states and RTL.

E4 closes this bounded browser delivery for existing contracts; see
[workbench scope and deferred scientific authoring](epics/E4-research-workbench.md).
Advanced metadata/frozen definitions use validated JSON editors. Current unit/context
gates do not implement biological bridges, independent quality review or spatial UI.

Exit: complete end-to-end user workflows on real APIs, with no decorative actions
or unlabeled illustrative scientific state. Swagger remains developer tooling.

## S13 — operation, scaling and release

- [x] S13.1 Docker API/Worker lifecycle and offline Linux regression/package checks.
- [ ] S13.2 Recovery drills, metrics, quotas, durable logging and operator runbooks.
- [ ] S13.3 PostgreSQL queue leases/outbox and object-store implementation with migration/backfill/rollback evidence.
- [ ] S13.4 SLURM/batch/external runtime agents, aggregate budgets and reproducible worker capabilities.
- [ ] S13.5 Multi-user permissions/tenant isolation, deployment threat model and data retention.
- [ ] S13.6 Reproducible releases, compatibility matrix, installed-wheel/client tests and monitored deployment.

## Current delivery priority — user instruction, 2026-10-18

Complete corneal publication reproduction before starting another model/domain.
Two configuration families are the immediate target: stem-cell tissue formation
and injury/recovery. Recover original analysis, exact scenario/initial state, full
horizon and reference cohorts; freeze comparison endpoints before new results.
A failed comparison remains a failed result requiring investigation. Report full
completion only after these checks; engineering smoke tests do not close P1/P2.
The earlier independently advancing workstreams resume after this corneal gate.

- [x] Recover65 original replicates/174 scalar files, unchanged plot/membrane source
  and17 final-paper supplements; replay the original analysis and retain its figures.
- [x] Audit71 named table controls against Local and HPC source, preserving conflicts.
- [x] Restore the source-based HPC formation preset as a separate version; install
  the paper's stated4.6.0 engine version in an independently pinned environment.
- [x] Repair two invalid native lifetimes in private versioned overlays; execute
  actual observer equivalence and membrane-injury probes without editing upstream.
- [x] Retain two actual completed later-source formation runs and their negative
  frozen count/thickness findings; do not relax acceptance to force a match.
- [x] Recover all50 July2024 source files from original licensed archive objects;
  retain the separate source identity, unchanged HPC mechanics and original controls.
- [x] Add durable actual-child-exit proofs and resume only verified completed jobs;
  four old complete-looking injury outputs without exit evidence remain unpromoted.
- [x] Finish historical formation seeds7/11 through43200MCS, apply all frozen
  count/trajectory/thickness checks, replay original math and inspect actual layers.
  Both pass the formation subset; the full four-group publication gate remains open.
- [x] Finish six earlier injury retries/unstarted jobs in
  `.peykar/cornea-hpc46-resumed-20261023`; inspect current records/PIDs before starting
  anything. Each job has a24-hour deadline; preserve failed runs.
  All six genuinely completed; full baseline returns not_reproduced. Original math,
  actual structure/EGF and complete50401-sample chemical traces were inspected.
- [x] Apply the unchanged frozen reference comparison to every admitted full run, replay the
  authors' math on new outputs, inspect actual spatial/EGF/recovery results and retain
  negative findings. Real failed children remain unadmitted; the full scalar gate
  is negative and earlier spatial rejection is preserved. Analysis completion is
  not acceptance or an exact-final-table claim.
- [x] Inspect10 actual original VTK snapshots/three descriptors, retaining group
  differences: MildComb0 is4620h/pulse300h, moderate spatial data ends1058h and
  CONTROL4265h. These are not the selected full scalar cohorts or independent experiments.
- [x] Check actual ASCII field lengths rather than native reader allocation alone:
  retain the truncated original CONTROL final Pressure and missing fields, read only
  complete required data, and export original per-cell EGF sensing with all outliers.
- [x] Audit all56 archived Git commit metadata and the retained notebook. Preserve
  the January2025 stash formation fingerprint and local4.5.0 probe separately;
  neither establishes exact cohort identity or changes active runtime/parameters.
- [x] Resolve useful injury initial constraints from original VTK pressure/volume,
  identity/type/lineage and measured pulse coordinates:4/32/127/46 cells, conditional
  targets40/40/25/25 and[100,75]. Exact pre-sweep state/source/seed remains unknown.
- [x] Independently solve both initial target and lambda from all418 original
  cell observations under the stated common-parameter/pressure model; retain
  ambiguity/assumptions and separate spatial/scalar cohorts. Full-horizon figures
  now show actual final180-day state, with every earlier numeric row unchanged.
- [x] Prepare explicit `observed-injury-4.0.0` with complete historical mechanisms,
  separately evidenced archived PIFF and declared frozen overrides; actual600MCS
  native grid/pulse/lattice checks pass. Preserve the old-state mismatch and every
  source identity; this is a declared reconstruction, not unchanged old Git execution.
- [x] Bring all six derived injury jobs to actual terminal states and retain their
  evidence: five full exits/one native SIGABRT. The full combined analysis correctly
  stops at the failed child; the four slight/mild completed diagnostic is negative.
  Separate serial moderate retries completed but fail trajectory coverage. No
  failed child or mixture of successful source families is promoted to full acceptance.
- [ ] Establish independently applicable biological measurements; calibration ranges
  and the unrelated monolayer assay do not supply independent tissue validation.
- [x] Search Hugging Face and primary repositories, inspect actual pinned samples,
  preserve original experimental sources/terms and reject nonmatching endpoints.
  Retain90actual rabbit wound rows,22human OCT subject aggregates,HF22972metadata
  rows/202biomechanics eyes; record missing fourth rabbit and failed Dryad downloads.
- [x] Implement/test offline inert data normalization and actual lattice epithelial
  span/occupancy/gap observation; compare all four completed formation outputs
  descriptively with human clinical thickness without fitting or a passing claim.
- [ ] Establish homologous anatomical region and validated observation mapping
  between lattice vertical span and human OCT-normal thickness, then freeze an
  independent acceptance protocol with calibration-use/subject-level review.
- [x] Define a separately versioned projected circular-area mechanical-debridement
  component and offline protocol; use published initial diameter and independent
  kinetics, retain all actual PBS animals/times and individual errors. Frozen
  cohort-mean target passes (RMSE4.02pp/max5.86pp); this does not validate cell layers.
- [x] Acquire/parse a second independent removal-method study; freeze criteria
  before candidate execution, retain every method/time and categorical closure.
  Actual challenge fails all three methods; do not tune it into a passing holdout.
- [x] Execute a separately versioned empirical corneal learner with whole-dose/
  whole-animal exclusion; retain the improved but failed5.96pp heldout result,
  worse2021 transfer, independent72h conservative graph rejection, all cost/raw
  evidence and immutable source/algorithm/criteria. No global validity claim.
- [ ] Recover original rabbit sample-count/raw-unit context and 2021 subject-level
  data/per-time counts; current ratios cancel image scale but do not recover it.
  Keep PBS and silk treatment separate; never equate a2D fluid-access width to
  fluorescein surface area or infer a chemical dose conversion from severity names.
- [ ] Explain method-specific early migration/lag using independently evidenced
  initial substrate, wound geometry and injury/care conditions; identify suitable
  new calibration and untouched study-level validation data before fitting a new
  variant. The 2021 failure is evidence of a gap, not proof of a particular cause.
- [x] Recover the complete January2025 archive snapshot and retain its distinct
  HPC formation/injury mechanisms, controls and original initial states as
  `stash-hpc-5.0.0`; no silent state/mechanism mixing or changed acceptance criteria.
- [x] Check both published Zenodo versions; verify/materialize all26 earlier
  ordinary source files and compare all238 original object files byte-for-byte.
  No extra numeric controls/history were found; keep its distinct archive identity.
- [x] Finish its two full formation and six injury runs under explicit native-worker
  capacity checks, then retain the complete unchanged comparison/original math/
  spatial analysis: full frozen result negative, one-site formation mismatch
  rejected. New explicit serial formation2 also completed, reused all injury6,
  and passed saved-state checks; full comparison still five failures/three passes.
- [x] Finish and analyze the four explicit serial6/7 jobs; verify actual completion
  proofs, all retained artifact hashes, full chemical traces and interim pulse
  continuity. Preserve original math/structure/EGF/count figures with negative results.
- [ ] Recover exact expanded original controls, initial/checkpoint/RNG states,
  cohort/replicate mapping and native build for the four selected published groups.
  Both releases/history and all data member names were checked within their
  recorded scope; no missing original run file has been identified. Keep the
  observed same-seed divergence unresolved; no bitwise or paired-state claim.
- [ ] Complete an accepted full four-group reproduction using that recovered
  evidence and unchanged criteria. Current passing subsets do not close this gate.

See the [source/analysis inventory](../experiments/vcornea/publication/README.md)
and [operator commands](../runtime_profiles/vcornea-publication/README.md).

### Source reuse backlog (candidates, not implemented claims)

| Retained source family | Potential future use | Evidence/implementation needed after corneal gate |
|---|---|---|
| Cellular/tissue mechanics and diffusion engines | Retinal cell organization, epithelial layers and extracellular transport | A retinal-specific original model, cell types/geometry, units and independent endpoints; a corneal model is not transferred automatically |
| Neuronal/electrophysiology engine | Retinal neurons and circuit activity | Reviewed retinal morphology/channel/synapse model and original electrophysiological benchmarks |
| Molecular reaction/ODE engines | Retinal/corneal signaling pathways | Applicable pathway/cell context, kinetic parameters, original benchmark and explicit process ownership |
| Pharmacokinetic models | Ocular/systemic exposure and delivery | Ocular compartments, permeability/exposure data and validated tissue interface |
| Perturbation/transcriptomic tools | Cell-type-specific corneal or retinal hypotheses | Suitable tissue data, independent splits/accuracy and an evidenced molecular-to-cell bridge |

## Program acceptance and next concrete sequence

1. **Delivered in this continuation:** S1–S4 shared source/graph/workflow backends,
   full Linux verification and live deployment without resetting data.
2. Native computational profiles are installed/executed, and S4.6 now delivers
   actual COPASI/OSP model jobs and E3 scalar DAGs. Extend remaining native adapters/mappings (S4.6b.2),
   richer data admission (S3.5) and independent original benchmarks (S1.5).
   E6 delivers pinned biological vocabularies and contextual identifier annotations
   (S2.4); E7 delivers bounded multiparty processes and declared sample frames (S2.5).
   Scientific claim/model correspondence authority remains S2.6/S11; engine coupling stays S5.
   E5 now delivers bounded CellML 2 admission and reviewed operator ODE execution
   (S3.4b); its general queue/flow adapter remains part of S4.6b.2.
3. E4 now delivers the shared six-workspace UI on those APIs. Extend richer
   authoring and independent quality/split workflows as S3/S6/S11 add their contracts.
4. Add one explicit numerical coupling and its biological bridge (S5), then expand
   the remaining specialist profiles and workflows according to their dependencies.
5. Advance improvement, integrated organs and cross-organ endpoints only when their
   stated numerical/context/evidence gates pass.

All 55 source sections remain traceable in the existing coverage map; the original
24 scientific architecture scenarios remain acceptance targets, distinct from
passing software tests. Project completion requires the relevant EPIC gates, not
just a checked list of cloned repositories or a single working eye-model example.

## Original scientific milestones retained

The earlier P0–P7 gates still apply; S0–S13 reorganizes delivery across the whole
system rather than weakening or silently completing scientific requirements.

| Original phase | Scientific exit | Current EPICs |
|---|---|---|
| P0 | Two different verified synthetic engines and tested failure paths | S0,S4 |
| P1 | Reproduce selected published vCornea scenarios with frozen stochastic acceptance | S1,S6,S9 |
| P2 | Context-specific independent evidence report or explicit data gap | S3,S6 |
| P3 | Supported model improvement or interpretable negative result, independently reviewed | S7,S11 |
| P4 | Molecular/tissue coupling passes units, semantics, conservation, convergence and biological endpoints | S5,S8,S9 |
| P5 | Gene-to-tissue perturbation path with evidence/uncertainty at every bridge | S2,S5,S6,S8,S9 |
| P6 | Limited organ endpoint compared with a simpler baseline and suitable reference | S9,S10 |
| P7 | Defined multi-organ exposure/response interaction with independent evidence | S10,S11 |

Milestones A–G remain reproducible simulation, independent validation, supported
improvement, validated composition, cross-scale prediction, cross-organ simulation
and controlled scientific evolution. No atomistic whole-body model, universal
solver, unsupported cross-species transfer or automatic self-certifying promotion
is added by a source catalogue or a workflow graph.
