# Third-party sources and attribution

Peykar's original `LICENSE`, `NOTICE` and `LICENSING.md` are unchanged. The projects
below retain their own copyright, authorship, license and bundled component terms.
Their original notices remain alongside their complete source files. Source changes
and compatibility work by Peykar are in separate runtime profiles/scripts.

| Original project / attribution | Permanent source and original terms |
|---|---|
| CompuCell3D; Biocomplexity Institute and contributors | [source](vendor/upstream/compucell3d), [MIT core notice](vendor/upstream/compucell3d/CompuCell3D/License.txt) |
| PhysiCell and BioFVM; Paul Macklin and the PhysiCell/BioFVM projects | [source](vendor/upstream/physicell), [original component notices](vendor/upstream/physicell/licenses) |
| Virtual Cell; Richard D. Berlin Center for Cell Analysis and Modeling, University of Connecticut Health Center | [source](vendor/upstream/vcell), [original-code MIT scope](vendor/upstream/vcell/license.txt); native solvers retain their separate notices |
| COPASI; Pedro Mendes, Rector and Visitors of the University of Virginia, University of Heidelberg, University of Connecticut School of Medicine, earlier contributors named in source | [source](vendor/upstream/copasi), [Artistic License 2.0](vendor/upstream/copasi/license.txt) |
| Chaste; University of Oxford and contributors | [source](vendor/upstream/chaste), [BSD-3-Clause](vendor/upstream/chaste/LICENSE) |
| NEURON; Michael Hines and contributors named in individual files | [source](vendor/upstream/neuron), [original BSD copyright/scope](vendor/upstream/neuron/Copyright) |
| PK-Sim and MoBi; Open Systems Pharmacology community and named contributors | [PK-Sim](vendor/upstream/pk-sim), [MoBi](vendor/upstream/mobi); each includes `Open Systems Pharmacology Suite License.pdf` (GPLv2 and Clarifying Addendum) |
| GEARS; Yusuf Roohani, Kexin Huang, Jure Leskovec | [source](vendor/upstream/gears), [MIT](vendor/upstream/gears/LICENSE) |
| V-Cornea; Joel Vanin, Michael Getz, Thomas Knudsen, Catherine Mahony, James Alexander Glazier | [exact Zenodo source](vendor/models/vcornea-16884414), [record 16884414](https://doi.org/10.5281/zenodo.16884414), CC-BY-4.0 declaration; [separate Git snapshot](vendor/models/vcornea-git-f5c03afb) has a distinct identity |
| MaBoSS; Institut Curie, Eric Viara, Gautier Stoll and contributors; PhysiBoSS authors credited in PhysiCell addon/source | [MaBoSS source](vendor/dependencies/maboss), [BSD notice](vendor/dependencies/maboss/LICENSE), [PhysiBoSS addon](vendor/upstream/physicell/addons/PhysiBoSS) |
| OSPSuite-R; Open Systems Pharmacology community, Michael Sevestre, Pavel Balazki, Juri Solodenko, Indrajeet Patil, Felix MIL | [source](vendor/dependencies/ospsuite-r), [GPLv2](vendor/dependencies/ospsuite-r/LICENSE) |
| rSharp; OSP community, Pavel Balazki, Juri Solodenko, Michael Sevestre, Robert McIntosh, Felix Mil, Jean-Michel Perraud, Ian Peter Du; based on rClr | [source and authors](vendor/dependencies/rsharp), [GPLv2](vendor/dependencies/rsharp/LICENSE) |
| OSP R utilities, TLF Library and OSPSuite.Plots; OSP community and authors in their DESCRIPTION files | [utilities](vendor/dependencies/ospsuite-utils), [TLF](vendor/dependencies/tlf), [plots](vendor/dependencies/ospsuite-plots); original GPL notices retained |
| VCell native solver projects and their upstream solver dependencies | [finite volume](vendor/dependencies/vcell-fvsolver), [ODE](vendor/dependencies/vcell-ode), [stochastic](vendor/dependencies/vcell-stochastic), [NFsim](vendor/dependencies/vcell-nfsim), [Hy3S](vendor/dependencies/vcell-hy3s), [Chombo](vendor/dependencies/vcell-chombo), [moving boundary](vendor/dependencies/vcell-mbsolver), [Langevin](vendor/dependencies/langevin) |

Recursive dependencies (including NEURON's CLI11, Random123, backward-cpp, Catch2,
Eigen, fmt, InterViews, nlohmann/json, nanobind, robin-map, pybind11, spdlog and test
data; OSP dimensions, PK parameters, templates, examples and build scripts; VCell
messaging, expression parser, libzippp and pybind11) retain original notices in
their pinned directories. [The manifest](vendor/manifest.json) and its linked
inventories record exact file hashes and materialized dependency commits. No
blanket license is substituted for these component-specific declarations.

## Corneal original analysis, data and optional publication runtimes

Joel Vanin, Michael Getz, Catherine Mahony, Thomas B. Knudsen and James A. Glazier:
V-Cornea, [final article](https://doi.org/10.1371/journal.pcbi.1013410) and
[Zenodo16884414](https://doi.org/10.5281/zenodo.16884414), CC-BY-4.0 declarations.
The [publication experiment](experiments/vcornea/publication/README.md) retains
unchanged original plotting/membrane files and17 final-paper supplements, with
exact original URLs/hashes. Its selected scalar acquisition is attributed to the
same archive; complete ZIP verification is not claimed. The two original model
method excerpts in `tests/fixtures/vcornea-*.py.txt` have the same attribution/terms.

The complete50-file July2024 snapshot at
[`vendor/models/vcornea-history-b6123adf`](vendor/models/vcornea-history-b6123adf)
comes directly from Git objects inside the same licensed Zenodo code ZIP, commit
`b6123adf992e5b48442c0ebedd34ba7cd3162ef2`. Its original bytes and modes are
retained, with original authors/CC-BY-4.0 attribution and an independent inventory.
Peykar's private lifetime and observation overlays are separately identified;
neither downloaded Git hooks/configuration nor batch launchers were executed.
The separate complete50-file January2025 stash at
[`vendor/models/vcornea-history-c7f95d28`](vendor/models/vcornea-history-c7f95d28),
commit `c7f95d28dfa2329abb7353caab724c52f9341dee`, was recovered from those same
licensed archive objects. It retains the same original authors/CC-BY-4.0 scope,
pristine bytes/modes and its own inventory. Derived recipe5 preserves its two
different family mechanisms; execution overlays and control overrides are explicit.
Peykar's private source overlays and observation/analysis wrappers are separately
identified modifications; no upstream endorsement or biological validity is inferred.

Optional CC3D4.6.0/4.7.0 ARM64 runtimes retain607 original package-shipped notice
files under [publication runtime notices](runtime_profiles/vcornea-publication/notices).
Their [inventory](runtime_profiles/vcornea-publication/notice-inventory.json) pins
484 distinct package distributions, SHA-256s, declared terms and notice hashes.
The complete installed environments keep their additional metadata/component files.
These terms do not replace Peykar's original project license or old source notices.

Runtime Python packages are pinned in each profile's `uv.lock`; native package and
R package inventories are retained inside the corresponding images. COPASI's
BasiCO interface and its compatibility bridge are credited in the runtime profile.
Official Chaste/VCell images preserve the upstream distributions' bundled notices.

## Browser workbench runtime

React 19.3.1, React DOM 19.3.1 and Scheduler 0.28.0: Meta Platforms, Inc. and
affiliates/contributors, under the original MIT notices. Exact npm distributions
are pinned in [the dashboard lock](apps/dashboard/package-lock.json); unchanged
notices are retained in [third-party](apps/dashboard/third-party) and copied into
the packaged `/app/licenses/` directory. The build verifies those notices against
the installed distributions. These components retain their own terms; Peykar's
project license and all specialist library notices remain unchanged.

## CellML solver source and optional runtime

libCellML 0.7.1, cellml/libcellml contributors, commit
`bc71bfa5816cd3e1e1a28430366eb5688fe9e501`: complete pristine source in
[vendor/dependencies/libcellml](vendor/dependencies/libcellml), under its original
[Apache-2.0 LICENSE](vendor/dependencies/libcellml/LICENSE). Upstream numerical
fixtures retain their comments and provenance. Peykar modifications live in the
separate `runtime_profiles/cellml` driver, not the upstream source. Optional
libCellML/NumPy/SciPy wheels are pinned by the independent runtime lock; their
original distribution notices remain installed in that separate image.

## Biological vocabulary and identifier data

The [E6 source catalog](biological_libraries/vocabularies/sources.json) pins original
release/entry bytes and names the creators, source URLs and declared terms. Cell
Ontology contributors (2026-06-08 basic) and Gene Ontology Consortium (2026-01-23
basic) declare CC-BY-4.0. The original Uberon 2026-10-01 basic header declares
CC-BY-3.0; this declaration is preserved. HGNC/HUGO Gene Nomenclature Committee
entry responses use CC0-1.0; UniProt Consortium entries use CC-BY-4.0; Reactome
data uses CC0-1.0. Original raw headers, attribution and content remain in the
artifact store, separate from normalized/indexed data and Peykar code. Linked
external images/media retain their own declarations and are not downloaded.
Sources: [GO terms](https://www.geneontology.org/docs/go-citation-policy/),
[CL](https://obofoundry.org/ontology/cl.html),
[Uberon](https://obofoundry.org/ontology/uberon.html),
[HGNC](https://www.genenames.org/about/license/),
[UniProt](https://www.uniprot.org/api-documentation/support-data),
[Reactome](https://reactome.org/license).

### E7 Reactome process query data

`biological_libraries/processes/reactome-egfr/` retains nineteen original Reactome
Knowledgebase query JSON responses (138,226 bytes), individually pinned by URL,
source version and SHA-256 in `bundle.json`. Attribution: Reactome Knowledgebase
contributors, https://reactome.org/. Data: CC0-1.0, https://reactome.org/license.
Source coordinates, molecular modifications, names and literature references are
preserved. The Peykar projection is an unreviewed subset; no approval, executable
kinetics or biological validity is asserted. Linked website figures/media are not
included, and their separate terms are not generalized to the dataset.
