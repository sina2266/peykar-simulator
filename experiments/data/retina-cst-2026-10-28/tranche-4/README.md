# Retina tranche 4: fresh sealed test set for retinal-thickness model 1.4

Declared on 2026-10-30, after the one-shot sealed test of model 1.3.0 and **before
tranche 4 was triaged**. No tranche-4 article had been triaged, extracted or read for
values by then.

## Rule

1. Tranche 4 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 360–394: the last 35 articles that passed the mechanical table rule.
2. Two reviewer sub-agents triage these articles from masked previews only, with the
   tranche-1 brief and criteria (`.peykar/retina-tranche4-triage-brief.md`, the
   tranche-3 brief with tranche-4 file locations). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 4 --chunks
   2`; the same script rebuilds the tranche-1, tranche-2 and tranche-3 chunks byte for
   byte as a check. The model developer does not read the chunks.
3. **Every admitted tranche-4 article is reserved for the one-shot sealed test of
   retinal-thickness model 1.4.** Tranche 4 has no development part.
4. Model 1.4 is developed only on the record studies of tranches 1–3: the 65 tranche-1
   record studies, the 63 tranche-2 sealed records consumed by the 1.1.0 test and the 67
   tranche-3 sealed records consumed by the 1.3.0 test.
5. Separate custodian sub-agents extract tranche-4 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the earlier briefs
   (`.peykar/retina-extraction-brief.md` and the tranche-3 addendum's clarifications
   with tranche-4 paths, applied the same way). Until the 1.4 protocol is frozen they
   report only structure and hashes, and their records stay in
   `.peykar/retina-tranche4-sealed-2026-10-30/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the record
   studies of tranches 1–3 as development and the admitted tranche-4 studies as the
   sealed test. A training study linked to a tranche-4 study is removed from that
   study's training set.
7. After tranche 4 no unread article of this dataset remains.

Known exposure before this declaration: on 2026-10-28 a web search result listed the
title of one tranche-4 article (PMC12744596), without any value. No other tranche-4
article has been seen.
