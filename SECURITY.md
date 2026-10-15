# Security

This release is a local research/development foundation. See `docs/operations.md`
for its supported trust boundary and known deployment gaps.

Do not post tokens, private datasets or executable exploit payloads in public
issues. Use the repository hosting provider's private vulnerability reporting
facility when enabled. No security response SLA or external reporting address is
established in this initial repository.

The HTTP API does not install executable models. Engine packages and operator
plugins are trusted code. A process/container boundary, reviewed image digest,
read-only model inputs and hard resource/network constraints are required before
admitting external engines or untrusted scientific code.
