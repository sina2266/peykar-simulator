# Deployment

Dockerfile and compose.yaml define a local API/worker deployment with a shared volume.
API credentials come from .env/environment; .env is excluded from source control.
Containers and remote CI must be verified on their target hosts before production use.
The Docker runtime uses non-editable workspace wheels, a non-root user and uv.lock.
A real external engine needs its own reviewed, digest-pinned image and hard sandbox.
The local browser workbench is served by the API at `/app/`. Its assets and
original frontend dependency notices are built in a separate pinned Node stage
and included in the platform wheel; runtime containers require no Node server.
No public hosting or cloud infrastructure is provisioned by this deployment.
