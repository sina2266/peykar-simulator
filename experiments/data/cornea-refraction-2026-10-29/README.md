# Refraction after corneal laser refractive surgery — dataset 1.0.0 (2026-10-29)

Sixth eye component and fourth corneal one, after epithelial wound closure,
endothelial cell density and central corneal thickness: how the spherical
equivalent (SE, dioptres) changes after photorefractive keratectomy and other
surface ablation, LASIK and SMILE or other lenticule extraction. The SE moves from
its preoperative value to near the target within days and then drifts over months
(regression or overcorrection) while the epithelium and stroma remodel. It is the
main refractive outcome of these operations. Extraction follows
[extraction protocol 1.0.0](extraction-protocol-1.0.0.md).

Refraction changes additively (dioptres), unlike the five earlier components, whose
outcomes are fractions of the baseline. Records keep the SE in dioptres; the model
works on the change from the preoperative SE (see
`peykar.validation.cornea_refraction_data` and
`peykar.validation.eye_additive_evaluation`).

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py --queries screening/queries.json`
   listed 2,169 open-access Europe PMC articles (one query on refraction terms in
   abstracts of refractive-surgery studies, one on refractive-surgery titles) and
   retrieved 2,168 full texts; PMC8651116 returned HTTP 500. Summary and SHA-256
   receipts: `screening/literature-screen-*.json`.
2. `scripts/screen_corneal_tables.py` with a refraction topic, a time pattern and a
   refractive-surgery context pattern wrote masked previews of 1,215 tables in 793
   articles, with data cells hidden (`screening/table-screen-*.json`). A first run
   without the `--time` argument used the script's default hour/day pattern; it was
   deleted unread and rerun.
3. The mechanical rule in `screening/triage_rule.py` uses only those previews. It
   keeps tables whose caption, header or row labels mention the spherical equivalent
   or manifest refraction, a preoperative/baseline reference and at least two
   follow-up time labels (hyphenated labels such as "3-month" included), with no
   exclusion term in the caption (statistical models, reviews, agreement studies,
   lens or intraocular surgery, keratoconus or ectasia, orthokeratology, ring
   segments, keratoplasty). **89 articles passed.** Most flagged tables that failed
   had a preoperative and a single postoperative column.
4. **Tranche.** [`tranche-order-1.0.0.json`](tranche-order-1.0.0.json)
   (`screening/tranche_order.py`, salt `peykar-refraction-tranche-v1:`, size 120)
   ranks the 89 articles by a salted hash of their DOI; all fall in tranche 1. A
   later model version needs a new screen.
5. Triage, the development/sealed split and the author-overlap rule follow the
   earlier components: three reviewer sub-agents see masked previews only
   (`screening/triage_chunks.py`, `.peykar/refraction-triage-brief.md`); the split
   is frozen with its own salt (`peykar-refraction-holdout-v1:`) before any value is
   read.

Records use schema `cornea.refraction-record-1.0.0`
(`peykar.validation.cornea_refraction_data`). Sealed studies are extracted by
separate custodians and stay private until the evaluation protocol is frozen.

## Triage and split (2026-10-29)

[Triage 1.0.0](triage-1.0.0.json): three reviewer sub-agents read the masked
previews of the 89 rule-passing articles (`.peykar/refraction-triage-chunks/`, chunks
of 30, 30 and 29) and gave a reason for every decision; there was no owner override.
**39 admitted, 50 excluded.** The exclusions were out-of-scope operations (phakic
lenses, keratoconus or ectasia with cross-linking, eyes with earlier corneal surgery,
keratoplasty, scleral surgery, a rabbit relaxation study), sphere or cylinder without
the SE, a single postoperative time, baseline-only tables, proportions or
literature comparisons, enhancement or retreatment groups, outcome-selected cases,
model estimates and a secondary analysis of regulatory trial summaries.

Before any value was read, [the split was frozen](split-reservation-1.0.0.json) by
`scripts/reserve_refraction_split.py` (salt `peykar-refraction-holdout-v1:`, first 8
hex digits of `sha256(salt + lower(doi))` modulo 3 equal to 0 → sealed): **31
development and 8 sealed-test studies.** [Author overlap](author-overlap-1.0.0.json)
(`scripts/link_study_authors.py`, at least two shared Europe PMC author names) found 4
pairs: two development clusters leave training together in leave-one-study-out; no
sealed study has a linked training study.

## Extraction outcome (2026-10-29)

Three development extractors (D1–D3) and two sealed custodians (C1–C2), all
sub-agents, followed `.peykar/refraction-extraction-brief.md` (assignments in
`.peykar/refraction-extraction-assignments.json`).

**Number reader.** Four admissible studies print a minus sign set apart from its
digits by a no-break, thin or ordinary space (and one prints a figure dash), which
the shared cell reader refused. `refraction_number` in
`peykar.validation.cornea_refraction_data` now reads such a sign as the sign of the
number; no other text is transformed. All 21 development records built before the
change rebuilt byte for byte with it, and so did the seven sealed records built
before it (`.peykar/verify_refraction_custody.py`; the resumed custodian C2 also
checked its three). The three blocked development studies were then built by the
developer and the blocked sealed study by the resumed custodian.

**Development: 24 records with 43 series**, all group means: 38 control and 5
treated series; contexts LASIK 18, lenticule extraction 17, surface ablation 7 and
other 1; 37 myopic and 6 hyperopic series. The 13 CC BY records are in
`development/records/` with their source XML in `development/raw/`; the 11 records
under non-commercial or publisher licences are private
(`.peykar/refraction-development-private/records/`). Specs (`development/specs/`)
cite cells and copy no values. Seven development studies have no record:

- non-plano targets for some eyes without a separate plano-target table:
  `ref-pmc10648579`, `ref-pmc13163326`, `ref-pmc7693991` (planned monovision);
- analysis sets selected after surgery: `ref-pmc6480872` (eyes beyond ±0.5 D removed),
  `ref-pmc6278781` (groups defined by return at one year), `ref-pmc9126459` (eyes
  with high postoperative keratometry removed);
- medians only in the cited cells: `ref-pmc7816348`.

**Sealed:** the custodians built **8 records with 14 series** (all group means, all
myopia; LASIK 10, lenticule extraction 3, surface ablation 1; 12 control and 2
treated) and reported only structure and hashes. No sealed study was excluded.
`.peykar/verify_refraction_custody.py` rebuilt all 8 records byte for byte from their
specs and found every raw copy equal to its screened source.

No refraction value of these articles had been read by the model developer when the
rule, the tranche order, the extraction protocol, the triage and the split were
committed.
