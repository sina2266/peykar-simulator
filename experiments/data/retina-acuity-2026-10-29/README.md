# Corrected visual acuity after treatment of macular disease — dataset 1.0.0 (2026-10-29)

Eighth eye component: how the best-corrected visual acuity (BCVA, logMAR or ETDRS
letters) of a group of eyes changes after macular edema (diabetic, vein occlusion,
postoperative, uveitic), neovascular age-related macular degeneration, myopic
choroidal neovascularization or central serous chorioretinopathy is treated with
intravitreal anti-VEGF agents, corticosteroids, laser, photodynamic therapy or
vitrectomy. Acuity is the functional outcome of these treatments and the counterpart
of the central retinal thickness component (`retina-cst-2026-10-28`), whose
vocabularies of indication, treatment, regimen and prior treatment it uses unchanged.
Extraction follows [extraction protocol 1.0.0](extraction-protocol-1.0.0.md); records
use schema `eye.retina-acuity-record-1.0.0`
(`peykar.validation.eye_retina_acuity_data`). Acuity in logMAR changes additively, as
for the corneal acuity component: the model works on the change from the
pretreatment acuity through `peykar.validation.eye_additive_evaluation`, in ETDRS
letters.

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json`
   listed 3,660 open-access Europe PMC articles (one query on acuity terms, a macular
   treatment and a macular disease in the abstract, one on macular-disease treatment
   titles with acuity terms in the text). Full texts already retrieved by the
   retinal-thickness screen on 2026-10-28 were reused and rehashed
   (`--reuse-fulltext`); 3,655 full texts were screened. Summary and SHA-256 receipts:
   `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with an acuity topic, a time pattern and a
   macular-disease context pattern wrote masked previews of 3,623 tables in 2,140
   articles, with data cells hidden (`screening/table-screen-*.json`).
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It is
   the corneal-acuity rule with pretreatment and pre-injection references added: a
   table qualifies when its caption, header or row labels mention corrected visual
   acuity (BCVA, logMAR, ETDRS), a baseline or pretreatment reference and at least two
   follow-up time labels, with no exclusion term in the caption. Articles that another
   eye component keeps for a test it has not run yet are dropped before any triage
   (`screening/reserved.py`: retinal-thickness tranches 3–4, IOP tranches 4–5 and
   corneal-acuity tranche 2; 694 identifiers). **378 articles passed the rule;
   119 further qualifying articles were dropped as reserved elsewhere.**
4. **Tranches.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-retina-acuity-tranche-v1:`, size 120)
   ranks the 378 articles by a salted hash of their DOI: **tranche 1 (120 articles) is
   triaged now; tranches 2–4 (120, 120 and 18 articles) stay untriaged and unread**,
   reserved as fresh test sets of later model versions.
5. Triage, the development/sealed split and the author-overlap rule follow the earlier
   components: four reviewer sub-agents see masked previews only
   (`screening/triage_chunks.py --tranche 1 --chunks 4`,
   `.peykar/retina-acuity-triage-brief.md`); the split is frozen with its own salt
   (`peykar-retina-acuity-holdout-v1:`, `scripts/reserve_retina_acuity_split.py`)
   before any value is read; possibly shared cohorts are linked by
   `scripts/link_study_authors.py`.

Sealed studies are extracted by separate custodians and stay private until the
evaluation protocol is frozen.

No visual-acuity value of these articles had been read by the model developer when
the queries, the rule, the reserved list, the tranche order and the extraction
protocol were committed. The retinal-thickness records of tranches 1–2 of the
thickness dataset (consumed by its tests) contain thickness values only; their
acuity values have not been read either.

## Triage and split (2026-10-29)

[Triage 1.0.0](triage-1.0.0.json) (`.peykar/write_retina_acuity_triage.py`): four
reviewer sub-agents read the masked previews of the 120 tranche-1 articles
(`.peykar/retina-acuity-triage-chunks/`, chunks of 30) and gave a reason for every
decision; there was no owner override. **62 admitted, 58 excluded.** The exclusions
were medians or IQR only, proportions or letter-gain categories only, model-based or
least-squares estimates, baseline-only or one-follow-up tables, decimal or Snellen
acuity, reviews, meta-analyses and cost-effectiveness models, out-of-scope conditions
(geographic atrophy, epiretinal membrane without edema, glaucoma, aphakia), subgroups
selected after baseline and one article without a DOI. Context hints of the admits
(not binding): diabetic macular edema 26, neovascular AMD 16, vein occlusion 10,
postoperative edema 2, uveitic edema 1, myopic CNV 1, other 1, not shown 5; anti-VEGF
34, steroid implant 9, combination 4, steroid injection 3, laser 3, vitrectomy 1, other
1, two arms of different types 2, not shown 5.

Before any value was read, [the split was frozen](split-reservation-1.0.0.json) by
`scripts/reserve_retina_acuity_split.py` (salt `peykar-retina-acuity-holdout-v1:`, first
8 hex digits of `sha256(salt + lower(doi))` modulo 3 equal to 0 → sealed): **41
development and 21 sealed-test studies.** [Author overlap](author-overlap-1.0.0.json)
found 3 pairs: two development clusters of two studies leave training together in
leave-one-study-out, and one sealed study loses one linked development study from its
training set.

## Extraction outcome (2026-10-29)

Five development extractors (D1–D5) and three sealed custodians (C1–C3), all
sub-agents, followed `.peykar/retina-acuity-extraction-brief.md` (assignments in
`.peykar/retina-acuity-extraction-assignments.json`). After the first four agents (D1–D3,
C1), `.peykar/retina-acuity-extraction-brief-addendum-1.md` fixed three readings from
their structural replies only: several baseline partitions of the same eyes (only the
first is extracted), how a mean is established (the Results text or another table of
the article that labels the same quantity also counts), and visit labels that merge two
arms' schedules (dated by the Methods for each arm, otherwise excluded).

**Development: 15 records with 52 series**: 24 group means and 28 per-eye series (one
study of a subretinal amniotic membrane graft for advanced AMD, 23 eyes with
neovascular AMD and 5 with geographic atrophy, all treated); 20 control and 4 treated
group means. Contexts of the group means: diabetic macular edema with anti-VEGF 5,
steroid implant 3, steroid injection 2 and anti-VEGF plus an oral statin 2 (treated);
vein occlusion with anti-VEGF 4, steroid implant 2 and anti-VEGF or steroid plus laser
2; neovascular AMD with anti-VEGF 1; uveitic edema with a steroid implant 1;
postoperative edema with micropulse laser 1 (treated); reticular pseudodrusen with
subthreshold laser 1 (treated). Value kinds: logMAR 43, ETDRS letters 7, letter changes
with their baseline 2. The 13 CC BY records are in `development/records/` with their
source XML in `development/raw/`; the 2 records under non-commercial licences are
private (`.peykar/retina-acuity-development-private/records/`). Specs
(`development/specs/`) cite cells and copy no values; 26 specs document exclusions
only:

- complete-case analysis sets (a minimum follow-up or a stated analysis set equal to
  the last tabulated visit, without a statement that no treated eye was left out):
  `rac-pmc11111667`, `rac-pmc11488816`, `rac-pmc11826659`, `rac-pmc11973271`,
  `rac-pmc12582924`, `rac-pmc12767068`, `rac-pmc13008709`, `rac-pmc13077981`,
  `rac-pmc13537940`, `rac-pmc13661413`, `rac-pmc4076260`, `rac-pmc5877501`,
  `rac-pmc6832619`, `rac-pmc7125496`, `rac-pmc8054474`, `rac-pmc8374782`,
  `rac-pmc8474890`, `rac-pmc8566718`, `rac-pmc8632047`, `rac-pmc9019565` (several of
  them also remove eyes after a post-baseline event);
- acuity not established as best-corrected logMAR or letters (habitual or pinhole
  acuity, decimal acuity, "VA" without a best-corrected statement, no unit):
  `rac-pmc11826692`, `rac-pmc4909676`, `rac-pmc6258584`, `rac-pmc6556523`,
  `rac-pmc6880423`;
- groups defined after baseline and a selected analysis set: `rac-pmc9333245`.

**Sealed:** the custodians built **9 records with 11 series** (all group means; 9
control and 2 treated; contexts diabetic macular edema with anti-VEGF 6 and steroid
implant 2, vein occlusion with bevacizumab plus an investigational adjunct 1 (treated),
macular edema of unstated cause with laser 1 and with bevacizumab plus laser 1
(treated); logMAR 8, letters 3) and reported only structure and hashes. Twelve sealed
studies were excluded: seven complete-case or post-baseline-selected sets, three whose
acuity is not established as best-corrected, one with unlabelled statistics and one
with multiple-imputation means. `.peykar/verify_retina_acuity_custody.py` rebuilt all 9
records byte for byte from their specs and found all 21 raw copies equal to their
screened sources; `.peykar/rebuild_retina_acuity_records.py development` rebuilt all 15
development records byte for byte. Process notes from the replies and reports: one
custodian (C3) once started a build before its spec was on disk (two tool calls at the
same time), and one extractor's first verification rebuild failed at argument parsing;
neither wrote anything, and the records were built from the unchanged specs. C3's text
search, restricted to this dataset folder, returned lines of development specs
(source-file metadata, licences and group descriptions, no values); the files were not
opened. This breaks custodian independence, not developer blindness.

No acuity value of these articles had been read by the model developer when the rule,
the tranche order, the extraction protocol, the triage and the split were committed.
Development records were then read for model development; sealed records stay private
and unread until the evaluation protocol is frozen.
