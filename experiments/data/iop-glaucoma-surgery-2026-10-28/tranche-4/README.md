# IOP tranche 4: fresh sealed test set for IOP model 1.3

Declared on 2026-10-29, while the one-shot tranche-3 sealed test of model 1.2.0 was
running and **before tranche 4 was triaged**. No tranche-4 article had been triaged,
extracted or read for values by then, with the exception noted below.

## Rule

1. Tranche 4 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds ranks
   360–479: 120 articles that passed the mechanical table rule.
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria (the tranche-3 brief with tranche-4 file
   locations). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 4`, which
   still rebuilds the tranche-1 chunks byte for byte (`--check-tranche-1`). The model
   developer does not read the chunks.
3. **Every admitted tranche-4 article is reserved for the one-shot sealed test of IOP
   model 1.3.** Tranche 4 has no development part.
4. Model 1.3 is developed only on the record studies of tranches 1–3: the 146 record
   studies of tranches 1 and 2 and the 83 tranche-3 sealed records consumed by the
   1.2.0 test.
5. Separate custodian sub-agents extract tranche-4 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the tranche-3
   brief's rulings and clarifications, applied the same way. Until the 1.3 protocol is
   frozen they report only structure and hashes, and their records stay in
   `.peykar/iop-tranche4-sealed-2026-10-29/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   to tranche-3 record studies as development and the admitted tranche-4 studies as
   the sealed test. A training study linked to a tranche-4 study is removed from that
   study's training set.
7. Tranche 5 (19 articles) stays untriaged and unread.

Known exposure: the developer earlier saw the IOP medians in one table of PMC11852638,
a medians-only micro-shunt study in tranche 4 (see the tranche-3 README). Medians are
not admissible under protocol 1.0.0, so triage or extraction is expected to exclude it;
if a custodian finds admissible means, the study is excluded from the sealed test.
