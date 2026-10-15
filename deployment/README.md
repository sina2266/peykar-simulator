# Deployment

Dockerfile and compose.yaml define a local API/worker deployment with a shared volume.
API credentials come from .env/environment; .env is excluded from source control.
Containers and remote CI must be verified on their target hosts before production use.
The Docker runtime uses non-editable workspace wheels, a non-root user and uv.lock.
A real external engine needs its own reviewed, digest-pinned image and hard sandbox.
No public hosting, remote GitHub repository or cloud infrastructure is created by this scaffold.
