# Corneal thickness model — development log (frozen with protocol 1.0.0)

## Round 0 — in-sample kinetic-law comparison (development only), 2026-10-28

`round0_law_comparison.py` (this folder): three-parameter laws `ln f = ell + (s - ell) h(t)`
fitted by least squares over a 400-point tau grid to every development series with
at least four follow-ups; dof-adjusted residual SD of ln f (dof = n - 3):

| law | host group means (20 dof) | graft group means (5 dof) | graft single eyes (12 dof) |
|---|---|---|---|
| exponential | 0.0120 | 0.0205 | 0.1156 |
| hyperbolic (p = 1) | **0.0094** | 0.0213 | 0.1172 |
| power p = 0.5 | 0.0123 | 0.0215 | 0.1187 |
| power p = 2 | 0.0105 | 0.0212 | 0.1164 |

Adopted: hyperbolic return, parametrized by the day-1 excess `e` over the long-term
level (`r(t) = (1 + t1/tau)/(1 + t/tau)`, t1 = 1 day) to reduce the s–tau ridge.
Graft family pools the long-term level as absolute thickness (`ell = L - ln(CCT0/550)`),
a structural choice made from physiology (a cleared or replaced cornea returns towards
a normal thickness regardless of its preoperative edema), not fitted.

Smoke fit on all development controls (in-sample, 1.2 s): cataract long-term fraction
0.992, day-1 excess 0.124, median tau 4.7 days; graft family long-term 546 um,
DMEK 537 um, DSAEK 556 um, PK/DALK 545 um.

## Round 1 declaration (written before the first LOSO run)

- Model as above; variants scored: population_assimilation (Gaussian prior),
  _t4 (Student-t 4), _robust (0.2 broad mixture), population_anchored (flat level),
  population_anchored_t4, weak_prior (unpooled); baselines persistence,
  preoperative_level, last_two_log_linear, reference_to_last_log_linear,
  pooled_curve_ratio, analog_knn.
- Primary-selection rule for U2/U3, fixed now: among the five population variants,
  the primary is the one with the lowest pooled development next-observation RMSE
  whose pooled next-observation 80% and 90% coverages lie within [0.70, 0.90] and
  [0.80, 0.97]; ties within 0.05 pp go to the simpler variant in the order
  Gaussian, t4, robust, anchored, anchored_t4. U1 primary: population_initial_state.
- Any change after round 1 is recorded here with its reason before the next run.

## Round 1 result (LOSO, 33 studies, 59 s; `.peykar/thickness-development-loso-r1-2026-10-28`)

Next observation (66 series / 148 targets with population forecasts):
robust 4.70 pp (56 within 5/10, 38 meet criteria incl. persistence gain; coverage
80% 0.845, 90% 0.872), t4 4.79 (0.838/0.872), Gaussian 5.09 (0.811/0.865),
anchored 4.98, anchored_t4 5.03; persistence 6.70, analog 6.90, pooled ratio 9.23,
preoperative level 18.06. Landmark: robust 4.73, t4 4.85, Gaussian 5.22, anchored_t4
5.82; persistence 9.24. Initial state, 47 control group means: 4.41 pp (33 within)
vs pooled curve 11.87 (30). By family: host next-observation 1.87 pp (44/45 within),
landmark 1.91; graft next-observation 7.5–8.1 pp vs persistence 7.83 (single eyes and
EndoArt jumps dominate), graft initial state 6.60 vs pooled curve 23.42.

Diagnosis: per-series weak-prior moment summaries are uninformative about ln tau
for most graft and two-point series (SD 2–4 against the grid's uniform SD 3.2), so
their means sit near the weak prior's centre and pull the REML estimates of ln tau
(and e) toward an arbitrary point; two-point host series give curved ridges whose
moments place the level far from plausible values.

## Round 2 declaration (before running)

Change: REML uses likelihood-information summaries — the weak prior's precision and
information are subtracted from each series' posterior moments (standardized by the
weak prior SDs, eigenvalues clipped at 1e-3), so uninformative directions carry no
weight. Nothing else changes. Adoption rule: round 2 replaces round 1 if the variant
chosen by the primary-selection rule has pooled next-observation and landmark RMSEs
no worse than round 1's best (4.70 and 4.73 pp) by more than 0.05 pp; otherwise
round 1 is kept. Either way the primary is then selected by the round-1 rule.

Implementation note before the round-2 run: with eigenvalues merely clipped at
1e-3, uninformed directions got pseudo-means tens of their SDs away (for example
ln tau = +1042 with SD 98), which still carry weight in the REML residuals. To meet
the declared intent ("uninformative directions carry no weight"), directions with
information below 0.05 weak-prior precisions keep the posterior mean's component and
get variance 1/0.05 = 20 weak-prior variances; informed directions use eta/d.
In-sample fit after this: cataract median tau 3.6 days (round 1: 4.7), DMEK 2.6
days (9.5), DSAEK 67 days (33), PK 2.4 days (9.6); long-term levels essentially
unchanged (cataract 0.994, DMEK 535 um, DSAEK 552 um).

## Round 2 result (`.peykar/thickness-development-loso-r2-2026-10-28`, report SHA b7a5c08f…)

Next observation: robust 4.67 pp (56 within, 40 meet criteria; coverage 0.845/0.885),
t4 4.77, Gaussian 5.07 (58 within, 42 criteria), anchored 5.08, anchored_t4 5.10;
persistence 6.70. Landmark: robust 4.75 (44 criteria; coverage 0.858/0.912), t4
4.88, Gaussian 5.25; persistence 9.24. Initial state group means 4.28 pp (33/47
within) vs pooled curve 11.87; graft initial state 6.01 (round 1: 6.60). Host
next observation 1.86 pp (44/45 within, 36 criteria); graft 7.34 vs persistence 7.83.
Worst series: EndoArt case 1 (treated; a jump from 0.59 to 0.93 and back),
DSAEK below-median group (flat at 0.88–0.90 while the absolute-level prior expects
thinning), single-eye DSAEK of one patient.

Adoption rule: robust next observation 4.67 <= 4.70 + 0.05 and landmark 4.75 <=
4.73 + 0.05, so round 2 is adopted. Primary by the round-1 rule: robust (4.67;
t4 4.77 differs by more than 0.05 pp; coverage inside both windows). U1 primary:
population_initial_state. Round 2 is the reference development run; no further
model change before the freeze.
