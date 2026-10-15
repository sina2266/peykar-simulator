# Engine adapter integrations

Execution protocol: packages/core/src/peykar_core/protocols.py.
Working reference adapters: packages/engines/src/peykar_engines/reference.py.

The folders below are designed integrations, not installed engines. Every admission
needs a verified license, pinned source/container digest, capabilities, units,
state/time mapping, native seed/checkpoint behavior, resource limits and a frozen
reference benchmark. Do not turn optional imports into fake successful results.
Heavy scientific dependencies remain in engine-specific environments.
