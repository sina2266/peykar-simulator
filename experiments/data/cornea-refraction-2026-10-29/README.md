# Refraction after corneal laser refractive surgery — dataset 1.0.0 (2026-10-29)

Sixth eye component and fourth corneal one, after epithelial wound closure,
endothelial cell density and central corneal thickness: how the spherical
equivalent (SE, dioptres) changes after photorefractive keratectomy and other
surface ablation, LASIK and SMILE or other lenticule extraction. The SE moves from
its preoperative value to near the target within days and then drifts over months
(regression or overcorrection) while the epithelium and stroma remodel. It is the
main refractive outcome of these operations. Extraction follows
[extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

Refraction changes additively (dioptres), unlike the five earlier components, whose
outcomes are fractions of the baseline. Records keep the SE in dioptres; the model
works on the change from the preoperative SE (see
`peykar.validation.cornea_refraction_data` and
`peykar.validation.eye_additive_evaluation`).

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json`
   listed 2,169 open-access Europe PMC articles (one query on refraction terms in
   abstracts of refractive-surgery studies, one on refractive-surgery titles) and
   retrieved 2,168 full texts; PMC8651116 returned HTTP 500. Summary and SHA-256
   receipts: `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with a refraction topic, a time pattern and a
   refractive-surgery context pattern wrote masked previews of 1,215 tables in 793
   articles, with data cells hidden (`screening/table-screen-*.json`). A first run
   without the `--time` argument used the script's default hour/day pattern; it was
   deleted unread and rerun.
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It
   keeps tables whose caption, header or row labels mention the spherical equivalent
   or manifest refraction, a preoperative/baseline reference and at least two
   follow-up time labels (hyphenated labels such as "3-month" included), with no
   exclusion term in the caption (statistical models, reviews, agreement studies,
   lens or intraocular surgery, keratoconus or ectasia, orthokeratology, ring
   segments, keratoplasty). **89 articles passed.** Most flagged tables that failed
   had a preoperative and a single postoperative column.
4. **Tranche.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-refraction-tranche-v1:`, size 120)
   ranks the 89 articles by a salted hash of their DOI; all fall in tranche 1. A
   later model version needs a new screen.
5. Triage, the development/sealed split and the author-overlap rule follow the
   earlier components: three reviewer sub-agents see masked previews only
   (`screening/triage_chunks.py`, `.peykar/refraction-triage-brief.md`); the split
   is frozen with its own salt (`peykar-refraction-holdout-v1:`) before any value is
   read.

Records use schema `cornea.refraction-record-1.0.0`
(`peykar.validation.cornea_refraction_data`). Sealed studies are extracted by
separate custodians and stay private until the evaluation protocol is frozen.
