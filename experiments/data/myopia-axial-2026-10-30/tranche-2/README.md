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

## Triage outcome (2026-10-30)

**Chunks.** The unchanged `screening/triage_chunks.py` rebuilt the four tranche-1 chunks
byte for byte and built three tranche-2 chunks (32, 31 and 31 articles; no decimal, no ±
followed by a digit and no digit followed by % outside the identifier lines). The model
developer did not read the chunks.

**Triage.** Three reviewer sub-agents worked from masked previews only
([`triage-1.0.0.json`](triage-1.0.0.json)); there is no owner override. **42 admitted,
52 excluded.** The exclusions were mostly axial-length changes without the groups'
baseline in any shown table, model-adjusted or mixed-model estimates only, baseline-only
tables, medians only, outcome-defined groups (fast against slow progressors, responders
against non-responders), out-of-scope populations (premyopic or mixed-refraction school
children, an animal study, eyes after scleral surgery), a review and a study design
report. Several admits carry reviewer notes for the custodians (complete-case sets to
check against the 70% rule, outcome-defined columns not to extract, groups of uncertain
scope).

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 33 tranche-1 record studies as development (training of model 1.1) and all 42 admits
as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) (the
dataset's screening receipts copied to `screening/` for the link script) found 20 linked
pairs: 7 tranche-2 studies lose 9 linked training studies, 7 pairs link two tranche-2
studies, and the 2 development clusters of tranche 1 (3 and 2 studies) leave training
together in leave-one-study-out.

## Extraction outcome (2026-10-30)

**Custodians.** Six custodian sub-agents (C1–C6, seven studies each) extracted the 42
reserved studies under extraction protocol 1.0.0 and the tranche-2 brief
(`.peykar/myopia-tranche2-extraction-brief.md`, SHA-256 6dbb6068…; the tranche-1 brief
with tranche-2 paths, the settled tranche-1 readings copied verbatim and the reporting
field `n_by_visit`). Assignments are in `.peykar/myopia-tranche2-extraction-assignments.json`
(SHA-256 a34b7792…). Records, specs, raw copies and one custody report per custodian are
private in `.peykar/myopia-tranche2-sealed-2026-10-30/`; the reports state
`contains_outcome_values: false`.

**Result.** **26 records with 60 series** (56 group means and 4 per-child series, all
role `control`); 16 studies have no record: eleven complete-case sets (mostly
retrospective cohorts whose inclusion required the last tabulated visit, with no number
of children who started), outcome-selected analysis sets or groups (three), a table with
model-adjusted values only (one) and a longitudinal sample of mixed refraction and age
(one).

**Blind custody check.** `.peykar/verify_myopia_tranche2_custody.py` (the tranche-1 check
with tranche-2 paths; output `.peykar/myopia-tranche2-custody-check.json`) found every
reserved study assigned once and reported once, all 42 screened sources equal to their
assignment hashes, all 26 records rebuilt byte for byte from their specs, all raw copies
equal to their sources, no record outside a record decision and no value-like key in the
custody reports. Structure (series per context): orthokeratology 15, defocus spectacles
14, single vision 12, atropine 7, multifocal contact lenses 6, combinations 5, `other` 1;
no red-light series. East Asia 38, Europe 12, Middle East 4, Southeast Asia 4, Africa 2;
38 absolute and 22 change series; 18 series have one follow-up visit and 42 have two or
more; 11 records in the public licence class and 15 in the private class; per-visit
numbers (`n_by_visit`) are given for 19 series.

**Process notes.** Two custodians ran a builder call in the same tool batch as the spec
write; both checked that their record equals a rebuild from the spec on disk, and the
custody check rebuilt all records identically. Contrary to the practice of earlier
tranches, the model developer opened custodian C4's custody report once (structure,
decisions and hashes of its seven studies: groups, reported group sizes, devices, months,
licences and exclusion reasons; no axial-length value, as the report states). The other
five reports were not opened. No tranche-2 value is read before the model 1.1 protocol is
frozen.
