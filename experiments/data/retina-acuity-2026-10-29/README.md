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
