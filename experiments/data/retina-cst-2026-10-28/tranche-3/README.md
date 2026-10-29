# Retina tranche 3: fresh sealed test set for retinal-thickness model 1.2

Declared on 2026-10-29, after the one-shot sealed test of model 1.1.0 and **before
tranche 3 was triaged**. No tranche-3 article had been triaged, extracted or read for
values by then.

## Rule

1. Tranche 3 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 240–359: 120 articles that passed the mechanical table rule (one has no DOI
   and cannot be admitted).
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria (`.peykar/retina-triage-brief.md`, with tranche-3
   file locations). The chunks are built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 3`; the
   same script rebuilds the tranche-1 and tranche-2 chunks byte for byte as a check.
   The model developer does not read the chunks.
3. **Every admitted tranche-3 article is reserved for the one-shot sealed test of
   retinal-thickness model 1.2.** Tranche 3 has no development part.
4. Model 1.2 is developed only on the record studies of tranches 1 and 2: the 65
   tranche-1 record studies and the 63 tranche-2 sealed records consumed by the 1.1.0
   test.
5. Separate custodian sub-agents extract tranche-3 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the earlier briefs
   (`.peykar/retina-extraction-brief.md` and the tranche-2 addendum's clarifications,
   applied the same way). Until the 1.2 protocol is frozen they report only structure
   and hashes, and their records stay in `.peykar/retina-tranche3-sealed-2026-10-29/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   and tranche-2 record studies as development and the admitted tranche-3 studies as
   the sealed test. A training study linked to a tranche-3 study is removed from that
   study's training set.
7. Tranche 4 (35 articles) stays untriaged and unread.

Known exposure before this declaration: on 2026-10-28 a web search result listed the
title of one tranche-4 article (PMC12744596), without any value. No tranche-3 article
has been seen.

## Triage outcome (2026-10-29)

**Triage.** The unchanged chunk script first rebuilt the tranche-1 and tranche-2
chunks byte for byte, then built the tranche-3 chunks (no decimal and no ± followed by
a digit outside identifier lines). Three reviewer sub-agents followed
`.peykar/retina-tranche3-triage-brief.md` (the tranche-1 brief with tranche-3 paths)
and saw masked previews only ([`triage-1.0.0.json`](triage-1.0.0.json)). Retina has
no owner rule from earlier triages, so there was no override. **88 admitted, 32
excluded.** The exclusions were medians only, changes without a baseline (including a
percent *reduction* reported as a positive number), one follow-up time or visits
pooled over a period, outcome-defined groups, a least-squares table, proportions, a
regression table, a baseline-only table, a review, a meta-analysis, cataract surgery
in eyes without treated retinal disease (out of scope), a criteria-only table and one
article without a DOI. One reviewer reported that the first column of PMC2694595 (a
review, excluded) showed thickness numbers that the masking had missed; the developer
has not seen it.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 128 record studies of tranches 1 and 2 as development (training of model 1.2) and
all 88 admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) found 60
linked pairs: 19 tranche-3 studies lose linked training studies, 4 pairs link two
tranche-3 studies, and 10 development clusters leave training together in
leave-one-study-out.

## Extraction outcome (2026-10-29)

**Custodians.** Seven custodian sub-agents (C1–C7) extracted the 88 reserved studies
under extraction protocol 1.0.0, the tranche-1 brief and a tranche-3 addendum
(`.peykar/retina-tranche3-extraction-brief.md`, SHA-256 ca470207…; the tranche-2
addendum with tranche-3 paths). Assignments are in
`.peykar/retina-tranche3-extraction-assignments.json` (SHA-256 5791e80d…; C1–C4 13
studies, C5–C7 12 each). Records, specs, raw copies and one custody report per
custodian are private in `.peykar/retina-tranche3-sealed-2026-10-29/`. Reports state
`contains_outcome_values: false`.

**Result.** 67 records with 134 series (107 group means, 27 individual eyes); 21
studies excluded. Most exclusions are analysis sets selected after the baseline
(responders, eyes removed after rescue, extra surgery or a switch, minimum numbers of
injections received); the others are model (GEE) estimates, medians only, follow-ups
timed from another event, a postoperative reference and one out-of-scope prophylaxis
study.

**Blind custody check.** `.peykar/verify_retina_tranche3_custody.py` (the IOP
tranche-3 check with retina paths) rebuilt every record from the screened full text:
67 of 67 are identical byte for byte. Every record is declared `sealed_test` and is a
reserved study; every reserved study was assigned exactly once and reported; every raw
copy equals its screened source; each report's record hash equals the stored record;
no stray record file exists. The check prints structure only.

Structure (series per treatment): anti-VEGF 47, steroid implant 34, combination 31,
steroid injection 9, laser 5, observation 4, vitrectomy 2, photodynamic therapy 1,
other 1. Indications: DME 73, nAMD 22, RVO 18, postoperative macular oedema 12, other
8, CSC 1. Fifteen sealed series fall in five contexts that have no training series
(postoperative oedema with a steroid implant 11, CSC implant, nAMD PDT, other
observation, other implant). By the custodians' classification 39 records carry a CC
BY or CC0 licence and 28 do not; the publisher classifies every licence again before
anything is published.

**Process notes.** The reports use slightly different layouts and decision labels;
the check reads each one. C6 wrote two temporary text dumps of one article outside
the sealed folder (in the agent's scratch folder) and deleted them; it says so in its
report. C6 also issued one build before its spec existed; that build failed and wrote
nothing.
