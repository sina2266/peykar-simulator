# Visual-acuity tranche 2: fresh sealed test set for acuity model 1.1

Declared on 2026-10-29, after the one-shot sealed test of acuity model 1.0.0 (commit
`f6c74736`) and **before tranche 2 was triaged**. No tranche-2 article had been
triaged or read by then.

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 120–174: 55 articles that passed the mechanical table rule.
2. Two reviewer sub-agents triage these articles from masked previews only. Their
   brief, `.peykar/acuity-tranche2-triage-brief.md`, is the tranche-1 brief with
   tranche-2 file locations; every criterion is unchanged. The chunks are built by
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 2
   --chunks 2` (28 and 27 articles, dealt round-robin by rank). Before this
   declaration the same script rebuilt the four tranche-1 chunks and their id lists
   byte for byte. The model developer does not read the chunks.
3. There is no owner override: none was used in tranche 1.
4. **Every admitted tranche-2 article is reserved for the one-shot sealed test of
   acuity model 1.1.** Tranche 2 has no development part.
5. Model 1.1 is developed only on the 42 tranche-1 records: the 27 development
   records and the 15 sealed records already consumed by the 1.0.0 test. The 25
   tranche-1 studies without a record (20 development, 5 sealed) stay without one.
6. Separate custodian sub-agents extract the tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md). Their brief,
   `.peykar/acuity-tranche2-extraction-brief.md`, is the tranche-1 brief with
   tranche-2 inputs and output locations, addendum 1 copied verbatim, and the
   tranche-1 rulings on ambiguous cases restated so that they are applied the same
   way. Until the 1.1 protocol is frozen the custodians report only structure and
   hashes, and their records stay in `.peykar/acuity-tranche2-sealed-2026-10-29/`.
7. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the
   tranche-1 record studies as development and the admitted tranche-2 studies as the
   sealed test. A tranche-1 study linked to a tranche-2 study is removed from that
   study's training set.
8. After tranche 2 no article of this screen is left unread. A later acuity version
   needs a new screen whose test part is reserved before triage.

At the tranche-1 rates (67 of 120 articles admitted; 15 of 20 sealed admits gave a
record) tranche 2 would give about 30 admits and 20 records. This is an expectation
for planning only; it is not a criterion.

## Briefs (local, not in git; SHA-256 at declaration)

| file | SHA-256 |
|---|---|
| `.peykar/acuity-tranche2-triage-brief.md` | `5f9c846197f988c6cbbfd948e280f22eb898545e87e960665c6c2e0f8954a20c` |
| `.peykar/acuity-tranche2-extraction-brief.md` | `50393d9704cc276e4518c1537b4eb414f902c5977097759186b42fe26a326d45` |
| `.peykar/write_acuity_tranche2_briefs.py` (writes both from the tranche-1 files) | `3926c350f8115b0379af34795f0ba90e087d48e4b717523442ee769d0992dd9b` |

The tranche-1 sources are unchanged: triage brief `19a39526…`, extraction brief
`786614de…` and addendum 1 `f93c4ef4…`.

## Triage outcome (2026-10-29)

**Triage.** Two reviewer sub-agents worked from masked previews only
([`triage-1.0.0.json`](triage-1.0.0.json), written by
`.peykar/write_acuity_tranche2_triage.py`). **27 admitted, 28 excluded**, with no
owner override. PMC5847305 was excluded only because its preview has no DOI (brief
criterion 4); it stays excluded, as declared. Context hints of the admits:
cross-linking 16, DSAEK 4, DMEK 4, DALK and PK compared 2, PK 1. The hints are not
binding; the custodians decide from the articles.

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 42 tranche-1 record studies as development (training of model 1.1) and all 27
admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) (run with
a byte-identical copy of the screening receipts in `screening/`) found 15 linked pairs:
10 tranche-2 studies lose linked training studies, two tranche-1 record studies form
one development cluster, and 3 pairs link two tranche-2 studies.

**Custody.** Four custodian sub-agents (C1–C4; 7, 7, 7 and 6 studies, dealt
round-robin over the sorted study keys in `.peykar/acuity-tranche2-extraction-assignments.json`,
SHA-256 `3f44109c…`) extract the admits into `.peykar/acuity-tranche2-sealed-2026-10-29/`.

## Custody outcome (2026-10-29)

The four custodians reported structure and hashes only (`custody-report-C1.json` to
`C4.json` in the sealed folder). **11 records with 23 series** were built: all group
means of logMAR at every time, 22 control and 1 treated; contexts cross-linking 10,
DMEK 9 (8 of them indication subgroups of one study), DSAEK 3 and other 1
(cross-linking combined with PTK and PRK for keratoconus); indications keratoconus
11, bullous keratopathy 5, mixed 3, Fuchs dystrophy 2 and graft failure 2; 7 records
public and 4 private by licence. The three admitted keratoplasty studies with PK
groups were all excluded, so the set has no PK series. Sixteen admitted studies have
no record:

- complete-case analysis sets (a minimum follow-up or a stated analysis set equal to
  the last tabulated visit): `acu-pmc11566242`, `acu-pmc12356456`, `acu-pmc4314761`,
  `acu-pmc5510105`, `acu-pmc6720021`, `acu-pmc6773968`, `acu-pmc7790794`,
  `acu-pmc8949731`, `acu-pmc9672700`;
- analysis sets selected by a postoperative outcome: `acu-pmc8786756`,
  `acu-pmc9983850` (also complete-case), `acu-pmc13002641` (repeat grafts inside the
  groups and outside grafts admitted by graft survival);
- decimal acuity only, or logMAR rows beside decimal rows without a per-eye
  conversion: `acu-pmc10573660`, `acu-pmc4128847`, `acu-pmc4527220`, `acu-pmc8199119`.

`.peykar/verify_acuity_tranche2_custody.py` rebuilt all 11 records byte for byte from
their specs and found the 19 raw copies equal to their screened sources; every
reserved study was assigned and reported once. Process notes from the reports: two
custodians (C2, C3) once started a build before the spec was on disk (two tool calls at
the same time); the builder wrote nothing and the record was rebuilt from the
unchanged spec. C3 edited its own report once to rename a summary key. Custodians
differ in whether they kept raw copies or exclusion-only specs for excluded studies;
no excluded study has a record.

**Status.** Acuity model 1.1 development (on the 42 tranche-1 records) adopted no
candidate, so there is no 1.1 protocol
([development log](../../../eye/acuity/model-1.1/development-log.md)). Tranche 2
stays sealed and unread for a later acuity version.
