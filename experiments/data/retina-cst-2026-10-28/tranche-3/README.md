# Retina tranche 3: fresh sealed test set for retinal-thickness model 1.2

Declared on 2026-10-29, after the one-shot sealed test of model 1.1.0 and **before
tranche 3 was triaged**. No tranche-3 article had been triaged, extracted or read for
values by then.

## Rule

1. Tranche 3 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 240–359: 120 articles that passed the mechanical table rule (one has no DOI
   and cannot be admitted).
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria (`.peykar/retina-triage-brief.md`, with tranche-3
   file locations). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 3`; the
   same script rebuilds the tranche-1 and tranche-2 chunks byte for byte as a check.
   The model developer does not read the chunks.
3. **Every admitted tranche-3 article is reserved for the one-shot sealed test of
   retinal-thickness model 1.2.** Tranche 3 has no development part.
4. Model 1.2 is developed only on the record studies of tranches 1 and 2: the 65
   tranche-1 record studies and the 63 tranche-2 sealed records consumed by the 1.1.0
   test.
5. Separate custodian sub-agents extract tranche-3 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the earlier briefs
   (`.peykar/retina-extraction-brief.md` and the tranche-2 addendum's clarifications,
   applied the same way). Until the 1.2 protocol is frozen they report only structure
   and hashes, and their records stay in `.peykar/retina-tranche3-sealed-2026-10-29/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   and tranche-2 record studies as development and the admitted tranche-3 studies as
   the sealed test. A training study linked to a tranche-3 study is removed from that
   study's training set.
7. Tranche 4 (35 articles) stays untriaged and unread.

Known exposure before this declaration: on 2026-10-28 a web search result listed the
title of one tranche-4 article (PMC12744596), without any value. No tranche-3 article
has been seen.

## Triage outcome (2026-10-29)

**Triage.** The unchanged chunk script first rebuilt the tranche-1 and tranche-2
chunks byte for byte, then built the tranche-3 chunks (no decimal and no ± followed by
a digit outside identifier lines). Three reviewer sub-agents followed
`.peykar/retina-tranche3-triage-brief.md` (the tranche-1 brief with tranche-3 paths)
and saw masked previews only ([`triage-1.0.0.json`](triage-1.0.0.json)). Retina has
no owner rule from earlier triages, so there was no override. **88 admitted, 32
excluded.** The exclusions were medians only, changes without a baseline (including a
percent *reduction* reported as a positive number), one follow-up time or visits
pooled over a period, outcome-defined groups, a least-squares table, proportions, a
regression table, a baseline-only table, a review, a meta-analysis, cataract surgery
in eyes without treated retinal disease (out of scope), a criteria-only table and one
article without a DOI. One reviewer reported that the first column of PMC2694595 (a
review, excluded) showed thickness numbers that the masking had missed; the developer
has not seen it.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 128 record studies of tranches 1 and 2 as development (training of model 1.2) and
all 88 admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) found 60
linked pairs: 19 tranche-3 studies lose linked training studies, 4 pairs link two
tranche-3 studies, and 10 development clusters leave training together in
leave-one-study-out.
