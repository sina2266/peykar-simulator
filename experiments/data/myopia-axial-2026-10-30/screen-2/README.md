# Myopia screen 2: fresh sealed test set for myopia model 1.2

Declared on 2026-10-30, after the one-shot sealed test of model 1.1 consumed tranche 2
of the first screen and **before this screen was run**. No article of this screen had
been searched, triaged, extracted or read by then.

## Rule

1. Queries ([`queries.json`](queries.json)), Europe PMC, open access only: (A)
   axial-length terms, myopia and children in the abstract, without the first screen's
   treatment and time terms; (B) myopia in the title and "axial length" or "axial
   elongation" anywhere in the text. The 1,004 articles of the first screen
   (`.peykar/myopia-literature-screen-2026-10-30/screen.json`) are skipped
   (`scripts/screen_corneal_wound_literature.py --exclude-screen`).
2. The full texts get the first screen's masked table screen (`scripts/screen_corneal_tables.py`
   with the patterns of `.peykar/run_myopia_table_screen.py`) and its mechanical rule,
   unchanged (`../screening/triage_rule.py`, `article_qualifies`).
3. Every article that passes the rule gets the rank of
   `sha256("peykar-myopia-axial-screen2-tranche-v1:" + lower(doi or pmcid))`
   ([`tranche_order.py`](tranche_order.py)); screen-2 tranche 1 is the first 120 ranks.
4. **Every article of screen-2 tranche 1 that masked triage admits is reserved for the
   one-shot sealed test of myopia model 1.2.** Later screen-2 tranches stay untriaged
   and unread, reserved for later versions.
5. Model 1.2 is developed only on the 59 consumed records: the 33 tranche-1 records and
   the 26 tranche-2 records consumed by the 1.1 test.
6. Three reviewer sub-agents triage screen-2 tranche 1 from masked previews only, with
   the tranche-1 criteria; the model developer does not read the chunks. Separate
   custodian sub-agents extract the admitted studies under extraction protocol 1.0.0 and
   the tranche-2 brief (with the readings settled during tranche-1 extraction and the
   reporting field `n_by_visit`), with screen-2 paths. Until the 1.2 protocol is frozen
   the custodians report only structure and hashes, their records stay in
   `.peykar/myopia-screen2-sealed-2026-10-30/`, and the developer opens no custody report.
7. Possibly shared cohorts (for example a later report of a trial already extracted):
   `scripts/link_study_authors.py` is run with the 59 consumed record studies as
   development and the admitted screen-2 studies as the sealed test; a consumed study
   linked to a sealed study is removed from that study's training set.
