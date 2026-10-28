# IOP tranche 2: fresh sealed test set for IOP model 1.1

Declared on 2026-10-28, after the one-shot sealed test of model 1.0.0 and **before
tranche 2 was triaged**. No tranche-2 article had been triaged or read by then.

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 120–239: 120 articles that passed the mechanical table rule.
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria. The chunks are built by
   [`screening/triage_chunks.py`](../screening/triage_chunks.py), which rebuilds the
   tranche-1 chunks byte for byte (`--check-tranche-1`). The model developer does not
   read the chunks.
3. **Every admitted tranche-2 article is reserved for the one-shot sealed test of IOP
   model 1.1.** Tranche 2 has no development part.
4. Model 1.1 is developed only on the 72 tranche-1 records: the 47 development
   records and the 25 sealed records already consumed by the 1.0.0 test.
5. Separate custodian sub-agents extract tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and its brief. Until
   the 1.1 protocol is frozen they report only structure and hashes, and their
   records stay in `.peykar/iop-tranche2-sealed-2026-10-28/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the
   tranche-1 record studies as development and the admitted tranche-2 studies as the
   sealed test. A tranche-1 study linked to a tranche-2 study is removed from that
   study's training set.
7. Tranches 3–5 (259 articles) stay untriaged and unread for later versions.

The IOP medians that the developer saw earlier in one table (PMC11852638, a
medians-only micro-shunt study) are in tranche 4.
