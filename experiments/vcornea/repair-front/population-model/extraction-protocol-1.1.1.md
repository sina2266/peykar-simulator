# Extraction protocol 1.1.1 — amendment to 1.1.0

Written on 2026-10-28 **after** development values of two 1.1.0 development
studies (the Peucedanum japonicum rat studies, PMC10295055 and PMC8614870) had
been read, because their records failed validation; no sealed-test value had been
read. Everything in [1.1.0](extraction-protocol-1.1.0.md) and
[1.0.0](extraction-protocol-1.0.0.md) still applies, plus one rule:

## Percent healed with a measured, non-zero hour 0

Some tables report "healed area (%)" relative to a fixed reference area (for
example the nominal wound) and give a **measured** hour-0 value that is not 0 %.
Such a series cannot use `percent_healed` (its hour 0 must normalize to one).
It is recorded as `value_kind: area_absolute` of the remaining percentage,
`100 − healed`, each value cited as `transform:100-minus:<file::cell>`. The
series is then normalized by its own measured hour-0 remaining value as usual.
For a fixed reference area this equals the open area relative to the hour-0
area exactly; the arithmetic is linear, so it also holds for group means.

The hour-0 entry is a measurement, so the definitional baseline of 1.1.0 §3 is
not used for these series.
