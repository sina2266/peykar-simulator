# Axial length in childhood myopia, treated or not — dataset 1.0.0 (2026-10-30)

Tenth eye component: how the ocular axial length (AL, millimetres) of a group of myopic
children or adolescents grows from its baseline value over months to years under
single-vision correction or a myopia-control treatment (atropine eye drops,
orthokeratology, myopia-control spectacle lenses, multifocal soft contact lenses,
repeated low-level red-light therapy, combinations, outdoor time or another non-surgical
treatment). Axial elongation is the main measure of myopia progression and of the
slowing these treatments aim at. Extraction follows
[extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

Axial length changes additively. Records keep axial length in millimetres; the model
will work on the change from the baseline axial length through the additive evaluation
(`peykar.validation.eye_additive_evaluation4`), with records of schema
`eye.myopia-axial-record-1.0.0` (`peykar.validation.myopia_axial_data`, builder
`scripts/build_myopia_record.py`). One reported pp is 0.01 mm.

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json` listed
   1,004 open-access Europe PMC articles (one query on axial-length terms in abstracts
   of myopia-control or myopia-progression studies of children with a time term, one on
   myopia-control or progression titles with axial length in the text) and retrieved
   1,004 full texts. Receipts and summary: `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with an axial-length topic pattern, a time pattern
   and a myopia context pattern (`.peykar/run_myopia_table_screen.py`) wrote masked
   previews of 1,190 tables in 568 of the 1,004 articles, with data cells hidden
   (`screening/table-screen-*.json`).
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. A table
   qualifies when its caption, header or row labels mention axial length (axial length,
   AL, AXL, axial elongation or growth), a baseline reference and at least one follow-up
   time label, with no exclusion term in the caption (statistical models, reviews,
   agreement or normative studies, survival, prevalence, incidence). Because many myopia
   trials give the baseline axial length in a characteristics table and its changes in
   another, an article also qualifies when one table gives axial-length changes at a
   follow-up time and another gives the baseline axial length. The second criterion and
   two baseline labels ("basic value", "basal") were added after counting why flagged
   tables failed (`.peykar/myopia_rule_reasons.py`, counts only) and reading the masked
   labels of tables that failed only on their baseline label; no value was shown.
   Articles that another eye component keeps for a test it has not run yet are dropped
   before any triage (`screening/reserved.py`: IOP tranche 5, 38 identifiers); none of
   them passed the rule. **214 articles passed**: 186 with one table, 28 by a change
   table and a baseline table.
4. **Tranches.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-myopia-axial-tranche-v1:`, size 120)
   ranks the 214 articles by a salted hash of their DOI: **120 are in tranche 1**, which
   is triaged and split now, and **94 in tranche 2**, which stays untriaged and unread,
   reserved as fresh test data for the next model version.
5. Triage, the development/sealed split and the author-overlap rule follow the earlier
   components: reviewer sub-agents see masked previews only
   (`screening/triage_chunks.py`), and the split is frozen with its own salt
   (`peykar-myopia-axial-holdout-v1:`) before any value is read.

Sealed studies are extracted by separate custodians and stay private until the
evaluation protocol is frozen.

No axial-length value of these articles had been read by the model developer when the
queries, the rule, the reserved list, the tranche order and the extraction protocol
were committed.
