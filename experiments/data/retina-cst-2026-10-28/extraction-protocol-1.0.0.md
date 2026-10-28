# Extraction protocol 1.0.0 — central retinal thickness after treatment

Frozen on 2026-10-28, before any thickness value of an admitted study was read by
the model developer. It governs how published tables become records of schema
`eye.retina-thickness-record-1.0.0` (validated by
`peykar.validation.eye_retina_data`). Extractors make **no modelling decisions**:
no fitting, plotting, comparisons or trend summaries.

## Scope

How the central thickness of the retina (micrometres, measured by optical coherence
tomography, OCT) changes after a retinal or macular disease is treated: macular
edema (diabetic, retinal vein occlusion, postoperative, uveitic), neovascular
age-related macular degeneration, myopic choroidal neovascularization, central
serous chorioretinopathy and other retinal conditions whose treatment changes the
central thickness (for example epiretinal membrane surgery or panretinal laser).
Out of scope: macular thickness of eyes without retinal disease after cataract or
other surgery, retinal nerve fibre layer, ganglion cell and choroidal thickness,
macular volume, ring or sector subfields without the central value, and studies of
OCT devices or segmentation.

## Inclusion

1. In vivo central retinal thickness of treated eyes, by OCT, reported for the same
   group at a **reference time** (before the first treatment of the reported study
   period) and at one or more later follow-ups.
2. Reference (time 0): the pretreatment value of the same eyes. A series without a
   reference value is excluded, never back-filled. A change from baseline is
   admissible only when the same series also reports its baseline thickness
   (`thickness_change`, see below) or when it is a percent change.
3. Group means (± SD or SE) only when no per-eye rows exist. Medians/IQR, ranges,
   proportions, p-values, counts and model-adjusted estimates (least-squares or
   MMRM means) are not admissible.
4. Exclude untreated fellow eyes, healthy eyes and groups or analysis sets selected
   by a post-baseline outcome: "responders", "non-responders", eyes "with complete
   fluid resolution", eyes "that needed no rescue", means computed only over eyes
   free of an outcome at every visit. Record them as excluded series.
   - Means over all treated eyes are kept whether missing visits were left out
     (observed cases) or imputed by last observation carried forward; record which
     in `notes`. Ordinary attrition (fewer eyes at later visits) is kept, with a
     note.
   - When values were removed or replaced after a post-baseline event (rescue
     treatment, switching agents, additional surgery), visits before the first such
     event that still have the full baseline number of eyes are kept and later
     visits are excluded. Otherwise the series is excluded.
   - Subgroups defined by a baseline characteristic (for example baseline thickness
     above a cut-off, prior treatment or lens status) are admissible as groups.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded, or `Sheet!A1` of a spreadsheet.
- `value_kind`:
  - `thickness_absolute`: micrometres, normalized by the series' own baseline;
  - `thickness_change`: thickness − baseline in micrometres (baseline = 0),
    admissible only when the same series also reports its baseline thickness, stored
    as `reference_um` with its `reference_cell`;
  - `percent_change`: 100 × (thickness − baseline)/baseline (baseline = 0 %); its
    baseline thickness, when the same series reports it, is stored as `reference_um`
    with its cell. A percent *reduction* reported as a positive number is not
    transformed by the builder; such series are excluded.
- Times are converted to months exactly: hours/730.5, days/30.4375,
  weeks × 7/30.4375, years × 12. Visits defined only by an injection count ("after
  the third injection"), ranges ("1–3 months"), "final visit" without a time and
  times counted from another event than the first treatment are excluded unless the
  table states the time since the first treatment.
- `n` is the number of eyes at the reference time; changes in follow-up numbers go to
  `notes`. Visual acuity and the number of treatments are not extracted (no such
  series in schema 1.0.0); they may be mentioned in `notes`.

## Measure, device and regimen (per series)

- `measure`: `central_subfield` (mean thickness of the central 1-mm ETDRS subfield:
  CST, CSFT, CSMT, CSRT, "central 1 mm"), `central_point` (thickness at the foveal
  centre point), or `central_unspecified` (CMT, CRT, CFT or "central macular
  thickness" without a definition).
- `device`: `spectralis` (Heidelberg), `cirrus` (Zeiss Cirrus), `topcon` (3D OCT,
  Triton, Maestro), `optovue` (RTVue, Avanti), `stratus` (Zeiss Stratus time-domain),
  `other`, or `unstated`.
- `regimen`: `single` (one injection, implant or treatment session in the reported
  period), `fixed_monthly`, `loading_then_prn`, `prn`, `treat_and_extend`,
  `fixed_interval` (another fixed schedule), `other`, or `unstated`.
- `prior_treatment`: `naive`, `previously_treated`, `mixed`, or `unstated`.

## Groups and context

- Indication (`indication`, per record and optionally per group):
  - `dme`: diabetic macular edema;
  - `rvo`: macular edema after branch, central or hemi-retinal vein occlusion;
  - `namd`: neovascular age-related macular degeneration, including polypoidal
    choroidal vasculopathy and type 3 neovascularization (subtype in `notes`);
  - `myopic_cnv`: myopic choroidal neovascularization;
  - `cme_postoperative`: cystoid macular edema after cataract or other surgery;
  - `uveitic`: uveitic macular edema;
  - `csc`: central serous chorioretinopathy;
  - `other`: anything else in scope (for example proliferative diabetic retinopathy
    without centre-involving edema, epiretinal membrane, macular hole, radiation
    retinopathy).
- Treatment (`treatment`, per record and optionally per group):
  - `anti_vegf`: ranibizumab, aflibercept, bevacizumab, faricimab, brolucizumab,
    conbercept and biosimilars (agent name in `groups[].agent`);
  - `steroid_implant`: dexamethasone or fluocinolone implant;
  - `steroid_injection`: intravitreal, sub-Tenon or suprachoroidal triamcinolone or
    another injected corticosteroid;
  - `laser`: focal/grid, subthreshold micropulse or panretinal photocoagulation;
  - `pdt`: verteporfin photodynamic therapy, full or reduced dose/fluence;
  - `vitrectomy`: pars plana vitrectomy with or without membrane peeling;
  - `topical`: eye drops (NSAID, corticosteroid, carbonic anhydrase inhibitor);
  - `combination`: two treatment types defining one arm (for example anti-VEGF plus
    laser, or anti-VEGF plus a steroid);
  - `observation`: untreated or sham arms;
  - `other`: anything else in scope (oral drugs, novel treatments).
- Role: `control` = a standard treatment and standard care without an
  investigational agent, device or variant declared as the experimental arm;
  `treated` = the experimental arm. Two standard treatments compared without an
  investigational agent are both `control`, separate groups. Observation or sham
  arms are `control` with treatment `observation`. Patient subgroups defined by a
  baseline condition are `control` groups.
- Species is recorded; human studies are the expected case.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-retina-holdout-v1:`) are extracted by a separate custodian who reports only
structure and SHA-256 values until the evaluation protocol is frozen.
