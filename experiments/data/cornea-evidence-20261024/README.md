# Acquired corneal experimental evidence and Hugging Face samples

Acquired2026-10-24 through explicit operator research. Original bytes and licenses
are retained; no downloaded script or model was executed. [Acquisition hashes](acquisition.json)
pin every admitted raw/normalized file and the exploratory plan.
[Catalog](catalog.json) records sources, exact Hugging Face commits, actual samples,
rejected applications and download failures. The search is not an exhaustive
statement about everything available on Hugging Face or elsewhere.

## Actual data, rather than dataset names

| Source | Actually retained/inspected | Applicability |
|---|---|---|
| Hugging Face longevity-db/human-cornea-snRNAseq, commitf7cd9aac | Complete303826byte cell metadata Parquet:22972rows,12sample prefixes,3columns; original card/file inventory | Only sample prefix, number of detected genes and cell barcode. No cell-type labels, repair times, wound areas or epithelial thickness. Not a repair-validation dataset |
| Hugging Face usama10/corneal-biomechanical-prediction-model, commitba3fb05e | Complete44575byte development CSV and original card/inventory |202eyes for cataract-incision astigmatism/whole-cornea biomechanics. CCT is NOT epithelial thickness; do not compare it with this model |
| Hugging Face davidm770/corneal, commitd4495614 |97byte card and exact file inventory |37.7GB H5 has no scientific endpoint mapping in the card. Not downloaded or treated as applicable |
| Buffault et al.2020, PLOS ONE e0239124 | Original publisher JATS XML;7normal-group aggregates from22subjects | Healthy human epithelial thickness; available for exploratory architecture comparison. No fabricated subject-level data or injury trajectory |
| Abdel-Naby et al.2017, PLOS ONE e0188154 | Full97548byte S1 ZIP; exact Figure2/3 XLSX members;90measured wound rows | Rabbit mechanical epithelial debridement/PBS or silk treatment,0–54h. Different from the human chemical model and2D basal-exposure proxy |
| Dryad10.5061/dryad.9cnp5hqw8, version467979 | All47file entries/3metadata pages and original dataset metadata | Ex vivo porcine mechlorethamine/histology; metadata only. Six small-data downloads failed anonymous API401/public UI403. No measurements or1.27GB image archive acquired |

The HF human card states approximately19472cells and the viewer displays141k;
the actual pinned **cell metadata** file has22972rows. Its contents, rather than
those displayed estimates, determine this inspection. The297MB expression matrix
was not acquired and no transcriptomic validation/annotation is claimed.

## Actual rabbit observations

`rabbit-wounds.csv` comes from Figure2's **measured** cells B7:K9, B14:K16 and
B21:K23. It retains treatment, actual within-group rabbit ID, raw area, time and
the source cell. Each of the3rabbits/group has10times0,6,…,54h. The original
caption's n4 is inconsistent with the three measured rows in this workbook; no
fourth rabbit is invented. The original workbook/archive remain unchanged.

The workbook does not declare a raw area unit. We preserve
`unreported_source_image_area`, without assuming pixels or square millimetres.
`wound_area_fraction = raw_area(t)/raw_area(0)` within the SAME rabbit;
`closure_fraction = 1 - wound_area_fraction`. Neither repeated timepoints nor
technical thickness measurements become extra independent animals. Original
computed averages/formulas are not substituted for the measured cells.

The Fig3 workbook is retained for later reviewed native/control/treated thickness
and ZO-1 work. Untreated native tissue and72h post-injury PBS tissue are distinct.
No Fig3 observations or treatment-effect validation have yet been admitted.

## Human experimental summary

`human-healthy-oct.json` is extracted programmatically from the original
Buffault2020 XML tables1/2, selecting **Normal**, not EBMD/dry-eye columns. There
are22subjects; central epithelial thickness is52.5±3.6µm (mean/sampleSD).
The7reported endpoints stay aggregates. No individual human measurements, dates,
seeds or uncertainty intervals are reconstructed from them.

The [declared exploratory plan](diagnostic-plan-1.0.0.json) precedes extraction of
the new physical-geometry endpoint, but existing simulations had already finished.
It is expressly NOT preregistered independent validation. It fixes both completed
formation series, the nominal2µm/site scale, contexts and limitations. There is
**no acceptance threshold** or tuned parameter. See the
[actual comparison and operator](../../vcornea/experimental/README.md).

## Attribution and original terms

- Juliette Buffault, Pierre Zéboulon, Hong Liang, Anthony Chiche, Jade Luzu,
  Mathieu Robin, Ghislaine Rabut, Marc Labetoulle, Antoine Labbé and Christophe
  Baudouin. [PLOS ONE15:e0239124](https://doi.org/10.1371/journal.pone.0239124).
  CC-BY-4.0; original permission statement is in the retained publisher XML.
- Waleed Abdel-Naby, Brigette Cole, Aihong Liu, Jingbo Liu, Pengxia Wan, Ryan
  Schreiner, David W. Infanger, Nicholas B. Paulson, Brian D. Lawrence and Mark I.
  Rosenblatt. [PLOS ONE12:e0188154](https://doi.org/10.1371/journal.pone.0188154),
  [original S1 data](https://doi.org/10.1371/journal.pone.0188154.s001). CC-BY-4.0.
- The HF human mirror declares CC-BY-4.0 and points to GEO GSE186433. Its original
  caveat about original-source conditions is retained verbatim in its card.
  The biomechanical dataset declares CC-BY-4.0; the H5 card declares MIT. These
  mirror declarations do not establish undocumented original scientific provenance.
- The Dryad metadata declares CC0-1.0; its original authors and terms remain in
  raw metadata. No author endorsement or model validity is implied by attribution.

Peykar's own license is unchanged and does not replace these original terms.
