# Extraction protocol 1.0.0 — central corneal thickness after surgery

Frozen on 2026-10-28, before any thickness value of this component was read by the
model developer. It governs how published tables become records of schema
`cornea.central-thickness-record-1.0.0` (validated by
`peykar.validation.cornea_thickness_data`). Extractors make **no modelling
decisions**: no fitting, plotting, comparisons or trend summaries.

## Scope

Corneal hydration after intraocular or endothelial surgery: central corneal
thickness (CCT, µm) swells after surgical stress and returns as the endothelial
pump recovers, or thins as an edematous cornea clears after endothelial
keratoplasty. Refractive tissue removal (LASIK, PRK, SMILE and other ablation or
lenticule extraction), corneal cross-linking, orthokeratology/contact-lens and
drug-only studies without surgery are out of scope.

## Inclusion

1. In vivo CCT measured by ultrasound pachymetry, anterior-segment OCT,
   Scheimpflug imaging, specular or confocal microscopy, reported for the same
   group at a **reference time** and at one or more later follow-ups after a
   surgical procedure. The measurement method is recorded per series.
2. Reference (time 0): the preoperative CCT of the operated eye (`patient_preoperative`).
   A series without a reference value is excluded, never back-filled. An
   intraoperative or "end of surgery" value is not a reference.
3. Group means only when no per-eye rows exist; medians/IQR, ranges, counts,
   p-values and "change" columns without the reference are not admissible.
4. Exclude unoperated fellow or contralateral "control" eyes, healthy volunteers,
   ex vivo donor corneas and groups selected by a postoperative outcome (for
   example "eyes with persistent edema"); record them as excluded series.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded, or `Sheet!A1` of a spreadsheet.
- `value_kind`: `cct_absolute` (µm, normalized by the series' own reference),
  `cct_change` (CCT − reference in µm, reference = 0, admissible only when the same
  series also reports its reference CCT, stored as `reference_cct` with its cell),
  or `percent_change` (100 × (CCT − reference)/reference; reference = 0 %).
- Times are converted to months exactly: hours/730.5, days/30.4375,
  weeks × 7/30.4375, years × 12. Ranges ("1–3 months"), "final visit" without a
  time and intraoperative times are excluded.
- `n` is the number of eyes at the reference time; changes in follow-up numbers go
  to `notes`.

## Groups and context

- Procedure context: `cataract` (phacoemulsification, ECCE, MSICS,
  femtosecond-assisted, alone), `dsaek` (DSEK/DSAEK, ultrathin, non-Descemet-
  stripping), `dmek` (DMEK and variants), `pk` (penetrating keratoplasty), `dalk`,
  `glaucoma` (trabeculectomy, tubes/shunts, MIGS, with or without combined
  cataract surgery when the glaucoma procedure defines the arm), `phakic_iol`,
  `vitreoretinal` (vitrectomy with or without tamponade or combined phaco),
  `other` (Descemet stripping only, laser iridotomy, anything else in scope).
  Indication (Fuchs dystrophy, bullous keratopathy, diabetes, glaucoma type,
  nucleus grade) is recorded.
- Role: `control` = the study's standard procedure and standard postoperative care
  without an investigational agent, device or technique variant declared as the
  experimental arm; `treated` = the experimental arm. Two standard techniques
  compared without an investigational agent are both `control`, separate groups.
  Patient subgroups defined by a condition are `control` groups with the condition
  in `indication`. A study comparing procedures in different contexts declares the
  procedure per group (`groups[].procedure`).
- Species is recorded; human studies are the expected case.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-cornea-thickness-holdout-v1:`) are extracted by a separate custodian who
reports only structure and SHA-256 values until the evaluation protocol is frozen.
