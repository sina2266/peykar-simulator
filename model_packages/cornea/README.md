# Corneal projected repair components

These immutable1.0.0 model packages use the optional `cornea.projected_front`1.0.0
engine. `projected-repair` represents the nominal7mm2017 PBS assay; the distinct
`mechanical-challenge` package declares the approximately6mm2021 removal-method
context. Neither is registered in the default API/Worker catalog.

Kinetic parameters come from Crosson/Klyce/Beuerman1986; the implementation is
Peykar code under the existing project license, not copied upstream source.
Original experimental authors/terms, independently selected parameters, frozen
criteria, complete actual outputs and both positive/negative results are in
[the experiment record](../../experiments/vcornea/repair-front/README.md).

A model's original unvalidated metadata is retained as admitted. Later evidence
is append-only in experiment records: the2017 mean-area check passes, the2021
transfer challenge fails. No cell-layer, chemical, drug-response or clinical
validity is implied by deterministic projected-area kinematics.

The separately versioned `learned-sdp-front`, `learned-brush-challenge` and
`learned-fischak-challenge`1.0.0 descriptors pin `cornea.learned_front`1.0.0 and
the exact four-parameter artifact from six actual calibration animals. Original
unvalidated model bytes are retained; later failed heldout/independent evidence
is appended in [the learning experiment](../../experiments/vcornea/repair-front/learned-front/README.md).
They are optional offline candidates, absent from the default catalog, and do
not replace the earlier independently parameterized models or cellular vCornea.
