# E7 — contextual multiparty processes, molecular states and ownership

Bounded S2.5 now has immutable multiparty process networks, exact rational
stoichiometry, molecular state/composition declarations, frozen process ownership
and retained instance sample frames. API, offline CLI and Biological Explorer
operate on the same real records. Earlier pairwise graph contracts are unchanged.

## Retained original biological example

The committed [Reactome bundle](../../biological_libraries/processes/reactome-egfr/bundle.json)
contains **19 exact original JSON query responses / 138,226 bytes**. SHA-256,
original version, source URL, CC0 data license and Reactome attribution accompany
every response. The CatalystActivity response exposes no stable version: its
revision is explicitly `dbId:9624430`, pinned by raw digest. These are response
snapshots, not a claim of a complete database release. No source code, website
media or external model is executed or
installed by import. The raw bytes remain downloadable and in content-addressed
storage. Dataset attribution is separate from Peykar's original project license.

The three selected reactions are **R-HSA-177942.5** (ligand binding),
**R-HSA-177922.5** (dimerization) and **R-HSA-177934.5** (autophosphorylation).
The pinned source's EGF-like ligand set contains seven **alternatives**. It must
not be collapsed to mature EGF alone, the EGF precursor or seven simultaneous
reactants. Fifteen molecular states retain their source identity, compartment,
reference molecule/accession, sequence range, six phosphorylation-site declarations,
complex components and alternative members. Mature EGF retains positions **971–1023**;
EGFR retains **25–1210**; coordinates are never silently renumbered.

Reactome's query serializer emits the first occurrence of an object in full and
later occurrences as its numeric database identity. The importer resolves both
forms against explicit pinned root records and **counts every occurrence**.
Dimerization consumes two ligand-bound receptor complexes. The source
phosphorylation reaction consumes twelve ATP and produces twelve ADP while
converting the unphosphorylated dimer to a separate phosphorylated dimer state.
Catalytic physical entity/activity is separately recorded; it is not an additional
stoichiometric input. Regulation references remain explicitly unexpanded.
Original review labels and literature remain in source bytes, without granting
independent Peykar approval.

This is a **three-reaction data projection**, not the full EGFR pathway, a calibrated
kinetic model, a complete regulatory mechanism or a tissue bridge. Following the
[Reactome data model](https://reactome.org/documentation/data-model), location,
modification, complex membership and reference molecular identity remain distinct.
[Content Service documentation](https://reactome.org/dev/content-service) describes
the explicitly retrieved query records. [Reactome data terms](https://reactome.org/license)
apply to the original data; linked media are not acquired.

## Network and stoichiometry behavior

New contracts leave historical `BiologicalGraphSpec` and edge literals untouched.
A `BiologicalProcessNetwork` freezes exact context, sources, state declarations,
reactions, evidence and limitations, always with `approval=unreviewed_declaration`.
Registration rejects duplicate local/external identities, unknown participants,
source/species mismatch, cyclic component/alternative composition and inappropriate
components on non-complex states or alternatives on non-set states. Complexes and
sets require explicit members. Every source digest/size is checked before storage;
raw source imports may leave unreferenced CAS bytes if immutable registration fails.

Coefficients are **positive reduced rational numbers**, with numerator and denominator
bounded to 1,000,000. Repeated participant rows within one side are rejected; source
multiplicities are aggregated during import. Gross inputs and outputs remain
separate, so a state on both sides does not disappear from the declaration. The
matrix endpoint returns exact rational strings for **output minus input**, stable
state/reaction ordering and the network digest. Catalysts/regulators do not alter
that matrix. No rate law, elemental/charge balance, conserved quantity, species
concentration conversion or executability is inferred from a matrix.

An offline Reactome bundle accepts 1–100 pinned query records, each at most 10 MiB
and at most 64 MiB combined. Duplicate JSON keys, nonfinite values, missing root
participants/activities, mismatched nested/root versions and changed source
checksums/revisions fail. The reader handles the demonstrated Reaction,
EntityWithAccessionedSequence, SimpleEntity, Complex, DefinedSet and CatalystActivity
subset. It is not a general BioPAX/Reactome importer or live synchronization service.

## Ownership and declared instance frames

`ProcessOwnershipSpec` partitions one exact network's reactions into one declared
model owner or a reasoned exclusion **per reaction**. Duplicate ownership, owner vs
exclusion overlap, unknown/missing reactions, missing model versions and differing
model/network contexts fail before registration. Reference/native/CellML model
identities and content digests are frozen with the network digest. Original model
objects remain unchanged. Reads recheck model/network lineage. Multiple models
cannot silently double-own the same process inside a plan; separate plans remain
alternative unreviewed proposals, not competing installed runtime authorities.

An ownership declaration **does not certify that a model implements that reaction,
authorize a solver, change workflow scheduling or replace an existing vCornea EGF
sink**. Those require scientific model mapping/review and a versioned bridge under
S2.6/S5/S11. The retained example explicitly excludes all three processes because
no applicable executable model has been admitted for this exact context. It does
not pretend a synthetic decay model implements EGFR biology.

`MolecularStateFrameSpec` distinguishes a named **sample instance** from the network's
state definitions and executable model versions. It stores an exact network/context,
original time/unit, explicit synthetic/experimental/model-output declaration,
nonnegative amount, volumetric concentration or integral molecule counts, evidence
and limitations. Time and quantity dimensionality are checked without rewriting
original units. Counts use literal `count`, bounded to exactly representable
integers (2^53−1). Missing states remain unknown, not zero. Alternative sets cannot
receive a quantity until resolved to a represented physical state.

Frames are immutable **unreviewed sample declarations**. Origin/evidence text is
not authenticated measurement or verified solver lineage. Separate frame identities
can express alternative observations; they do not update a shared runtime state,
interpolate, reconcile conflicting observations, initialize/resume a solver or prove
that a compartment supports a stated volumetric quantity. Geometry/volume authority,
measurement operators, independent claims and checkpoint restoration remain open.

## Operator and API

```sh
uv run peykar processes admit-reactome biological_libraries/processes/reactome-egfr/bundle.json
uv run peykar processes list
uv run peykar processes show biology.reactome-egfr-processes 1.0.0
uv run peykar processes stoichiometry biology.reactome-egfr-processes 1.0.0
uv run peykar processes ownership OWNERSHIP_SPEC.json
uv run peykar processes frame STATE_FRAME_SPEC.json
uv run python scripts/verify_processes.py --output .peykar/new-process-verification
```

Use a new verification directory; `PEYKAR_DATA_DIR` selects an operator workspace.
Replay uses the committed original files **offline**, creates explicit exclusions
and a separately labeled synthetic sample, checks original hashes/coefficient
values, rejects conflicting registrations and reopens the retained declarations.
No remote fetch, solver or scientific approval occurs. Production import excludes
synthetic verification/browser frames.

Authenticated read routes:

- `/v1/biology/process-networks` and `/{id}/versions/{version}`
- `/{id}/versions/{version}/stoichiometry`
- `/{id}/versions/{version}/sources/{source_id}`
- `/v1/biology/process-ownership` and `/{id}/versions/{version}`
- `/v1/biology/state-frames` and `/{id}/versions/{version}`

The abbreviated source/matrix paths extend `/v1/biology/process-networks`.
Lists default to 20, cap at 50 and support offset. Administrator POST on each base
route registers declarations; readers cannot write. HTTP network registration
requires declared source bytes already in the operator-admitted CAS. HTTP never
fetches a source URL. Records use the existing immutable object registry; no table
or historical SQLite user_version change is required.

## Workbench scope

**Biological Explorer → Processes & states** presents actual equations, clickable
participants, compartment/modification/complex/set inspection, exact matrix export,
original source downloads, structured ownership/exclusion and sample-frame forms,
retained records and full declaration JSON. Original scientific names/IDs/units
stay visible; labels support Persian RTL. Lists paginate, loading/empty/error/
forbidden/conflict states use the established shared components. A read-only session
disables writes; changed content under an existing identity reports immutable
conflict rather than optimistic success. Ownership pickers disclose the first
50 models per format and exact context matching; other refs remain available in CLI.

Engineering verification is recorded only after observed checks in
[verification](../verification.md). Scientific claim authority, biological review,
rate laws, elemental balance, feedback coupling and runtime checkpoints remain open.
