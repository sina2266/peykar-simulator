# E5 — CellML admission and reviewed ODE execution

Delivered bounded S3.4b: exact CellML 2 files, original unit definitions, a pinned
libCellML/SciPy operator solver, retained numerical artifacts and browser inspection.
The older SBML/PKML contracts and queue protocol remain immutable. This delivery
adds separate `CellML*` contracts and `/v1/cellml-*` endpoints; it does not expand
the two existing native queue kinds or claim biological validation.

## Source and executable scope

The complete libCellML **v0.7.1**, commit
`bc71bfa5816cd3e1e1a28430366eb5688fe9e501`, is permanently materialized under
`vendor/dependencies/libcellml`: **1,094 tracked files**, including the original
Apache-2.0 LICENSE, documentation and tests, with no unresolved Git dependencies.
Its upstream source is pristine. Peykar's driver is a separate overlay in
`runtime_profiles/cellml/probe.py`.

Admission accepts bounded UTF-8 CellML 2 XML, including its BOM/line endings. It
rejects DTD/entities, XInclude, model imports, unsupported namespaces, duplicate
declarations and missing unit references. It retains component-qualified variables,
initial-value declarations, original units and exact raw bytes. This structural
inspection is deliberately distinct from full semantic validation.

The explicitly invoked offline solver uses libCellML Parser, Validator, Analyser
and Python Generator, followed by SciPy `solve_ivp`. It supports fully determined,
self-contained ODEs with RK45, Radau or BDF; **imports, resets, external variables,
DAE/NLA and checkpoint restoration are unsupported**. The original variable of
integration determines the required time-unit identifier; controls never silently
convert it. The scalar absolute tolerance applies to each state in its original
numeric units; it is not a common physical tolerance across different quantities.
Outputs identify every state/constant/algebraic variable by component,
name, kind and original units. Maximum 256 states, 4,096 total variables and
400,000 output cells bound this reviewed profile.

Generated mathematical Python executes only inside the operator container, after
generator/import/call checks. This is not a general uploaded-code sandbox.
The image/driver must match an admitted runtime and a local pinned catalog. No
automatic image pulls, API Docker socket, remote model imports or package installs
occur during admission or execution. The non-root container has no network,
read-only root and inputs, dropped capabilities, CPU/memory/PID/tmpfs bounds and an
explicit wall deadline. Cleanup follows acknowledged creation even on timeout.

## Run and retain evidence

The independent runtime lock pins libCellML 0.7.1, NumPy 2.4.4 and SciPy 1.17.1.
The Dockerfile pins Python's base digest and the libxml2 package required by the
official wheel. This dependency is essential: the first actual probe exposed a
missing `libxml2.so.2`, fixed in the dedicated image rather than the API image.

```sh
docker build --platform linux/arm64 -f runtime_profiles/cellml/Dockerfile -t peykar-cellml:0.7.1 .
uv run python scripts/verify_cellml.py --output .peykar/my-cellml-verification
```

Use a **new** output directory. The replay requires the explicit source checkout
at `.peykar/external/extra-sources/cellml--libcellml` (or `--source`) and the already
built image. It records an immutable source receipt, actual runtime evidence,
image/driver pins and four execution receipts in its own local data directory.
It does not access the production volume. No network or solver downloads occur.

For an already admitted operator workspace/runtime:

```sh
uv run peykar cellml register-model model_packages/cellml/decay/1.0.0/spec.json model_packages/cellml/decay/1.0.0/model.cellml
uv run peykar cellml run cellml.decay 1.0.0 model_packages/cellml/decay/1.0.0/controls.json runtime_profiles/cellml/local-catalog.json .peykar/my-cellml-run cellml.my-run
uv run peykar cellml executions
```

The runtime/source must first be admitted through the existing trusted offline
source/runtime workflow; see `runtime_profiles/cellml/verified-2026-10-17.json`.
`PEYKAR_DATA_DIR` selects that workspace. Runtime evidence is an operator assertion,
not independent scientific approval. Each successful execution retains generated
code, original CSV, result metadata, frozen controls and SHA-256 lineage. Failure
retains its isolated output/log, publishes no execution receipt and does not retry.
`peykar cellml import-execution RECEIPT BUNDLE` checks exact model/runtime/driver,
artifact hashes, finite sample grid, original variable identities and unit metadata.

## API and workbench

Authenticated administrator `POST /v1/cellml-models` admits `{spec, xml_text}`.
Readers can discover exact model versions, download original bytes and discover
retained execution receipts/artifacts. Discovery is paginated (20 default, 50 max).
There is no HTTP execution or execution-import endpoint. The general 2 MiB HTTP
limit remains; offline model admission supports up to 10 MiB.

Model Studio's CellML tab provides admission, original-unit inspection, original
binary download, retained execution selection, computed plots/tables and artifact
downloads. It explicitly describes operator-only execution. Existing responsive,
keyboard, reader-role, error/retry, conflict and Persian/RTL patterns apply. Binary
downloads preserve UTF-8 BOM and line endings; text decoding is used only for plots.

## Numerical acceptance and remaining work

The observed receipt is [cellml-verification-2026-10-17.json](../cellml-verification-2026-10-17.json).
Actual checks include analytic decay, original upstream Robertson ODE with two
stiff solvers and mass conservation, original upstream time-unit scaling, rejection
of incorrect units/undefined equations, and real deadline/container cleanup.
These are numerical engineering checks; the upstream Robertson fixture uses
dimensionless variables/time and is not promoted to a physiological model.

Next native integration work requires new queue/agent contracts and capability
negotiation rather than mutating historical native schemas. Multi-file import
flattening, CellML 1.x conversion with transformation receipts, resets/DAE/NLA,
biological benchmarks and native flow port mappings remain open. No new biological
bridge or independent validation is claimed.

Primary references: [CellML 2 normative specification](https://www.cellml.org/cellml/2.0),
[official libCellML source](https://github.com/cellml/libcellml/tree/bc71bfa5816cd3e1e1a28430366eb5688fe9e501),
[official solver tutorial](https://libcellml-tutorials.readthedocs.io/en/latest/common_users/simulation_tool_dev.html).
