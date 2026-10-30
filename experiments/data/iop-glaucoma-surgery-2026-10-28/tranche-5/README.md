# IOP tranche 5: fresh sealed test set for IOP model 1.4

Declared on 2026-10-30, after the one-shot tranche-4 sealed test of model 1.3.0 and
**before tranche 5 was triaged**. No tranche-5 article had been triaged, extracted or
read for values by then.

## Rule

1. Tranche 5 of [`tranche-order-1.0.0.json`](../tranche-order-1.0.0.json) holds ranks
   480–498: the last 19 articles that passed the mechanical table rule.
2. One reviewer sub-agent triages these articles from masked previews only, with the
   tranche-1 brief and criteria (`.peykar/iop-tranche5-triage-brief.md`, the tranche-4
   brief with tranche-5 file locations). The chunk is built by the unchanged
   [`screening/triage_chunks.py`](../screening/triage_chunks.py) `--tranche 5 --chunks
   1`, which still rebuilds the tranche-1 chunks byte for byte (`--check-tranche-1`).
   The model developer does not read the chunk.
3. **Every admitted tranche-5 article is reserved for the one-shot sealed test of IOP
   model 1.4.** Tranche 5 has no development part.
4. Model 1.4 is developed only on the record studies of tranches 1–4: the 229 record
   studies of tranches 1–3 and the 79 tranche-4 sealed records consumed by the 1.3.0
   test.
5. Separate custodian sub-agents extract tranche-5 records under
   [extraction protocol 1.0.0](../extraction-protocol-1.0.0.md) and the tranche-4
   brief's rulings and clarifications, applied the same way. Until the 1.4 protocol is
   frozen they report only structure and hashes, and their records stay in
   `.peykar/iop-tranche5-sealed-2026-10-30/`. The model developer uses the custody
   verifier only and does not read the custody reports.
6. Possibly shared cohorts: `scripts/link_study_authors.py` is run with the tranche-1
   to tranche-4 record studies as development and the admitted tranche-5 studies as
   the sealed test. A training study linked to a tranche-5 study is removed from that
   study's training set.
7. The owner rule of the earlier IOP triages applies unchanged: a reviewer exclusion
   whose reason is a percent IOP change or reduction from baseline at follow-up times
   is admitted for the custodians to decide (tranche-1 rule).
8. After tranche 5 no unread article of this dataset remains.

Known exposure before this declaration (`.peykar/check_iop_tranche5_exposure.py` and
`check_iop_tranche5_exposure2.py`, which print file names and status fields only): the
19 articles appear in the IOP screen's own files, where tranche-1 triage lists them as
`reserved`, and two of them (PMC11190021, PMC13117507) appear in the search results of
two corneal literature screens; neither reached those datasets' triage files. No
tranche-5 table or value has been read.
