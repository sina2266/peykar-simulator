# Peykar

**Biological Modeling & Simulation Platform**

Peykar is a source-available foundation for registering, composing, executing, and
evaluating versioned biological models. Its long-term scope extends from molecular
and cellular models to tissues, organs, and interacting systems.

**Release 0.3.0 is an executable engineering foundation, not a validated biological
simulator.** It includes two synthetic reference engines and a working, explicit
operator workflow for the archived vCornea model in isolated CompuCell3D, plus a
separate pinned 4.6.0 full-horizon publication replay with retained actual results.
General multiscale execution and independent biological validation remain future
work. The local six-workspace [research workbench](docs/epics/E4-research-workbench.md)
now exposes the implemented scientific workflows.

The current engineering continuation adds checked worker result/runtime contracts,
offline admission of digest-pinned external files, operator-mapped CompuCell3D
scalar exports, frozen seed-paired comparisons and persistent knowledge-gap
declarations. [EPIC E1](docs/epics/E1-vcornea-native-runtime.md) adds licensed source
acquisition, a locked native runtime, real CSV/Parquet ingestion and seeded native
regression. The [corneal publication reconstruction](experiments/vcornea/publication/README.md)
now retains 65 original replicates, unchanged original analysis and all 17 final
paper supplements. Original analysis was actually replayed. Later-source formation
failed the frozen comparison; both complete180-day runs using recovered July2024
mechanisms now pass all frozen count/trajectory/thickness checks. Original math and
actual tissue layers were inspected. All four most recent one-worker jobs have
now also completed their full180/210-day horizons. The new full January comparison
reuses six already completed injuries: three of eight candidates pass every
scalar criterion, five fail. The separate July moderate pair fails its trajectory
criterion. Original mathematics and exact saved-state consistency checks pass for
both new analyses; the earlier native abort and one-site volume inconsistency
remain retained as failures. No outputs, parameters or frozen thresholds were
changed to force agreement. See the [actual terminal results and figures](experiments/vcornea/publication/serial-final-results-2026-10-24.md)
and [plain Persian report](experiments/vcornea/publication/results-2026-10-24.fa.md).
The selected published output groups still lack their exact original run inputs;
the [concrete missing-input inventory](experiments/vcornea/publication/original-run-metadata-needed.md)
records what is needed. Full publication reproduction and independent biological
validation remain open. No native simulation is still running at the latest check.
All source families, negative results, original bytes and failed attempts remain.

The [experimental corneal evidence path](experiments/vcornea/experimental/README.md)
now retains inspected pinned Hugging Face samples, original rabbit wound data and
healthy human OCT aggregates from22subjects. A new offline observation operator
measures actual epithelial lattice spans; four completed formation outputs were
compared descriptively with the52.5±3.6µm clinical aggregate. Source3 means54.01/
55.63µm and source7 means61.69/61.40µm; region/measurement mappings remain unresolved,
so these are not clinical accuracy scores or biological passes. All original
data/terms and incompatible endpoints remain explicit. [Plain Persian results](experiments/vcornea/experimental/results.fa.md).

A separate offline [rabbit superficial-repair component](experiments/vcornea/repair-front/README.md)
now predicts projected wound area using independently published 1986 radius-motion
parameters. Against all three actual 2017 PBS rabbits over 0–54h, the nonbaseline
cohort-mean RMSE is **4.02 percentage points**, maximum error **5.86 points**, passing
the frozen 5/10-point engineering targets without fitting. Individual errors remain
explicit. A second independent 2021 removal-method challenge **fails all three
groups** (RMSE 24.81–26.45 points); this evidence rules out generalizing the first
agreement to arbitrary debridement. Both actual executions, raw observations,
negative results and figures are retained. This is projected surface kinematics,
not validated cellular/chemical vCornea behavior or clinical approval.

The new [data-driven corneal candidate](experiments/vcornea/repair-front/learned-front/README.md)
was actually trained on six rabbits, with three entire intermediate-dose animals
excluded from training. Its heldout mean-area error improves from10.26 to5.96
percentage points, but **fails** the unchanged5-point RMSE/10-point maximum-error
targets. Independent2021 comparisons also fail; a newly acquired72h experimental
study rejects it even using conservative graph-derived lower constraints. All
frozen splits, original sources, actual outputs and negative results are retained.
This is an optional offline empirical component, without biological promotion.

The [2026-10-27 corneal continuation](experiments/vcornea/repair-front/conditioned-forecast/README.md)
adds a separately versioned forecast using actual0h/24h wound areas, a new original
2019 table reader and an executable prefix-only precision audit. Its full84h
future comparison fails (RMSE14.99pp/max19.66pp). Two early areas leave lag/speed
ambiguous; the audit withholds a5/10pp precision claim under its explicit model
assumptions. Additional original images/metadata are retained with incomplete
cohort/measurement limits. Corneal scope remains insufficient for broad biological
use; no earlier failure, data/model version or acceptance criterion is replaced.

The separate [causal observation-update replay](experiments/vcornea/repair-front/online-forecast/README.md)
now updates a radius trend after each real measurement. All98 actual isolated
children completed;96 numeric next observations across16 separate animal/group
series were compared with persistence, first-to-current trend and the unchanged
initial-only prior. All16 improve persistence, but only5 meet the fixed gates;
the general precision claim is rejected. This is observation-informed forecasting
on consumed data, not fresh independent validation or initial-state simulation.
The reusable [Persian continuation prompt](docs/continuation-prompt.fa.md) and
[current scientific handoff](docs/continuation-state.fa.md) explain the user's
whole-platform goal, actual tests, open failures and next evidence dependency.

The [2026-10-27 cross-study population model](experiments/vcornea/repair-front/population-model/README.md)
replaces single-study rules with a hierarchical radius–lag model (learned study
noise, censored zero areas, cohort assimilation) and an instance-based ML
comparator, trained on a new [outcome-blind literature dataset](experiments/data/cornea-wound-population-2026-10-27/README.md)
built from 1,993 screened open-access papers. Its protocol was frozen and pushed
before a one-shot **sealed test on two independent mouse studies**: initial-state
prediction of both control-group mean curves passed 5/10 pp (2.26/2.83 and
3.60/6.34 pp), and 52/64 animal series passed the next-observation criterion
(42/64 for the previous last-two trend). The 90% interval covered 79.6%, below its
frozen 80% floor. Development cross-study prediction for rabbits and rats fails,
so the supported scope is mouse 2 mm central debridement controls.
On 2026-10-28 dataset release 1.1.0 added article-table studies (rabbit/rat alkali
burns, rat debridement under insults, human PRK and cross-linking; 16 development
studies), and **model 2.0.0** added a slowing closure tail. Its protocol was frozen
and pushed (`2cee1b01`) before a second one-shot sealed test (IL-10 mice, 55
series; i-PRF patients): cohort intervals were calibrated within both frozen
windows (80%: 70.3%, 90%: 82.4%; 1.0.0 missed the 80% window), cohort RMSE beat
the last-two trend (13.1 vs 16.3 pp), but accuracy did not improve over 1.0.0 and
initial-state prediction failed for both new control groups (18.0 and 25.0 pp). The
[Persian audit](docs/corneal-work-audit-2026-10-27.fa.md) reviews the earlier work
and pauses further unchanged vCornea replays until original run inputs exist.
The next eye component is [corneal endothelial cell density after surgery](experiments/vcornea/endothelium/README.md)
(keratoplasty, cataract, glaucoma, phakic-lens and vitreoretinal surgery), built
from an [outcome-blind dataset](experiments/data/cornea-endothelium-2026-10-28/README.md)
of 843 screened open-access papers (59 development and 17 custodian-extracted sealed
records). **Model 1.0.0** (hierarchical power-law decline per tissue family, with
Student-t, offset-anchored forecasting) was frozen and pushed (`00d2cd09`) before a
one-shot sealed test: next-follow-up forecasts 5.60 pp versus 7.36 pp for
persistence, intervals inside both calibration windows (71.1%/82.9%), landmark
forecasts 6.85 versus 12.41 pp, and 23/30 series within 5/10 pp. Initial-state
prediction of new control groups reached 13/29 within criteria, and a late
post-60-month drop in a 10-year DMEK series was followed slowly
([Persian results](experiments/vcornea/endothelium/results.fa.md)).
The third corneal component is [central corneal thickness after surgery](experiments/vcornea/thickness/README.md):
swelling and return after cataract and other intraocular surgery, and thinning of
an edematous cornea after keratoplasty. It is built from an
[outcome-blind dataset](experiments/data/cornea-thickness-2026-10-28/README.md) of
1,179 screened open-access papers, with 33 development and 15 custodian-extracted
sealed records. **Model 1.0.0** uses a hyperbolic return from the day-1 swelling,
exact Gaussian-mixture posteriors, and a graft family that pools the long-term level
as absolute thickness, so the preoperative CCT sets the expected fraction. It was
frozen and pushed (`deef7664`) before a one-shot sealed test (15 studies, 29
series, mostly cataract), with these results:

- next-follow-up forecasts 1.60 pp versus 2.06 for persistence;
- intervals inside both windows (89.2%/96.9%);
- landmark forecasts 2.31 versus 4.01 pp;
- 28/29 series within 5/10 pp.

Laser capsulotomy groups extracted as `other` were wrongly given the graft-family
swelling prior. A 10-year DMEK series thickened late, which the model does not
represent ([Persian results](experiments/vcornea/thickness/results.fa.md)).
The fourth eye component, and the first outside the cornea, is
[intraocular pressure after glaucoma procedures](experiments/eye/iop/README.md):
trabeculectomy, tubes, micro-shunts, angle surgery, cyclodestruction, laser
trabeculoplasty and cataract surgery in glaucoma eyes. It is built from an
[outcome-blind dataset](experiments/data/iop-glaucoma-surgery-2026-10-28/README.md)
of 2,572 screened open-access papers. Only the first of five salted tranches was
used, so 379 screened papers stay unread for later tests. There are 47 development
and 25 custodian-extracted sealed records. **Model 1.0.0** uses an exponential return
from the day-1 deviation, one shared regression of the long-term level on the
preoperative pressure, one hierarchy per species and predictive-median forecasts. It
was frozen and pushed (`ff470345`) before a one-shot sealed test (25 studies, 43
group-mean series), with these results:

- next-follow-up forecasts 7.74 pp versus 8.64 for persistence;
- intervals inside both windows (76.3%/89.4%);
- landmark forecasts 9.92 versus 12.89 pp;
- initial state 10.96 versus 17.99 pp for the pooled training curve.

All four declared hypotheses passed, but only 21/43 group means were within 5/10 pp.
Early pressure rises after angle surgery and a waning micropulse-laser effect are not
represented ([Persian results](experiments/eye/iop/results.fa.md)).
**Model 1.1** ([experiment](experiments/eye/iop/model-1.1/README.md)) adds a fast
early transient (7-day time constant) and group-mean noise scaled by the number of
eyes. It was trained on all 72 tranche-1 records, frozen and pushed (`ac8abf83`),
then tested once on tranche 2, a set reserved before triage: 87 blind-triaged
articles, from which seven custodians built 74 records. All five declared hypotheses
passed on 113 human group-mean series:

- next-follow-up forecasts 6.69 pp versus 8.06 for persistence;
- intervals inside both windows (80.2%/89.6%);
- landmark forecasts 8.95 versus 15.10 pp;
- initial state 9.71 versus 18.06 pp for the pooled curve;
- 1.1 better than 1.0.0 trained on the same records (6.69 versus 6.92 and 8.95 versus
  9.40 pp).

The gain is confined to the first postoperative month. Early and long-horizon
intervals were too narrow, and 58/113 group means were within 5/10 pp
([Persian results](experiments/eye/iop/model-1.1/results.fa.md)).
**IOP model 1.2.0** keeps the 1.1.0 law and fit (on generic eye model version 2)
and widens only the forecast intervals of the first postoperative month. It was
trained on the 146 studies of tranches 1 and 2, frozen and pushed (`df8f389a`), then
tested once on tranche 3 (83 custodian-extracted records). All five declared
hypotheses passed on 130 human group-mean series: coverage 83.5%/93.1%, next
follow-up 5.89 pp versus 7.21 for persistence, landmark 7.76 versus 11.89, initial
state 10.17 versus 18.06, and first-month coverage 82.4%/93.9% against 67.9%/81.7%
for 1.1.0 at the same accuracy. First-month errors are still about twice the later
ones, and the month-1 pressure rise after Ahmed valves is missed
([model 1.2](experiments/eye/iop/model-1.2/README.md),
[Persian results](experiments/eye/iop/model-1.2/results.fa.md)).
The fifth eye component is
[central retinal thickness after treatment](experiments/eye/retina/README.md): the
OCT thickness of the macula after anti-VEGF injections, steroid implants or
injections, laser, photodynamic therapy or vitrectomy for macular edema, neovascular
AMD and related conditions. It is built from an
[outcome-blind dataset](experiments/data/retina-cst-2026-10-28/README.md) of 3,128
screened open-access papers (tranche 1 of 120 used, 275 papers unread). There are 46
development and 19 custodian-extracted sealed records. Its model is the generic
[eye recovery population model](packages/platform/src/peykar/model_learning/eye_recovery_population.py),
the IOP 1.1 law and hierarchy written once for every component (it reproduces IOP
1.1 to 1e-9). Development showed that a series stays above or below the smooth law
for months, so **model 1.0.0** adds persistent (Ornstein–Uhlenbeck) deviations that
forecasts carry forward. It was frozen and pushed (`d40b6ed2`) before a one-shot
sealed test. All five declared hypotheses passed on 40 group-mean series:

- next-follow-up forecasts 5.75 pp versus 6.34 for persistence and 6.14 for the same
  model without deviations;
- intervals inside both windows (81.6%/91.8%);
- landmark forecasts 7.50 versus 9.50 pp;
- initial state 11.35 versus 17.09 pp for the pooled curve.

Anti-VEGF series were forecast within 1.1–3.7 pp. The gain is in the first three
months. The rebound after a single dexamethasone implant wears off is missed, and
only 5/40 group means met every per-series criterion
([Persian results](experiments/eye/retina/model-1.0/results.fa.md)).
**Retinal-thickness model 1.1.0** was then tested once on tranche 2, which was
reserved before triage (63 custodian-extracted records). It runs on
[generic eye model version 2](packages/platform/src/peykar/model_learning/eye_recovery_population2.py)
with a declared wear-off of one dexamethasone implant and forecast intervals from the
pooled study noise. Four of its five declared hypotheses passed on 92 group-mean
series: coverage 84.4%/89.6%, next follow-up 7.91 pp versus 8.81 for persistence,
landmark 9.90 versus 11.55, initial state 13.00 versus 18.51. The fifth failed: model
1.0.0 with the same training forecast slightly better (7.79 and 9.73 pp), but its
intervals covered only 66.0%/77.2% of the fresh data. The wear-off rule cut the
initial-state error of single-implant groups from 14.61 to 5.42 pp, but one group
treated after vitrectomy did not re-thicken as predicted
([model 1.1](experiments/eye/retina/model-1.1/README.md),
[Persian results](experiments/eye/retina/model-1.1/results.fa.md)).
Retinal-thickness version 1.2 found no change worth testing: a prior-treatment level,
an OCT-device level and a regimen level each gained less than 0.10 pp over 1.1.0 in
leave-one-study-out development, so no 1.2 protocol was frozen and tranche 3 stays
sealed and unread ([development log](experiments/eye/retina/model-1.2/development-log.md)).
The sixth eye component, and the fourth corneal one, is
[refraction after corneal laser refractive surgery](experiments/eye/refraction/README.md):
the spherical equivalent (dioptres) after PRK and other surface ablation, LASIK and
SMILE. It is built from an
[outcome-blind dataset](experiments/data/cornea-refraction-2026-10-29/README.md) of
2,169 screened open-access papers (89 passed a masked-table rule, 39 admitted blind,
split 31/8 before any value was read): 24 development and 8 custodian-extracted sealed
records. Refraction changes in dioptres, not as a fraction of its baseline, so the
series give the generic eye model the change as an exact pseudo-fraction and
[an additive evaluation](packages/platform/src/peykar/validation/eye_additive_evaluation.py)
scores it in tenths of a dioptre, with full correction (the plano target) as a
comparator. **Refraction model 1.0.0** (procedure → refractive-error hierarchy) was
frozen and pushed (`a2eccf2c`) and tested once on 13 group-mean forecast series. All
five declared hypotheses passed: coverage 82.1%/85.7%, next follow-up 0.29 D versus
0.37 for persistence, landmark 0.47 versus 0.78, initial state 0.40 versus 0.50 D for
full correction, and the refractive-error level beat the procedure-only model in the
initial state. The gain over full correction is largest after month 6, where eyes
regress about 0.4 D towards myopia, but one severe-myopia LASIK group regressed to
−1.9 D, outside the 90% interval; the test is small, myopic only and weighted to LASIK
([model 1.0](experiments/eye/refraction/model-1.0/README.md),
[Persian results](experiments/eye/refraction/model-1.0/results.fa.md)).
The seventh eye component, and the fifth corneal one, is
[corrected visual acuity after keratoplasty and cross-linking](experiments/eye/acuity/README.md)
(BCVA/CDVA in logMAR after PK, DALK, DSAEK, DMEK and corneal cross-linking). Its
[outcome-blind dataset](experiments/data/cornea-acuity-2026-10-29/README.md) comes
from 1,994 screened open-access papers: 175 passed a masked-table rule after articles
kept by other components' untested tranches were dropped, the 120 of tranche 1 were
triaged blind (67 admitted, split 47/20 before any value was read) and the 55 of
tranche 2 stay unread for the next version. Complete-case and outcome-selected
analysis sets were excluded, leaving 27 development and 15 custodian-extracted sealed
records. Acuity is scored on the same additive scale, in ETDRS letters. **Acuity
model 1.0.0** (procedure contexts; an indication level, per-family hierarchies and
persistent deviations were each rejected in development) was frozen and pushed
(`baafe4cd`) although on development it lost both forecast tasks to the
pooled-curve increment. Tested once on 21 group-mean forecast series, it passed all
four declared hypotheses: coverage 82.9%/92.7%, next follow-up 5.00 letters versus
5.06 for persistence and 6.07 for the pooled-curve increment, landmark 5.24 versus
7.27, initial state 9.04 versus 9.79 for the pooled curve. The sealed set is mostly
cross-linking and DMEK and has no PK group; keratoplasty groups that recover poorly
for their preoperative acuity (one DSAEK group after glaucoma surgery in the sealed
set) are still forecast too optimistically
([model 1.0](experiments/eye/acuity/model-1.0/README.md),
[Persian results](experiments/eye/acuity/model-1.0/results.fa.md)).
The eighth eye component,
[best-corrected acuity after macular treatment](experiments/eye/retina-acuity/README.md),
failed its sealed tests in three versions (1.0.0 all four hypotheses, 1.1.0 four of five,
1.2.0 all five) and is not offered. Later versions of tested components that did not
pass all their hypotheses (retinal thickness 1.3.0 and 1.4.0, acuity 1.2.0 with forecast
stacking, IOP 1.3.0 and 1.4.0) left their predecessors in place; their results are in
[the status table](docs/status.md). IOP 1.4.0, which kept a sub-procedure level only for
the two procedures whose training errors differed by device, passed H1–H4 on the last
reserved IOP tranche but missed its comparison with 1.2.0 by 0.007 pp in the next
follow-up and 0.13 pp in the landmark task, decided by one study
([model 1.4](experiments/eye/iop/model-1.4/README.md),
[Persian results](experiments/eye/iop/model-1.4/results.fa.md)). The ninth eye component, and the sixth corneal one,
is [maximum keratometry (Kmax) after cross-linking or ring segments](experiments/eye/kmax/README.md),
built from an [outcome-blind dataset](experiments/data/cornea-kmax-2026-10-30/README.md)
of 1,526 screened open-access full texts (102 passed a masked-table rule, 62 admitted
blind, split 40/22 before any value was read; 25 development and 17 custodian-extracted
sealed records). **Kmax model 1.0.0** (no early transient, forecast-only pooled noise)
was frozen and pushed (`9fe023ae`) and tested once: coverage 74.5%/85.5% and the next
follow-up (1.02 D against 1.19 D for persistence) passed, but the landmark (1.26 D
against 1.24 D for the pooled-curve increment) and the initial state (1.46 D against
1.42 D for the pooled training curve) failed. One study of eyes treated under 18 years
kept flattening for five years while the law levelled off after the first year; it is
not offered by `peykar eye`
([model 1.0](experiments/eye/kmax/model-1.0/README.md),
[Persian results](experiments/eye/kmax/model-1.0/results.fa.md)).
The tenth eye component is
[axial length in childhood myopia](experiments/eye/myopia/README.md), treated or not
(atropine, orthokeratology, myopia-control spectacles and contact lenses, red light). Its
[outcome-blind dataset](experiments/data/myopia-axial-2026-10-30/README.md) comes from
1,004 screened open-access articles: 214 passed a masked-table rule, the 120 of tranche 1
were triaged blind (54 admitted, split 34/20 before any value was read) and the 94 of
tranche 2 were reserved for the next version before triage. With the free start of the generic model the
population read a shortening of 0.66 mm at day 1, so generic eye model version 5 adds
laws that are unchanged at the start. **Myopia model 1.0.0** (such a law with a
treatment-family hierarchy; frozen and pushed as `17ba8feb`) failed all four
hypotheses of its sealed test: coverage 63.8%/78.7%, next follow-up 0.120 mm against
0.111 mm for the pooled-curve increment and 0.102 mm for analog kNN, landmark 0.181 mm
against 0.171 mm, initial state 0.192 mm against 0.173 mm for the pooled curve. A
retrospective cohort that lost half or more of its atropine children by month 24 and a
4-year trial whose controls grew 0.42 mm a year throughout decide the verdicts; it is not
offered by `peykar eye`
([model 1.0](experiments/eye/myopia/model-1.0/README.md),
[Persian results](experiments/eye/myopia/model-1.0/results.fa.md)).
**Myopia model 1.1.0** (developed on all 33 tranche-1 records: model 1.0.0 with
persistent deviations in forecasting only; frozen and pushed as `b5195458`) was tested
once on tranche 2 (26 records, 42 group-mean forecast series). Its forecasts beat every
comparator (next follow-up 0.068 mm against 0.082 mm for the pooled-curve increment;
landmark 0.089 mm against 0.108 mm), but it failed H1 (coverage 90.7%/97.3%, intervals
slightly too wide), H4 (initial state 0.113 mm against 0.111 mm for the pooled curve)
and H5 (landmark 0.089 mm against 0.087 mm for model 1.0.0 on the same training). The
pooled forecast noise is dominated by the two studies that had failed model 1.0; axial
length stays out of `peykar eye`
([model 1.1](experiments/eye/myopia/model-1.1/README.md),
[Persian results](experiments/eye/myopia/model-1.1/results.fa.md)).
The tested eye components can be run as forecasters with `peykar eye`
([guide](docs/eye-simulation.md)): give a group's context, baseline, size and any
follow-ups seen so far, and it returns the frozen model's forecasts with 80% and 90%
intervals, its sealed-test result and its context of use. It covers retinal thickness
1.1.0, IOP 1.2.0, corneal endothelial density 1.0.0, corneal thickness 1.0.0,
refraction 1.0.0 (in dioptres) and visual acuity 1.0.0 (in logMAR); it is a research
tool, not a clinical one.

The [whole-platform roadmap](docs/roadmap.md) covers fourteen system EPICs; vCornea
is one integration within it. Shared backends now include an immutable
[source-library registry](docs/library-ecosystem.md),
[persistent biological graphs](docs/biological-graph.md), and
[durable multi-model experiment workflows](docs/experiment-workflows.md).
The complete named source trees and recursive dependencies now live as ordinary
project files in [vendor](vendor/README.md): 29 pinned distributions, 53,387 files,
37 materialized Git dependencies, and preserved original notices. [Installed
runtime profiles](runtime_profiles/README.md) execute real COPASI, NEURON,
PhysiCell/PhysiBoSS, Chaste, VCell, GEARS and PK-Sim/MoBi headless checks alongside
vCornea. Retained source/runtime evidence is discoverable through the API/CLI. The
workflow worker actually transfers terminal scalar outputs into downstream initial
parameters, with atomic dispatch, units/context checks, cancellation and provenance.
A separate [native execution agent](docs/native-agent.md) now runs admitted SBML
through COPASI and original PKML through OSPSuite from durable API jobs, with
frozen identities, cancellation and checksum-verified outputs. Other native queue
adapters and continuous feedback coupling remain explicit system tasks.
[E5 CellML](docs/epics/E5-cellml-models.md) adds original CellML 2 admission, a
reviewed isolated libCellML/SciPy ODE solver, retained operator results and browser
inspection; CellML queue submission remains a separate integration task.
[E2 job cohorts](docs/epics/E2-reproducible-cohorts.md) now add bounded parameter
sweeps, deterministic reference seed allocation, mixed native/reference batches
and lineage-checked terminal statistics with explicit excluded members.
[E3 hybrid flows](docs/epics/E3-native-scalar-flows.md) now connect actual native
and reference stages through explicit scalar ports, unit conversions and immutable
SBML initial-state derivatives; original source models and their notices are preserved.

[راهنمای فارسی](README.fa.md) · [Master blueprint](docs/master-blueprint.md) · [Architecture](docs/architecture.md) ·
[Implementation status](docs/status.md) · [Roadmap](docs/roadmap.md) ·
[Scientific contracts](docs/scientific-contracts.md) · [Blueprint coverage](docs/requirements-traceability.md) ·
[Licensing](LICENSING.md) · [Research sources](docs/sources.md)

[E6 biological vocabularies](docs/epics/E6-biological-vocabularies.md) add pinned
CL/Uberon/GO and HGNC/UniProt/Reactome identifier snapshots, contextual annotations
and preserved competing declarations in Biological Explorer. These remain
unreviewed source-linked declarations, separate from executable biological bridges.

The complete original Persian v3.0 architecture, supplied before the Peykar name
was chosen, is [archived verbatim](docs/blueprints/README.md). Its
[55-section and 24-scenario coverage map](docs/blueprint-v3-coverage.md) keeps future
requirements distinct from the implemented inventory.

## Run in two commands

Requirements: Linux for the bounded worker, Python 3.12 or 3.13 and
[uv](https://docs.astral.sh/uv/).

```bash
uv sync --frozen --all-packages --group dev
uv run peykar demo
```

The demo registers a model, stores its immutable run manifest, enqueues a job,
executes it through the worker, saves a checksum-verified artifact, and compares
the result with an independently calculated analytical reference. The verification
report must say `synthetic_verification`, never independent biological validation.
It requires no API keys, GPU, external datasets, or engine downloads.

```bash
uv run peykar init
uv run peykar submit experiments/manifests/logistic.json
uv run peykar worker --once
uv run peykar models
```

Use `PEYKAR_DATA_DIR` to change the default local `.peykar` state directory.

## Open the research workbench

The Docker build includes the browser application at **http://127.0.0.1:8000/app/**.
Connect with the existing API administrator or reader token. The client supports
model/data/evidence inspection, reference/native run submission, scalar flow design,
cohorts, persistent results and frozen evaluation. Tokens stay in page memory and
are cleared on reload/disconnect. Native jobs still require the separate host agent.

For source development, use Node 22.14+ and run `make install` (frozen Python/npm
locks) or `npm --prefix apps/dashboard ci --ignore-scripts` followed by
`make dashboard`. `make check` includes frontend checks; `make build` packages the
client into the platform wheel/sdist. See [dashboard development](apps/dashboard/README.md).

## Import data and freeze an evaluation

```bash
uv run peykar import-csv experiments/data/analytic-decay.csv --spec experiments/data/analytic-decay-import.json
uv run peykar freeze-evaluation experiments/benchmarks/decay-evaluation-set.json
uv run peykar submit experiments/manifests/decay.json
uv run peykar worker --once
uv run peykar evaluate RUN_ID decay.holdout 1.0.0
uv run peykar report REPORT_ID
```

Replace `RUN_ID` and `REPORT_ID` with the IDs printed by the preceding commands.
These fixtures are synthetic. Raw CSV bytes and normalized observations are stored
separately with checksums. The frozen set pins dataset versions, declared split
groups, endpoint and tolerance. See [data and evaluation](docs/data-evaluation.md).

## Start the API and worker

Set a private administrator token first:

```bash
export PEYKAR_API_TOKEN="$(python -c 'import secrets; print(secrets.token_urlsafe(32))')"
uv run peykar init
uv run peykar serve
```

In a second terminal, in the same checkout and with the same `PEYKAR_DATA_DIR`:

```bash
uv run peykar worker
```

Swagger UI: http://127.0.0.1:8000/docs. Authorize with the token before using `/v1`.
`/healthz`, documentation and the OpenAPI schema are public; all data routes require
authentication. Optional `PEYKAR_READ_TOKEN` grants read access only.

```bash
curl -X POST http://127.0.0.1:8000/v1/simulations \
  -H "Authorization: Bearer $PEYKAR_API_TOKEN" \
  -H 'Content-Type: application/json' \
  -H 'Idempotency-Key: example-logistic-1' \
  --data-binary @experiments/manifests/logistic.json
```

The response contains a run ID and `Location` for polling. GET that location until
the status is terminal, then GET `/v1/simulations/RUN_ID/results`. The API and
worker must share the same persistent directory. With the existing administrator
token exported, verify this complete path against running services:

```bash
uv run python scripts/verify_api.py experiments/manifests/decay.json
uv run python scripts/verify_api.py experiments/manifests/logistic.json
```

Each verification creates one retained synthetic run and checks idempotency,
manifest/result hashes and lifecycle events. It never resets existing state.
See [external artifact admission](docs/external-artifacts.md) and
[mapped scalar imports](docs/compucell-output.md) for the new offline commands.
The [knowledge-gap API and CLI](docs/knowledge-gaps.md) record missing evidence or
mechanisms without pretending they are implemented or scientifically resolved.

## Run the admitted vCornea source

This optional runtime is separate from the default API/worker image. Execution is
an explicit local operator action; the API does not fetch or execute model code.

```bash
uv run python scripts/acquire_vcornea.py code --destination .peykar/external/archives
uv run python scripts/prepare_vcornea.py \
  --archive .peykar/external/archives/vCornea-16884414.zip \
  --pins engine_adapters/compucell3d/vcornea-source-16884414.json \
  --destination .peykar/external/vcornea-archive-16884414
docker build --platform linux/amd64 \
  -f engine_adapters/compucell3d/runtime/Dockerfile \
  -t peykar-cc3d:4.7.0-linux64 engine_adapters/compucell3d/runtime
uv run python scripts/run_vcornea.py \
  --source .peykar/external/vcornea-archive-16884414 \
  --pins engine_adapters/compucell3d/vcornea-source-16884414.json \
  --output .peykar/external/my-native-run --scenario injury \
  --steps 600 --seed 7 --image peykar-cc3d:4.7.0-linux64 --wall-seconds 900
```

Use a new output directory for each run. Original source is checksum-verified and
preserved; only a private copy receives the recorded horizon and seed controls.
Native counts retain four cell types and hours; Parquet heights retain lattice
coordinates. See [integration](docs/vcornea-integration.md),
[native import/API](docs/vcornea-native-output.md), and
[dataset/tool assessment](docs/integration-candidates.md).

## Work across reusable models and biological evidence

```bash
uv run peykar register-model experiments/workflows/decay-transfer-model.json
uv run peykar experiments register experiments/workflows/decay-transfer.json
uv run peykar experiments submit workflow.decay-transfer 1.0.0 --idempotency-key example-chain
uv run peykar worker
```

Use `peykar experiments status EXPERIMENT_RUN_ID` and `results EXPERIMENT_RUN_ID`
from another terminal. This fixture verifies two serial decay intervals, including
the actual transferred concentration; it makes no biological coupling claim.
The same lifecycle is available through `/v1/experiments` and `/v1/experiment-runs`.
Use `/v1/libraries` for source readiness and `/v1/biology/graphs` for evidence-linked
contextual relations. `/v1/runtimes` and `peykar runtimes list` expose actual native
execution evidence. Source acquisition/inspection commands and original license
scope are in [the library guide](docs/library-ecosystem.md).

## Repository boundaries

| Path | Responsibility |
|---|---|
| `packages/core` | Independent contracts, units, hashing and engine protocol |
| `packages/engines` | Trusted reference engines behind the shared protocol |
| `packages/platform` | Domain modules, registries, local infrastructure, API, CLI |
| `apps` | Entrypoints and the implemented six-workspace research workbench |
| `biological_libraries` | Reusable mechanisms and promotion criteria |
| `model_packages` | Immutable descriptors, separate from executable engines |
| `engine_adapters` | Integration plans and evidence for specialist engines |
| `dataset_connectors` | Data connector boundaries and external-source policies |
| `experiments` | Portable plans, baselines and benchmarks |
| `schemas` | Generated JSON Schema and OpenAPI contracts |
| `tests` | Unit, scientific verification and integration tests |
| `docs` | Architecture, ADRs, scientific limits and implementation roadmap |
| `infrastructure`, `deployment` | Local persistence and container operations |

The three Python distributions share one uv lockfile and have explicit one-way
dependencies. They can be extracted later without creating a microservice fleet.

## Quality checks

```bash
uv run ruff check .
uv run ruff format --check .
uv run mypy
uv run pytest --cov --cov-report=term-missing
uv run python scripts/export_contracts.py --check
uv build --all-packages
```

On macOS or Windows, run `make check-docker` for the Linux worker regression and
package build in an isolated verification container. It uses the frozen lockfile,
has no network during the tests and does not mount the live data volume.

See [CONTRIBUTING.md](CONTRIBUTING.md) for scientific and software change rules.
The GitHub workflow runs these checks for Python 3.12 and 3.13. A configured CI
workflow is not evidence that those remote jobs have already run.

## Scientific boundary

An executable model is not automatically a scientifically useful model. A unit
conversion is not a mechanistic mapping. Two individually validated models do not
automatically form a validated composite. A simulation result is not an experimental
observation. Model improvement needs held-out evidence for a stated context of use.

Peykar preserves these distinctions in contracts, outputs, tests and promotion gates.

## License

Original material in this distribution is offered under the **Peykar Noncommercial
Source License 1.1.0**, a project-specific draft requiring qualified legal review.
Its public grant permits defined noncommercial work and reserves commercial/paid
uses. It is **not an OSI-approved open-source license**. See [LICENSE](LICENSE) and
[LICENSING.md](LICENSING.md) for the actual scope, source conditions and exceptions.

Free humanitarian services are expressly permitted under the public conditions.
Section 7A preserves the owner's ability to use and separately license rights he
actually owns, including commercially, without buying a license from himself.

An earlier private 0.1.0 snapshot contained Apache-2.0 terms. Any valid rights actually
obtained under that grant remain effective; this change cannot withdraw them.
External packages, engines, models and datasets retain their own licenses. Pristine
specialist sources retain their original terms; optional native runtimes are
built separately from the API/Worker image. A licensing label is not a guarantee of ownership
or enforceability. [Licensing rationale](docs/licensing-rationale.md).

E7 adds [multiparty biological processes](docs/epics/E7-multiparty-processes.md):
exact rational stoichiometry, source-defined molecular states/complexes/alternative
sets, unreviewed process ownership/exclusions and original-unit sample frames.
Biological Explorer exposes the real APIs; no kinetic/biological approval is inferred.
