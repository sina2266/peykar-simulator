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

## Briefs and chunks (local, not in git; SHA-256 before triage)

| file | SHA-256 |
|---|---|
| `.peykar/retina-acuity-tranche34-triage-brief.md` | `01754f784091bb5c9d4e709a1bbebf4a41556a73a703dee209b7b1c2fd2152f8` |
| `.peykar/retina-acuity-tranche34-extraction-brief.md` | `32a8895b302a61dd14a39813e6c801e1a0538fc7045404788c74104069f25406` |
| `.peykar/write_retina_acuity_tranche34_briefs.py` (writes both from the tranche-2 briefs) | `007bd8b625e50a4b663553481c68d80c50d2a433cc455904fcf3405de3be39e5` |

The chunks were built with `screening/triage_chunks.py --tranche 3 --chunks 4` (30
articles each) and `--tranche 4 --chunks 1` (18 articles), in
`.peykar/retina-acuity-tranche3-triage-chunks/` and
`.peykar/retina-acuity-tranche4-triage-chunks/`. A pattern check found no decimal, ±
or percent sign outside the DOI and licence lines of any chunk. Five reviewer
sub-agents triage them, one chunk each.

## Triage outcome (2026-10-29)

**Triage.** Five reviewer sub-agents worked from masked previews only
([`triage-1.0.0.json`](triage-1.0.0.json), written by
`.peykar/write_retina_acuity_tranche34_triage.py`). **70 admitted (59 of tranche 3, 11
of tranche 4), 68 excluded**, with no owner override. The exclusions were medians or
IQR only, proportions, lines gained or changes without a baseline value, one follow-up
time, Snellen means, model-based estimates, groups defined after baseline, baseline-only
or criteria tables, reviews, out-of-scope surgery or diseases, and two articles without
a DOI. Context hints of the admits (not binding): neovascular AMD 25, diabetic macular
edema 21, vein occlusion 12, other 5, myopic CNV 2, postoperative edema 2, uveitic 1,
not shown 2; anti-VEGF 44, combination 10, steroid implant 6, vitrectomy 4, laser 2,
steroid injection 1, other 1, not shown 2.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 53 record studies of tranches 1–2 as development (training of model 1.2) and all 70
admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) (run with
a byte-identical copy of the screening receipts in `screening/`) found 12 linked pairs:
3 between development studies (they leave leave-one-study-out training together), 7
between a development and a sealed study (6 sealed studies lose 7 training studies), and
2 between sealed studies (kept as separate studies of the test and reported as pairs).

**Custody.** Nine custodian sub-agents (C1–C9; 8 or 7 studies each, dealt round-robin
over the sorted study keys in `.peykar/retina-acuity-tranche34-extraction-assignments.json`,
SHA-256 `8cb0306e…`) extract the admits into
`.peykar/retina-acuity-tranche34-sealed-2026-10-29/`.
