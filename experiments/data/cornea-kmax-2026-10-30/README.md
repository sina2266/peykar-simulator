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

## Triage and split (2026-10-30)

[Triage 1.0.0](triage-1.0.0.json): `screening/triage_chunks.py --tranche 1 --chunks 3`
built three chunks of 34 articles (no decimal, no ± followed by a digit and no digit
followed by % outside the identifier lines). Three reviewer sub-agents followed
`.peykar/kmax-triage-brief.md` (SHA-256 25911943…) and saw masked previews only; there
is no owner override rule. **62 admitted, 40 excluded.** The exclusions were out-of-scope
treatments (keratoplasty, refractive surgery or ablation without cross-linking,
laser asymmetric keratectomy, phakic lenses, ring segments in non-ectatic corneas,
lenticule implants, wedge resection), only K1/K2/Kmean or a change without its
baseline, medians only, model-adjusted means, baseline-only tables, reviews and
meta-analyses, a follow-up given only as a range, and outcome-selected eyes or single
cases.

[`split-reservation-1.0.0.json`](split-reservation-1.0.0.json)
(`scripts/reserve_kmax_split.py`, salt `peykar-kmax-holdout-v1:`): **40 development
and 22 sealed-test studies.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json)
(`scripts/link_study_authors.py` over the Europe PMC search results of both screening
rounds): 11 linked pairs; 7 sealed studies lose linked training studies, one pair
links two sealed studies, and 2 development clusters leave training together.

## Extraction outcome (2026-10-30)

Four development extractors (D1–D4, 10 studies each) and three sealed custodians
(C1–C3: 8, 7 and 7 studies), all sub-agents, followed `.peykar/kmax-extraction-brief.md`
(SHA-256 536059c9…) with assignments `.peykar/kmax-extraction-assignments.json`
(SHA-256 fbf1f10a…). After D1 and D2 had finished,
`.peykar/kmax-extraction-brief-addendum-1.md` (SHA-256 2c221633…) added one rule the
protocol did not name, written after D2 reported three nested blocks: when a table tabulates
the same eyes again for the subset that reached a later visit, each block with its own
baseline, only one block per group is extracted (the one with the most visits, then the
most eyes). D3, D4 and all three custodians worked under it. D2 had extracted three
nested blocks of the same eyes for `kmx-pmc467032`; the developer applied the addendum
to that record (`.peykar/curate_kmax_pmc467032.py`): the spec keeps block A (35
corneas, months 0 and 3), lists blocks B and C in its exclusions with a note, and the
private record was rebuilt with the unchanged builder.

**Development: 25 records with 35 series**: 27 group means and 8 per-eye series (two
studies), 33 control and 2 treated; contexts cross-linking epithelium-off 11 series
(6 studies), accelerated cross-linking 7 (6), ring segments 7 (6), cross-linking
combined with another treatment 4 (3), transepithelial cross-linking 3 (3) and
observation 3 (3); keratoconus 33 series, ectasia after refractive surgery 2; Pentacam
27, Sirius 4, device not stated 3, other 1; 34 absolute series and 1 change series;
13 series have one follow-up visit and 22 have two or more. The 12 CC BY records are in
`development/records/` with their source XML in `development/raw/`; the 13 records
under non-commercial or share-alike licences are private
(`.peykar/kmax-development-private/records/`). Specs (`development/specs/`) cite cells
and copy no values. Fifteen development studies have no record. Their reasons were
restated afterwards from the binding rules by a development reviewer sub-agent, because
the extractors' replies were not saved to a file
([`development/no-record-reasons-1.0.0.json`](development/no-record-reasons-1.0.0.json)):

- complete-case analysis sets (a minimum follow-up equal to the last tabulated visit,
  or only the eyes seen at the last visit, out of more treated eyes):
  `kmx-pmc12885941`, `kmx-pmc10302271`, `kmx-pmc11142646`, `kmx-pmc12027619`,
  `kmx-pmc10352124`, `kmx-pmc12028667` (its scleral-lens arm is also out of scope),
  `kmx-pmc12565262`, `kmx-pmc9231688`, `kmx-pmc12803647`, `kmx-pmc4314761`,
  `kmx-pmc9672700`;
- an analysis set selected by a post-treatment outcome (eyes stable after
  cross-linking, then given a phakic lens): `kmx-pmc4383407`;
- a follow-up given only as a cohort mean ± SD of months, and a recomputed tangential
  maximum instead of the device's Kmax: `kmx-pmc12468094`;
- "maximum keratometry" rows that the Results define as the steep meridian K:
  `kmx-pmc4128847`;
- `kmx-pmc8434931`: the review found its Kmax table otherwise admissible, but the two
  group labels of that table conflict with the article's other table and its Results.
  No binding rule resolves conflicting labels, the context of each column depends on
  which label is right, and the study stays without a record, as D3 left it.

**Sealed:** the custodians built **17 records with 33 series** (all group means, all
control; contexts cross-linking epithelium-off 15, accelerated 11, transepithelial 4,
combined 1, observation 1, other 1; all keratoconus; Pentacam 16, Placido topography 6,
Orbscan 5, Sirius 2, other 2, Galilei 1, not stated 1; 31 absolute and 2 change
series; 13 series with one follow-up visit and 20 with two or more; 10 records in the
public licence class, CC BY or CC0, and 7 in the private class) and reported only
structure and hashes. Five sealed
studies have no record: two complete-case sets, one with simulated keratometry only, one
with model-based estimates only and one whose follow-up is given only as intervals.
`.peykar/verify_kmax_custody.py` (output `.peykar/kmax-custody-check.json`) found every
reserved sealed study assigned once and reported once, every screened source and raw
copy equal to its assignment hash, all 17 records rebuilt byte for byte from their
specs, no record outside a record decision, and no key in the custody reports that
usually holds a value.

Process note: the developer opened custody report C1 once, by mistake, while checking
its format. By design a custody report holds structure only (decisions, groups, eye
counts, months and numbers of cited cells, no Kmax value); reports C2 and C3 were
checked only through the verifier.

No Kmax value of these articles had been read by the model developer when the rule,
the tranche order, the extraction protocol, the triage and the split were committed
(commits `7ddf3da7`, `fe6446c1`). Development records are now read for model
development; sealed records stay private and unread until the evaluation protocol is
frozen.
