# Installed scientific runtime profiles

The complete named source projects are in [vendor](../vendor/README.md), including
recursive Git dependencies and separate VCell native solvers. Runtime builds use
private copies. Heavy scientific packages stay outside the API/core environment.
Original notices and attribution are in [THIRD_PARTY_NOTICES](../THIRD_PARTY_NOTICES.md).

The local installation has actual native execution evidence for the following
profiles. These are operator executions; the general worker queue still supports
the reference engines. The [registry evidence](../engine_adapters/runtime-evidence-2026-10-17.json)
records exact image/artifact/source identities separately from biological validity.

| Profile | Installed environment / actual executed check |
|---|---|
| `vcornea` | CC3D 4.7.0, 424-package conda lock; original admitted archive, 600 MCS across injury; E1 also has repeated seed-7/11 checks |
| `copasi` | python-copasi 4.47.309 + BasiCO 0.88, own uv lock; real SBML reaction integration, 11 samples, analytical error 6.09e-8 and conservation error 2.22e-16 |
| `neuron` | NEURON 9.0.2 native wheel, own uv lock; actual HH compartment/current clamp, 401 samples, peak 39.1181 mV |
| `physicell` | C++/OpenMP build from vendored 1.14.2; original template, bounded one-minute run, native MultiCellDS/MAT/SVG outputs |
| `physiboss` | PhysiCell 1.14.2 + source-built MaBoSS 2.5.6; original six-cell-line Boolean fixture, 486 cells over ten minutes, native intracellular states |
| `chaste` | Official 2026.1 ARM64 image at pinned digest and matching source commit; all three original ODE tutorial tests pass |
| `chaste-delta-notch` | Both original vertex- and node-based Delta/Notch monolayer tests pass, with retained per-cell/mechanical output |
| `vcell` | Official 8.2.1.01 Linux/AMD64 image; original BioModel1 OMEX runs with all tasks/reports successful and real HDF5 output |
| `gears` | Actual vendored GEARS 0.1.2 with locked CPU Torch/PyG/Scanpy; three forward/backward optimizer steps with finite gradients |
| `gears-data` | Real Dixit/scFoundation H5AD: 37,160 K562 cells, 19,264 genes, 21 conditions; bounded matrix QC, no model pickle loaded |
| `osp` | OSPSuite-R 12.4.5 + rSharp 1.2.3 + .NET 8; original Aciclovir PKML executes in the real SimModel solver, 491 times, original µmol/l and minute metadata |

The OSP profile provides headless execution of existing PK-Sim/MoBi PKML models.
The Windows graphical applications are separate and are not installed in Linux.
GEARS' gradient probe uses a synthetic graph; the real measurement reader does
not train/evaluate the published predictor. Full paper reproduction, independent
validation, cross-cell-type transfer and general cross-engine feedback remain open.
PhysiBoSS's tested cell-line fixture is an engineering example; it is not an
evidence-based corneal signaling bridge. Chaste's original Delta/Notch tutorial
warns that vertex movement was restricted at its original timestep; numerical
convergence under refinement has not been established by that fixture run.

## Build and execute explicitly

On this ARM64 Docker Desktop host (the GEARS/OSP/VCell profiles use AMD64 emulation):

```sh
docker build --build-arg PROFILE=copasi -f runtime_profiles/python.Dockerfile -t peykar-copasi:4.47.309 .
docker build --build-arg PROFILE=neuron -f runtime_profiles/python.Dockerfile -t peykar-neuron:9.0.2 .
docker build -f runtime_profiles/physicell/Dockerfile -t peykar-physicell:1.14.2 .
docker build -f runtime_profiles/physiboss/Dockerfile -t peykar-physiboss:1.14.2-2.5.6 .
docker build -f runtime_profiles/chaste/Dockerfile -t peykar-chaste:2026.1 .
docker build --platform linux/amd64 -f runtime_profiles/gears/Dockerfile -t peykar-gears:0.1.2 .
docker build --platform linux/amd64 -f runtime_profiles/osp/Dockerfile -t peykar-osp:12.4.5 .
docker pull --platform linux/amd64 ghcr.io/virtualcell/biosimulators_vcell:8.2.1.01

uv run python scripts/probe_native_runtimes.py copasi --output .peykar/my-copasi-run
uv run python scripts/probe_native_runtimes.py chaste-delta-notch --output .peykar/my-signaling-run
uv run python scripts/probe_native_runtimes.py osp --output .peykar/my-pkml-run
```

Other profile names use the same explicit probe command. vCornea uses its existing
[dedicated runner](../docs/epics/E1-vcornea-native-runtime.md), retaining model seed,
horizon and source admission controls. Every new run requires a fresh directory.
Failures and timeouts retain their receipt/log and cannot produce a passed record.
The runner resolves a local immutable image ID, never pulls automatically, disables
network access, uses a non-root UID/read-only image, bounds CPU/memory/process/time,
and exposes only the job directory plus explicitly selected read-only data.

The OSP build needs the two exact release distributions in `osp/distributions/`;
each has retained URL, length and SHA-256 provenance. Its three R helper packages
build from pinned vendored source. Native/R package version inventories remain
inside the image. Build-time apt repositories are not a historical snapshot;
the observed final image digest is the retained execution identity.

## Compatibility overlays

[overlays.json](overlays.json) retains versioned motivations/base commits/file hashes:

- COPASI 4.47.309's SWIG binding exposes opaque output flags while `initializeRaw`
  expects an integer mask. `copasi_compat.py` checks the exact version/enum layout
  and restores the masks declared by the original C++ code. BasiCO credits the
  COPASI Team and declares Artistic-2.0; original package notices remain installed.
- PhysiCell/PhysiBoSS builds replace x86-only flags with portable `-O3`, OpenMP
  and C++11 flags. The original scientific source files are unchanged.
- The official Chaste home directory is 0750; its wrapper grants traversal to
  the caller's non-root UID. Executable bytes and original source are unchanged.

These are compatibility repairs. No speedup or biological improvement is inferred.

## Retain and discover execution evidence

```sh
uv run python scripts/package_native_evidence.py --output .peykar/native-admission
peykar libraries import-receipt .peykar/native-admission/source-copasi.json --bundle .peykar/native-admission/artifacts
peykar runtimes import-evidence .peykar/native-admission/runtime-copasi.json --bundle .peykar/native-admission/artifacts
peykar runtimes list --engine copasi
```

Import the referenced source receipts first. Source version 2.0.0 records complete
recursive inventories; earlier source receipts are retained as historical versions.
The native registry verifies all hashes, source references and successful execution
identity before immutable admission. It imports a trusted operator's assertion;
the API does not certify that another host has the image installed or can execute it.

Authenticated `GET /v1/runtimes` and `GET /v1/runtimes/{id}/versions/{version}` expose
these records, with optional `?engine=copasi` filtering. There is no HTTP installation
or native-code execution endpoint and no Docker socket in API/Worker containers.
