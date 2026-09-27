#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${BUILD_DIR:-$repo_root/build/Debug}"

usage() {
    cat <<'USAGE'
Usage: ./tidy.sh [--fix] [files...]
  (no args)   check every .cpp git knows about; exits non-zero if any warning is found
  --fix       apply the fixes clang-tidy offers, in place
  files...    check only these files

Needs build/Debug/compile_commands.json (run `cmake --preset conan-debug` first).
Override the build directory with BUILD_DIR=... and the binary with CLANG_TIDY=/path/to/clang-tidy.
USAGE
}

resolve_clang_tidy() {
    local candidate
    if [ -n "${CLANG_TIDY:-}" ]; then
        if ! command -v "$CLANG_TIDY" >/dev/null 2>&1; then
            echo "tidy.sh: CLANG_TIDY=$CLANG_TIDY is not executable" >&2
            exit 1
        fi
        printf '%s\n' "$CLANG_TIDY"
        return
    fi
    for candidate in \
        clang-tidy \
        clang-tidy-22 \
        clang-tidy-21 \
        /opt/homebrew/opt/llvm/bin/clang-tidy \
        /opt/homebrew/opt/llvm@22/bin/clang-tidy \
        /opt/homebrew/opt/llvm@21/bin/clang-tidy \
        /usr/local/opt/llvm/bin/clang-tidy; do
        if command -v "$candidate" >/dev/null 2>&1; then
            printf '%s\n' "$candidate"
            return
        fi
    done
    echo "tidy.sh: clang-tidy not found on PATH." >&2
    echo "  macOS:  brew install llvm" >&2
    echo "  Debian: apt install clang-tidy" >&2
    echo "  Or set CLANG_TIDY=/path/to/clang-tidy" >&2
    exit 1
}

fix=0
files=()
for arg in "$@"; do
    case "$arg" in
        --fix) fix=1 ;;
        -h | --help)
            usage
            exit 0
            ;;
        -*)
            echo "tidy.sh: unknown option '$arg'" >&2
            usage >&2
            exit 1
            ;;
        *) files+=("$arg") ;;
    esac
done

if [ ! -f "$build_dir/compile_commands.json" ]; then
    echo "tidy.sh: $build_dir/compile_commands.json not found; run 'cmake --preset conan-debug' first" >&2
    exit 1
fi

if [ ${#files[@]} -eq 0 ]; then
    while IFS= read -r -d '' path; do
        files+=("$repo_root/$path")
    done < <(git -C "$repo_root" ls-files -z --cached --others --exclude-standard -- '*.cpp')
fi

if [ ${#files[@]} -eq 0 ]; then
    echo "tidy.sh: no C++ sources found"
    exit 0
fi

clang_tidy="$(resolve_clang_tidy)"
echo "tidy.sh: $("$clang_tidy" --version | grep -i version)"

if [ "$fix" -eq 1 ]; then
    "$clang_tidy" -p "$build_dir" --quiet --fix "${files[@]}"
    echo "tidy.sh: applied fixes to ${#files[@]} file(s); run ./format.sh afterwards"
elif "$clang_tidy" -p "$build_dir" --quiet --warnings-as-errors='*' "${files[@]}"; then
    echo "tidy.sh: ${#files[@]} file(s) clean"
else
    echo "tidy.sh: warnings found; fix them or run ./tidy.sh --fix" >&2
    exit 1
fi
