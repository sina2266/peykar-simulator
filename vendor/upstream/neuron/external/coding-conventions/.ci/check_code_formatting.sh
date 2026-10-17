#!/usr/bin/env bash

set -euo pipefail

VENV=build/venv-code-formatting

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
    "$VENV/bin/pip" install               \
      jinja2                              \
      pyyaml 
fi

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

cd cpp/formatting
make distclean all
if ! git diff --exit-code README.md ;then
    cat >&2 <<EOF

Error: README.md has changed! 💥 💔 💥
Please rebuild it and commit its changes with commands:

  .ci/check_code_formatting.sh
  git add cpp/formatting/README.md
  git commit
EOF
    exit 1
fi
