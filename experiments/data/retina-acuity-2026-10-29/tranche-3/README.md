# Retinal visual-acuity tranches 3 and 4: fresh sealed test set for retinal acuity model 1.2

Declared on 2026-10-29, after the one-shot sealed test of retinal acuity model 1.1.0
(results commit `2dc48d3d`) and **before tranches 3 and 4 were triaged**. No article of
either tranche had been triaged or read by then.

## Rule

1. Tranches 3 and 4 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) hold
   ranks 240–377: 138 articles that passed the mechanical table rule (120 in tranche 3,
   18 in tranche 4). They are triaged and reserved together, because tranche 4 alone is
   too small to test anything.
2. Reviewer sub-agents triage these articles from masked previews only, with the
   tranche-1 triage brief and its criteria unchanged; only the file locations change.
   The chunks are built by [`screening/triage_chunks.py`](../screening/triage_chunks.py)
   with `--tranche 3` and `--tranche 4`. The model developer does not read the chunks.
3. There is no owner override, as in tranches 1 and 2.
4. **Every admitted article of tranches 3 and 4 is reserved for the one-shot sealed test
   of retinal acuity model 1.2.** These tranches have no development part.
5. Model 1.2 is developed only on the 53 record studies of tranches 1 and 2: the 24
   tranche-1 records (15 development records and 9 records consumed by the 1.0.0 test)
   and the 29 tranche-2 records consumed by the 1.1.0 test. Studies admitted in
   tranches 1 or 2 without a record stay without one.
6. Separate custodian sub-agents extract the records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md), with the tranche-2
   extraction brief and its restated rulings unchanged except for the input and output
   locations. Until the 1.2 protocol is frozen the custodians report only structure and
   hashes, and their records stay in a private folder under `.peykar/`. A custodian
   report must name the licence class under the field `licence_class` (the tranche-2
   verifier missed two records written as `license_class`).
7. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the 53 record
   studies of tranches 1–2 as development and the admitted tranche 3–4 studies as the
   sealed test. A training study linked to a sealed study is removed from that study's
   training set.
8. After this test the dataset has no unread article left; a later version needs a new
   screen whose test part is reserved before triage.

At the earlier rates (62 and 64 of 120 articles admitted; 9 of 21 and 29 of 64 admits
gave a record) tranches 3 and 4 would give about 70 admits and 25–30 records. This is
an expectation for planning only; it is not a criterion.
