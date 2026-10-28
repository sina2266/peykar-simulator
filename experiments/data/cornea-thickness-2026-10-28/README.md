# Central corneal thickness after surgery — dataset 1.0.0 (2026-10-28)

Third corneal component, after epithelial wound closure and endothelial cell
density: how central corneal thickness (CCT, µm) swells after cataract and other
intraocular surgery and recovers, or thins as an edematous cornea clears after
endothelial keratoplasty. Thickness reflects corneal hydration, which the
endothelial pump controls. Refractive tissue removal, cross-linking and contact
lens studies are out of scope ([extraction protocol 1.0.0](extraction-protocol-1.0.0.md)).

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json
   --reuse-fulltext <endothelium screen>` retrieved 1,179 open-access Europe PMC full
   texts (313 reused byte-for-byte from the endothelium screen and rehashed; no
   failure). Summary and SHA-256 receipts: `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with a thickness topic, a time pattern and an
   eye-surgery context pattern wrote masked previews of 631 tables in 433 articles
   (data cells hidden): `screening/table-screen-*.json`.
3. [Triage 1.0.0](triage-1.0.0.json) used only those previews. The mechanical rule
   in `screening/triage_rule.py` kept tables whose caption, header or row labels
   mention corneal thickness, a preoperative/baseline reference and at least two
   follow-up time labels, with no exclusion term in the caption (85 articles).
   Manual review of their masked structure admitted **56** and excluded 29 with
   reasons (cross-linking and refractive surgery, perforation repair, no surgery,
   regression or baseline-only tables, medians, edema grades, missing DOI).
4. Before any thickness value was read, [the split was frozen](split-reservation-1.0.0.json)
   with its own salt (`peykar-cornea-thickness-holdout-v1:`): **37 development,
   19 sealed test**.

Many admitted articles also report endothelial cell density and appear in the
endothelium dataset; the two components' splits are independent. The model
developer has seen endothelium development values but no thickness value.
Records use schema `cornea.central-thickness-record-1.0.0`
(`peykar.validation.cornea_thickness_data`) and are built from cell-citing specs by
`scripts/build_thickness_record.py`. Sealed studies are extracted by separate
custodians and stay private until the evaluation protocol is frozen.

## Extraction outcome (2026-10-28)

Three development extractors wrote cell-citing specs (`development/specs/`) for
35 of the 37 development studies. Two had nothing admissible under protocol
1.0.0 and have no spec:

- `cct-pmc11852638` (glaucoma micro-shunt): CCT only as median (quartiles), not
  admissible under inclusion 3.
- `cct-pmc13159946` (DMEK): the table reports only pre-minus-post mean
  differences. The absolute means appear only in running text, which protocol 1.0.0
  cannot cite.

**33 development records with 69 series** were admitted: 50 control and 19
treated series, 58 group means and 11 single eyes. The 17 CC BY records are in
`development/records/`, with their source XML in `development/raw/`. The 16
records under non-commercial, share-alike or no-derivatives licences are kept
private, as in the endothelium dataset. Their specs are public; the specs cite
cells but copy no values.

Outcome-selected analysis sets: inclusion 4 also applies to whole analysis sets.
A series is excluded when the study computes its postoperative means only over eyes
without graft failure, repeat keratoplasty or another postoperative outcome. Such
means are conditional on the outcome being predicted. Ordinary attrition (fewer
eyes at later visits, without an outcome-defined exclusion) is kept, with a note.
This reading was fixed before the thickness evaluation protocol and before any
sealed value was read. Two development records were excluded on these grounds
after extraction:

- `cct-pmc8786756` (DMEK): eyes needing secondary keratoplasty were removed from
  every postoperative analysis. Its public record is kept in `development/excluded/`.
- `cct-pmc12882917` (DMEK): only eyes with confirmed graft integration and resolved
  edema at one year. Private.

Two records are kept with attrition notes: `cct-pmc12952605` and `cct-pmc6438261`.
The sealed custodians applied inclusion 4 on their own, according to their
structure-only reports. Custodian 1 excluded `cct-pmc10032283` (a cohort selected
for postoperative edema). Custodian 2 excluded `cct-pmc8180280` (clear-graft
subset). Two other sealed studies reported only medians or model-based estimates
(`cct-pmc10735023`, `cct-pmc10011236`). That leaves 15 sealed records.

Builder correction: `cell_number` in `cornea_thickness_data` now removes thin, hair
and no-break space thousands separators, so "1 087" reads as 1087. The pinned
`cornea_table_records.leading_number` is unchanged. After the correction, all 35
development specs and all 15 sealed specs rebuilt byte-identical records; sealed
records were compared by SHA-256 only. The `cct-pmc12488337` spec then added
patients 1 and 5. Their preoperative CCT is above 1,000 µm, and those cells could
not be parsed before the correction. The study now has 7 EndoArt eyes.
