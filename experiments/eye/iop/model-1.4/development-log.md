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
