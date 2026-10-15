# ADR 0005: Bounded trusted execution and immutable evaluation evidence

Status: accepted for 0.3.0.

Cooperative engine checkpoints cannot stop a blocked engine. The Linux default
therefore executes each trusted engine in a fresh child with parent-controlled
wall/cancellation checks and process-group cleanup. Standard resource limits bound
CPU, address space and files. This does not provide hostile-code isolation. The
API cannot choose commands, plugin names or the cooperative fallback.

CSV ingestion preserves raw and normalized artifacts separately. Related dataset
and lineage records commit atomically. Frozen evaluation sets capture immutable
declared data splits and tolerances; stored reports bind metrics to exact run,
result and dataset hashes. Independence still requires external review.

No additional service, remote code installer, implicit resampling, independent
scientific endorsement or new biological algorithm is introduced by this decision.
