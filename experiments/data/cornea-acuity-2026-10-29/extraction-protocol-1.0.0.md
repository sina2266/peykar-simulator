# Extraction protocol 1.0.0 — corrected visual acuity after keratoplasty and cross-linking

Frozen on 2026-10-29, before any visual-acuity value of an admitted study was read by
the model developer. It governs how published tables become records of schema
`cornea.acuity-record-1.0.0` (validated by `peykar.validation.cornea_acuity_data`).
Extractors make **no modelling decisions**: no fitting, plotting, comparisons or trend
summaries.

## Scope

Recovery of corrected distance visual acuity after corneal surgery: how the best
spectacle-corrected (or corrected distance) acuity of a group of eyes moves from its
preoperative value over weeks to years after penetrating keratoplasty, deep anterior
lamellar keratoplasty, endothelial keratoplasty (DSAEK and its variants, DMEK and its
variants) or corneal collagen cross-linking. Acuity is the functional outcome of
these operations and follows the graft, the endothelium, corneal thickness and
corneal shape. In scope are human eyes operated for any corneal indication. Out of
scope: refractive laser surgery alone, cataract or lens surgery alone, glaucoma or
retinal surgery, keratoprostheses, amniotic membrane or limbal stem-cell procedures,
and intracorneal ring segments without cross-linking or keratoplasty. Combined
operations (keratoplasty with cataract surgery, cross-linking with a ring segment or
a refractive ablation) are in scope as declared groups.

## Inclusion

1. In vivo corrected distance visual acuity (BCVA, BSCVA, CDVA or "best-corrected
   visual acuity" with spectacles or manifest refraction) of operated eyes, reported
   for the same group at the **preoperative reference** and at one or more later
   follow-ups after surgery. Uncorrected acuity, pinhole acuity, contact-lens acuity
   and near acuity are not admissible.
2. Units: group means (or per-eye values) in **logMAR**, or ETDRS letter scores.
   Letter scores are stored as printed with `value_kind` `letters_*` and converted by
   the record module with the standard ETDRS relation logMAR = (85 − letters) / 50.
   Means of Snellen fractions, decimal acuity or other non-logarithmic scales are not
   admissible, because a mean of decimal acuities is not a mean of logMAR values; a
   table that prints decimal means with a logMAR conversion of each mean is
   admissible only if the logMAR column is itself a mean of per-eye logMAR values
   (stated), otherwise excluded.
3. Reference (time 0): the preoperative acuity of the same group, measured as stated.
   A series without its preoperative acuity is excluded, never back-filled.
4. Group means (± SD/SE) only when no per-eye rows exist. Medians/IQR, ranges,
   proportions reaching a threshold (for example 20/40 or better), lines gained,
   p-values, model-adjusted estimates and "change" columns without the reference are
   not admissible.
5. Exclude groups or analysis sets selected by a postoperative outcome ("clear
   grafts only", "eyes without rejection", "eyes that reached 0.3 logMAR", eyes with
   complete data at every visit chosen after the outcome), repeat grafts counted
   inside a primary-graft group, and unoperated fellow eyes. Record them as excluded
   series. Ordinary attrition (fewer eyes at later visits without an outcome-defined
   exclusion) is kept, with a note. Groups defined before surgery (indication, graft
   type, ocular comorbidity) are kept as separate groups.
6. Counting-finger, hand-motion or light-perception values converted to logMAR by the
   authors are kept as printed, with the convention in `notes`; the extractor never
   converts them.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded, or `Sheet!A1` of a spreadsheet.
  Signs are kept as printed (a unicode minus is a minus).
- `value_kind`: `logmar_absolute` or `letters_absolute` (acuity at every time,
  month 0 = preoperative), or `logmar_change` or `letters_change` (acuity minus the
  preoperative acuity, change at month 0 = 0; admissible only when the same series
  also reports its preoperative acuity, stored as `reference_value` with its cell, in
  the same unit).
- Times are converted to months exactly: hours/730.5, days/30.4375,
  weeks × 7/30.4375, years × 12. Ranges ("1–3 months"), "final visit" or "last
  follow-up" without a time and intraoperative times are excluded.
- `n` is the number of eyes at the preoperative reference; changes in follow-up
  numbers go to `notes`. Endothelial cell density, corneal thickness, refraction,
  keratometry or uncorrected acuity, when tabulated, are mentioned in `notes` only
  (no such series in schema 1.0.0).

## Groups and context

- Procedure context per group:
  - `pk`: penetrating keratoplasty (any trephination, including femtosecond-assisted).
  - `dalk`: deep anterior lamellar keratoplasty (big bubble, manual dissection, any
    other anterior lamellar technique leaving the host endothelium).
  - `dsaek`: Descemet stripping (automated) endothelial keratoplasty, DSEK,
    ultrathin and nanothin DSAEK.
  - `dmek`: Descemet membrane endothelial keratoplasty, PDEK, DMAEK and other
    Descemet-membrane grafts.
  - `cxl`: corneal collagen cross-linking with riboflavin and ultraviolet A
    (epithelium-off, epithelium-on, accelerated, pulsed or customized).
  - `other`: anything else in scope (for example a combined procedure that defines no
    single arm, or Descemet stripping without a graft).
- Indication per group: `fuchs` (Fuchs endothelial corneal dystrophy),
  `bullous_keratopathy` (pseudophakic or aphakic bullous keratopathy and other
  endothelial decompensation), `keratoconus` (including other ectasia),
  `graft_failure` (repeat keratoplasty), `scar_or_infection` (stromal scars,
  dystrophies, infectious keratitis), `mixed` (several indications reported together)
  or `unstated`.
- Combined cataract surgery per group: `combined` (`true` when the group had cataract
  surgery in the same session, `false` otherwise, `null` when unstated).
- Role: `control` = the study's standard procedure and standard care without an
  investigational agent, device or technique variant declared as the experimental
  arm; `treated` = the experimental arm. Two standard techniques compared without an
  investigational agent (for example DMEK versus DSAEK, or epithelium-off versus
  accelerated cross-linking) are both `control`, separate groups, each with its own
  procedure.
- Species: human only.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-acuity-holdout-v1:`) are extracted by a separate custodian who reports only
structure and SHA-256 values until the evaluation protocol is frozen.
