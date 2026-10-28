# IOP tranche 3: fresh sealed test set for IOP model 1.2

Declared on 2026-10-28, after the one-shot tranche-2 sealed test of model 1.1 and
**before tranche 3 was triaged**. No tranche-3 article had been triaged, extracted or
read for values by then.

## Rule

1. Tranche 3 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 240–359: 120 articles that passed the mechanical table rule.
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria (the tranche-2 brief with tranche-3 file
   locations). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 3`, which
   still rebuilds the tranche-1 chunks byte for byte (`--check-tranche-1`). The model
   developer does not read the chunks.
3. **Every admitted tranche-3 article is reserved for the one-shot sealed test of IOP
   model 1.2.** Tranche 3 has no development part.
4. Model 1.2 is developed only on the 146 records of tranches 1 and 2: the 47
   tranche-1 development records, the 25 tranche-1 sealed records consumed by the
   1.0.0 test and the 74 tranche-2 sealed records consumed by the 1.1 test.
5. Separate custodian sub-agents extract tranche-3 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the tranche-2
   brief's rulings, applied the same way. Until the 1.2 protocol is frozen they report
   only structure and hashes, and their records stay in
   `.peykar/iop-tranche3-sealed-2026-10-28/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   and tranche-2 record studies as development and the admitted tranche-3 studies as
   the sealed test. A training study linked to a tranche-3 study is removed from that
   study's training set.
7. Tranches 4–5 (139 articles) stay untriaged and unread for later versions. The IOP
   medians that the developer saw earlier in one table (PMC11852638, a medians-only
   micro-shunt study) are in tranche 4.

## Triage outcome (2026-10-28)

**Triage.** `screening/triage_chunks.py --check-tranche-1` confirmed that the three
tranche-1 chunks still rebuild byte for byte; the tranche-3 chunks contain no decimal,
± or other data value outside identifier lines. Three reviewer sub-agents followed
`.peykar/iop-tranche3-triage-brief.md` (the tranche-2 brief with tranche-3 paths) and
saw masked previews only ([`triage-1.0.0.json`](triage-1.0.0.json)). They admitted 96
articles and excluded 24. The owner applied the tranche-1 rule for percent-change
tables to one exclusion whose reason states percent IOP change from baseline at every
follow-up (PMC7127814); the custodian decides whether it is extractable. **97
admitted, 23 excluded.** The exclusions were medians only, model estimates, reviews
and meta-analyses, success counts, correlation or p-value tables, changes without a
baseline, one follow-up time, keratoplasty and intravitreal injection (out of scope).
One admit is a canine study.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 146 record studies of tranches 1 and 2 as development (training of model 1.2) and
all 97 admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) found 72
linked pairs: 25 tranche-3 studies lose linked training studies, 8 pairs link two
tranche-3 studies, and 24 development clusters leave training together in
leave-one-study-out.
