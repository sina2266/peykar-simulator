# IOP after glaucoma procedures — model 1.4.0

Fifth version of the intraocular-pressure component ([model 1.0.0](../README.md),
[1.1.0](../model-1.1/README.md), [1.2.0](../model-1.2/README.md),
[1.3.0](../model-1.3/README.md)). It was developed on the 308 record studies of
tranches 1–4 (779 series) and tested once on **tranche 5**, reserved before triage,
triaged blind and extracted by custodians: 14 records, 24 series
([tranche-5 README](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-5/README.md)).
**Model 1.4.0 passed H1–H4 and failed H5, its declared comparison with 1.2.0**, so
`peykar eye` keeps model 1.2.0. Nothing here is a clinical or biological validation
claim.

## What changed from 1.2.0, and why

Model 1.3 nested a sub-procedure (device or technique, read by declared keyword rules)
in every procedure that had rules; on tranche 4 it helped where the training
diagnostics had shown a bias between sub-procedures and hurt where they had not.
Candidate **d1s** keeps the sub-procedure level only for procedures selected by a
mechanical rule declared before the diagnostics were read: two sub-procedures with at
least 3 studies whose mean signed initial-state errors under model 1.2.0 differ by at
least 5 pp. The rule selected **cyclodestruction** (micropulse −10.58 against endoscopic
+6.07 pp) and **subconjunctival MIGS** (PreserFlo +8.32 against XEN −1.74); tubes and
angle MIGS keep one level. Everything else is 1.2.0. Configuration
`peykar.model_learning.iop_population_1_4`, reader `peykar.validation.eye_iop_data_1_4`,
operator `scripts/evaluate_iop_population_1_4.py`, publisher
`scripts/publish_iop_population_1_4.py`, [development log](development-log.md).

## Development (leave-one-study-out over the 308 studies)

Human group means, pooled RMSE in pp of the preoperative IOP (2,015 forecast and 2,263
initial-state targets):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| d0 = model 1.2.0 | 6.553 | 8.232 | 10.268 | 0.793 / 0.894 |
| **d1s (adopted)** | **6.512** | **8.087** | **10.054** | 0.796 / 0.895 |

The declared H5 for the sealed test: initial-state RMSE lower than 1.2.0's from the same
fit, next-observation and landmark RMSE each at most 0.05 pp higher. It passed on
development.

## One-shot sealed test on tranche 5 (2026-10-30)

The test was run once, after freeze commit `dfd88dbb`, with protocol
[`protocol-1.4.0.json`](protocol-1.4.0.json) (SHA `a0499d76…`), all 69 pinned files and
the 308 training and 14 sealed record hashes matching. The operator scored each sealed
study with model 1.4 and, in the same run, with the frozen 1.2.0 (d0) as the declared
reference; for 5 sealed studies the training studies linked to them by shared authors
were left out of their fits. Exit 0, 5,427 s, reports `750348ec…` (1.4) and `8f9b1832…`
(1.2.0). Public artifacts are in
[`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`. Values of 5 of the 14 studies are redacted;
`.peykar/check_iop_redaction_1_4.py` found no value of them in the 10 redacted study
reports of the two public reports. Before the freeze the developer had opened one
custodian's custody report (structure, decisions and hashes; no IOP value), as the
tranche-5 README records.

Declared checks, human group means (20 forecast series, 72 targets; 23 control series
with 87 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | 55/72 = 0.764 and 65/72 = 0.903: **both inside** (windows 0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 8.34 pp against persistence 10.86, pooled-curve ratio 11.57, analog kNN 11.06 and last-two trend 54.22 |
| H3 landmark | **pass**: 9.14 pp against persistence 18.06, pooled-curve ratio 20.20 and analog kNN 16.26 |
| H4 initial state | **pass**: 9.46 pp against the pooled training curve's 15.59; 11/23 groups within 5/10 pp |
| H5 against 1.2.0 trained on the same studies | **fail**: next observation 8.34 against **8.28** pp (+0.057, allowed +0.05); landmark 9.14 against **8.95** (+0.182); initial state 9.46 against 9.57 (lower, as required) |

Per series, 9/20 group means were within 5/10 pp for the next observation and 3 of them
also at least 0.5 pp better than persistence; for the landmark task 9 and 4 (1.2.0: the
same counts).

Reported, not tested (`.peykar/iop14_sealed_breakdown.py`,
`.peykar/iop14_reference_analysis.py`):

- **Model 1.2.0 on the same sealed set** meets H1–H4 as well under the same declared
  analysis: coverage 0.792/0.903; next observation 8.28 pp against persistence 10.86;
  landmark 8.95 against 18.06; initial state 9.57 against its pooled training curve
  15.15. It has now held on tranches 3, 4 and 5.
- **One study decides H5.** Without one private-licence study that compared
  phacoemulsification with endoscopic cyclophotocoagulation (ECP) and with
  trabeculectomy, the differences would be +0.026 (next observation), +0.026 (landmark)
  and −0.025 pp (initial state), and H5 would pass; no other single study changes the
  verdict (figure, upper right). In its ECP arm, the only sealed cyclodestruction
  series, the endoscopic level lowered the initial-state error (9.10 against 9.97 pp)
  and raised the forecast errors (next observation 10.60 against 10.29; landmark 11.93
  against 10.60). The training data held four endoscopic studies.
- **Subconjunctival MIGS** (19 forecast targets): next observation 5.15 against 5.20 pp
  and landmark 8.00 against 7.98; initial state 9.01 against 9.10. PreserFlo (one study)
  improved from 5.90 to 5.01 pp and XEN (two studies) from 5.09 to 4.56 (figure, lower
  left).
- **A device name the frozen keyword rule missed.** The rule reads XEN with the pattern
  `\bxen\b`, which does not match "XEN45". One public XEN45 study
  ([PMC11204868](https://doi.org/10.3390/jcm13123406), medically controlled and
  uncontrolled subgroups) was therefore read as `other` subconjunctival MIGS, and its
  initial state missed by 16.8 and 8.7 pp (1.2.0: 16.3 and 9.1); IOP of the medically
  controlled eyes rose to 102% of the preoperative value at month 1 (figure, lower
  right). The training data may contain such arms too; a later reader needs tests on
  device names with numerals.
- **First-month next-observation coverage** (18 targets): 0.722/0.944 against 1.2.0's
  0.778/1.000.
- The sealed set has no single-eye series and no non-human series.

Tranche 5 is now consumed, as are tranches 1–4; the IOP screen of 2026-10-28 has no
unread article left. A later IOP version needs a new screen whose test part is reserved
before triage. The tests of 1.3 and 1.4 both point the same way: a sub-procedure level
helps the initial state where the training diagnostics show a bias, and moves the
forecasts of a single sealed study enough to decide a comparison with a 0.05 pp margin.
