# Scientific Instrument integration

Working facade: peykar.integrations.scientific_instrument.ScientificInstrument.
Lifecycle: describe, validate, estimate_cost, submit, status, collect, cancel.
No dependency on HomBon is installed. A remote adapter can map this lifecycle to
/v1 HTTP endpoints with explicit credentials and an idempotency key.
Cost estimation currently reports sample count and a deadline; monetary cost is unknown.
