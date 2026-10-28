# IOP tranche 2: fresh sealed test set for IOP model 1.1

Declared on 2026-10-28, after the one-shot sealed test of model 1.0.0 and **before
tranche 2 was triaged**. No tranche-2 article had been triaged or read by then.

## Rule

1. Tranche 2 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds
   ranks 120–239: 120 articles that passed the mechanical table rule.
2. Three reviewer sub-agents triage these articles from masked previews only, with
   the tranche-1 brief and criteria. The chunks are built by
   [`screening/triage_chunks.py`](../screening/triage_chunks.py), which rebuilds the
   tranche-1 chunks byte for byte (`--check-tranche-1`). The model developer does not
   read the chunks.
3. **Every admitted tranche-2 article is reserved for the one-shot sealed test of IOP
   model 1.1.** Tranche 2 has no development part.
4. Model 1.1 is developed only on the 72 tranche-1 records: the 47 development
   records and the 25 sealed records already consumed by the 1.0.0 test.
5. Separate custodian sub-agents extract tranche-2 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and its brief. Until
   the 1.1 protocol is frozen they report only structure and hashes, and their
   records stay in `.peykar/iop-tranche2-sealed-2026-10-28/`.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the
   tranche-1 record studies as development and the admitted tranche-2 studies as the
   sealed test. A tranche-1 study linked to a tranche-2 study is removed from that
   study's training set.
7. Tranches 3–5 (259 articles) stay untriaged and unread for later versions.

The IOP medians that the developer saw earlier in one table (PMC11852638, a
medians-only micro-shunt study) are in tranche 4.

## Triage and extraction outcome (2026-10-28)

**Triage.** Three reviewer sub-agents worked from masked previews only
([`triage-1.0.0.json`](triage-1.0.0.json)). They admitted 84 articles and excluded 36.
The owner applied the tranche-1 rule for percent-change tables to three exclusions
(PMC10478966, PMC8186654, PMC12748536); the custodian decides whether such a table is
extractable. **87 admitted, 33 excluded.**

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json) lists
the 72 tranche-1 record studies as development (training of model 1.1) and all 87
admits as `sealed_test`.

**Author overlap.** [`author-overlap-1.0.0.json`](author-overlap-1.0.0.json) found 37
linked pairs: 18 tranche-2 studies lose linked training studies, and 10 pairs link two
tranche-2 studies.

**Custody.** Seven custodian sub-agents (C1–C7) extracted the admits into
`.peykar/iop-tranche2-sealed-2026-10-28/` and reported only structure and hashes.
**74 records with 192 series** were built:

- 120 group means and 72 single eyes;
- 149 control and 43 treated series;
- 189 absolute IOP series and 3 percent-change series;
- 73 human records and 1 rabbit record (18 single-eye series);
- 17 series with fewer than two follow-ups.

Contexts: angle MIGS 63, tube 27, trabeculectomy 25, subconjunctival MIGS 31,
cyclodestruction 15, non-penetrating surgery 7, cataract 5 and `other` 1 (human), and
rabbit trabeculectomy 18. All 74 records rebuild byte for byte from their specs and
hash-checked source copies.

Thirteen admitted studies have no record:

- censored at additional surgery before the first tabulated follow-up, or without
  per-visit eye numbers: `iop-pmc11482709`, `iop-pmc12089333`, `iop-pmc12533712`,
  `iop-pmc2664114`;
- analysis set selected by a postoperative outcome (complications, reoperation,
  extra medication, failures removed): `iop-pmc10024434`, `iop-pmc11952685`,
  `iop-pmc12801984`, `iop-pmc13605410`, `iop-pmc6822852`, `iop-pmc7323794`,
  `iop-pmc8057832`;
- imputed follow-up values: `iop-pmc6134405`;
- mixed-model estimates only: `iop-pmc12748536`.

Process notes:

- **Interrupted run.** The first launch of all seven custodians stopped at a usage
  limit. Only C5 had written files (7 of its 12 studies, no report). The resumed C5
  rebuilt those 7 records from their specs with identical SHA-256 values, kept them,
  and finished the other 5. The other six custodians had written nothing and started
  fresh.
- **Shared-folder read (C2).** One C2 validation step globbed the shared `records/`
  folder and parsed the other custodians' records present at that time in memory, to
  filter for C2's own studies. Nothing from them was printed or kept. This breaks
  custodian independence, not developer blindness; it is recorded in C2's report.
  Later custodians were told to address only their own files by exact name.
- **Censoring ruling applied by design (C4, `iop-pmc6980562`).** IOP was not collected
  after retreatment, and the Methods allow retreatment only after month 3. The
  custodian dropped month 6 and kept day 1 to month 3. Their eye counts are below
  baseline because of missed visits, not censoring. The owner accepted this without
  seeing any value: the censoring ruling exists to exclude outcome-selected visits,
  and the design rules them out before month 3.
