#!/usr/bin/env bash
# Stage a single-architecture macOS release directory (solver-repo plan §1.1–1.2); the
# MacOS-Universal job then lipo's the arm64 and x86_64 stages together and signs the result.
#
#   package_macos.sh <build-dir> <stage-dir> <version>
#
# <stage-dir> gets, flat: the executables under VCell's names, every non-system dylib they load
# (transitively), LICENSE and VERSION. Every reference between them is rewritten to @loader_path
# and absolute LC_RPATHs are removed, so nothing points at the build tree or /opt/homebrew.
set -euo pipefail

build="$1"; stage="$2"; version="$3"
repo="$(cd "$(dirname "$0")/../.." && pwd)"
EXES=(SundialsSolverStandalone_x64)

is_system() { case "$1" in /usr/lib/*|/System/*) return 0 ;; *) return 1 ;; esac; }
# the load commands only (indented lines; skips the file/architecture header lines)
deps() { otool -L "$1" | grep -E '^[[:space:]]' | awk '{print $1}'; }

# resolve a load command (absolute, @rpath/, @loader_path/, @executable_path/) to a file
resolve() {
    local ref="$1" from="$2" base
    base="${ref##*/}"
    case "$ref" in
        /*) [ -e "$ref" ] && { echo "$ref"; return; } ;;
    esac
    for d in "$build/lib" "$build/bin" "$(dirname "$from")" $(otool -l "$from" | awk '/LC_RPATH/{getline; getline; print $2}'); do
        [ -e "$d/$base" ] && { echo "$d/$base"; return; }
    done
    echo "error: cannot resolve $ref (from $from)" >&2; return 1
}

rm -rf "$stage"; mkdir -p "$stage"
for exe in "${EXES[@]}"; do cp "$build/bin/$exe" "$stage/"; done

# copy the transitive closure of non-system dylibs (loop until no new file appears)
changed=1
while [ "$changed" = 1 ]; do
    changed=0
    for f in "$stage"/*; do
        for ref in $(deps "$f"); do
            is_system "$ref" && continue
            base="${ref##*/}"
            [ "$base" = "${f##*/}" ] && continue          # a dylib's own id
            [ -e "$stage/$base" ] && continue
            src="$(resolve "$ref" "$f")"
            cp -L "$src" "$stage/$base"; chmod u+w "$stage/$base"; changed=1
        done
    done
done

# rewrite ids and references to @loader_path, drop absolute rpaths
for f in "$stage"/*; do
    chmod u+w "$f"
    case "$f" in *.dylib) install_name_tool -id "@loader_path/${f##*/}" "$f" ;; esac
    for ref in $(deps "$f"); do
        is_system "$ref" && continue
        base="${ref##*/}"
        [ "$ref" = "@loader_path/$base" ] && continue
        install_name_tool -change "$ref" "@loader_path/$base" "$f"
    done
    for rp in $(otool -l "$f" | awk '/LC_RPATH/{getline; getline; print $2}'); do
        install_name_tool -delete_rpath "$rp" "$f"
    done
    codesign --force --sign - "$f"
done
cp "$repo/LICENSE" "$stage/LICENSE"
echo "$version" > "$stage/VERSION"

# verify: only system and @loader_path references remain
bad=0
for f in "$stage"/*_x64 "$stage"/*.dylib; do
    [ -e "$f" ] || continue
    echo "== ${f##*/}"; deps "$f"
    if deps "$f" | grep -Ev '^(/usr/lib/|/System/|@loader_path/)'; then bad=1; fi
done
[ "$bad" = 0 ] || { echo "error: non-portable load paths (above)" >&2; exit 1; }
ls "$stage" | grep -E '\.a$|^unit_tests|gtest|gmock' && { echo "error: test/static files staged" >&2; exit 1; }
echo "staged $stage:"; ls -l "$stage"
