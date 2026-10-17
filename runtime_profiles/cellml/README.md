# Reviewed CellML 2 ODE runtime

Read [E5 scope, operation and remaining adapters](../../docs/epics/E5-cellml-models.md).
The complete upstream source and original Apache-2.0 LICENSE are preserved in
`vendor/dependencies/libcellml`; this directory contains Peykar's separate driver.

The independent `uv.lock` pins libCellML 0.7.1 / NumPy 2.4.4 / SciPy 1.17.1.
`Dockerfile` adds explicitly pinned libxml2, required by the official wheel.
No dependency is added to the API/Worker lock or image. Build explicitly:

```sh
docker build --platform linux/arm64 -f runtime_profiles/cellml/Dockerfile -t peykar-cellml:0.7.1 .
uv run python scripts/verify_cellml.py --output .peykar/my-cellml-verification
```

The local catalog pins the **observed installed image and driver**, independently
checked against retained runtime evidence before execution. A different rebuild
requires new evidence/version and a separately reviewed catalog; never silently
replace an admitted runtime's identity. No automatic pull or source execution.

Only self-contained ODEs without imports/resets/external variables/DAE/NLA are
supported. Time controls must match the original integration-variable unit name.
RK45, Radau and BDF are available with explicit bounded tolerances and maximum step.
The operator container is offline/non-root/resource-bounded, with read-only input
files and root. Admission itself performs no numerical execution. See the E5 guide
for retained result import, failure handling, numerical receipts and queue limits.
