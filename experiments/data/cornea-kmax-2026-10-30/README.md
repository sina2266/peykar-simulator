# Maximum keratometry after cross-linking or ring segments — dataset 1.0.0 (2026-10-30)

Ninth eye component and sixth corneal one, after epithelial wound closure, endothelial
cell density, central corneal thickness, refraction and corrected visual acuity: how
the maximum keratometry (Kmax, the steepest anterior corneal power, in dioptres) of a
group of eyes with keratoconus or corneal ectasia moves from its baseline value over
weeks to years after corneal collagen cross-linking (any protocol), after intracorneal
ring segments, after both, or under observation without treatment. Kmax is the main
measure of keratoconus progression and of the flattening these treatments aim at.
Extraction follows [extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

Kmax in dioptres changes additively, like refraction. Records keep Kmax in dioptres;
the model works on the change from the baseline Kmax through the additive evaluation
(`peykar.validation.eye_additive_evaluation`), with records of schema
`cornea.kmax-record-1.0.0` (`peykar.validation.cornea_kmax_data`, builder
`scripts/build_kmax_record.py`).

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json` listed
   1,147 open-access Europe PMC articles (one query on Kmax or keratometry terms in
   abstracts of cross-linking, ring-segment, keratoconus or ectasia studies, one on
   keratoconus, cross-linking, ectasia or ring-segment titles with Kmax terms in the
   text) and retrieved 1,147 full texts (full texts already downloaded by the acuity
   screen were reused after rehashing). A second round
   (`screening/queries-round2.json`: the same treatment and Kmax terms anywhere in the
   text) added 384 articles not screened before and retrieved 379 full texts. Receipts
   and summaries of both rounds: `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with a Kmax topic pattern, a time pattern and a
   keratoconus/cross-linking/ring-segment context pattern
   (`.peykar/run_kmax_table_screen.py`) wrote masked previews of 586 tables in 368 of
   the 1,526 articles, with data cells hidden (`screening/table-screen-*.json`).
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It keeps
   tables whose caption, header or row labels mention maximum keratometry (Kmax, K max,
   maximum or maximal keratometry, maximum K), a baseline reference and **at least one**
   follow-up time label, with no exclusion term in the caption (statistical models,
   reviews, agreement or normative studies, survival). Unlike the earlier corneal
   datasets, one follow-up time is enough: pre/post tables give the initial-state task
   and the population curve their late levels; series with two or more follow-ups give
   the forecast tasks as well. With two follow-up times 68 articles would pass. Articles
   that another eye component keeps for a test it has not run yet are dropped before
   any triage (`screening/reserved.py`: IOP tranche 5, 38 identifiers); none of them
   passed the rule. **102 articles passed.** The rule was settled by counting why
   flagged tables failed (`.peykar/kmax_rule_reasons.py`, counts only); widening the
   baseline labels changed nothing.
4. **Tranches.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-kmax-tranche-v1:`, size 120) ranks the
   102 articles by a salted hash of their DOI: all of them are in tranche 1. No later
   tranche exists, so after the sealed test below a new version needs a new screen.
5. Triage, the development/sealed split and the author-overlap rule follow the earlier
   components: three reviewer sub-agents see masked previews only
   (`screening/triage_chunks.py --chunks 3`, `.peykar/kmax-triage-brief.md`); the
   split is frozen with its own salt (`peykar-kmax-holdout-v1:`,
   `scripts/reserve_kmax_split.py`) before any value is read.

Sealed studies are extracted by separate custodians and stay private until the
evaluation protocol is frozen.

No Kmax value of these articles had been read by the model developer when the queries,
the rule, the reserved list, the tranche order and the extraction protocol were
committed.
