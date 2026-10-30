# Myopia tranche 2: fresh sealed test set for myopia model 1.1

Declared on 2026-10-30, after the one-shot sealed test of model 1.0.0 and **before
tranche 2 was triaged**. No tranche-2 article had been triaged, extracted or read by
then; the tranche order was committed with the screen (`766a4ed4`).

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds the 94
   articles of ranks 120–213 that passed the mechanical table rule.
2. Three reviewer sub-agents triage these articles from masked previews only, with the
   tranche-1 brief and criteria (`.peykar/myopia-tranche2-triage-brief.md`, the
   tranche-1 brief with tranche-2 paths). The chunks are built by
   [`screening/triage_chunks.py`](../screening/triage_chunks.py), whose tranche-1 output
   is first rebuilt and compared byte for byte with the chunks the tranche-1 reviewers
   read. The model developer does not read the chunks.
3. **Every admitted tranche-2 article is reserved for the one-shot sealed test of myopia
   model 1.1.** Tranche 2 has no development part.
4. Model 1.1 is developed only on the 33 tranche-1 records: the 22 development records
   and the 11 sealed records already consumed by the 1.0.0 test.
5. Separate custodian sub-agents extract tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the tranche-1
   extraction brief with the readings settled during tranche-1 extraction, copied
   verbatim. One addition, for reporting only: when a table gives the number of children
   at each visit, the custodian copies it into an optional series field `n_by_visit`
   (one entry per month, `null` where not given). It is not used by the model, by the
   tested subset or by any hypothesis; it lets a later analysis describe series whose
   membership changes between visits. Until the 1.1 protocol is frozen the custodians
   report only structure and hashes, and their records stay in
   `.peykar/myopia-tranche2-sealed-2026-10-30/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the 33 tranche-1
   record studies as development and the admitted tranche-2 studies as the sealed test.
   A tranche-1 study linked to a tranche-2 study is removed from that study's training
   set.
7. No later tranche exists; a version after 1.1 needs a new screen whose test part is
   reserved before triage.
