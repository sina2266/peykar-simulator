# Retinal-thickness model 1.2 — development log

Development data: the 128 record studies of tranches 1 and 2 of dataset
`retina-cst-2026-10-28` (46 tranche-1 development records, 19 tranche-1 sealed records
consumed by the test of model 1.0.0, 63 tranche-2 sealed records consumed by the test
of model 1.1.0; 315 series). The tranche-3 reservation lists them as development
([`tranche-3/split-reservation-1.0.0.json`](../../../data/retina-cst-2026-10-28/tranche-3/split-reservation-1.0.0.json)).
No tranche-3 value is read before the 1.2 protocol is frozen. Studies of one
author-overlap cluster (`tranche-3/author-overlap-1.0.0.json`, 10 development
clusters) leave training together in leave-one-study-out (LOSO). Each round declares
its change and its adoption rule before its LOSO results are seen.

## Starting point and diagnostics (training data only)

Model 1.1.0 passed H1–H4 of its tranche-2 sealed test and failed H5: model 1.0.0 with
the same training forecast slightly better, but its intervals were too narrow.
Out-of-sample errors of 1.1 on all 128 training studies (its development LOSO on
tranche 1 plus its sealed report on tranche 2; `.peykar/retina12_diag_oos.py`,
`.peykar/retina12_diag_regimen_bias.py`, `.peykar/retina12_diag_device.py`; control
group means; signed error = forecast − observed, pp of the baseline thickness):

- Next-observation forecasts were unbiased (−0.02 pp, coverage 0.85/0.91); landmark
  forecasts nearly so (+0.06 pp).
- Initial-state forecasts (no follow-up seen) were 3.8 pp too low on average (too much
  predicted thinning; RMSE 12.57 pp), at every time after treatment. The bias differs
  by prior treatment (naive −3.1, previously treated −5.5, unstated −8.8, mixed
  −0.1 pp) and by OCT device (Spectralis −7.2 pp on 219 targets, the others between
  −2.7 and +2.4 pp).
- A least-squares fit of the last fraction at or after month 3 on the baseline
  covariate, indication, measure and device (139 control group means) gave Spectralis
  series +11.0 pp (SE 3.1) against Cirrus series; the other devices lay between +0.5
  and +8.6 pp. Published normal values of central thickness are similar for
  Spectralis and Cirrus (Wolf-Schnurrbusch et al., IOVS 2009), so device normals do not
  explain this; it may reflect the studies that use each device. A device level lets
  the data decide.
- Wear-off of one dexamethasone implant: 28 training series carry the dose rule, most
  with few visits; in-sample fits do not pin the duration down. No change is proposed.
- A point rule using the lognormal mean instead of the predictive median was tested
  for IOP model 1.3 (candidate d2) and rejected there: it removed the initial-state
  bias without lowering the RMSE and made the other tasks worse. It is not tried here.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.retina_thickness_population_1_2`; series
adapters `peykar.validation.eye_retina_data_1_1` (model 1.1's series and dose rule)
and `peykar.validation.eye_retina_data_1_2` (the same series with a sub-context);
operator `scripts/evaluate_retina_population_1_2.py`. With a sub-context, a context
`species/indication/treatment` becomes `species/<sub>/indication|treatment` and the
grouping is `("treatment", "context")`, so the hierarchy is species → indication and
treatment (as in 1.1) → sub-context.

## Round 1 — declared before results (2026-10-29)

- **e0**: model 1.1.0 (the frozen candidate `s5`, its series), the reference.
- **e3**: `s5` with a prior-treatment level (naive, previously treated, mixed,
  unstated): 53 control contexts on the training records.
- **e4**: `s5` with an OCT-device level (Spectralis, Cirrus, Topcon, Optovue, Stratus,
  other, unstated): 66 control contexts on the training records.

Adoption rule (as for model 1.1): a candidate replaces the incumbent only if, on human
group means, it improves the pooled RMSE of its primary method by at least 0.10 pp in
at least one task (initial state, next observation, landmark) and loses no more than
0.05 pp in any. The primary method is the lowest pooled group-mean next-observation
RMSE among the population variants whose group-mean coverage lies inside both H1
windows ([0.70, 0.90] and [0.80, 0.97]); ties within 0.05 pp go to the simpler variant
(Gaussian). If both e3 and e4 are adopted against e0, the one with the larger summed
gain over the three tasks is the incumbent and round 2 runs **e5**, with both levels
(prior treatment within device is not possible in two levels, so e5 nests device
within prior treatment: contexts `species/<prior>:<device>/indication|treatment`),
adopted if it meets the rule against that incumbent. Each run is split into four shards
(`--shard i/4`) and joined by `--merge`.

## Round 1 — results (2026-10-29)

LOSO over the 128 training studies, human group means (493 next-observation and
landmark targets, 565 initial-state targets); the selection rule gives the Gaussian
variant `population_assimilation` as the primary of every candidate. Pooled RMSE in
pp of the baseline thickness (`.peykar/compare_iop_12.py`):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| e0 (1.1.0) | 7.193 | 9.376 | 12.892 | 0.880 / 0.931 |
| e3 (prior treatment) | 7.162 | 9.407 | 12.945 | 0.878 / 0.933 |
| e4 (OCT device) | 7.196 | 9.381 | 12.818 | 0.880 / 0.933 |

Persistence: 8.337 (next observation) and 11.699 (landmark).

- **e3 rejected**: no gain of 0.10 pp (best −0.031, next observation) and the initial
  state is 0.053 pp worse.
- **e4 rejected**: its largest gain is 0.074 pp (initial state), below the 0.10 pp
  threshold.
- e5 is not run (it was declared only for the case that both are adopted). The
  out-of-sample biases by prior treatment and device that motivated e3 and e4 are
  mostly absorbed by shrinkage: the fitted between-sub-context variances are small
  and the series' own visits dominate once they are seen.

## Round 2 — declared before results (2026-10-29)

Diagnostic on e0's round-1 LOSO (training data only,
`.peykar/retina12_diag_regimen.py`; initial state, signed error = forecast −
observed): loading then PRN −5.8 pp (137 targets), treat and extend −5.9 (25), fixed
monthly −11.9 (10), other −6.9 (37), unstated −8.3 (10), against PRN −1.7 (202),
single dose −1.9 (127) and fixed interval +2.2 (17). The regimen is set before
treatment and decides how often the retina is re-treated.

- **e6**: `s5` with a regimen level: contexts `species/<regimen>/indication|treatment`
  (`eye_retina_data_1_2.to_retina_series_1_2_regimen`), grouping
  `("treatment", "context")`; 55 control contexts on the training records.

Adoption rule as in round 1, against e0. Run in eight shards (`--shard i/8`). If e6 is
rejected, model 1.2 has no adopted change: no 1.2 protocol is frozen, and tranche 3
stays sealed and unread for a later version.

## Round 2 — results (2026-10-29)

The first run of the eight e6 shards stopped soon after 13:03 CET, when the machine
hung; the hardware watchdog restarted it at 13:09 (reset report: boot faults
`wdog,reset_in_1`; cause not determined). The shards had written no report. They were rerun unchanged from 13:18 to 14:24 and
joined by `--merge` (`.peykar/retina-1.2-dev/round2-e6`). Pooled RMSE in pp of the
baseline thickness, human group means (`.peykar/compare_iop_12.py`; Gaussian primary
for both):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| e0 (1.1.0) | 7.193 | 9.376 | 12.892 | 0.880 / 0.931 |
| e6 (regimen level) | 7.181 | 9.412 | 12.922 | 0.882 / 0.933 |

- **e6 rejected**: its largest gain is 0.012 pp (next observation), below the 0.10 pp
  threshold, and the initial state is 0.030 pp worse.
- The regimen biases that motivated e6 remain almost unchanged out of sample
  (`.peykar/retina12_diag_regimen.py`, initial state, mean signed error e0 → e6):
  fixed monthly −11.93 → −11.76 pp (10 targets), loading then PRN −5.78 → −5.69
  (137), treat and extend −5.93 → −6.14 (25), PRN −1.70 → −1.92 (202), single dose
  −1.91 → −1.55 (127). The regimen level does not separate these groups once their
  studies are held out: the bias belongs to the held-out studies, not to the regimen.

**Model 1.2 has no adopted change.** No 1.2 protocol is frozen, and retina tranche 3
(67 records, 134 series) stays sealed and unread for a later version. Tranche 4 stays
unread.
