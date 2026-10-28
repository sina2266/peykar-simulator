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

5. [Triage 1.0.0](triage-1.0.0.json) reviewed the masked structure of tranche 1
   (`screening/triage_chunks.py`, three chunks of 40). Three reviewer sub-agents
   followed `.peykar/retina-triage-brief.md`, saw only masked previews and gave a
   reason for every decision; there was no owner override. **78 admitted, 42
   excluded.** The exclusions were reviews and meta-analyses, medians only,
   model-adjusted or regression results, change without a baseline, a single
   follow-up time, baseline-only tables, groups selected by a post-baseline event,
   and thickness after cataract surgery in eyes without treated retinal disease.
6. Before any thickness value was read, [the split was frozen](split-reservation-1.0.0.json)
   by `scripts/reserve_retina_split.py` with its own salt
   (`peykar-retina-holdout-v1:`): **54 development, 24 sealed test**.
7. [Author overlap](author-overlap-1.0.0.json) (`scripts/link_study_authors.py`,
   at least two shared Europe PMC author names) found 8 pairs. Three development
   clusters leave training together in leave-one-study-out, and three sealed
   studies have linked development studies removed from their training set.

Reviewer notes kept for extraction: `ret-pmc6079526` is a corrigendum reproducing a
corrected table (its original article was not in tranche 1); several admits need
the statistic (mean or median), the time origin or the group definition checked in
the full text, and are excluded at extraction if they fail protocol 1.0.0.

Records use schema `eye.retina-thickness-record-1.0.0`
(`peykar.validation.eye_retina_data`) and are built from cell-citing specs by
`scripts/build_retina_record.py`. Sealed studies are extracted by separate
custodians and stay private until the evaluation protocol is frozen.

No thickness value of these articles had been read by the model developer when the
rule, the tranche order, the extraction protocol, the triage and the split were
committed.
