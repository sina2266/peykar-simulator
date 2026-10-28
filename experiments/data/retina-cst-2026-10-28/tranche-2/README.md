# Retina tranche 2: fresh sealed test set for retinal-thickness model 1.1

Declared on 2026-10-28, after the one-shot sealed test of model 1.0.0 and **before
tranche 2 was triaged**. No tranche-2 article had been triaged, extracted or read for
values by then.

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 120–239: 120 articles that passed the mechanical table rule (4 have no DOI
   and cannot be admitted).
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria (`.peykar/retina-triage-brief.md`, with tranche-2
   file locations in an addendum). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 2`; the
   same script rebuilds the tranche-1 chunks byte for byte as a check. The model
   developer does not read the chunks.
3. **Every admitted tranche-2 article is reserved for the one-shot sealed test of
   retinal-thickness model 1.1.** Tranche 2 has no development part.
4. Model 1.1 is developed only on the 65 tranche-1 records: the 46 development
   records and the 19 sealed records already consumed by the 1.0.0 test.
5. Separate custodian sub-agents extract tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and its brief
   (`.peykar/retina-extraction-brief.md`, with a tranche-2 addendum for locations).
   Until the 1.1 protocol is frozen they report only structure and hashes, and their
   records stay in `.peykar/retina-tranche2-sealed-2026-10-28/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   record studies as development and the admitted tranche-2 studies as the sealed
   test. A tranche-1 study linked to a tranche-2 study is removed from that study's
   training set.
7. Tranches 3–4 (155 articles) stay untriaged and unread for later versions.

While checking the mechanical rule's printed output on 2026-10-28, the developer saw
the masked previews (titles, captions and row labels; every data cell hidden) of
three tranche-2 articles: PMC10025720, PMC10051192 and PMC10054468. No value of any
tranche-2 article has been seen.

## Triage outcome (2026-10-28)

**Triage.** The unchanged chunk script rebuilt the three tranche-1 chunks byte for
byte, then built the tranche-2 chunks (no decimal, ± or other data value outside
identifier lines). Three reviewer sub-agents followed
`.peykar/retina-tranche2-triage-brief.md` (the tranche-1 brief with tranche-2 paths)
and saw masked previews only ([`triage-1.0.0.json`](triage-1.0.0.json)). There was no
owner override. **85 admitted, 35 excluded.** The exclusions were medians only,
model-adjusted estimates, changes without a baseline, one follow-up time,
baseline-only or regression tables, outcome-defined groups, reviews and
meta-analyses, out-of-scope surgery, and four articles without a DOI. One reviewer
reported that a first-column label of PMC7592807 showed a thickness interval that the
masking had missed; the article was excluded (medians layout), and the developer has
not seen it.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 65 tranche-1 record studies as development (training of model 1.1) and all 85
admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) found 30
linked pairs: 11 tranche-2 studies lose linked training studies, 9 pairs link two
tranche-2 studies, and 3 development clusters leave training together in
leave-one-study-out.

## Extraction outcome (2026-10-28)

**Custodians.** Seven custodian sub-agents (C1–C7) extracted the 85 reserved studies
under extraction protocol 1.0.0, the tranche-1 brief and a tranche-2 addendum
(`.peykar/retina-tranche2-extraction-brief.md`, SHA-256 c4c3010d…). Assignments are in
`.peykar/retina-tranche2-extraction-assignments.json` (SHA-256 6f2f01de…; C1 13
studies, C2–C7 12 each). Records, specs, raw copies and one custody report per
custodian are private in `.peykar/retina-tranche2-sealed-2026-10-28/`. Reports state
`contains_outcome_values: false`.

**Result.** 63 records with 135 series (93 group means, 42 individual eyes); 22
studies excluded. Most exclusions are analysis sets selected after the baseline
(responders, completers defined by outcome) or tables of medians; one statistic could
not be established. C4 left out two visits of one study because their table cells are
malformed (a missing parenthesis), so the value could not be read without guessing.

**Blind custody check.** `.peykar/verify_retina_tranche2_custody.py` rebuilt every
record from the screened full text: 63 of 63 are identical byte for byte. Every
record is declared `sealed_test` and is a reserved study; every reserved study was
assigned exactly once and reported; every raw copy equals its screened source; each
report's record hash equals the stored record. The check prints structure only.

Structure (series per treatment): anti-VEGF 46, steroid implant 46, combination 17,
other 8, vitrectomy 7, steroid injection 6, laser 5. Indications: DME, nAMD, RVO,
CSC, myopic CNV, uveitic and postoperative macular oedema, other. Fifteen sealed
series fall in five contexts that have no training series (RVO laser, other
anti-VEGF, other steroid injection, other/other, postoperative CME laser). By the
custodians' classification 34 records carry a CC BY or CC0 licence and 29 do not;
the publisher classifies every licence again before anything is published.

**Process notes.** The reports use slightly different layouts; the check script reads
each one. C1 deleted and rewrote its own spec and record for two studies (three
times in total) and rewrote its report once; only the final files count, and they
rebuild. C3 kept raw copies of its two excluded studies. The brief told each
custodian to open only its own files, by exact name.
