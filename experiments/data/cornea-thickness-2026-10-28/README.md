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
