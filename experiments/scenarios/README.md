# Scenario branching

Use the Scenario contract with id/version, immutable SimulationPlan, rationale and
optional parent Reference. POST /v1/scenarios registers a baseline or branch.
Branch by registering a new scenario whose parent points to the old exact version.
The service checks that the parent exists and never mutates its plan.
