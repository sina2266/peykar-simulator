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

## Triage outcome (2026-10-29)

**Triage.** `screening/triage_chunks.py --check-tranche-1` confirmed that the three
tranche-1 chunks still rebuild byte for byte; the tranche-4 chunks contain no decimal
and no ± followed by a digit outside identifier lines. Three reviewer sub-agents
followed `.peykar/iop-tranche4-triage-brief.md` (the tranche-3 brief with tranche-4
paths) and saw masked previews only ([`triage-1.0.0.json`](triage-1.0.0.json)). They
admitted 91 articles and excluded 29. The owner applied the tranche-1 rule for
percent-change tables to the three exclusions whose reason states a percent IOP
reduction from baseline at follow-up times (PMC12708801, PMC9150991, PMC4817329); the
custodians decide whether they are extractable. Absolute mean reductions without a
baseline (PMC10559792) and a table without a preoperative row (PMC13220826) are not
covered by that rule and stay excluded. **94 admitted, 26 excluded.** The exclusions
were medians only (including PMC11852638, the study whose medians the developer saw
earlier), outcome-defined groups or analysis sets, changes without a baseline, fewer
than two dated postoperative times, range times, success or count tables, p-value or
posture tables, baseline-only tables, a review, keratoplasty (out of scope) and one
article without a DOI.

The first write of the triage file listed only the first two percent-rule overrides.
The omission of PMC4817329 was found from the reviewers' reasons before any commit or
extraction; the triage, reservation, author-overlap and assignment files were then
regenerated (hashes of the superseded first write:
`.peykar/iop-t4-first-write-superseded.sha256`).

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 229 record studies of tranches 1–3 as development (training of model 1.3) and all
94 admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) found 154
linked pairs: 34 tranche-4 studies lose linked training studies, 17 pairs link two
tranche-4 studies, and 35 development clusters leave training together in
leave-one-study-out.
