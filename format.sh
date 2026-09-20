#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
style="file:$repo_root/.clang-format"

usage() {
    sed -n '3,12p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

resolve_clang_format() {
    local candidate
    if [ -n "${CLANG_FORMAT:-}" ]; then
        if ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
            echo "format.sh: CLANG_FORMAT=$CLANG_FORMAT is not executable" >&2
            exit 1
        fi
        printf '%s\n' "$CLANG_FORMAT"
        return
    fi
    for candidate in \
        clang-format \
        clang-format-22 \
        clang-format-21 \
        /opt/homebrew/opt/llvm/bin/clang-format \
        /opt/homebrew/opt/llvm@22/bin/clang-format \
        /opt/homebrew/opt/llvm@21/bin/clang-format \
        /usr/local/opt/llvm/bin/clang-format; do
        if command -v "$candidate" >/dev/null 2>&1; then
            printf '%s\n' "$candidate"
            return
        fi
    done
    echo "format.sh: clang-format not found on PATH." >&2
    echo "  macOS:  brew install llvm" >&2
    echo "  Debian: apt install clang-format" >&2
    echo "  Or set CLANG_FORMAT=/path/to/clang-format" >&2
    exit 1
}

check=0
files=()
for arg in "$@"; do
    case "$arg" in
        --check) check=1 ;;
        -h | --help)
            usage
            exit 0
            ;;
        -*)
            echo "format.sh: unknown option '$arg'" >&2
            usage >&2
            exit 1
            ;;
        *) files+=("$arg") ;;
    esac
done

# No explicit paths: every C++ source git knows about, tracked or not.
# .gitignore keeps build/ and other generated trees out.
if [ ${#files[@]} -eq 0 ]; then
    while IFS= read -r -d '' path; do
        files+=("$repo_root/$path")
    done < <(git -C "$repo_root" ls-files -z --cached --others --exclude-standard \
        -- '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx')
fi

if [ ${#files[@]} -eq 0 ]; then
    echo "format.sh: no C++ sources found"
    exit 0
fi

clang_format="$(resolve_clang_format)"
echo "format.sh: $("$clang_format" --version)"

if [ "$check" -eq 1 ]; then
    if "$clang_format" --style="$style" --dry-run --Werror "${files[@]}"; then
        echo "format.sh: ${#files[@]} file(s) already formatted"
    else
        echo "format.sh: formatting needed; run ./format.sh" >&2
        exit 1
    fi
else
    "$clang_format" --style="$style" -i "${files[@]}"
    echo "format.sh: formatted ${#files[@]} file(s)"
fi
