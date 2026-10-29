# Extraction protocol 1.0.0 — corrected visual acuity after treatment of macular disease

Frozen on 2026-10-29, before any visual-acuity value of an admitted study was read by
the model developer. It governs how published tables become records of schema
`eye.retina-acuity-record-1.0.0` (validated by
`peykar.validation.eye_retina_acuity_data`). Extractors make **no modelling
decisions**: no fitting, plotting, comparisons or trend summaries.

## Scope

How the best-corrected visual acuity (BCVA) of a group of eyes changes after a
macular disease is treated: macular edema (diabetic, retinal vein occlusion,
postoperative, uveitic), neovascular age-related macular degeneration (including
polypoidal choroidal vasculopathy), myopic choroidal neovascularization, central
serous chorioretinopathy and other macular conditions treated with intravitreal
anti-VEGF agents, corticosteroids, laser, photodynamic therapy or vitrectomy. Acuity
is the functional outcome of these treatments, the counterpart of the central retinal
thickness component (`retina-cst-2026-10-28`), whose vocabularies of indication,
treatment, regimen and prior treatment this protocol uses unchanged. Out of scope:
eyes without macular disease (cataract or refractive surgery alone), near acuity,
contrast sensitivity, microperimetry and reading speed.

## Inclusion

1. In vivo corrected visual acuity (BCVA, best spectacle-corrected acuity, ETDRS
   letter score at the protocol refraction) of treated eyes, reported for the same
   group at a **reference time** (before the first treatment of the reported study
   period) and at one or more later follow-ups. Uncorrected, pinhole and near acuity
   are not admissible; "VA" or "visual acuity" is admissible only when the Methods
   state that the acuity reported is the best-corrected acuity.
2. Units: group means (or per-eye values) in **logMAR**, or ETDRS letter scores.
   Letter scores are stored as printed with `value_kind` `letters_*` and converted by
   the record module with the ETDRS relation logMAR = (85 − letters) / 50. Means of
   Snellen fractions, decimal acuity or other non-logarithmic scales are not
   admissible; a logMAR row next to a Snellen or decimal row is admissible only when
   the article states that acuity was converted to logMAR per eye for the analysis.
   Approximate ETDRS letter scores derived from logMAR by the authors are stored as
   printed (`letters_*`), with a note.
3. Reference (time 0): the pretreatment acuity of the same eyes. A series without its
   reference value is excluded, never back-filled. A change from baseline
   (`logmar_change`, `letters_change`) is admissible only when the same series also
   reports its baseline acuity in the same unit, stored as `reference_value` with its
   `reference_cell`.
4. Group means (± SD or SE) only when no per-eye rows exist in the article's own
   tables (supplementary files are not acquired). Medians/IQR, ranges, proportions
   (for example eyes gaining 15 letters or reaching 20/40), lines gained, p-values,
   counts and model-adjusted estimates (least-squares or MMRM means) are not
   admissible.
5. Exclude untreated fellow eyes, healthy eyes and groups or analysis sets selected by
   a post-baseline outcome: "responders", "non-responders", eyes "with complete fluid
   resolution", eyes "that needed no rescue", eyes that completed every visit chosen
   after the outcome. Record them as excluded series.
   - Means over all treated eyes are kept whether missing visits were left out
     (observed cases, `missing_data` = `observed`) or imputed by last observation
     carried forward (`locf`); `unstated` when the article does not say. Ordinary
     attrition (fewer eyes at later visits) is kept, with a note.
   - An inclusion criterion requiring a minimum follow-up equal to (or longer than)
     the last tabulated visit, or a stated analysis set of the eyes seen at that
     visit, makes a complete-case set: excluded unless the article states that no
     treated eye of the group was left out. A minimum follow-up shorter than the last
     tabulated visit is kept with a note.
   - When values were removed or replaced after a post-baseline event (rescue
     treatment, switching agents, additional surgery), visits before the first such
     event that still have the full baseline number of eyes are kept and later visits
     are excluded. Otherwise the series is excluded.
   - Subgroups defined by a baseline characteristic (for example baseline acuity or
     thickness, prior treatment, lens status, disease subtype) are admissible as
     groups; when a table gives a whole cohort and its baseline subgroups, the
     subgroups are extracted, not the whole cohort.
6. Counting-finger, hand-motion or light-perception values converted to logMAR by the
   authors are kept as printed, with the convention in `notes`; the extractor never
   converts them.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded. Signs are kept as printed (a unicode
  minus is a minus). A cell that prints a Snellen or decimal value with its logMAR
  value in parentheses is cited with `"take": "parenthesized"`.
- Times are converted to months exactly: hours/730.5, days/30.4375,
  weeks × 7/30.4375, years × 12. Visits defined only by an injection count ("after
  the third injection"), ranges ("1–3 months"), "final visit" without a time and
  times counted from another event than the first treatment of the reported period
  are excluded unless the table states the time since that first treatment.
- `n` is the number of eyes at the reference time; changes in follow-up numbers go to
  `notes`. Retinal thickness and the number of treatments are not extracted here
  (they may be mentioned in `notes`).

## Regimen, prior treatment and missing data (per series)

- `regimen`: `single` (one injection, implant or treatment session in the reported
  period), `fixed_monthly`, `loading_then_prn`, `prn`, `treat_and_extend`,
  `fixed_interval` (another fixed schedule), `other`, or `unstated`.
- `prior_treatment`: `naive`, `previously_treated`, `mixed`, or `unstated`.
- `missing_data`: `observed`, `locf` or `unstated` (inclusion 5).

## Groups and context

- Indication (`indication`, per record and optionally per group): `dme`, `rvo`
  (branch, central or hemi-retinal vein occlusion), `namd` (including polypoidal
  choroidal vasculopathy and type 3 neovascularization; subtype in `notes`),
  `myopic_cnv`, `cme_postoperative`, `uveitic`, `csc`, or `other` (anything else in
  scope).
- Treatment (`treatment`, per record and optionally per group): `anti_vegf`
  (ranibizumab, aflibercept, bevacizumab, faricimab, brolucizumab, conbercept and
  biosimilars; agent in `groups[].agent`), `steroid_implant` (dexamethasone or
  fluocinolone implant), `steroid_injection` (intravitreal, sub-Tenon or
  suprachoroidal triamcinolone or another injected corticosteroid), `laser` (focal or
  grid, subthreshold micropulse or panretinal photocoagulation), `pdt` (verteporfin
  photodynamic therapy), `vitrectomy` (pars plana vitrectomy with or without membrane
  peeling), `topical` (eye drops), `combination` (two treatment types defining one
  arm, for example anti-VEGF plus laser or plus PDT), `observation` (untreated or sham
  arms), or `other`.
- Role: `control` = a standard treatment and standard care without an
  investigational agent, device or variant declared as the experimental arm;
  `treated` = the experimental arm. Two standard treatments compared without an
  investigational agent are both `control`, separate groups. Observation or sham arms
  are `control` with treatment `observation`. Patient subgroups defined by a baseline
  condition are `control` groups.
- Species: human only.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-retina-acuity-holdout-v1:`) are extracted by a separate custodian who reports
only structure and SHA-256 values until the evaluation protocol is frozen.
