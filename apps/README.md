# Application entrypoints

The actual installed entrypoints are `peykar serve`, `peykar worker`, and `peykar`.
Transport code lives in the platform distribution, not inside scientific models.
The local six-workspace React/TypeScript dashboard lives in `dashboard/` and is
compiled into the platform package, served by FastAPI at `/app/`. It uses existing
authenticated scientific services; see dashboard/README.md for its exact scope.
