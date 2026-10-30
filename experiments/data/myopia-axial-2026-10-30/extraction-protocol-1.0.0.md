# Extraction protocol 1.0.0 — axial length in childhood myopia, treated or not

Frozen on 2026-10-30, before any axial-length value of an admitted study was read by the
model developer. It governs how published tables become records of schema
`eye.myopia-axial-record-1.0.0` (validated by `peykar.validation.myopia_axial_data`,
built by `scripts/build_myopia_record.py`). Extractors make **no modelling decisions**:
no fitting, plotting, comparisons or trend summaries.

## Scope

How the ocular axial length (AL, millimetres) of a group of myopic children or
adolescents grows from its baseline value over months to years under single-vision
correction or a myopia-control treatment: atropine eye drops (any concentration),
orthokeratology, myopia-control spectacle lenses, multifocal or extended-depth-of-focus
soft contact lenses, repeated low-level red-light therapy, combinations of these,
outdoor-time interventions, or another non-surgical treatment. Axial elongation is the
main measure of myopia progression and of the slowing these treatments aim at. In scope
are human children and adolescents with myopia (mean age at enrolment 3–18 years).
Out of scope: premyopic, emmetropic or hyperopic children (prevention trials), adults,
eyes after any surgery (including posterior scleral reinforcement), pathological or
syndromic myopia studied as such, and animals.

## Inclusion

1. In vivo **axial length** of the same group at the **baseline** (enrolment or the
   start of treatment) and at one or more later follow-ups, by optical biometry or
   ultrasound: labelled axial length, AL, AXL or ocular axial length. Vitreous chamber
   depth, choroidal thickness, spherical equivalent or any refraction are not axial
   length and are not admissible.
2. Units: millimetres.
3. Reference (time 0): the baseline axial length of the same group. A series without its
   baseline axial length is excluded, never back-filled.
4. Group means (± SD/SE) only when no per-child rows exist. Medians/IQR, ranges,
   proportions (for example "elongation of at least 0.3 mm"), p-values, model-adjusted
   estimates (least-squares, mixed-model, ANCOVA, GEE or otherwise adjusted means) and
   "change" columns without the baseline are not admissible. When a table gives both raw
   and adjusted means, the raw means are used.
5. Exclude groups or analysis sets selected by a post-treatment outcome ("fast
   progressors", "responders", children with complete data chosen after the outcome).
   A group restricted to the children who completed every tabulated visit is admitted
   only when the article reports how many children were enrolled or randomised in that
   group and at least 70% of them are in the tabulated set; otherwise it is excluded as
   a complete-case set. Ordinary attrition (fewer children at later visits without an
   outcome-defined exclusion) is kept, with a note. Groups defined before treatment
   (age, baseline refraction or axial length, sex, region, protocol) are kept as
   separate groups.
6. A whole cohort tabulated together with its pre-treatment subgroups is extracted as
   the subgroups, so no child is counted twice.
7. When right and left eyes of the same children are tabulated separately, only the
   right eyes are extracted; a table of both eyes pooled or of one eye per child is used
   as tabulated.
8. Phases after a treatment was stopped (washout, rebound) and phases after a group
   switched to another treatment (crossover) are not extracted; a switching group is
   extracted up to the switch.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted over
  thead then tbody, cells within a row, from 1, no span expansion) of the article JATS
  XML whose SHA-256 is recorded. Signs are kept as printed (a unicode minus is a
  minus); the builder reads decimal commas and a leading point without its zero.
- `value_kind`: `axial_absolute` (axial length at every time, month 0 = baseline) or
  `axial_change` (axial length minus the baseline axial length, change at month 0 = 0,
  cited as `{"definition": "definition:<where>", "value": 0}`; admissible only when the
  same group's baseline axial length is tabulated, stored as `reference_cell`). If a
  table gives both, the absolute series is used.
- Times are converted to months exactly: days/30.4375, weeks × 7/30.4375, years × 12.
  Ranges ("6–12 months"), "final visit" or "last follow-up" without a time are excluded.
- `n` is the number of children (or eyes, as tabulated) at baseline; changes in
  follow-up numbers go to `notes`.
- `device` per series: `iolmaster` (IOLMaster 500 or 700), `lenstar` (Lenstar), `aladdin`
  (Topcon Aladdin), `myopia_master` (Oculus Myopia Master), `ultrasound` (A-scan
  ultrasound biometry), `other` or `unstated`.
- Refraction, choroidal thickness, corneal or other biometry, when tabulated, are
  mentioned in `notes` only.

## Groups and context

- Treatment context per group (`treatment`):
  - `single_vision`: single-vision spectacles or single-vision contact lenses, with or
    without placebo drops (the usual control arm);
  - `atropine`: atropine eye drops at a stated concentration (`atropine_percent`,
    required) with single-vision correction;
  - `orthokeratology`: overnight orthokeratology lenses of any design;
  - `defocus_spectacles`: myopia-control spectacle lenses (defocus incorporated multiple
    segments, highly aspherical lenslets, diffusion optics, peripheral-defocus or other
    myopia-control designs, bifocal or progressive-addition lenses);
  - `multifocal_contacts`: multifocal, dual-focus or extended-depth-of-focus soft contact
    lenses;
  - `red_light`: repeated low-level red-light therapy;
  - `combination`: two of the above in the same group (for example atropine with
    orthokeratology); `atropine_percent` is given when atropine is one of them;
  - `outdoor`: an outdoor-time intervention without one of the above;
  - `other`: anything else in scope (for example another drug or undercorrection).
- `region` per group: where the study was done: `east_asia`, `south_asia`,
  `southeast_asia`, `europe`, `north_america`, `latin_america`, `middle_east`, `africa`,
  `oceania`, `mixed` or `unstated`.
- `mean_age_years` per group: the mean age at baseline when reported, else `null`.
- Role: every group is `control` in dataset 1.0: each treatment is a context of its own
  and every group enters the population fit of its context.
- Species: human only.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-myopia-axial-holdout-v1:`) are extracted by separate custodians who report only
structure and SHA-256 values until the evaluation protocol is frozen.
