# Central retinal thickness after treatment — dataset 1.0.0 (2026-10-28)

This is the fifth eye component, after epithelial wound closure, endothelial cell
density, central corneal thickness and intraocular pressure after glaucoma
procedures. It covers how the central thickness of the retina (micrometres, by OCT)
falls after macular edema, neovascular AMD and related conditions are treated with
anti-VEGF injections, steroid implants or injections, laser, photodynamic therapy or
vitrectomy, and how it can return over months. Central thickness reflects fluid in
and under the retina, so it is the usual anatomical outcome of these treatments.
Extraction follows [extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

The component was chosen by data availability: open-access Europe PMC queries for
treated central retinal thickness returned 1,938 and 2,250 hits, against 67 for
corneal nerve regeneration and 181 for retinal nerve fibre layer thinning.

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json`
   listed 3,128 open-access Europe PMC articles and retrieved 3,124 full texts. Four
   downloads returned HTTP 500: PMC11238598, PMC11764373, PMC8005793 and PMC8898177.
   Summary and SHA-256 receipts are in `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` was run with a central-thickness topic, a time
   pattern and a retinal-treatment context pattern. It wrote masked previews of 2,548
   tables in 1,638 articles, with data cells hidden (`screening/table-screen-*.json`).
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It
   keeps tables whose caption, header or row labels mention central retinal
   (macular, foveal, subfield) thickness, a pretreatment/baseline reference and at
   least two follow-up time labels other than time 0, with no exclusion term in the
   caption. **395 articles passed.**
4. **Tranches.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-retina-tranche-v1:`) ranks the 395
   articles by a salted hash of their DOI. Tranche 1 holds the first 120 and is used
   now for development and the first sealed test. Tranches 2–4 (275 articles) stay
   **untriaged and unread**, reserved for testing later model versions on studies
   no model has seen.

Next, before any thickness value is read: blind triage of tranche 1 from masked
previews, then a split frozen with its own salt (`peykar-retina-holdout-v1:`) into
development and sealed-test studies. Records use schema
`eye.retina-thickness-record-1.0.0` (`peykar.validation.eye_retina_data`) and are
built from cell-citing specs by `scripts/build_retina_record.py`. Sealed studies are
extracted by separate custodians and stay private until the evaluation protocol is
frozen.

No thickness value of these articles had been read by the model developer when this
file, the rule, the tranche order and the extraction protocol were committed.
