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
