# Experimental context, separate from simulation reproduction

Update2026-10-24: a separate [experimental-data path](../experimental/README.md)
now retains original rabbit debridement observations and healthy human OCT
aggregates, with inspected pinned HF samples. Actual lattice-thickness comparisons
are exploratory: anatomical region/measurement mappings remain unresolved, and
rabbit mechanical injury does not validate the human chemical model. The earlier
source-specific findings below remain preserved; no new biological pass is claimed.

No new acceptance threshold or candidate parameter is introduced by this audit.

The [original model paper, sections2.4–2.5 and4.3](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1013410)
describes generic chemical transport/death rules and a lumped growth-factor field.
Its chemical variable named SLS does not establish chemical-specific exposure
kinetics. Its EGF decay also abstracts uptake and receptor-mediated processes;
an additional explicit pathway would need separately partitioned process ownership.
These statements describe the published scope, not a new biological calibration.

The paper's section3.4 describes emergent endpoints as independent evaluations.
The retained final S12 supplement also describes using injury-healing ranges for
parameter screening. Peykar retains that calibration-use conflict: none of those
same ranges is automatically treated as an untouched independent holdout.

[Maurer et al.,1997, DOI10.1006/taap.1996.8097](https://pubmed.ncbi.nlm.nih.gov/9144446/)
reports rabbit surfactant experiments, six animals per group and10µL exposure.
Its abstract describes confocal observations through35days, including epithelial
and whole-cornea thickness and stromal injury. Slight effects normalized by day3;
mild/moderate groups were essentially normal by day14. Individual raw trajectories
and chemical-to-model dose conversion were **not acquired** from that abstract.
Species, exposure, endpoint and stromal differences prevent treating its severity
names as automatic matches to the model's750/1500/2500 pulses.

The currently retained original scalar/VTK cohorts are simulation observations.
The previously acquired Rho2015 scratch assay has a different monolayer/GM-CSF
context. Neither closes an independent corneal-tissue validation gate. Any future
experimental comparison needs an applicable observation/dose mapping and a separately
frozen dataset whose use in calibration is explicitly accounted for. No matching
raw independent tissue dataset or biological pass is claimed here.

Checked against the primary model article and the original study's PubMed record
on2026-10-23. The1997 publisher full text was not acquired; inaccessible publisher
links were not represented as inspected full text or downloaded raw data.
