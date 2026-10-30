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

## Triage and split (2026-10-30)

[Triage 1.0.0](triage-1.0.0.json): `screening/triage_chunks.py --tranche 1 --chunks 4`
built four chunks of 30 articles (no decimal, no ± followed by a digit and no digit
followed by % outside the identifier lines). Four reviewer sub-agents followed
`.peykar/myopia-triage-brief.md` and saw masked previews only; there is no owner override
rule. **54 admitted, 66 excluded.** The exclusions were mostly axial-length changes
without the groups' baseline in any shown table (about 20), baseline-only tables (10),
model-adjusted values only (9), reviews and meta-analyses (9) and out-of-scope
populations (7: premyopic or non-myopic children, adults, an animal study, an unselected
school cohort); the rest were outcome-selected groups, medians only, a study protocol,
pooled randomized arms and tables without exact follow-up times.

[`split-reservation-1.0.0.json`](split-reservation-1.0.0.json)
(`scripts/reserve_myopia_split.py`, salt `peykar-myopia-axial-holdout-v1:`): **34
development and 20 sealed-test studies.**
[`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) (`scripts/link_study_authors.py`
over the Europe PMC search results of the screen): 5 linked pairs; 2 development clusters
(3 and 2 studies) leave training together, and one sealed study loses one linked
training study. Tranche 2 (94 articles) stays untriaged and unread.

## Extraction outcome (2026-10-30)

Four development extractors (D1–D4: 9, 9, 8 and 8 studies) and three sealed custodians
(C1–C3: 7, 7 and 6), all sub-agents, followed `.peykar/myopia-extraction-brief.md`
(SHA-256 7cb5022e…) with assignments `.peykar/myopia-extraction-assignments.json`
(SHA-256 99bd1107…). The brief included from the start the rules that the Kmax dataset
had needed an addendum for (nested follow-up cohorts) and asked development extractors
to write their decisions to a file. After D1 and D2, three readings were settled for D3,
D4 and the custodians, from the extractors' structural replies only: a retrospective
cohort whose inclusion criterion already requires the last tabulated follow-up and that
reports no number of children who started treatment is a complete-case set (excluded);
intention-to-treat means with the last observation carried forward are raw means; an
untreated, uncorrected fellow eye of a treated child may be its own group with treatment
`other`; a listed treatment plus a drug that is not a listed treatment is `other`.

**Development: 22 records with 59 series** (all group means): defocus spectacles 15
series (6 studies), orthokeratology 13 (10), single vision 13 (9), atropine 8 (6;
concentrations 0.01–1%), combinations 6 (5), other 3 (2) and red-light therapy 1 (1);
East Asia 47 series, Europe 8, South Asia 2, Southeast Asia 2; IOLMaster 43, Lenstar 7,
other 4, device not stated 5; 43 absolute and 16 change series; 19 series have one
follow-up visit and 40 have two or more, up to 11 years. The 12 CC BY records are in
`development/records/` with their source XML in `development/raw/`; the 10 records under
non-commercial or share-alike licences are private
(`.peykar/myopia-development-private/records/`). Specs (`development/specs/`) cite cells
and copy no values. Twelve development studies have no record; their reasons are the
extractors' own ([`development/no-record-reasons-1.0.0.json`](development/no-record-reasons-1.0.0.json)):
nine complete-case sets (eight retrospective cohorts restricted to children who reached
the last tabulated visit without a count of those who started, and one trial whose
completers were below 70% of each arm), two with model-adjusted estimates only, and one
analysis set from which a child was removed for fast progression.

**Sealed:** the custodians built **11 records with 23 series** (all group means;
atropine 6, single vision 5, orthokeratology 4, combinations 2, multifocal contacts 2,
red-light therapy 2, defocus spectacles 1, other 1; East Asia 16, Europe 6, Southeast
Asia 1; 18 series with two or more follow-up visits; 5 records in the public licence
class and 6 in the private class) and reported only structure and hashes. Nine sealed
studies have no record: seven complete-case sets, one analysis set with withdrawals by an
outcome-based rule, and one with least-squares estimates only.
`.peykar/verify_myopia_custody.py` (output `.peykar/myopia-custody-check.json`) found
every reserved sealed study assigned once and reported once, every screened source and
raw copy equal to its assignment hash, all 11 records rebuilt byte for byte from their
specs, no record outside a record decision, and no value-like key in the custody
reports. The developer did not open any custody report.

No axial-length value of these articles had been read by the model developer when the
rule, the tranche order, the extraction protocol, the triage and the split were
committed (commits `766a4ed4`, `fb4c0ef0`). Development records are now read for model
development; sealed records stay private and unread until the evaluation protocol is
frozen.
