# Real K562 perturbation measurements

[manifest.json](manifest.json) freezes scFoundation's processed Dixit dataset at
Figshare article **24049200 version 3**, file **44498681**, with original study
GSE90063, provider Minsheng Hao/scFoundation, declared GPL 3.0+ terms, byte length
and provider MD5. Preserve this preprocessing/provenance separately from the
original GEO data and from GEARS's different paper split/cache.

The explicit local acquisition is retained under `.peykar/datasets/scfoundation-dixit/`:
253,158,197 ZIP bytes, provider MD5 verified, local SHA-256
`36a4b94110b6194e816d8a59e7cb5a9afed08b9e2e5bf8abf9d063c3d01c4a0f`.
Its single H5AD member is 1,003,772,584 bytes, ZIP CRC verified, SHA-256
`9828a9078ef027eb132af3f8fd5d228762c65e72bcbdf5444c0cb302936942c2`.
No archive model/pickle is executed or deserialized.

```sh
uv run python scripts/probe_native_runtimes.py gears-data \
  --input .peykar/datasets/scfoundation-dixit/gse90063_k562_ko_tf20_37160_19264.h5ad \
  --output .peykar/dixit-qc-new
```

The isolated backed AnnData reader reports 37,160 cells, 19,264 genes, K562 cell
identity and 21 conditions (including 3,047 controls). Only a bounded 256×512
matrix sample is checked for finite values. Matrix scale is not inferred as raw
counts. There is no original GEARS benchmark score, donor split, training,
cross-cell-type transfer or tissue validation claim. Harvard Dataverse's three
GEARS download metadata endpoints returned HTTP 403 here; this separate primary
provider release is not silently substituted for GEARS's original preprocessed data.

Primary records: [scFoundation release](https://doi.org/10.6084/m9.figshare.24049200.v3),
[original GEO study](https://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?acc=GSE90063).
