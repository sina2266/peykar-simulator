# Myopia registry tranche 1: posted trial results as additional sealed test data for model 1.2

Declared on 2026-10-30, after the screen-2 triage admitted only 4 articles and **before
any registry result was read**. Until then only the list of matching ClinicalTrials.gov
records had been seen (identifiers, titles and enrolment counts of 21 records with posted
results; no outcome or baseline value).

The open-access literature is nearly exhausted for this component (the two screens found
100 admissible articles in 3,771), while sponsors post group results of many myopia
trials on ClinicalTrials.gov: arm-level means at each time point with the number of
participants or eyes analysed, in a structured form and under no publication licence.
They give fresh studies that no model of this dataset has seen.

## Rule

1. **Source.** One query of the ClinicalTrials.gov API v2 (`/api/v2/studies`,
   `query.cond = myopia`, `query.outc = "axial length"`, `aggFilters = results:with`).
   Every returned record is saved whole with a SHA-256 receipt in
   `.peykar/myopia-registry-1-2026-10-30/raw/` by
   [`fetch_registry.py`](fetch_registry.py).
2. **Structure screen** ([`registry_screen.py`](registry_screen.py); reads structure only
   and prints no value). A record is kept if (a) its eligibility admits only participants
   younger than 19 years, (b) it has at least one posted outcome measure whose title
   names axial length or axial elongation with a millimetre unit, and (c) its NCT number
   does not appear in the source article of any of the 59 consumed record studies or of
   the 4 screen-2 admits (the same trial is then already in the data), and no other kept
   record has the same NCT number.
3. **Tables.** Each kept record is converted deterministically into one JATS-like XML
   file of tables ([`registry_tables.py`](registry_tables.py)): one table of baseline
   measures and one table per posted outcome measure, with the groups as columns, the time
   points or classes as rows and the numbers analysed as their own rows, so that
   custodians cite cells (`file::table!RrCc`) exactly as for article tables and the
   unchanged record builder and validator apply.
4. **Every kept record is reserved for the one-shot sealed test of myopia model 1.2**,
   with study key `myo-<nct number>` (lower case), together with the 4 screen-2 admits.
   Model 1.2 stays developed on the 59 consumed records.
5. **Extraction.** Custodian sub-agents extract the kept records under extraction
   protocol 1.0.0 and the tranche-2 brief with registry paths and one addendum: a trial
   arm is a group; a record needs the arm's baseline axial length (a baseline measure, or
   an outcome measure at time 0) and its axial length or change at one or more later time
   points; the posted number analysed at each time point goes into `n_by_visit`; a record
   whose posted axial lengths are adjusted (least-squares) means only, or that gives no
   baseline axial length, is excluded with that reason. The custodians report only
   structure and hashes, their records stay in `.peykar/myopia-registry1-sealed-2026-10-30/`,
   and the developer opens no custody report before the 1.2 protocol is frozen. Registry
   records are in the **private** licence class (revised before any extraction; the first
   wording said public): the posted results carry no open licence statement (the
   [ClinicalTrials.gov terms](https://classic.clinicaltrials.gov/ct2/about-site/terms-conditions)
   describe a US Government database, not a CC licence), and the publishers' frozen
   classifier makes public only CC BY and CC0 statements. Their per-series metrics are
   published; their values are not.
6. Possibly shared cohorts that the NCT check misses are handled as in the screens:
   `scripts/link_study_authors.py` cannot link registry records (they have no author
   list), so registry records keep their full training sets; this is a known limitation.

## Screen outcome (2026-10-30)

**Fetch.** `fetch_registry.py` returned one page with 21 records
([`registry-fetch-summary.json`](registry-fetch-summary.json); receipts with SHA-256 of
every page and record in [`registry-receipts.json`](registry-receipts.json)).

**Structure screen** ([`registry-screen-1.0.0.json`](registry-screen-1.0.0.json); no
value read): **17 kept, 4 dropped**: three without a posted axial-length outcome in
millimetres (one of them a presbyopia implant trial in adults, one a trial named in the
article of the consumed study myo-pmc10887607) and one whose eligibility reaches age 19.
None of the 17 kept trials is named in a consumed or screen-2 article.

**Tables.** `registry_tables.py` 1.1.0 (version 1.0.0 plus a table `l1` of sites per
country, added before any extraction so that custodians can give each group's region)
converted the 17 kept records ([`registry-tables-1.1.0.json`](registry-tables-1.1.0.json):
SHA-256 of every derived file; the files stay in `.peykar/myopia-registry-1-2026-10-30/tables/`).

**Reservation.** [`split-reservation-1.0.0.json`](split-reservation-1.0.0.json): the 59
consumed record studies as development and the 17 kept records as `sealed_test`
(study keys `myo-nct…`, with their registry URLs).
