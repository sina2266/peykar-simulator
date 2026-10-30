# Retina tranche 4: fresh sealed test set for retinal-thickness model 1.4

Declared on 2026-10-30, after the one-shot sealed test of model 1.3.0 and **before
tranche 4 was triaged**. No tranche-4 article had been triaged, extracted or read for
values by then.

## Rule

1. Tranche 4 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 360–394: the last 35 articles that passed the mechanical table rule.
2. Two reviewer sub-agents triage these articles from masked previews only, with the
   tranche-1 brief and criteria (`.peykar/retina-tranche4-triage-brief.md`, the
   tranche-3 brief with tranche-4 file locations). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 4 --chunks
   2`; the same script rebuilds the tranche-1, tranche-2 and tranche-3 chunks byte for
   byte as a check. The model developer does not read the chunks.
3. **Every admitted tranche-4 article is reserved for the one-shot sealed test of
   retinal-thickness model 1.4.** Tranche 4 has no development part.
4. Model 1.4 is developed only on the record studies of tranches 1–3: the 65 tranche-1
   record studies, the 63 tranche-2 sealed records consumed by the 1.1.0 test and the 67
   tranche-3 sealed records consumed by the 1.3.0 test.
5. Separate custodian sub-agents extract tranche-4 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the earlier briefs
   (`.peykar/retina-extraction-brief.md` and the tranche-3 addendum's clarifications
   with tranche-4 paths, applied the same way). Until the 1.4 protocol is frozen they
   report only structure and hashes, and their records stay in
   `.peykar/retina-tranche4-sealed-2026-10-30/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the record
   studies of tranches 1–3 as development and the admitted tranche-4 studies as the
   sealed test. A training study linked to a tranche-4 study is removed from that
   study's training set.
7. After tranche 4 no unread article of this dataset remains.

Known exposure before this declaration: on 2026-10-28 a web search result listed the
title of one tranche-4 article (PMC12744596), without any value. No other tranche-4
article has been seen.

## Triage outcome (2026-10-30)

**Triage.** The unchanged chunk script first rebuilt the tranche-1, tranche-2 and
tranche-3 chunks byte for byte, then built two tranche-4 chunks (18 and 17 articles; no
decimal, no ± followed by a digit and no digit followed by % outside identifier lines).
Two reviewer sub-agents followed `.peykar/retina-tranche4-triage-brief.md` (SHA-256
f5b49713…, the tranche-3 brief with tranche-4 paths; 58 criterion lines unchanged) and
saw masked previews only ([`triage-1.0.0.json`](triage-1.0.0.json)). Retina has no owner
rule, so there was no override. **22 admitted, 13 excluded.** The exclusions were one
follow-up time only (3), medians or percentiles only (2), a review or meta-analysis (2;
one also without a DOI), a change without a baseline, model-adjusted (MMRM) values, a
regression table, adjusted p-values only, incidence counts only and groups defined by a
post-treatment pressure rise.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 195 record studies of tranches 1–3 as development (training of model 1.4) and all
22 admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) (the
dataset's screening receipts copied to `screening/` for the link script) found 82
linked pairs: 8 tranche-4 studies lose linked training studies, 2 pairs link two
tranche-4 studies, and 21 development clusters leave training together in
leave-one-study-out.

## Extraction outcome (2026-10-30)

**Custodians.** Three custodian sub-agents (C1–C3) extracted the 22 reserved studies
under extraction protocol 1.0.0, the tranche-1 brief and a tranche-4 addendum
(`.peykar/retina-tranche4-extraction-brief.md`, SHA-256 28b55825…; the tranche-3
addendum with tranche-4 paths, every ruling copied verbatim). Assignments are in
`.peykar/retina-tranche4-extraction-assignments.json` (SHA-256 277b1fc4…; C1 8 studies,
C2 and C3 7 each). Records, specs, raw copies and one custody report per custodian are
private in `.peykar/retina-tranche4-sealed-2026-10-30/`. Reports state
`contains_outcome_values: false`.

**Result.** 17 records with 26 series (24 group means, 2 individual eyes; 22 control
and 4 treated); 5 studies excluded: medians only (3, one of them also restricted to
eyes classed as improved), changes from baseline in a reduction convention with no
absolute follow-up value (1), and an exclusion criterion removing eyes with surgery or
laser during follow-up from every visit (1).

**Blind custody check.** `.peykar/verify_retina_tranche4_custody.py` (the tranche-3
check with tranche-4 paths) rebuilt every record from the screened full text: 17 of 17
are identical byte for byte. Every record is declared `sealed_test` and is a reserved
study; every reserved study was assigned exactly once and reported; every raw copy (20)
equals its screened source; no stray record file exists. The check prints structure
only.

Structure (series per context): DME anti-VEGF 5, DME steroid implant 5, nAMD anti-VEGF
7, nAMD combination 3, other vitrectomy 2, and one each of DME combination, RVO
anti-VEGF, RVO combination and other/other. By the custodians' classification 6
records carry a CC BY or CC0 licence and 11 do not.

**Process notes.** Each custodian issued some spec writes and builds in one tool
batch; two builds failed before their inputs existed and wrote nothing, and every
record was rebuilt with an identical SHA-256. The model developer read the three
custody reports (structure, decisions and hashes only; no thickness value), which the
custody design allows but which earlier runs avoided by using the verifier only.
