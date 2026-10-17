# Tests

`ctest --test-dir build` runs four tests per solver.

| test | what it proves |
| --- | --- |
| `VCellChombo<N>D_x64_usage` | the binary starts and reports its usage |
| `VCellChombo<N>D_x64_smoke` | a simulation runs end to end and writes the `.log`, `.mesh.hdf5` and `.hdf5.zip` VCell expects |
| `VCellChombo<N>D_x64_regression` | the solver still computes what it used to |
| `VCellChombo<N>D_x64_analytic` | what it computes agrees with the closed-form solution |

All the inputs are hand-written — VCell normally generates `.fvinput` files. The
smoke and regression cases share a geometry: a disc (2D) or sphere (3D) of `cyt`
inside a box of `ec`, one volume PDE, zero flux across the membrane, no reaction.
The analytic cases use a single subdomain covering the whole domain, so that the
closed form below holds without a membrane condition perturbing it.

The **smoke** cases start uniform at 1.0 and stay there. That exercises the whole
pipeline cheaply, so it is the first thing to fail when something is broken, but
it is not a numerical check: a uniform field satisfies almost any assertion.

The **regression** cases start as a Gaussian bump, so the field actually
diffuses, and the final timepoint is compared value by value against a stored
baseline. The final timepoint is the sensitive one — error accumulates into it.

## Regenerating a baseline

Only when the numbers are *supposed* to move. A failing regression test is a
question, not a chore.

```bash
ctest --test-dir build -R regression        # produces the run, then fails
./build/bin/compare_solution --write \
    build/tests/regress2d/extracted/SimID_regress2d_0_0004.sim.hdf5 \
    tests/resources/regress2d.baseline.txt
```

The failure message prints this command with the right paths filled in. Say in
the commit message *why* the numbers moved — a baseline updated without an
explanation is indistinguishable from a regression that was papered over.

## Tolerances

`compare_solution` uses a mixed test, `|got - want| <= atol + rtol * |want|`. The
absolute floor matters because these fields contain values at and near zero,
where a pure relative comparison is meaningless. Cells outside the solved region
carry VCell's `1.23456789e+300` sentinel and must match exactly — a change in
which cells are covered is a real regression, not rounding.

Defaults are `rtol=1e-9`, `atol=1e-12`, overridable at configure time:

```bash
cmake -B build -DVCELL_CHOMBO_REGRESSION_RTOL=1e-8 ...
```

What has actually been measured, rather than assumed:

- **Sensitivity.** Perturbing the diffusion coefficient by a relative `1e-6` is
  caught; `1e-10` is not. So these tests detect algorithmic changes without being
  hostage to last-bit noise.
- **Portability.** The baselines are generated on Linux with GCC 13. The same
  source compiled for a different architecture will not reproduce these fields
  bit for bit, and the tolerance that survives that is an empirical question. CI
  runs the same baselines on macOS arm64 and x86_64, so if `rtol` ever needs
  loosening, that is the evidence for it — record the observed worst-case
  relative difference here when it changes.

## Validation against the analytic solution

The regression baselines pin the solver against its own past output. The
`analytic` cases check it against mathematics instead.

On the unit square (or cube) with `u = 0` held on the boundary, pure diffusion
with `D = 1` has the separable solution

```
2D:  u = sin(pi x) sin(pi y)              exp(-2 pi^2 t)
3D:  u = sin(pi x) sin(pi y) sin(pi z)    exp(-3 pi^2 t)
```

It is an eigenfunction of the operator, so the profile decays in place without
changing shape. A single subdomain covers the whole domain, so no embedded
boundary or membrane condition perturbs it.

The solver does the work itself: given an `EXACT` expression it evaluates the
closed form at every cell and records `max error` and `relative L2 error` as
HDF5 attributes. `compare_solution --analytic` reads those and compares against
what the discrete scheme is predicted to produce.

### Where the expected number comes from

Two refinement studies, not a previous run.

**Refining the mesh at a fixed time step** showed the error flattening rather
than converging — 5.33e-3, 4.88e-3, 4.77e-3, 4.74e-3 at 16², 32², 64², 128².
It converges to 4.73e-3, which is exactly the error backward Euler produces at
that time step: `|(1 + lambda dt)^-n - exp(-lambda T)| / exp(-lambda T)` with
`lambda = 2 pi^2` gives 4.727e-3. So the time discretisation, not the mesh, was
the limit, and the integrator is first-order backward Euler.

**Refining the time step at a fixed mesh** confirmed it from the other side:

| dt | measured | backward-Euler term | remainder |
| --- | --- | --- | --- |
| 0.005 | 9.331e-3 | 9.186e-3 | 1.46e-4 |
| 0.0025 | 4.879e-3 | 4.727e-3 | 1.52e-4 |
| 0.00125 | 2.554e-3 | 2.399e-3 | 1.55e-4 |
| 0.000625 | 1.365e-3 | 1.208e-3 | 1.57e-4 |

The remainder is constant across an eightfold change in `dt` — that is the
spatial error, which does not depend on the time step. The solver is therefore
demonstrably solving the diffusion equation, first-order in time and
second-order in space.

### Why the time step is small

The tests run at `dt = 1e-4` rather than the coarser step those studies used,
and that is not incidental. At a coarse `dt` the backward-Euler error dominates,
and it has the **opposite sign** to the error from an overstated diffusion
coefficient. The two cancel: at `dt = 2.5e-3`, a 5% error in `D` *lowered* the
measured deviation, from 4.88e-3 to 4.51e-3, and sailed through the check.

Driving the time error down to the same order as the spatial one removes that
blind spot. Measured sensitivity at `dt = 1e-4`, where the correct answer gives
3.53e-4:

| mutation | relative L2 | caught |
| --- | --- | --- |
| `D` 2% high | 3.58e-3 | yes, 10x |
| `D` 5% high | 9.44e-3 | yes, 27x |
| `D` 2% low | — | yes |
| spurious reaction term | — | yes |
| wrong boundary value | — | yes |

The expected figures are `3.528e-04` in 2D and `1.386e-03` in 3D, accepted
within a 15% band (`VCELL_CHOMBO_ANALYTIC_BAND`). Both decompose into a derived
backward-Euler term — 1.95e-4 and 4.38e-4 at this step — plus the spatial error
of the respective mesh. A failure here means the solver is computing something
different, not that the number needs updating.

## What is still missing

The analytic cases cover pure diffusion on a domain with no embedded boundary.
The embedded-boundary machinery, which is the reason this solver exists, is
still only covered by regression against its own output. A closed form on a disc
with a zero-flux boundary exists — Bessel eigenfunctions — but the expression
parser has no Bessel functions, so it would need a different approach.

## Release checks

`tests/release/check_release.py` runs the same inputs, plus
`reference{2,3}d.fvinput`, against what actually ships: an unpacked release
archive, the image, or the SIF. It needs only numpy and h5py on the host, and
it runs every case with `-tid 0` against a fake broker. The reference models
exchange mass between two species across the embedded-boundary membrane. They
check that the summed total is conserved while a substantial fraction of the
mass crosses. [SOLVER-RELEASE.md](../SOLVER-RELEASE.md) records the numbers.
