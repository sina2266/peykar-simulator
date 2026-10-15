# Dataset connectors

The implemented local normalized path is peykar.data_fabric.service.DatasetService.
External connectors are extension points, not network fetchers included in this release.

Connector admission: explicit source identity and consent/access basis; license;
immutable dataset revision/digest; schema and unit normalization; quality report;
provenance; donor/experiment-level split groups; observation mapping; and resource
limits. Never load pickle or execute downloaded preprocessing without review.
