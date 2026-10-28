# Intraocular pressure after glaucoma procedures — dataset 1.0.0 (2026-10-28)

This is the fourth eye component, after the three corneal ones (epithelial wound
closure, endothelial cell density and central corneal thickness). It covers how
intraocular pressure (IOP, mmHg) falls after a glaucoma procedure and drifts back
over months to years: trabeculectomy, drainage tubes, micro-shunts, angle surgery,
non-penetrating surgery, cyclodestruction, laser trabeculoplasty and cataract
surgery. IOP reflects the balance between aqueous production and outflow.
Extraction follows [extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json
   --reuse-fulltext <earlier screens>` listed 2,572 open-access Europe PMC articles
   and retrieved 2,570 full texts. 132 were reused byte for byte from the endothelium
   and thickness screens and rehashed. Two downloads returned HTTP 500: PMC8197767 and
   PMC9125576. Summary and SHA-256 receipts are in `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` was run with an IOP topic, a time pattern and a
   glaucoma-procedure context pattern. It wrote masked previews of 3,040 tables in
   1,525 articles, with data cells hidden (`screening/table-screen-*.json`).
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It
   kept tables whose caption, header or row labels mention IOP, a
   preoperative/baseline reference and at least two follow-up time labels, with no
   exclusion term in the caption. 499 articles passed.
4. **Tranches.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-iop-tranche-v1:`) ranks the 499
   articles by a salted hash of their DOI. Tranche 1 holds the first 120 and is used
   now. The other 379 articles stay **untriaged and unread**, reserved for testing
   later model versions on studies no model has seen. This keeps fresh test data
   available after the sealed set below is used once.
5. [Triage 1.0.0](triage-1.0.0.json) reviewed the masked structure of tranche 1. Three
   reviewer sub-agents followed `.peykar/iop-triage-brief.md`, saw only masked
   previews and gave a reason for every decision. The owner made one override: a
   percent-change table is admissible without a reference under protocol 1.0.0.
   **86 admitted, 34 excluded.** The exclusions were medians only, model estimates,
   outcome-selected or censored analysis sets, change without a baseline, reviews,
   out-of-scope surgery, fellow eyes and missing DOIs.
6. Before any IOP value was read, [the split was frozen](split-reservation-1.0.0.json)
   with its own salt (`peykar-iop-glaucoma-holdout-v1:`): **54 development, 32 sealed
   test**.

The model developer had seen the IOP medians in one table, PMC11852638, a
micro-shunt study reported only as medians, while checking the thickness
extraction. That study is in reserved tranche 4 and is not admissible under
inclusion 3, because it reports medians only. No other IOP value of these articles
had been read. Records use schema `eye.iop-record-1.0.0`
(`peykar.validation.eye_iop_data`) and are built from cell-citing specs by
`scripts/build_iop_record.py`. Sealed studies are extracted by separate custodians
and stay private until the evaluation protocol is frozen.

## Extraction outcome (2026-10-28)

Four development extractors and three sealed custodians (sub-agents) worked from
`.peykar/iop-extraction-brief.md` and protocol 1.0.0.

**Development:** 54 specs (`development/specs/`; cells cited, no values copied)
and **47 records with 101 series**:

- 95 control and 6 treated series;
- 82 group means and 19 single eyes.

The 23 CC BY records are in `development/records/`, with their source XML in
`development/raw/`. The 24 non-commercial, share-alike, no-derivatives or
publisher-licensed records are private. Seven studies have no record:

- `iop-pmc11897240`: medians only.
- `iop-pmc12206618`, `iop-pmc4417344`, `iop-pmc5892968`, `iop-pmc9328986`: means
  computed after removing eyes with reoperation or repeat laser, or an inclusion
  rule requiring no additional procedure. These are outcome-selected sets.
- `iop-pmc12611114` (rabbits): the implant-extrusion eye was left out, and the
  control eyes are unoperated fellow eyes.
- `iop-pmc9641924`: only percent *reductions*, whose sign is opposite to schema
  1.0.0's `percent_change`; the builder does not transform values.

**Sealed:** the custodians built **25 records with 68 series** (44 group means and
24 single eyes) and reported only structure and hashes. Seven studies were
excluded:

- five were censored at additional surgery or outcome-selected (`iop-pmc10471744`,
  `iop-pmc6680218`, `iop-pmc12858626`, `iop-pmc13059178`, `iop-pmc12932313`);
- `iop-pmc9880568`: values limited to eyes that needed no further intervention;
- `iop-pmc8544104`: medians only.

How the rules were applied, the same way in both splits:

- **Censoring.** Censoring IOP at additional glaucoma surgery makes later visits an
  outcome-selected set (inclusion 4). When censoring starts after some visits that
  still have the full baseline number of eyes, those visits are kept and later ones
  excluded (development `iop-pmc6886553`, `iop-pmc8745353`; sealed
  `iop-pmc13262184`). Otherwise the study has no record.
- **Subgroups.** A whole cohort tabulated with its preoperative subgroups is
  extracted as subgroups, so no eye is counted twice.
- **Imputed values.** An arm whose follow-up means were LOCF-imputed, including
  reoperated eyes, was excluded (`iop-pmc12325217`, kept for the other arm).

Builder correction: `iop_number` in `eye_iop_data` reads decimal commas ("15,3 ±
4,2" is 15.3 mmHg). A comma before three digits stays a thousands separator.
After the correction, all 46 development records built so far and all 25 sealed
records rebuilt byte-identical; sealed records were compared by SHA-256 only.
`iop-pmc7125472`, which uses decimal commas throughout, was then built.
