# ADR 0001: Modular monolith in one independent workspace

Status: accepted. Date: 2026-10-15.

Scientific boundaries are evolving. A repository per conceptual submodule would
multiply version and integration work before a coupled scientific model is proven.
Use three independently buildable packages with one lockfile. Core has no platform
or engine dependency. External heavy engines run in separate processes/containers.
HomBon depends on an adapter contract, not on Peykar's database or engine internals.

Consequence: local development is simple; changes to contracts can be tested across
all packages together. Later extraction requires explicit API/version guarantees,
but no initial microservice/network protocol is imposed.
