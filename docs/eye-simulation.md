# Eye component forecasts (`peykar eye`)

`peykar eye` runs the frozen eye component models that were each tested once on a
sealed set. It turns a model into a usable forecaster: give a group's context, its
pretreatment value, its size and any follow-ups observed so far, and it returns the
model's tested primary forecasts with 80% and 90% intervals.

It is a research tool, not a clinical one. Every output states the component's frozen
version, its sealed-test result and its context of use.

## Components

| component | model | quantity (unit) | context | sealed test |
|---|---|---|---|---|
| `retina-thickness` | 1.1.0 (`s5`) | central retinal thickness (µm) | `human/<indication>/<treatment>` | tranche 2: H1–H4 pass, H5 fails; tranche 3 (reference of the 1.3 test, reported): H2–H4 hold, 80% coverage 0.903, just above its window; tranche 4 (reference of the 1.4 test, reported): H1, H2 and H4 hold, H3 fails narrowly (landmark 9.99 vs the pooled-curve increment 9.80 pp) ([README](../experiments/eye/retina/model-1.1/README.md)) |
| `iop` | 1.2.0 (`c1`) | intraocular pressure (mmHg) | `human/<procedure>` | tranche 3: all five hypotheses pass; tranches 4 and 5 (references of the 1.3 and 1.4 tests, reported): H1–H4 hold ([README](../experiments/eye/iop/model-1.2/README.md)) |
| `corneal-endothelium` | 1.0.0 | endothelial cell density (cells/mm²) | `<species>/<procedure>` | H1–H4 pass ([README](../experiments/vcornea/endothelium/README.md)) |
| `corneal-thickness` | 1.0.0 | central corneal thickness (µm) | `<species>/<procedure>` | H1–H4 pass ([README](../experiments/vcornea/thickness/README.md)) |
| `refraction` | 1.0.0 (`f1`) | spherical equivalent after laser refractive surgery (D) | `human/<refractive error>/<procedure>` | all five hypotheses pass on 13 series ([README](../experiments/eye/refraction/model-1.0/README.md)) |
| `acuity` | 1.0.0 (`a0`) | corrected distance visual acuity after keratoplasty or cross-linking (logMAR) | `human/<procedure>` (pk, dalk, dsaek, dmek, cxl, other) | H1–H4 pass on 21 series, mostly cross-linking and DMEK; failed H2/H3 on development; tranche 2 (reference of the 1.2 test, reported): H1–H4 hold ([README](../experiments/eye/acuity/model-1.0/README.md)) |

`retina-thickness` and `iop` run on the generic eye model version 2
(`peykar.model_learning.eye_recovery_population2`); their fits can be stored as JSON.
The two corneal components keep their own frozen model APIs and are fitted when a
forecast is asked for. `refraction` runs on generic eye model version 2 on the change
from the preoperative SE (an additive outcome) and is also fitted at forecast time; its
baseline is the preoperative SE and its values are SEs in dioptres. `acuity` works the same way
on logMAR: its baseline is the preoperative corrected acuity and its values are logMAR
(lower is better). Epithelial wound
closure is not included: its models apply to mouse debridement wounds only. Nor is the
best-corrected acuity after macular treatment: its model 1.0.0 failed all four
hypotheses of its sealed test
([README](../experiments/eye/retina-acuity/model-1.0/README.md)) and model 1.1.0 failed
four of five on a fresh set
([README](../experiments/eye/retina-acuity/model-1.1/README.md)), and only components
that passed are offered. A newer version replaces an offered one only when it passes
all its declared hypotheses, including the comparison with its predecessor, or when the
predecessor fails H1–H4 on the newer sealed set: retinal thickness 1.1.0 replaced 1.0.0,
whose intervals missed H1 on tranche 2, while IOP 1.3.0 and 1.4.0 failed their comparisons
with 1.2.0 ([1.3](../experiments/eye/iop/model-1.3/README.md),
[1.4](../experiments/eye/iop/model-1.4/README.md)) and 1.2.0 stays. Acuity 1.2.0
(model 1.0.0 with forecast stacking) improved both forecast tasks on tranche 2 but its
stacked initial state was worse than 1.0.0's from the same fit
([README](../experiments/eye/acuity/model-1.2/README.md)), so 1.0.0 stays. Retinal
thickness 1.3.0 (1.1.0 with forecast stacking) improved both forecast tasks on tranche
3 but failed H1 (80% coverage 0.911) and H5 (initial state 0.01 pp worse)
([README](../experiments/eye/retina/model-1.3/README.md)), so 1.1.0 stays. Retinal
thickness 1.4.0 (1.1.0 with stacked forecasts from follow-up and scaled intervals)
passed H1, H2 and H4 on tranche 4 but its landmark forecasts were worse than 1.1.0's
(10.58 against 9.99 pp) ([README](../experiments/eye/retina/model-1.4/README.md)), so
1.1.0 stays. Maximum keratometry (Kmax) after cross-linking or ring segments is not offered either: its model 1.0.0 passed H1 and H2 but failed H3 (landmark 12.60 against the pooled-curve increment 12.35 pp) and H4 (initial state 14.61 against the pooled training curve 14.17) ([README](../experiments/eye/kmax/model-1.0/README.md)).

Each component uses its own tested primary methods:

| component | no follow-up yet (U1) | with follow-ups (U2/U3) | point |
|---|---|---|---|
| `retina-thickness`, `iop` | Gaussian population prior | Gaussian prior updated by the follow-ups | predictive median |
| `corneal-endothelium` | Gaussian population prior | Student-t prior, level anchored to the follow-ups | predictive mean |
| `corneal-thickness` | Gaussian population prior | robust mixture prior updated by the follow-ups | predictive mean |
| `refraction` | Gaussian population prior | robust mixture prior updated by the follow-ups | predictive median |
| `acuity` | Gaussian population prior | robust mixture prior updated by the follow-ups | predictive median |

## Use

```bash
# Generic components: fit once to record folders, store the fit as plain JSON.
uv run --frozen peykar eye fit --component retina-thickness \
  --records experiments/data/retina-cst-2026-10-28/development/records \
  --output retina-fit.json

# Initial state: no follow-up observed yet (a dose rule applies to one implant).
uv run --frozen peykar eye forecast --component retina-thickness --fit retina-fit.json \
  --context human/dme/steroid_implant --baseline 500 --n 30 --months 1,2,4,6 \
  --regimen single --agent "dexamethasone intravitreal implant (Ozurdex)"

# Forecast from the follow-ups observed so far (month:value pairs).
uv run --frozen peykar eye forecast --component retina-thickness --fit retina-fit.json \
  --context human/dme/anti_vegf --baseline 450 --n 40 --months 6,12 --observed 1:360,3:340

# Corneal components: fit from record folders at forecast time.
uv run --frozen peykar eye forecast --component corneal-endothelium \
  --records experiments/data/cornea-endothelium-2026-10-28/development/records \
  --context human/cataract --baseline 2500 --n 40 --months 1,3,12

# Refraction: preoperative SE as the baseline, observed SEs in dioptres.
uv run --frozen peykar eye forecast --component refraction \
  --records experiments/data/cornea-refraction-2026-10-29/development/records \
  --context human/myopia/lasik --baseline -5.0 --n 40 --months 6,12 --observed 1:-0.1,3:-0.2

# Visual acuity: preoperative logMAR as the baseline, observed logMAR values.
uv run --frozen peykar eye forecast --component acuity \
  --records experiments/data/cornea-acuity-2026-10-29/development/records \
  --context human/dmek --baseline 0.9 --n 30 --months 3,12 --observed 1:0.4

uv run --frozen peykar eye components   # versions, methods, sealed results
```

- Record folders are given explicitly. The repository holds the CC BY records;
  records derived from NC/ND/SA-licensed articles stay on this machine (`.peykar/`)
  and can be added as further `--records` folders. A fit made from the public records
  alone uses fewer studies than the tested models did.
- Values are given and returned in the component's unit; fractions of the baseline
  are returned as well, except for `refraction` and `acuity`, which return the change
  from the preoperative value instead.
- `--reference-kind` (endothelium) says what the baseline count is:
  `patient_preoperative` (default) or a donor count (`donor_eye_bank`, `donor_other`).
- Warnings name what the sealed tests did not cover: a context without training
  series (the prior then comes from the species or family level), single eyes (no
  `--n`), and forecast months that are not after the last observed follow-up.
- Every forecast also reports `training_support`: the earliest and latest visit month
  of the control series that fitted the population, and the months of the context's
  own training series. It warns when an observed follow-up or a forecast month lies
  before the earliest training visit, where the early part of the law (a transient of
  one or two weeks) is not constrained by any data, and when a forecast month lies
  after the latest one. The retinal acuity model 1.0 failed its sealed test this way:
  no training visit before day 30, and one sealed study with a day-1 visit. Version 1.1
  failed the same way through its recovery term: its sealed forecasts from origins
  before the first training visit of their context had about twice the error of the
  others (next observation 6.04 against 2.78 letters). The check changes no forecast
  value. With the public development records the earliest training
  visit is day 1 for `iop`, the corneal components, `refraction` and `acuity`, and day
  7 for `retina-thickness`.
- A stored fit records the configuration, the SHA-256 of every record it used and its
  training visit months; loading refuses a fit made with another configuration or
  component. A fit stored before the visit months were recorded gives a note instead
  of the support check.

## What the forecasts mean

The intervals are posterior predictive intervals of a group mean (or one eye). In the
sealed tests the group-mean next-observation intervals covered 71–89% (80% interval)
of the observed values, depending on the component. Per-series accuracy is limited;
for example, 47 of 92 retinal group means stayed within 5 percentage points RMSE and
10 points maximum for the next observation. Treatment effects are not modelled: a
treated arm uses the control prior of its context. See each component's README for
its failures.
