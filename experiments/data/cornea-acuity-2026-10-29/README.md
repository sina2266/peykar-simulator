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

## Triage and split (2026-10-29)

[Triage 1.0.0](triage-1.0.0.json): four reviewer sub-agents read the masked previews of
the 120 tranche-1 articles (`.peykar/acuity-triage-chunks/`, chunks of 30) and gave a
reason for every decision; there was no owner override. **67 admitted, 53
excluded.** The exclusions were out-of-scope operations (refractive surgery, lens or
glaucoma surgery alone, keratoprostheses, stromal lenticule or xenograft implants),
uncorrected, Snellen-only or decimal-only acuity, medians, proportions, a single
postoperative time, baseline-only tables, reviews and outcome-selected cases. A first
triage attempt was cut off when the machine restarted (13:09 CET) and wrote no
decision; the four reviewers were run again from the same chunks, which were
regenerated and found identical.

Before any value was read, [the split was frozen](split-reservation-1.0.0.json) by
`scripts/reserve_acuity_split.py` (salt `peykar-acuity-holdout-v1:`, first 8 hex
digits of `sha256(salt + lower(doi))` modulo 3 equal to 0 → sealed): **47 development
and 20 sealed-test studies.** [Author overlap](author-overlap-1.0.0.json) found 7 pairs:
two development clusters leave training together in leave-one-study-out, and two
sealed studies each lose one linked development study from their training sets.

## Extraction outcome (2026-10-29)

Five development extractors (D1–D5) and three sealed custodians (C1–C3), all
sub-agents, followed `.peykar/acuity-extraction-brief.md` (assignments in
`.peykar/acuity-extraction-assignments.json`). After the first four agents (D1–D3,
C1), `.peykar/acuity-extraction-brief-addendum-1.md` fixed three readings the brief
had left open, written from their structural replies only: per-eye data that exist
only in a supplement do not displace the article's group means (supplements are not
acquired); a minimum follow-up equal to the last tabulated visit makes a complete-case
set, excluded unless the article says no operated eye was left out; and `combined` is
`null` unless the article states or rules out same-session cataract surgery. One
development study that D3 had left without a record under the first reading
(`acu-pmc6173378`) was then extracted by D5. Wave-1 records set `combined` to `false`
for cross-linking groups whose described operation includes no cataract surgery;
`combined` is not used by any model.

**Development: 27 records with 62 series**: 45 group means and 17 per-eye series (two
studies), 60 control and 2 treated; contexts cross-linking 21 series (12 studies),
penetrating keratoplasty 15 (4), DSAEK 12 (6), DMEK 9 (3), other 4 (4; cross-linking
combined with a ring segment or a refractive ablation) and DALK 1 (1). The 17 CC BY
records are in `development/records/` with their source XML in `development/raw/`;
the 10 records under non-commercial or publisher licences are private
(`.peykar/acuity-development-private/records/`). Specs (`development/specs/`) cite
cells and copy no values; three specs document exclusions only. Twenty development
studies have no record:

- complete-case analysis sets (the eyes that completed a long follow-up, out of more
  operated eyes): `acu-pmc10302271`, `acu-pmc10504154`, `acu-pmc10565913`,
  `acu-pmc11620334`, `acu-pmc12439834`, `acu-pmc12537206`, `acu-pmc12565262`,
  `acu-pmc12885941`, `acu-pmc13398343`, `acu-pmc13634280` (also restricted to
  successful grafts), `acu-pmc4624596`, `acu-pmc6438261`, `acu-pmc8203272`;
- analysis sets selected by a postoperative outcome (graft failure, complications,
  unmeasurable endothelium, steroid-induced hypertension, clear healing):
  `acu-pmc6174748`, `acu-pmc7338484`, `acu-pmc8157468`, `acu-pmc8665866`,
  `acu-pmc8675763`;
- Snellen or decimal means, or a logMAR row converted from decimal means:
  `acu-pmc8836535`, `acu-pmc9917846`.

**Sealed:** the custodians built **15 records with 25 series** (21 group means and 4
per-eye series; 24 control and 1 treated; contexts cross-linking 12, DMEK 8, other 3,
DSAEK 1, DALK 1; all logMAR) and reported only structure and hashes. Five sealed
studies were excluded: three complete-case sets (one also tabulating covariate-adjusted
means) and two whose acuity unit could not be established as logMAR.
`.peykar/verify_acuity_custody.py` rebuilt all 15 records byte for byte from their
specs and found every raw copy equal to its screened source.

No acuity value of these articles had been read by the model developer when the rule,
the tranche order, the extraction protocol, the triage and the split were committed
(commits `bc8fbb9b`, `3877950b`). Development records were then read for model
development; sealed records stay private and unread until the evaluation protocol is
frozen.
