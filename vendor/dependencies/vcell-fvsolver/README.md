<p align="center" width="100%">
 <a href="https://vcell.org">
    <img width="10%" src="https://github.com/biosimulations/biosimulations/blob/dev/docs/src/assets/images/about/partners/vcell.svg">
 </a>
</p>

---
![CI](https://github.com/virtualcell/vcell-solvers/actions/workflows/cd.yml/badge.svg)

# vcell-fvsolver
Virtual Cell Finite Volume solver [virtualcell/vcell-fvsolver](https://github.com/virtualcell/vcell-fvsolver) 
is a reaction-diffusion-advection PDE solver for computational cell biology. 
This solver is used within the Virtual Cell modeling and simulation application [virtualcell/vcell](https://github.com/virtualcell/vcell) 
and as a component in the Virtual Cell Python API [virtualcell/pyvcell](https://github.com/virtualcell/pyvcell) (coming soon).

## The Virtual Cell Project
The Virtual Cell is a modeling and simulation framework for computational biology.  For details see http://vcell.org and http://github.com/virtualcell.

## Releases, Docker image and SIF
Each release `vX.Y.Z` carries `linux64.tgz`, `linux64arm.tgz`, `mac64.tgz` (universal), `win64.zip` and
`SHA256SUMS`, each with `FiniteVolume_x64` and `smoldyn_x64` at the archive root. The solvers are also
published as the multi-arch image `ghcr.io/virtualcell/vcell-fvsolver:X.Y.Z` and, for VCell's cluster, the SIF
`oras://ghcr.io/virtualcell/vcell-fvsolver_singularity:X.Y.Z`:

```bash
docker run --rm ghcr.io/virtualcell/vcell-fvsolver:latest            # version and executables
docker run --rm -v "$PWD:/simdata" ghcr.io/virtualcell/vcell-fvsolver:latest \
    FiniteVolume_x64 /simdata/SimID_1_0_.fvinput
```

See [SOLVER-RELEASE.md](SOLVER-RELEASE.md) for the release contract: asset layout, the entrypoint, messaging
(`-tid`), and the CI smoke test. Builds: `.github/workflows/cd.yml` (macOS, Windows, wheels) and
`.github/workflows/container.yml` (Linux archives, image and SIF, from the top-level `Dockerfile`).

## Python API - pyvcell_fvsolver
The Python API for the VCell Finite Volume solver is a low level wrapper which 
accepts VCell solver input files (.fvinput, .vcg) 
and generates the output files (.log, .zip, .mesh, .meshmetrics, .hdf5).  The 
.functions file is not used by the solver, but is helpful for interpreting the 
results in the context of the original model.

This package is intended to be used by the Virtual Cell Python API [virtualcell/pyvcell](https://github.com/virtualcell/pyvcell) (coming soon).
