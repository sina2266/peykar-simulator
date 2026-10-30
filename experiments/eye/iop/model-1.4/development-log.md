# IOP model 1.4 — development log

Development data: the 308 record studies of tranches 1–4 of dataset
`iop-glaucoma-surgery-2026-10-28` (the 229 training studies of model 1.3 and the 79
tranche-4 sealed records consumed by its test; 779 series: 755 human, 22 rabbit, 2 dog).
The tranche-5 reservation lists them as development
([`tranche-5/split-reservation-1.0.0.json`](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-5/split-reservation-1.0.0.json))
and the 17 admitted tranche-5 articles as the sealed test of model 1.4; custodians hold
14 tranche-5 records (24 series) in `.peykar/iop-tranche5-sealed-2026-10-30/`. No
tranche-5 value is read before the 1.4 protocol is frozen. Studies of one author-overlap
cluster (`tranche-5/author-overlap-1.0.0.json`, 43 development clusters) leave training
together in leave-one-study-out (LOSO). Operator `scripts/evaluate_iop_population_1_4.py`;
candidates `peykar.model_learning.iop_population_1_4`. Each round declares its change and
its adoption rule before its results are seen.

## Starting point (consumed data only)

- Model 1.2.0 is the IOP component of `peykar eye`: it passed all five hypotheses of
  its tranche-3 sealed test and held H1–H4 as the reference of the 1.3 test on tranche
  4 (coverage 0.774/0.872; next observation 6.67 pp; landmark 8.00; initial state 10.69).
- Model 1.3's sub-procedure level failed its comparison with 1.2.0 on tranche 4: it
  helped after micropulse cyclophotocoagulation and PreserFlo, where the training
  diagnostics had shown a bias, and hurt after tubes and stents, where they had not
  (`../model-1.3/README.md`).
- Three sealed tests of forecast stacking with a fixed persistence weight improved the
  forecasts from follow-up twice and worsened the landmark forecasts once (retinal
  thickness 1.4, `../../retina/model-1.4/README.md`). Fixed-weight stacking is therefore
  not reused here. Scaling an interval on development forecasts did calibrate the
  next-observation intervals.

## Round 0 — reference and diagnostics, declared before results (2026-10-30)

- **d0**: model 1.2.0 (the frozen candidate `c1` of `iop_population_1_2`, its series),
  run by harness 2 over the 308 studies in 16 shards (`--shard i/16`) joined by
  `--merge`. It is the reference of every later round and of the declared H5.
- Its out-of-sample errors (human group means, signed error = forecast − observed, by
  procedure, sub-procedure as read by `eye_iop_data_1_3`, time since surgery and
  interval coverage) are the diagnostics from which round-1 candidates are declared.
  Each round-1 candidate and its adoption rule are written here before its LOSO run.
- Adoption rule for later rounds (as for models 1.1–1.3): a candidate replaces the
  incumbent only if, on human group means, it improves the pooled RMSE of its primary
  method by at least 0.10 pp in at least one task (initial state, next observation,
  landmark) and loses no more than 0.05 pp in any, with its next-observation coverage
  inside both H1 windows ([0.70, 0.90] and [0.80, 0.97]). The primary is chosen as in
  model 1.3: the lowest pooled group-mean next-observation RMSE among the population
  variants whose coverage lies inside both windows; ties within 0.05 pp go to the
  Gaussian variant.
- A 1.4 protocol tests H1–H4 as in model 1.3 and an H5 against model 1.2.0 from the
  same fit that names the tasks the adopted change targets. If no candidate is adopted,
  there is no 1.4 protocol and tranche 5 stays sealed.

## Round 1 — declared before the round-0 results (2026-10-30)

Written while the round-0 shards run; no LOSO number over the 308 studies has been seen.

- **d1s** (selective sub-procedure level): model 1.3's sub-procedure contexts
  (`species/<sub>/procedure` by the frozen keyword rules of `eye_iop_data_1_3`) and
  grouping `("treatment", "context")`, but only for the procedures selected by the rule
  below; every other procedure keeps one sub-procedure `all` (as model 1.3 did for
  procedures without rules). Everything else equals `c1`.
- **Selection rule**, applied mechanically to d0's round-0 out-of-sample errors
  (`.peykar/iop14_diagnostics.py`; initial states of human control group means, signed
  error = forecast − observed in pp, pooled over all months): among the sub-procedures
  of a procedure with keyword rules (tube, angle MIGS, cyclodestruction,
  subconjunctival MIGS), excluding `other`, keep those with at least 3 studies. A
  procedure is selected if the mean signed errors of two kept sub-procedures differ by
  at least 5 pp. Reason: model 1.3's split helped where such a difference existed in the
  training diagnostics (micropulse against continuous-wave cyclophotocoagulation,
  PreserFlo against XEN) and hurt where none existed (valved against non-valved tubes).
  If no procedure is selected, d1s equals d0 and round 1 has no candidate.
- **Adoption rule**: as declared in round 0 (at least 0.10 pp gain in one task, at most
  0.05 pp loss in any, coverage inside both windows), against d0.
- If d1s is adopted, the 1.4 protocol's H5 against model 1.2.0 from the same fit is:
  pooled initial-state RMSE of human control group means lower, and pooled
  next-observation and landmark RMSE each at most 0.05 pp higher.

## Round 0 — results (2026-10-30)

d0 run by harness 2 in 16 shards (`.peykar/iop-1.4-dev/round0-d0`, joined by `--merge`;
7,118–7,894 s per shard; report SHA-256 `d1f7f315…`). Out-of-sample errors of the
Gaussian primary on human group means (`.peykar/iop14_diagnostics.py`, output
`.peykar/iop14-diagnostics.txt`): next observation 6.55 pp (2,015 targets, 80%
coverage 0.79), landmark 8.23 pp (coverage 0.77), initial state 10.27 pp (2,263 targets,
mean signed error −1.46 pp).

Initial-state mean signed error by sub-procedure (studies, pp), for the procedures with
keyword rules:

| procedure | sub-procedures |
|---|---|
| angle MIGS | canaloplasty −3.02 (15), goniotomy −2.23 (52), stent −3.11 (27), trabeculotomy −0.00 (48), other −0.25 (3) |
| cyclodestruction | continuous −0.75 (10), endoscopic +6.07 (4), micropulse −10.58 (16), ultrasound −8.23 (4) |
| subconjunctival MIGS | PreserFlo +8.32 (6), XEN −1.74 (11) |
| tube | non-valved +0.62 (15), valved −0.03 (14) |

**The declared selection rule selects cyclodestruction** (largest difference between
kept sub-procedures 16.65 pp) **and subconjunctival MIGS** (10.06 pp); angle MIGS (3.11
pp) and tubes (0.65 pp) stay at one level. Other notable biases, not addressed by
round 1: the first postoperative week after trabeculectomy (−7.37 pp, 30 studies) and
after angle MIGS (−4.82, 76), the only laser-trabeculoplasty study (−24.47).

## Round 1 — the candidate as run (2026-10-30)

d1s as declared, with the selected procedures fixed by the rule above
(`iop_population_1_4.SELECTED_PROCEDURES["d1s"]` = cyclodestruction and subconjunctival
MIGS; reader `peykar.validation.eye_iop_data_1_4`, unit-tested to equal model 1.3's
reader when every rule procedure is selected and model 1.2's series otherwise). Its
series carry the same values as d0's (checked on the 779 training series). LOSO by
harness 2 in 16 shards, joined by `--merge`.

## Round 1 — results (2026-10-30)

d1s run by harness 2 in 16 shards (`.peykar/iop-1.4-dev/round1-d1s`, joined by `--merge`;
exit 0 for all shards). `.peykar/compare_iop_12.py round0-d0 round1-d1s`
(`.peykar/iop-1.4-dev/round1-compare.txt`), human group means; by the selection rule the
primary of both d0 and d1s is the Gaussian variant (`population_assimilation`):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| d0 (model 1.2.0) | 6.553 pp (2,015 targets) | 8.232 | 10.268 (2,263 targets) | 0.793 / 0.894 |
| d1s | 6.512 | 8.087 | 10.054 | 0.796 / 0.895 |

**d1s meets the adoption rule**: it gains 0.041 pp (next observation), 0.145 pp
(landmark) and 0.214 pp (initial state), loses in no task, and its coverage lies inside
both windows. The initial-state gain comes from the selected procedures:
cyclodestruction 13.16 → 11.65 pp (mean signed error −5.95 → −4.43) and subconjunctival
MIGS 9.45 → 9.09; every other procedure changes by at most 0.1 pp except `other`
(7.49 → 7.23) and the single laser-trabeculoplasty study (25.31 → 24.82).

## Freeze decision (2026-10-30)

Model 1.4.0 is **d1s** (model 1.2.0 with the sub-procedure level for cyclodestruction and
subconjunctival MIGS only), with primaries `population_assimilation` for U2 and U3 and
`population_initial_state` for U1, and model 1.2.0 (d0) as the reference scored in the
same sealed run. The publisher `scripts/publish_iop_population_1_4.py` (the 1.3 publisher
with the tranche-5 record sets and the H5 declared in round 1) gives on development
(`development-loso-2026-10-30/`, reference d0): H1 passes (1,603/2,015 = 0.796 and
1,804/2,015 = 0.895), H2 passes (6.51 against persistence 7.91, the last-two trend 38.34,
the pooled-curve increment 8.29 and analog kNN 9.71), H3 passes (8.09 against 14.02,
15.29 and 15.37), H4 passes (10.05 against the pooled training curve 16.87), and H5 passes
(initial state 10.05 against 10.27; next observation 6.51 against 6.55 and landmark 8.09
against 8.23, both within the 0.05 pp allowance). `.peykar/check_iop_redaction_1_4.py`
found no value of the 170 private studies in the public development files. The sealed
set has 6 series of the selected procedures (subconjunctival MIGS 5, cyclodestruction 1)
of its 24, so H5 rests mostly on procedures where d1s equals d0.
