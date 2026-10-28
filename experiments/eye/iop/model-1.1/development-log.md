# IOP model 1.1 — development log (frozen with protocol 1.1.0)

Training and development data: the 72 tranche-1 records (47 development records and
the 25 sealed records consumed by the 1.0.0 test; 169 series). The sealed test of 1.1
is tranche 2 (74 records, extracted by custodians; the developer has seen structure
only). Every declaration below is written before the run it governs.

## Round 0 declaration: reference

Model 1.0.0 unchanged (frozen harness `eye_iop_evaluation`), leave-one-study-out over
the 72 records with the development clusters of `tranche-2/author-overlap-1.0.0.json`.
Its numbers are the reference that any 1.1 change must beat on group means.

## Round 0 result (1.0.0 reference, LOSO over 72 records, 380.2 s; report SHA 8ddcd757…)

Group means (123 series; 521 next-observation targets):

| variant | next obs RMSE | cov 80/90 | landmark RMSE | landmark cov 80/90 |
|---|---|---|---|---|
| Gaussian | 6.849 | 0.777/0.893 | 8.757 | 0.706/0.852 |
| t4 | 6.840 | 0.789/0.900 | 8.745 | 0.752/0.881 |
| robust | 6.880 | 0.787/0.904 | 8.850 | 0.747/0.887 |
| anchored | 7.992 | 0.810/0.908 | 13.456 | 0.762/0.862 |
| persistence | 8.05 | | 13.98 | |

Robust: 64/123 within 5/10 pp, 30 meet all criteria. Initial state, 117 control group
means: 11.43 pp (29 within) against the pooled curve's 19.30 (8 within). By target
time, next observation: first month (125 targets) 9.40 against persistence 12.40,
coverage 0.680/0.856; later (396) 5.86 against 6.07. Landmark coverage is lowest for
horizons of at most one month (0.659/0.833) and beyond 18 months (0.681/0.861).

## In-sample diagnostics behind 1.1 (training data only; [`diagnostics_1_1.py`](diagnostics_1_1.py))

- Best exponential fits of group means with at least four follow-ups leave
  systematic early residuals: angle MIGS −2.8 at day 1–3, +3.4 at 3 days–2 weeks;
  tubes +3.8 and −6.9 (×100 in ln f). Week-1 rises above both day 1 and month 1 occur
  in 8/30 angle-MIGS group means with all three visits. A fast transient with a fixed
  time constant tq, fitted on group means with at least five follow-ups (84 series),
  gives dof-adjusted residual SD 0.0755 without it and, with it: 0.0636 (tq = 3 d),
  **0.0621 (7 d)**, 0.0629 (14 d), 0.0656 (30 d), 0.0668 (61 d). The systematic early
  residuals of angle MIGS and tubes vanish at 7 days.
- The residual SD of group means falls with the number of eyes: `ln sd = −1.99 −
  0.31 ln n` (103 series); median SD 0.068 (n ≤ 20), 0.058 (20–40), 0.046 (40–80),
  0.034 (80–160).
- Residuals do not grow with time since surgery (RMS 0.060–0.087 in every bin), and
  consecutive residuals are negatively correlated (−0.29), so no random-walk drift
  term is supported for group means.

## Round 1 declaration (before the first 1.1 run)

Model 1.1.0 (`iop_population_1_1`): (a) the early transient `s·exp(−(t − t1)/tq)`,
with tq = 7 days fixed from the diagnostic above; (b) group-mean noise SD scaled by
`(n/40)^−g`, with g estimated in each fit (least squares over study estimates,
clipped to [0, 0.5]); (c) study noise from series with at least five follow-ups;
(d) Gamma(2) SD mode 0.1 for the transient at every level. Nothing else changes:
same variants, baselines, harness tasks and point forecast (predictive median).

Declared rules, written now:

- **Selection (group means; the context of use since the 1.0.0 freeze).** Among the
  five population variants, the primary is the one with the lowest pooled
  group-mean next-observation RMSE whose group-mean next-observation 80% and 90%
  coverages lie within [0.70, 0.90] and [0.80, 0.97]. Ties within 0.05 pp go to the
  simpler variant, in the order Gaussian, t4, robust, anchored, anchored_t4. Applied
  to round 0, this rule picks the 1.0.0 Gaussian variant (6.849, with t4 6.840 within
  0.05): reference 6.849 pp next observation, 8.757 pp landmark.
- **Adoption.** 1.1 replaces 1.0.0 if its selected variant improves the reference
  group-mean next-observation RMSE or landmark RMSE by at least 0.10 pp, neither is
  worse by more than 0.05 pp, and its group-mean next-observation coverage lies in
  both windows. Otherwise 1.0.0's structure is kept and the next round must give a
  reason.

## Round 1 result (model 1.1.0, LOSO over 72 records, 642.2 s; report SHA 02ec89d5…)

Public artifacts: [`development-loso-2026-10-28/`](development-loso-2026-10-28/tables.md)
(values of 38 studies without CC BY/CC0 terms are redacted).

Group means (123 series, 521 targets):

| variant | next obs RMSE | cov 80/90 | within / criteria | landmark RMSE | landmark cov 80/90 |
|---|---|---|---|---|---|
| Gaussian | **6.773** | 0.754/0.866 | 68 / 35 | **8.555** | 0.679/0.829 |
| t4 | 6.789 | 0.772/0.891 | 66 / 34 | 8.404 | 0.750/0.887 |
| robust | 6.832 | 0.772/0.891 | 66 / 33 | 8.446 | 0.758/0.894 |
| anchored | 8.209 | 0.783/0.879 | 55 / 20 | 13.387 | 0.737/0.841 |
| anchored_t4 | 8.381 | 0.795/0.889 | 55 / 20 | 13.384 | 0.789/0.912 |

Initial state, 117 control group means: 11.46 pp (31 within) against 19.30 for the
pooled curve. Next observation by target time (robust): first month 9.38 against
persistence 12.40 (coverage 0.664/0.856), later 5.80 against 6.07. Landmark (robust)
improves most for horizons of 1–18 months (6.91 against 7.21 in round 0).

Selection rule: Gaussian 6.773 pp; t4 6.789 lies within 0.05 pp, so the simpler
Gaussian is chosen; its coverage 0.754/0.866 lies inside both windows. Adoption rule
against the 1.0.0 Gaussian reference (6.849 / 8.757): landmark improves by 0.202 pp
(≥ 0.10), next observation by 0.076 pp, neither is worse, coverage inside both
windows, so **round 1 is adopted**. Primary U2/U3: population_assimilation (Gaussian);
U1: population_initial_state. No further model change before the freeze.

Left as they are (reported for the next version, not tuned now):

- First-month next-observation targets are under-covered (robust 0.664/0.856).
- The Gaussian primary's landmark 80% coverage is 0.679, below the window, which H3
  does not test.
- With the transient term, in-sample residuals in the first two weeks become small
  (RMS 0.019 at ≤3 days, 0.045 up to 6 weeks, 0.068–0.080 later), so the early
  dynamics are flexible in sample but vary between studies out of sample.

## Sealed-test hypotheses (declared before the freeze; human group means)

H1–H4 as for 1.0.0, with the 1.1 primary: H1 next-observation coverage in both
windows; H2 next-observation RMSE below persistence, last_two_log_linear,
pooled_curve_ratio and analog_knn; H3 landmark RMSE below persistence,
pooled_curve_ratio and analog_knn; H4 initial state below the pooled curve. New
**H5**: the 1.1 primary has lower pooled RMSE than the frozen 1.0.0 Gaussian variant
trained on the same 72 records, over common group-mean targets, for both next
observation and landmark. The operator scores 1.0.0 in the same sealed run.
