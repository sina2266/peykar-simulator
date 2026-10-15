# Dashboard design

Status: designed, not implemented. The target frontend is Next.js/TypeScript.

Six API-driven workspaces:

1. Biological Explorer: anatomy, interactions, ontology IDs, evidence and model gaps.
2. Model Studio: exact-version packages, capability search, ports and compatibility.
3. Dataset Studio: origins, quality, roles, context, licenses and observation mappings.
4. Experiment Studio: baseline, branching, interventions, resource estimate and submit.
5. Simulation Viewer: structural/dynamic views of genuinely computed quantities.
6. Validation Center: endpoint-specific evidence, errors, uncertainty and limitations.

Use a graph library such as React Flow for editable composition and specialized
scientific plots for results. A graph edge must not imply numerical or biological
validity. Do not put credentials in frontend source. Do not turn population-only
outputs into supposedly measured cell trajectories. Build against the generated
OpenAPI schema after P1 contracts are verified on a real engine.
