# ADR 0003: Verification, evidence and promotion are separate

Status: accepted. Date: 2026-10-15.

Synthetic tests can prove software and numerical properties; they cannot establish
biological efficacy. Validity belongs to a context and endpoint with evidence and
limitations. Dataset origin and independence are not inferred by an LLM. No report
automatically promotes a model. Registration via HTTP cannot self-certify validity.

Consequence: there is no global trust score or self-approval API. Future promotion
needs an independent evaluator, frozen holdouts and auditable change control.
