# Intervention framework

Implemented contract: peykar_core.contracts.Intervention (parameter_set only).
Validation: peykar_core.planning.validate_plan and engine-specific runtime allowlists.
At time t, a parameter change affects the interval starting at t.

Designed extensions: genetic perturbation, molecular dose, cell removal/addition,
tissue injury, mechanical loading and environmental changes. Each needs a typed
schema, target identity, units, spatial/time scope, permissions, a mechanistic
mapping supported by the selected model and evidence for its intended context.
Unsupported interventions must fail explicitly, never be silently ignored.
