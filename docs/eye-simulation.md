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
| `retina-thickness` | 1.1.0 (`s5`) | central retinal thickness (µm) | `human/<indication>/<treatment>` | tranche 2: H1–H4 pass, H5 fails ([README](../experiments/eye/retina/model-1.1/README.md)) |
| `iop` | 1.2.0 (`c1`) | intraocular pressure (mmHg) | `human/<procedure>` | tranche 3: all five hypotheses pass ([README](../experiments/eye/iop/model-1.2/README.md)) |
| `corneal-endothelium` | 1.0.0 | endothelial cell density (cells/mm²) | `<species>/<procedure>` | H1–H4 pass ([README](../experiments/vcornea/endothelium/README.md)) |
| `corneal-thickness` | 1.0.0 | central corneal thickness (µm) | `<species>/<procedure>` | H1–H4 pass ([README](../experiments/vcornea/thickness/README.md)) |
| `refraction` | 1.0.0 (`f1`) | spherical equivalent after laser refractive surgery (D) | `human/<refractive error>/<procedure>` | all five hypotheses pass on 13 series ([README](../experiments/eye/refraction/model-1.0/README.md)) |
| `acuity` | 1.0.0 (`a0`) | corrected distance visual acuity after keratoplasty or cross-linking (logMAR) | `human/<procedure>` (pk, dalk, dsaek, dmek, cxl, other) | H1–H4 pass on 21 series, mostly cross-linking and DMEK; failed H2/H3 on development ([README](../experiments/eye/acuity/model-1.0/README.md)) |

`retina-thickness` and `iop` run on the generic eye model version 2
(`peykar.model_learning.eye_recovery_population2`); their fits can be stored as JSON.
The two corneal components keep their own frozen model APIs and are fitted when a
forecast is asked for. `refraction` runs on generic eye model version 2 on the change
from the preoperative SE (an additive outcome) and is also fitted at forecast time; its
baseline is the preoperative SE and its values are SEs in dioptres. `acuity` works the same way
on logMAR: its baseline is the preoperative corrected acuity and its values are logMAR
(lower is better). Epithelial wound
closure is not included: its models apply to mouse debridement wounds only.

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
- A stored fit records the configuration and the SHA-256 of every record it used;
  loading refuses a fit made with another configuration or component.

## What the forecasts mean

The intervals are posterior predictive intervals of a group mean (or one eye). In the
sealed tests the group-mean next-observation intervals covered 71–89% (80% interval)
of the observed values, depending on the component. Per-series accuracy is limited;
for example, 47 of 92 retinal group means stayed within 5 percentage points RMSE and
10 points maximum for the next observation. Treatment effects are not modelled: a
treated arm uses the control prior of its context. See each component's README for
its failures.
