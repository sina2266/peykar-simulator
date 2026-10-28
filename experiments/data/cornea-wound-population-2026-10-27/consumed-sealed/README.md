# Consumed sealed-test records (CC BY sources)

These records were sealed-test data and have now each been used by exactly one
one-shot test, so they are **consumed**: any later model version may use them for
development, never as an independent test again.

| Record | Used by | Source and licence |
|---|---|---|
| `zeb1-commun-biol-2023.json` | protocol 1.0.0 sealed test (2026-10-27) | Zhang Y. et al., *Communications Biology* 2023, DOI 10.1038/s42003-023-04831-0, Supplementary Data workbook; CC BY 4.0 |
| `il10-c-mast-cell-rep-2026.json` | protocol 2.0.0 sealed test (2026-10-28) | *Cell Reports* 2026, DOI 10.1016/j.celrep.2026.117064; raw data Mendeley Data 10.17632/kwhys2zfwd.1; both CC BY 4.0 |
| `iprf-cxl-human-arq-bras-oftalmol-2025.json` | protocol 2.0.0 sealed test (2026-10-28) | *Arquivos Brasileiros de Oftalmologia* 2025, DOI 10.5935/0004-2749.2024-0326, Table 2; CC BY 4.0 |

The records and the files under `raw/` are byte-identical copies of the custody
files whose SHA-256 values are pinned in the protocols; the frozen operators keep
reading the private canonical copies. The records were produced by a custodian
agent; values were not changed. The LSC 2026 record (CC BY-NC-ND 4.0) is consumed
too but stays private, as do derived values of the FL2 rat development study.

Notes kept from custody: the IL-10 authors analysed only wounds that healed at
least 15% by 19 h (the deposited rows were not filtered further), and the IL-10
hour 0 is definitional (values are per-mouse percentages of each mouse's own T0
area); the i-PRF study reports group means only and its per-patient deposit was
not publicly accessible.
