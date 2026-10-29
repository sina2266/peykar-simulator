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

## Triage outcome (2026-10-29)

**Triage.** Four reviewer sub-agents worked from masked previews only
([`triage-1.0.0.json`](triage-1.0.0.json), written by
`.peykar/write_retina_acuity_tranche2_triage.py`). **64 admitted, 56 excluded**, with no
owner override. The exclusions were proportions or letter categories only, medians or
IQR only, model-based or least-squares estimates, one follow-up time or no baseline
value in the shown tables, groups defined after baseline, reviews, meta-analyses,
cost-effectiveness models, out-of-scope surgery or treatments, and two articles without
a DOI. Context hints of the admits (not binding): neovascular AMD 26, diabetic macular
edema 16, vein occlusion 8, myopic CNV 3, several indications 2, uveitic edema 1,
central serous chorioretinopathy 1, not shown 7; anti-VEGF 39, two arms of different
types 6, steroid implant 4, combination 3, vitrectomy 2, photodynamic therapy 2,
steroid injection 1, not shown 7.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 24 tranche-1 record studies as development (training of model 1.1) and all 64
admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) (run with
a byte-identical copy of the screening receipts in `screening/`) found 4 linked pairs,
each of two tranche-2 studies; no tranche-1 record study is linked to a tranche-2
study, so no training set loses a study. The linked sealed pairs may share a cohort;
they stay separate studies of the test and are reported as pairs.

**Custody.** Eight custodian sub-agents (C1–C8; 8 studies each, dealt round-robin over
the sorted study keys in `.peykar/retina-acuity-tranche2-extraction-assignments.json`,
SHA-256 `5fece57f…`) extract the admits into
`.peykar/retina-acuity-tranche2-sealed-2026-10-29/`.

## Custody outcome (2026-10-29)

The eight custodians reported structure and hashes only (`custody-report-C1.json` to
`C8.json` in the sealed folder). A first attempt to start C1–C4 failed on a usage
limit of the agent service before any file was written; the folder did not exist yet,
and the same custodians were started again. **29 records with 81 series** were built:
50 group means and 31 per-eye series (two studies of single eyes), 64 control and 17
treated; contexts neovascular AMD with anti-VEGF 20 and combination 3, vein occlusion
with anti-VEGF 20, myopic CNV with anti-VEGF 21 (19 of them per-eye), diabetic macular
edema with anti-VEGF 6, steroid implant 6, vitrectomy 3 and combination 2; value kinds
logMAR 61, ETDRS letters 12 and letter changes with their baseline 8; 13 records public
and 16 private by licence. Several series have visits in the first month (day 1, week 1,
day 3). **35 admitted studies have no record**: most are complete-case sets (a minimum
follow-up equal to the last tabulated visit, or an analysis set of completers); the
others have acuity not established as best-corrected, logMAR or ETDRS letters (Snellen,
decimal or unit not stated), medians only, analysis sets that removed eyes after a
post-baseline event, groups defined after baseline, or (one) a correction notice that
reprints another article's table without its Methods.

`.peykar/verify_retina_acuity_tranche2_custody.py` rebuilt all 29 records byte for byte
from their specs and found all 64 raw copies equal to their screened sources; every
reserved study was assigned and reported once, and no problem was found. Process notes
from the reports: several custodians ran the builder in the same tool batch as the spec
write or check (in one case before the spec existed; that call failed and wrote
nothing); every such record was rebuilt from its unchanged spec and kept only because
the SHA-256 was identical. Two custodians (C4, C8) deleted and recreated their own
report once (a field name; neutral wording of exclusion labels); no spec, record or
raw copy changed. One custodian (C3) wrote a temporary text rendering of one article to
`/tmp`, read it once and deleted it. In three of the four author-linked pairs both
studies gave records (two vein-occlusion anti-VEGF cohorts; the XTEND UK cohort and the
XTEND country cohorts; two dexamethasone implant reports with the same design and visit
times). They may share eyes; the author-overlap rule keeps them as separate studies of
the test, and the pairs are reported.
