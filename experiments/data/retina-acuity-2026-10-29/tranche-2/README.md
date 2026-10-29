# Retinal visual-acuity tranche 2: fresh sealed test set for retinal acuity model 1.1

Declared on 2026-10-29, after the one-shot sealed test of retinal acuity model 1.0.0
(results commit `d9d34b8c`) and **before tranche 2 was triaged**. No tranche-2
article had been triaged or read by then.

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds ranks
   120–239: 120 articles that passed the mechanical table rule.
2. Four reviewer sub-agents triage these articles from masked previews only. Their
   brief, `.peykar/retina-acuity-tranche2-triage-brief.md`, is the tranche-1 brief with
   tranche-2 file locations; every criterion is unchanged. The chunks are built by
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 2
   --chunks 4` (30 articles each, dealt round-robin by rank). Before this declaration
   the same script rebuilt the four tranche-1 chunks and their id lists byte for byte.
   The model developer does not read the chunks.
3. There is no owner override: none was used in tranche 1.
4. **Every admitted tranche-2 article is reserved for the one-shot sealed test of
   retinal acuity model 1.1.** Tranche 2 has no development part.
5. Model 1.1 is developed only on the 24 tranche-1 records: the 15 development records
   and the 9 sealed records already consumed by the 1.0.0 test. The 38 tranche-1
   studies without a record (26 development, 12 sealed) stay without one.
6. Separate custodian sub-agents extract the tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md). Their brief,
   `.peykar/retina-acuity-tranche2-extraction-brief.md`, is the tranche-1 brief with
   tranche-2 inputs and output locations, addendum 1 copied verbatim, and the
   tranche-1 rulings on ambiguous cases restated (from the custodians' structural
   reports) so that they are applied the same way. Until the 1.1 protocol is frozen
   the custodians report only structure and hashes, and their records stay in
   `.peykar/retina-acuity-tranche2-sealed-2026-10-29/`.
7. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   record studies as development and the admitted tranche-2 studies as the sealed
   test. A tranche-1 study linked to a tranche-2 study is removed from that study's
   training set.
8. Tranches 3 and 4 (138 articles) stay untriaged and unread, for later versions.

At the tranche-1 rates (62 of 120 articles admitted; 9 of 21 sealed admits gave a
record) tranche 2 would give about 60 admits and 25 records. This is an expectation
for planning only; it is not a criterion.

## Briefs (local, not in git; SHA-256 at declaration)

| file | SHA-256 |
|---|---|
| `.peykar/retina-acuity-tranche2-triage-brief.md` | `f1ecd2f1b7023318a84f19d91877def82a67c822555f9eef698ee817d9e691bd` |
| `.peykar/retina-acuity-tranche2-extraction-brief.md` | `2f72160fc76e61fe08875ab2a776f3c7aafdb8c6af1dd0a63d24a22e6473a1cb` |
| `.peykar/write_retina_acuity_tranche2_briefs.py` (writes both from the tranche-1 files) | `abe60a332df83875353e0430e6947b8a09d988e0ea7c0882c925bc42adb02e6b` |

The tranche-1 sources are unchanged: triage brief `52eaf97b…`, extraction brief
`1bad2e98…` and addendum 1 `53a56d7c…`.
