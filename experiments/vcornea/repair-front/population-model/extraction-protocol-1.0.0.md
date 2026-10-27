# Extraction protocol 1.0.0 — corneal wound-closure population dataset

Frozen on 2026-10-27, before any outcome value of a newly admitted study was read.
It governs how original supplementary data become study records
(`cornea.wound-study-record-1.0.0`, validated by
`peykar.validation.cornea_population_data`). Extractors make **no modelling
decisions**: they do not fit, plot, compare with any model or summarize trends.

## Inclusion

1. In vivo corneal epithelial defects, measured repeatedly over time by
   fluorescein (or an equivalent vital stain) as projected open area: absolute
   area in any unit, percent of the initial area remaining, or percent healed.
2. The first value of every series is the post-wounding baseline at hour 0. A
   series without its own hour-0 measurement is excluded, never back-filled.
3. Per-animal/eye rows are used whenever present. Published group means are used
   only when no individual rows exist. Extractors never compute group means from
   individual rows and never create individual rows from means or SD.
4. Exclude in vitro scratch assays, organ culture, opacity/neovascular scores,
   stromal-only endpoints and any table whose quantity cannot be identified from
   labels plus Methods/figure captions. Record every exclusion with its reason.

## Values, times and identities

- Every value cites its exact source cell (`sheet!cell`) of the original file,
  whose SHA-256 is recorded in `source_files`.
- Time labels are converted exactly: days×24, minutes/60. The original label is
  kept in `notes` when it is not already in hours. No interpolation.
- `value_kind`: `area_absolute` (normalized by the same row's hour-0 value),
  `percent_remaining`, `percent_healed`, or `area_fraction`. If the meaning of a
  percentage is ambiguous, the series is excluded and the ambiguity recorded.
- Repeated measures of one eye keep one `series_id`. Both eyes of one animal stay
  separate series with the animal noted; they are not averaged.
- Values recorded as closed (0) stay 0. Text such as "healed" without a number is
  not converted to 0; it goes to `notes`.

## Groups and covariates

- `role: control` only for healthy, young-adult, wild-type animals receiving no
  treatment, vehicle, saline/PBS or sham. Everything else — drugs, devices,
  genotype changes including heterozygotes, diabetes, other disease, aged or
  neonatal animals, nerve lesions — is `role: treated`.
- `wound`: nominal diameter (mm) from Methods (trephine/marker/defect size), or a
  calibrated measured baseline area; method (Algerbrush, blade, burr, heptanol,
  ethanol, alkali, excimer...); `injury_family`; `basement_membrane` is `intact`
  only when the paper states it, `removed` for keratectomy/PTK/PRK/lamellar
  removal, otherwise `unknown`. Per-group diameters may override the default.
- Species group, strain, sex and age are recorded when stated.
- Licence: the article licence statement/URL from the publisher XML, plus any
  different supplementary-file terms.

## Sealed-test custody

Studies reserved for `sealed_test` by the frozen reservation rule are extracted
by a separate custodian who reports to the model developer **only** the output
of `structural_summary` (counts, covariates, observation hours) and the SHA-256
of each record. Records are stored outside the repository until the evaluation
protocol is frozen; their hashes are committed first. The model developer does
not open sealed records, plots or previews with numbers before that freeze.
