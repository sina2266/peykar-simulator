# Extraction protocol 1.0.0 — intraocular pressure after glaucoma procedures

Frozen on 2026-10-28, before any IOP value of an admitted study was read by the
model developer. It governs how published tables become records of schema
`eye.iop-record-1.0.0` (validated by `peykar.validation.eye_iop_data`).
Extractors make **no modelling decisions**: no fitting, plotting, comparisons or
trend summaries.

## Scope

Aqueous outflow and production after a glaucoma procedure: how intraocular
pressure (IOP, mmHg) falls after surgery or laser and drifts back over months to
years. Drug-only studies without a procedure, retinal surgery and studies of IOP
measurement devices are out of scope.

## Inclusion

1. In vivo IOP of operated eyes reported for the same group at a **reference
   time** (preoperative) and at one or more later follow-ups after the procedure.
   The tonometer is recorded per series (`goldmann`, `rebound`, `noncontact`,
   `tonopen`, `pneumatonometry`, `other`, `unstated`).
2. Reference (time 0): the preoperative IOP of the operated eye. Record whether it
   was measured on the usual medication (`preoperative_medicated`), after washout or
   in untreated eyes (`preoperative_unmedicated`), or not stated
   (`preoperative_unstated`). A series without a reference value is excluded, never
   back-filled. An intraoperative or "end of surgery" value is not a reference.
3. Group means only when no per-eye rows exist; medians/IQR, ranges, counts,
   p-values, success proportions, Kaplan–Meier estimates and "change" columns
   without the reference are not admissible.
4. Exclude unoperated fellow eyes, healthy volunteers, and groups or analysis sets
   selected by a postoperative outcome: "successful eyes", "complete/qualified
   success" subgroups, eyes "without additional glaucoma surgery", eyes that needed
   needling, or means computed only over eyes free of an outcome at every visit.
   Record them as excluded series. Ordinary attrition (fewer eyes at later visits
   without an outcome-defined exclusion) is kept, with a note.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded, or `Sheet!A1` of a spreadsheet.
- `value_kind`: `iop_absolute` (mmHg, normalized by the series' own reference),
  `iop_change` (IOP − reference in mmHg, reference = 0, admissible only when the same
  series also reports its reference IOP, stored as `reference_iop` with its cell),
  or `percent_change` (100 × (IOP − reference)/reference; reference = 0 %).
- Times are converted to months exactly: hours/730.5, days/30.4375,
  weeks × 7/30.4375, years × 12. Ranges ("1–3 months"), "final visit" without a
  time and intraoperative times are excluded.
- `n` is the number of eyes at the reference time; changes in follow-up numbers go
  to `notes`. The number of glaucoma medications, when tabulated, is mentioned in
  `notes` only (no medication series in schema 1.0.0).

## Groups and context

- Procedure context:
  - `trabeculectomy`: with or without an antimetabolite, and with or without
    combined cataract surgery when the trabeculectomy defines the arm.
  - `tube`: Ahmed, Baerveldt, Molteno, Paul and other glaucoma drainage devices.
  - `subconjunctival_mims`: XEN, PreserFlo and other bleb-forming micro-shunts.
  - `angle_mims`: iStent, Hydrus, Kahook Dual Blade, GATT, ab interno
    trabeculotomy, goniotomy, canaloplasty and other angle procedures, alone or
    combined with cataract surgery.
  - `nonpenetrating`: deep sclerectomy and viscocanalostomy.
  - `cyclodestruction`: transscleral, micropulse or endoscopic
    cyclophotocoagulation, and cyclocryotherapy.
  - `laser_trabeculoplasty`: SLT, ALT and micropulse laser trabeculoplasty.
  - `cataract`: phacoemulsification or other lens surgery alone.
  - `other`: anything else in scope, such as iridotomy, a combined procedure that
    defines no single arm, or a novel procedure.

  Indication (POAG, PACG, pseudoexfoliation, neovascular, uveitic, pediatric,
  refractory, ocular hypertension, normal eyes) is recorded.
- Role: `control` = the study's standard procedure and standard postoperative care
  without an investigational agent, device or technique variant declared as the
  experimental arm; `treated` = the experimental arm. Two standard techniques
  compared without an investigational agent are both `control`, separate groups.
  Patient subgroups defined by a preoperative condition are `control` groups with
  the condition in `indication`. A study comparing procedures in different contexts
  declares the procedure per group (`groups[].procedure`).
- Species is recorded; human studies are the expected case.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-iop-glaucoma-holdout-v1:`) are extracted by a separate custodian who
reports only structure and SHA-256 values until the evaluation protocol is frozen.
