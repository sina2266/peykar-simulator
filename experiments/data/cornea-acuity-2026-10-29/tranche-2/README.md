# Visual-acuity tranche 2: fresh sealed test set for acuity model 1.1

Declared on 2026-10-29, after the one-shot sealed test of acuity model 1.0.0 (commit
`f6c74736`) and **before tranche 2 was triaged**. No tranche-2 article had been
triaged or read by then.

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 120–174: 55 articles that passed the mechanical table rule.
2. Two reviewer sub-agents triage these articles from masked previews only. Their
   brief, `.peykar/acuity-tranche2-triage-brief.md`, is the tranche-1 brief with
   tranche-2 file locations; every criterion is unchanged. The chunks are built by
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 2
   --chunks 2` (28 and 27 articles, dealt round-robin by rank). Before this
   declaration the same script rebuilt the four tranche-1 chunks and their id lists
   byte for byte. The model developer does not read the chunks.
3. There is no owner override: none was used in tranche 1.
4. **Every admitted tranche-2 article is reserved for the one-shot sealed test of
   acuity model 1.1.** Tranche 2 has no development part.
5. Model 1.1 is developed only on the 42 tranche-1 records: the 27 development
   records and the 15 sealed records already consumed by the 1.0.0 test. The 25
   tranche-1 studies without a record (20 development, 5 sealed) stay without one.
6. Separate custodian sub-agents extract the tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md). Their brief,
   `.peykar/acuity-tranche2-extraction-brief.md`, is the tranche-1 brief with
   tranche-2 inputs and output locations, addendum 1 copied verbatim, and the
   tranche-1 rulings on ambiguous cases restated so that they are applied the same
   way. Until the 1.1 protocol is frozen the custodians report only structure and
   hashes, and their records stay in `.peykar/acuity-tranche2-sealed-2026-10-29/`.
7. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the
   tranche-1 record studies as development and the admitted tranche-2 studies as the
   sealed test. A tranche-1 study linked to a tranche-2 study is removed from that
   study's training set.
8. After tranche 2 no article of this screen is left unread. A later acuity version
   needs a new screen whose test part is reserved before triage.

At the tranche-1 rates (67 of 120 articles admitted; 15 of 20 sealed admits gave a
record) tranche 2 would give about 30 admits and 20 records. This is an expectation
for planning only; it is not a criterion.

## Briefs (local, not in git; SHA-256 at declaration)

| file | SHA-256 |
|---|---|
| `.peykar/acuity-tranche2-triage-brief.md` | `5f9c846197f988c6cbbfd948e280f22eb898545e87e960665c6c2e0f8954a20c` |
| `.peykar/acuity-tranche2-extraction-brief.md` | `50393d9704cc276e4518c1537b4eb414f902c5977097759186b42fe26a326d45` |
| `.peykar/write_acuity_tranche2_briefs.py` (writes both from the tranche-1 files) | `3926c350f8115b0379af34795f0ba90e087d48e4b717523442ee769d0992dd9b` |

The tranche-1 sources are unchanged: triage brief `19a39526…`, extraction brief
`786614de…` and addendum 1 `f93c4ef4…`.
