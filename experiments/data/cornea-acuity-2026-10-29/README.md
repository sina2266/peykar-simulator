# Corrected visual acuity after keratoplasty and cross-linking — dataset 1.0.0 (2026-10-29)

Seventh eye component and fifth corneal one, after epithelial wound closure,
endothelial cell density, central corneal thickness and refraction: how the
corrected distance visual acuity (BCVA or CDVA, logMAR) of a group of eyes recovers
after penetrating keratoplasty, deep anterior lamellar keratoplasty, endothelial
keratoplasty (DSAEK, DMEK) or corneal collagen cross-linking. Acuity is the
functional outcome of these operations; it follows the graft, the endothelium, the
corneal thickness and the corneal shape. Extraction follows
[extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

Acuity in logMAR changes additively, like refraction. Records keep the acuity in
logMAR (or ETDRS letters, converted exactly); the model works on the change from the
preoperative acuity through the additive evaluation
(`peykar.validation.eye_additive_evaluation`).

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json`
   listed 1,994 open-access Europe PMC articles (one query on acuity terms in
   abstracts of keratoplasty or cross-linking studies, one on keratoplasty or
   cross-linking titles with acuity terms in the text) and retrieved 1,992 full
   texts; PMC13186544 and PMC5179223 returned HTTP 500. Summary and SHA-256 receipts:
   `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with an acuity topic, a time pattern and a
   corneal-surgery context pattern wrote masked previews of 1,185 tables in 780
   articles, with data cells hidden (`screening/table-screen-*.json`).
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It
   keeps tables whose caption, header or row labels mention corrected visual acuity
   (BCVA, CDVA, logMAR, ETDRS), a preoperative/baseline reference and at least two
   follow-up time labels, with no exclusion term in the caption (statistical models,
   reviews, agreement or normative studies, graft survival). Articles that another
   eye component keeps for a test it has not run yet are dropped before any triage
   (`screening/reserved.py`: retinal-thickness tranches 3–4 and IOP tranches 4–5, 294
   articles); none of them passed the rule. **175 articles passed.**
4. **Tranches.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-acuity-tranche-v1:`, size 120) ranks
   the 175 articles by a salted hash of their DOI: **tranche 1 (120 articles) is
   triaged now; tranche 2 (55 articles) stays untriaged and unread**, reserved as the
   fresh test set of the next model version.
5. Triage, the development/sealed split and the author-overlap rule follow the
   earlier components: four reviewer sub-agents see masked previews only
   (`screening/triage_chunks.py --chunks 4`, `.peykar/acuity-triage-brief.md`); the
   split is frozen with its own salt (`peykar-acuity-holdout-v1:`) before any value
   is read.

Records use schema `cornea.acuity-record-1.0.0`
(`peykar.validation.cornea_acuity_data`). Sealed studies are extracted by separate
custodians and stay private until the evaluation protocol is frozen.

No visual-acuity value of these articles had been read by the model developer when
the queries, the rule, the reserved list, the tranche order and the extraction
protocol were committed.
