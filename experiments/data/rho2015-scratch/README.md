# Rho et al. 2015 scratch-wound aggregate

Source: Rho CR, Park M-y, Kang S (2015), *Effects of Granulocyte-Macrophage
Colony-Stimulating (GM-CSF) Factor on Corneal Epithelial Cells in Corneal Wound Healing Model*,
[PLOS ONE 10:e0138020](https://doi.org/10.1371/journal.pone.0138020).
Source license: [CC-BY-4.0](https://creativecommons.org/licenses/by/4.0/).
This attributed experimental transcription retains that license; Peykar's license
does not replace it. No endorsement by the authors is implied.

`aggregate.csv` transcribes eight reported means/SDs at 24 and 48 hours for four
GM-CSF doses from Results, “Accelerated migration of HCECs by rhGM-CSF”.
`control.csv` keeps the untreated control and divides means/SD percentages by 100.
`provenance.json` records extraction, source XML hash, units and limitations.
No zero-hour value, interpolation, donor identity or individual replicate was invented.
The caption reports three independent experiments; the available values are aggregates.

```bash
PEYKAR_DATA_DIR=.peykar/vcornea-epic \
  uv run peykar import-csv experiments/data/rho2015-scratch/control.csv \
  --spec experiments/data/rho2015-scratch/import.json
```

The observed import retains exact raw bytes and two normalized control means.
The extra SD column remains in raw CSV; the scalar evaluator does not use it as
an uncertainty model. This SV40-immortalized HCEC monolayer/GM-CSF experiment is
not an independent validation set for stratified vCornea/EGF. A justified wound-area
observation operator, matched conditions and an independent evaluation protocol
are absent; see [candidate assessment](../../../docs/integration-candidates.md).
