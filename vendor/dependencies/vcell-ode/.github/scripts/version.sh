#!/usr/bin/env bash
# Print VCELL_ODE_VERSION=<version> for $GITHUB_ENV.
#   release event (tag vX.Y.Z or X.Y.Z): X.Y.Z, which must equal pyproject.toml's version (the wheels
#                                        published to PyPI carry that one)
#   anything else:                       <pyproject version>-dev+<short sha>
set -euo pipefail
here="$(cd "$(dirname "$0")/../.." && pwd)"
pyver="$(sed -n 's/^version *= *"\(.*\)"/\1/p' "$here/pyproject.toml" | head -1)"
if [ "${GITHUB_EVENT_NAME:-}" = "release" ]; then
    tag="${GITHUB_REF_NAME#v}"
    if [ "$tag" != "$pyver" ]; then
        echo "::error::release tag ${GITHUB_REF_NAME} does not match pyproject.toml version ${pyver}" >&2
        exit 1
    fi
    echo "VCELL_ODE_VERSION=${tag}"
else
    echo "VCELL_ODE_VERSION=${pyver}-dev+${GITHUB_SHA:0:7}"
fi
