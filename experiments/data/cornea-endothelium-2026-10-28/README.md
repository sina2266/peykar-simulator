# Corneal endothelial cell density after surgery — dataset 1.0.0 (2026-10-28)

Next eye component after the epithelial wound-closure work: how corneal
endothelial cell density (ECD, cells/mm²) falls after keratoplasty, cataract,
glaucoma, phakic-lens, vitreoretinal and other eye surgery. ECD decides graft
survival and corneal clarity, and it is reported in many open-access articles.
The search and split reuse the outcome-blind machinery of the epithelial dataset.

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json`
   screened 843 open-access Europe PMC full texts (841 retrieved). Summary and
   SHA-256 receipts: `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with an ECD topic, a months/years time
   pattern and a cornea/keratoplasty/cataract context pattern wrote masked
   previews of 679 tables in 425 articles (data cells hidden): `screening/table-screen-*.json`.
3. [Triage 1.0.0](triage-1.0.0.json) used only those previews: a mechanical rule
   kept tables with ECD, a reference label (preoperative, baseline, donor) and at
   least two follow-up time labels outside baseline/regression/review captions
   (101 articles); manual review of their masked structure admitted **87** and
   excluded 14 with reasons (meta-analyses, natural history without surgery,
   medians only, outcome-selected groups, one without DOI).
4. Before any ECD value was read, [the split was frozen](split-reservation-1.0.0.json)
   with its own salt (`peykar-cornea-endothelium-holdout-v1:`): **66 development,
   21 sealed test**.

Extraction follows [extraction protocol 1.0.0](extraction-protocol-1.0.0.md):
reference = preoperative patient ECD or donor ECD; values as absolute ECD, percent
loss or change from the cited reference; times in months; procedure contexts
`pk`, `dsaek`, `dmek`, `dalk`, `cataract`, `glaucoma`, `phakic_iol`,
`vitreoretinal`, `crosslinking`, `other`; standard-procedure groups and patient
subgroups are `control`, investigational arms `treated`. Records use schema
`cornea.endothelial-density-record-1.0.0` (`peykar.validation.cornea_endothelium_data`)
and are built from cell-citing specs by `scripts/build_endothelium_record.py`.
Sealed studies are extracted by a separate custodian and stay private until the
evaluation protocol is frozen.
