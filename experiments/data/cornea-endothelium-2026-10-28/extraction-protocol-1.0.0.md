# Extraction protocol 1.0.0 — corneal endothelial cell density after surgery

Frozen on 2026-10-28, before any value of this component was read. It governs how
published tables become records of schema `cornea.endothelial-density-record-1.0.0`
(validated by `peykar.validation.cornea_endothelium_data`). Extractors make **no
modelling decisions**: no fitting, plotting, comparisons or trend summaries.

## Inclusion

1. In vivo corneal endothelial cell density (ECD, cells/mm²) measured by specular
   or confocal microscopy, reported for the same group at a **reference time** and
   at one or more later follow-ups after a surgical procedure.
2. Reference (time 0): the preoperative ECD of the operated eye for cataract and
   other non-graft surgery; for keratoplasty the donor ECD (eye bank or
   preoperative donor count) as reported. The reference kind is recorded
   (`patient_preoperative`, `donor_eye_bank`, `donor_other`). A series without a
   reference value is excluded, never back-filled. A reported percent loss from
   the reference counts as defining the reference (value 100 %).
3. Group means only when no per-eye rows exist; medians/IQR, ranges, counts,
   p-values and "change" columns without the reference are not admissible.
4. Exclude studies of endothelial disease progression without surgery, ex vivo
   donor storage, cell culture, animal studies of endothelial injury without a
   time course, and contralateral/fellow-eye "control" eyes that were not
   operated (record them as excluded series).

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded, or `Sheet!A1` of a spreadsheet.
- `value_kind`: `ecd_absolute` (cells/mm², normalized by the series' own reference
  value), `percent_loss` (100 × (reference − ECD)/reference; reference = 0 %), or
  `ecd_change` (ECD − reference in cells/mm², reference = 0, admissible only when
  the same series also reports its reference ECD, stored as `reference_ecd` with
  its cell). Mean changes of the same eyes add exactly to the mean reference.
- Times are converted to months exactly: days/30.4375, weeks × 7/30.4375,
  years × 12. Ranges ("6–12 months") and "last visit" without a time are excluded.
- `n` is the number of eyes at the reference time; changes in follow-up numbers
  go to `notes`.

## Groups and context

- Procedure context: `pk` (penetrating keratoplasty), `dsaek` (DSEK/DSAEK,
  including ultrathin and non-Descemet-stripping), `dmek` (DMEK and DMEK
  variants), `dalk` (deep anterior lamellar; host endothelium), `cataract`
  (phacoemulsification, ECCE, MSICS, femtosecond-assisted, alone), `glaucoma`
  (trabeculectomy, tubes/shunts, MIGS, with or without combined cataract surgery
  when the glaucoma device/procedure defines the arm), `phakic_iol` (ICL,
  iris-fixated and other phakic lenses), `vitreoretinal` (vitrectomy with or
  without tamponade or combined phaco), `crosslinking` (corneal cross-linking),
  `other` (laser iridotomy, Descemet stripping only, anything else). Indication
  (Fuchs dystrophy, bullous keratopathy, keratoconus, regraft, cataract only,
  glaucoma type) is recorded.
- Role: `control` = the study's standard procedure and standard post-operative
  care without an investigational agent, device or technique variant declared as
  the experimental arm; `treated` = the experimental arm. Two standard techniques
  compared without an investigational agent are both `control`, separate groups.
  Patient subgroups defined by a condition (diabetes, pseudoexfoliation, Fuchs
  versus other indications, high preoperative pressure, nanophthalmos, a baseline
  ECD category) are `control` groups with the condition in `indication`; they are
  not experimental arms. Unoperated fellow or contralateral eyes are excluded.
- A study comparing procedures in different contexts (for example PK versus DALK,
  DSAEK versus DMEK) declares the procedure per group (`groups[].procedure`).
- Groups selected by a post-operative outcome (for example "ECL above 30 % at one
  month") are excluded.
- Species is recorded; human studies are the expected case.

## Sealed-test custody

As for the epithelial dataset: studies reserved for the sealed test by the frozen
reservation (salt `peykar-cornea-endothelium-holdout-v1:`) are extracted by a
separate custodian who reports only structure and SHA-256 values until the
evaluation protocol is frozen.
