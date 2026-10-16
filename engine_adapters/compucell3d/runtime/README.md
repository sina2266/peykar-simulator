# Isolated CompuCell3D 4.7.0 runtime

This directory pins an engine-specific Linux x86-64 environment. It is separate
from Peykar's API/worker environment and contains no vCornea model or dataset.
The [official installation guide](https://compucell3d.org/SrcBin) documents the
`conda-forge` and `compucell3d` channels and the headless entrypoint
`python -m cc3d.run_script`. Version 4.7.0 is the version requested by vCornea.

## Exact identities

- Official core package: `cc3d-4.7.0-py312hb93dfd8_5.tar.bz2`, SHA-256
  `89300bd9a8737e3a5292fcd04f45ad49c3f6a10a3e072bd0e250ac5d8c1d8e31`.
  [Publisher metadata](https://api.anaconda.org/package/compucell3d/cc3d).
- Official Player package: `cc3d-player5-4.7.0-py312_5.tar.bz2`, SHA-256
  `2dbfa3270a73e260efd5333942c68fc2d232e8c9b6be2be2afb0fdf8df814559`.
  [Publisher metadata](https://api.anaconda.org/package/compucell3d/cc3d-player5).
- The core package's `cc3d/__init__.py` declares version `4.7.0`, revision `5`
  and internal commit label `f401d6d`. That label resolves to upstream commit
  [`f401d6d087414b86e21d78b9bcf9410123c05245`](https://github.com/CompuCell3D/CompuCell3D/commit/f401d6d087414b86e21d78b9bcf9410123c05245).
  The annotated `4.7.0` tag separately resolves to
  [`cea3fa8323e25394db290c0899b7fbd68f17d338`](https://github.com/CompuCell3D/CompuCell3D/commit/cea3fa8323e25394db290c0899b7fbd68f17d338).
  These are different identities; a binary package checksum does not prove a
  reproducible source build from either commit.
- The Micromamba 2.3.3 base image is pinned to its Linux AMD64 manifest digest
  `sha256:e583f8e151fd1ebbc78b05cf4feb37ddbb518892dbabf6c0c03ea8ad613bc519`.

`conda-lock.yml` is a Micromamba-compatible version-1 lock written in JSON, which
is also valid YAML. It freezes all 424 package URLs/builds with SHA-256 and MD5.
It was constructed from the successful Micromamba 2.3.3 solver's `LINK` records
for [environment.yml](environment.yml); the `conda-lock` executable was not used.
[Micromamba's lock-file documentation](https://mamba.readthedocs.io/en/stable/user_guide/micromamba.html)
describes this input format. Builds consume the lock without resolving the
environment again. [package-inventory.json](package-inventory.json) additionally
retains sizes and package-declared licenses.

The final resolution includes Python 3.12.2, NumPy 2.2.6, VTK 9.2.6, pandas
3.0.6 and PyArrow 16.1.0. PyArrow supports the native model's Parquet output.
This is a recorded reconstruction; it is not evidence of the model authors'
complete original dependency environment or of published-result reproduction.
The first native import exposed a missing `libGL.so.1`; explicitly locked `libgl`
and `libglx` plus the solver-selected X11/XCB dependencies repaired it. Native CC3D
imports and a real Parquet round trip then passed. The exact image and lock hashes
are in [the local receipt](runtime-receipt-2026-10-16.json).

## Build and inspect

From the repository root:

```bash
docker build --platform linux/amd64 \
  -f engine_adapters/compucell3d/runtime/Dockerfile \
  -t peykar-cc3d:4.7.0-linux64 engine_adapters/compucell3d/runtime
docker image inspect peykar-cc3d:4.7.0-linux64 --format '{{.Id}} {{.Architecture}}'
docker run --rm --platform linux/amd64 --network none \
  --cpus 2 --memory 4g --pids-limit 256 --cap-drop ALL \
  --security-opt no-new-privileges --read-only \
  --tmpfs /tmp:rw,nosuid,nodev,size=256m \
  peykar-cc3d:4.7.0-linux64 python -m cc3d.run_script --help
```

The upstream 4.7.0 channel supplies Linux `linux-64` packages and macOS ARM64
packages, but no Linux ARM64 build. Docker on this host reports `aarch64`;
the Linux image therefore requires explicit AMD64 emulation. Do not substitute
a native ARM64 environment or a newer CC3D version while retaining this identity.
Use the inspected immutable image ID when recording an execution manifest.

The default image user is the base image's unprivileged `mambauser`; no Docker
socket, API credential, live data volume or user home directory is required.
The image uses offscreen Qt and one thread per numerical/OpenMP library.
Network access is needed to build, not to execute an admitted local model.

## Model execution boundary

Use an explicitly reviewed, licensed and checksum-pinned model with the host
execution profile. The profile must bound wall time, CPU, memory, processes and
output size and disable network access. `scripts/run_vcornea.py` verifies original
source on the host and mounts only a private execution copy as the writable job
directory; original source is not exposed to the container. Native vCornea
writers can write beside their Simulation script; `--output-dir` alone does not
redirect every model-defined file. Preserve original bytes and record any seed,
duration or output-control changes made to the execution copy.

No model is installed or executed by this Dockerfile. This image alone is not a
reviewed hostile-code sandbox and does not establish biological or clinical
validity. See [vCornea integration](../../../docs/vcornea-integration.md) for the
scientific and source-permission gates.

## Upstream licenses and notices

The downloaded core archive contains an MIT license, preserved verbatim as
[LICENSE.cc3d.txt](LICENSE.cc3d.txt). Its publisher metadata also says MIT.
The Player package declares LGPL. The full `compucell3d` suite metapackage declares
GPL; it and the Twedit editor are not installed by this environment. Dependencies
retain their own licenses, including GPL components in the resolved environment;
the complete runtime must not be described as uniformly MIT or covered solely by
Peykar's license.

Before cleaning the installation cache, `preserve_notices.py` copies each package's
provided `info/licenses` directory to `/opt/conda/cc3d-runtime/notices/` and records which
packages supplied notices. This preserves available notices; package metadata
alone is not a legal determination of all distribution obligations. This runtime
does not transfer or expand the rights of any separately supplied biological model
or dataset.

The built image retained available notice directories for 405 of the 424 locked
packages, alongside the complete package/license inventory. Missing package-provided
notice directories are not silently replaced with Peykar's license.
