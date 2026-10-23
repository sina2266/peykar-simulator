# Original source history observations

The [retained audit](source-history-observations-2026-10-23.json) reads all56
commit objects from the exact licensed code archive. It does not execute model
code or identify an original run from a Git timestamp. Original source blobs,
licenses, current execution plans and frozen comparison criteria remain unchanged.

The recovered July14,2024 commit `b6123adf` is followed by January23,2025 source
revisions and August15,2025 publication packaging. The archive also retains the
January23 stash `c7f95d28dfa2329abb7353caab724c52f9341dee`, based on that July
commit. Its full tree has50 files. For **formation**, AST comparison finds no
changed methods outside `PlotSteppable` between the July commit and that stash.
The stash records ten thickness bins, temperature8 and superficial EGF diffusion12;
the July source records five bins, temperature10 and diffusion10. Both retain
the same formation PIFF and basal40/EGF-half-max8 controls. Launch paths and
other source parts differ; this is a separate original snapshot, not a suggested
parameter fit. The ten-bin output resembles the measurement layout of the scalar
archive, but that resemblance does not prove per-run source identity.

The stored historical notebook reports `cc3d.__version__` as4.5.0, Python metadata
3.10.13 and kernel name `cc3d_4_5`. Its simulation cell ended with `PermissionError`
while opening a Windows local project path. In the same source tree, setup scripts
request CompuCell3D4.6.0. The notebook therefore documents a separate local probe;
it does not establish that the published HPC cohorts used4.5.0. The paper-stated
4.6.0 runtime and every current candidate manifest remain fixed.

A read-only path search over all56 snapshots found no paths matching the selected
cohort names,43201/50401 output names or July15/16 directory fragments. This is
a bounded path search, not proof that every blob lacks relevant content. It did
not recover the unknown original seeds, complete environment or per-run controls.

These observations guide further source correspondence work if current full
comparisons fail. They establish no passing candidate, independent biological
validation or automatic authorization to replace a running source version.
