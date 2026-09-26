# 2026_Research_MetaforfnoTestiranje

Metamorphic testing of a search engine: indexes a PDF page by page and checks that search results satisfy a set of metamorphic relations. See [SystemDescription.md](SystemDescription.md) for details.

## Build requirements

| Tool | Version |
|---|---|
| CMake | 4.2 or newer |
| Conan | 2.0 or newer |
| Ninja | any recent |
| Clang | C++20 support |
| Docker| any recent |
| clang-format | 16+ |

## Build

Dependencies are managed by conan. Install dependencies:

```sh
conan install . -pr:a profiles/linux-clang-release --build=missing
```

Use the profile that matches your platform and build type. Profiles are in `profiles/`: `linux-clang-debug`, `linux-clang-release`, `macos-arm-debug`, `macos-arm-release`. With a debug profile, use the `conan-debug` preset below, and the binary ends up in `build/Debug/`.

Build:

```sh
cmake --preset conan-release
cmake --build --preset conan-release
```

## Run

Start search engine:

```sh
docker compose up -d
```

Index a PDF and test relations:

```sh
./build/Release/runner/test_run <file.pdf> [--seed <n>]
```

Any PDF works as input. Each page is indexed as a separate document, so PDFs with more pages give more meaningful results. The vocabulary of the indexed PDF is written to `tokens.txt` in the current directory, and queries are built from it. If `--seed` is omitted, a random seed is used. The seed is always printed, so passing it back with `--seed` repeats the run exactly.

Example:

```sh
./build/Release/runner/test_run my_pdf.pdf --seed 42
```

```text
[2026-09-26 20:16:42.635] [test_run] [info] seed: 42
[2026-09-26 20:16:54.868] [test_run] [info] indexed N pages
[2026-09-26 20:16:55.568] [test_run] [info] wrote M tokens to tokens.txt
[2026-09-26 20:16:55.571] [test_run] [info] Checking if capitalization_irrelevance holds for original: "x" and modified: "X" (operator or)
[2026-09-26 20:16:55.584] [test_run] [info] result: true
[2026-09-26 20:16:55.589] [test_run] [info] Checking if term_addition_monotonicity holds for original: "x" and modified: "x y" (operator or)
[2026-09-26 20:16:55.594] [test_run] [info] result: true
[2026-09-26 20:16:55.609] [test_run] [info] Checking if input_permutation holds for original: "x y" and modified: "y x" (operator and)
[2026-09-26 20:16:55.613] [test_run] [info] result: true
[2026-09-26 20:16:55.617] [test_run] [info] Checking if invalid_term_relevance holds for original: "x" and modified: "x invalid" (operator and)
[2026-09-26 20:16:55.622] [test_run] [info] result: true
...
SUCCESS: all 8 relations hold (seed 42)
```

Log lines go to stderr. The final summary goes to stdout. When one or more relations fail, the summary names them:

```text
FAILURE: 2 of 8 relations failed (seed 42): input_permutation, multiple_term_reduction
```

The exit code is `0` if all relations hold, and `1` if any relation fails or an error occurs.

## VS Code setup

Install the CMake Tools and C/C++ extensions. Then:

1. Run `conan install` as shown above. This creates the presets.
2. Open the project folder.
3. Pick the `conan-release` (or `conan-debug`) preset when VS Code asks.
4. Press F7 to build.

## Formatting

Style lives in `.clang-format` at the repo root. Format every C++ source in place:

```sh
./format.sh
```

Report violations without touching anything (exits non-zero if any are found):

```sh
./format.sh --check
```

Format specific files:

```sh
./format.sh library/src/pdf/pdf_splitter.cpp
```

The script picks the first `clang-format` it finds on `PATH`. Override it with `CLANG_FORMAT=/path/to/clang-format`.

## Authors

- Đorđe Marić 1020/2025
- Lazar Cvijić 1030/2025
